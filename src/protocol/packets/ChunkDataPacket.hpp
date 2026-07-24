#pragma once

#include "protocol/Packet.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include "../../world/Chunk.hpp"

namespace mc {
    
    class ChunkDataPacket : public Packet {
        private:
            std::shared_ptr<Chunk> m_chunk;

            void writeSection(ByteBuffer& buffer, int sectionY) const {
                int blockCount = 0;
                std::vector<uint16_t> palette;
                const auto& blocks = m_chunk->getBlocks();
                
                int startY = (sectionY * 16);
                int startIndex = startY * 256;

                // Trova palette
                for (int i = 0; i < 4096; ++i) {
                    uint16_t block = blocks[startIndex + i];
                    if (block != 0) blockCount++;
                    
                    bool found = false;
                    for (uint16_t p : palette) {
                        if (p == block) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        palette.push_back(block);
                    }
                }

                buffer.writeShort(blockCount);

                if (palette.size() == 1) {
                    buffer.writeByte(0); // Single value
                    buffer.writeVarInt(palette[0]);
                    buffer.writeVarInt(0); // 0 longs
                } else if (palette.size() <= 16) {
                    buffer.writeByte(4); // Indirect palette, 4 bits per entry
                    buffer.writeVarInt(static_cast<int32_t>(palette.size()));
                    for (uint16_t p : palette) {
                        buffer.writeVarInt(p);
                    }

                    int longsCount = 256; // 4096 / (64 / 4) = 256
                    buffer.writeVarInt(longsCount);

                    uint64_t currentLong = 0;
                    int shift = 0;
                    for (int i = 0; i < 4096; ++i) {
                        uint16_t block = blocks[startIndex + i];
                        uint64_t paletteIndex = 0;
                        for (size_t p = 0; p < palette.size(); ++p) {
                            if (palette[p] == block) {
                                paletteIndex = p;
                                break;
                            }
                        }

                        currentLong |= (paletteIndex & 0xF) << shift;
                        shift += 4;
                        if (shift >= 64) {
                            buffer.writeLong(static_cast<int64_t>(currentLong));
                            currentLong = 0;
                            shift = 0;
                        }
                    }
                } else {
                    buffer.writeByte(15);
                    buffer.writeVarInt(1024);
                    
                    uint64_t currentLong = 0;
                    int shift = 0;
                    for (int i = 0; i < 4096; ++i) {
                        uint64_t block = blocks[startIndex + i];
                        currentLong |= (block & 0x7FFF) << shift;
                        shift += 15;
                        if (shift >= 60) {
                            buffer.writeLong(static_cast<int64_t>(currentLong));
                            currentLong = 0;
                            shift = 0;
                        }
                    }
                }

                // Biomes (Uniform = Plains)
                buffer.writeByte(0); // bits per entry = 0
                buffer.writeVarInt(1); // palette = 1 (Plains)
                buffer.writeVarInt(0); // data array length
            }

        public:
            ChunkDataPacket(std::shared_ptr<Chunk> chunk) : m_chunk(chunk) {}

            [[nodiscard]] int32_t getId() const override { return 0x25; }
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

            void write(ByteBuffer& buffer) const override {
                buffer.writeInt(m_chunk->getX());
                buffer.writeInt(m_chunk->getZ());

                std::vector<uint8_t> dummyNbt = {0x0A, 0x00};
                buffer.writeBytes(dummyNbt);

                ByteBuffer dataArray;
                for(int i = 0; i < 24; i++) {
                    writeSection(dataArray, i);
                }

                buffer.writeVarInt(static_cast<int32_t>(dataArray.size()));
                buffer.writeBytes(dataArray.vector());

                buffer.writeVarInt(0); // blockEntities length

                // Light map
                buffer.writeVarInt(1); // sky light mask array length
                buffer.writeLong(0x01FFFFFE); // sky light mask (24 chunk sections, bits 1 to 24)
                
                buffer.writeVarInt(1); // block light mask array length
                buffer.writeLong(0); // block light mask
                
                buffer.writeVarInt(1); // empty sky light mask array length
                buffer.writeLong(0x02000001); // empty sky light mask (bits 0 and 25)
                
                buffer.writeVarInt(1); // empty block light mask array length
                buffer.writeLong(0x03FFFFFF); // empty block light mask (all 26 bits)
                
                buffer.writeVarInt(24); // sky light array count
                const auto& skyLightData = m_chunk->getSkyLightData();
                for (int i = 0; i < 24; ++i) {
                    buffer.writeVarInt(2048); // byte array length
                    buffer.writeBytes(std::span<const uint8_t>(skyLightData.data() + (i * 2048), 2048));
                }
                
                buffer.writeVarInt(0); // block light array count
            }

            void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}

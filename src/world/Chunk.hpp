#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

namespace mc {

    // Forward declaration
    enum class Biome;

    class Chunk {
    private:
        int32_t m_chunkX;
        int32_t m_chunkZ;

        // 16x384x16 = 98304 blocchi
        // Utilizziamo un array 1D per massima efficienza di cache
        std::vector<uint16_t> m_blocks;
        std::vector<uint8_t> m_skyLight; // 1 nibble per blocco
        std::atomic<bool> m_isReady{false};

        inline size_t getIndex(int x, int y, int z) const {
            // y va da -64 a 319. Offset a 0 -> y + 64
            // x, z da 0 a 15
            return ((y + 64) * 16 * 16) + (z * 16) + x;
        }

    public:
        Chunk(int32_t x, int32_t z);

        [[nodiscard]] int32_t getX() const { return m_chunkX; }
        [[nodiscard]] int32_t getZ() const { return m_chunkZ; }

        void setBlock(int x, int y, int z, uint16_t blockStateId);
        [[nodiscard]] uint16_t getBlock(int x, int y, int z) const;
        [[nodiscard]] const std::vector<uint16_t>& getBlocks() const { return m_blocks; }
        
        void setSkyLight(int x, int y, int z, uint8_t light);
        [[nodiscard]] uint8_t getSkyLight(int x, int y, int z) const;
        [[nodiscard]] const std::vector<uint8_t>& getSkyLightData() const { return m_skyLight; }

        void generateTerrain();
        void calculateLighting();

        // Decorazioni per bioma
        void decorateUnderwater(int x, int z, int surfaceY, Biome biome);
        void decorateSurface(int x, int z, int surfaceY, Biome biome);

        [[nodiscard]] bool isReady() const { return m_isReady.load(); }
        void setReady(bool ready) { m_isReady.store(ready); }
    };
}

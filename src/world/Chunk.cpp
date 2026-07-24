#include "Chunk.hpp"
#include "../utils/FastNoiseLite.h"
#include <cmath>
#include <algorithm>

namespace mc {

    // =========================================================================
    // Block State IDs per Minecraft 1.20.4 (Protocol 765)
    // Estratti da PrismarineJS/minecraft-data
    // =========================================================================
    namespace BlockId {
        constexpr uint16_t AIR             = 0;
        constexpr uint16_t STONE           = 1;
        constexpr uint16_t GRANITE         = 2;
        constexpr uint16_t DIORITE         = 4;
        constexpr uint16_t ANDESITE        = 6;
        constexpr uint16_t GRASS_BLOCK     = 9;    // snowy=false
        constexpr uint16_t DIRT            = 10;
        constexpr uint16_t COARSE_DIRT     = 11;
        constexpr uint16_t PODZOL          = 13;   // snowy=false
    //  constexpr uint16_t COBBLESTONE     = 14;
        constexpr uint16_t BEDROCK         = 79;
        constexpr uint16_t WATER           = 80;   // level=0
        constexpr uint16_t SAND            = 112;
    //  constexpr uint16_t RED_SAND        = 117;
        constexpr uint16_t GRAVEL          = 118;
        constexpr uint16_t GOLD_ORE        = 123;
        constexpr uint16_t IRON_ORE        = 125;
        constexpr uint16_t COAL_ORE        = 127;
        constexpr uint16_t OAK_LOG         = 131;  // axis=y
        constexpr uint16_t SPRUCE_LOG      = 134;  // axis=y
        constexpr uint16_t BIRCH_LOG       = 137;  // axis=y
        constexpr uint16_t JUNGLE_LOG      = 140;  // axis=y
        constexpr uint16_t ACACIA_LOG      = 143;  // axis=y
        constexpr uint16_t DARK_OAK_LOG    = 149;  // axis=y
        constexpr uint16_t OAK_LEAVES      = 264;  // default
        constexpr uint16_t SPRUCE_LEAVES   = 292;  // default
        constexpr uint16_t BIRCH_LEAVES    = 320;  // default
        constexpr uint16_t JUNGLE_LEAVES   = 348;  // default
        constexpr uint16_t ACACIA_LEAVES   = 376;  // default
        constexpr uint16_t DARK_OAK_LEAVES = 432;  // default
        constexpr uint16_t LAPIS_ORE       = 520;
        constexpr uint16_t SANDSTONE       = 535;
        constexpr uint16_t SHORT_GRASS     = 2005;
        constexpr uint16_t FERN            = 2006;
        constexpr uint16_t DEAD_BUSH       = 2007;
        constexpr uint16_t SEAGRASS        = 2008;
     // constexpr uint16_t TALL_SEAGRASS_U = 2009; // upper
     // constexpr uint16_t TALL_SEAGRASS_L = 2010; // lower
        constexpr uint16_t DANDELION       = 2075;
        constexpr uint16_t POPPY           = 2077;
        constexpr uint16_t BLUE_ORCHID     = 2078;
        constexpr uint16_t ALLIUM          = 2079;
        constexpr uint16_t AZURE_BLUET     = 2080;
        constexpr uint16_t RED_TULIP       = 2081;
        constexpr uint16_t ORANGE_TULIP    = 2082;
        constexpr uint16_t WHITE_TULIP     = 2083;
        constexpr uint16_t PINK_TULIP      = 2084;
        constexpr uint16_t OXEYE_DAISY     = 2085;
        constexpr uint16_t CORNFLOWER      = 2086;
        constexpr uint16_t LILY_OF_VALLEY  = 2088;
        constexpr uint16_t BROWN_MUSHROOM  = 2089;
        constexpr uint16_t RED_MUSHROOM    = 2090;
        constexpr uint16_t DIAMOND_ORE     = 4274;
        constexpr uint16_t SNOW_LAYER      = 5772; // layers=1
    // constexpr uint16_t ICE             = 5780;
        constexpr uint16_t SNOW_BLOCK      = 5781;
        constexpr uint16_t CACTUS          = 5782; // age=0
    //  constexpr uint16_t CLAY            = 5798;
        constexpr uint16_t SUGAR_CANE      = 5799; // age=0
        constexpr uint16_t MYCELIUM        = 7270; // snowy=false
        constexpr uint16_t LILY_PAD        = 7271;
        constexpr uint16_t EMERALD_ORE     = 7511;
    //  constexpr uint16_t PACKED_ICE      = 10746;
        constexpr uint16_t SUNFLOWER_L     = 10748; // lower
    //  constexpr uint16_t LILAC_L         = 10750; // lower
    //  constexpr uint16_t ROSE_BUSH_L     = 10752; // lower
    //  constexpr uint16_t PEONY_L         = 10754; // lower
    //  constexpr uint16_t TALL_GRASS_L    = 10756; // lower
    //  constexpr uint16_t LARGE_FERN_L    = 10758; // lower
        constexpr uint16_t KELP_PLANT      = 12786;
        constexpr uint16_t TUBE_CORAL_BLK  = 12808;
        constexpr uint16_t BRAIN_CORAL_BLK = 12809;
        constexpr uint16_t BUBBLE_CORAL_BLK= 12810;
        constexpr uint16_t FIRE_CORAL_BLK  = 12811;
        constexpr uint16_t HORN_CORAL_BLK  = 12812;
        constexpr uint16_t TUBE_CORAL_FAN  = 12843; // waterlogged=true
        constexpr uint16_t BRAIN_CORAL_FAN = 12845;
        constexpr uint16_t BUBBLE_CORAL_FAN= 12847;
        constexpr uint16_t FIRE_CORAL_FAN  = 12849;
        constexpr uint16_t HORN_CORAL_FAN  = 12851;
        constexpr uint16_t SEA_PICKLE      = 12933; // 1 pickle, waterlogged=true
    //  constexpr uint16_t BLUE_ICE        = 12941;
        constexpr uint16_t TUFF            = 21081;
        constexpr uint16_t CALCITE         = 22316;
        constexpr uint16_t COPPER_ORE      = 22942;
    //  constexpr uint16_t DRIPSTONE_BLK   = 24768;
    //  constexpr uint16_t MOSS_CARPET     = 24826;
    //  constexpr uint16_t MOSS_BLOCK      = 24843;
        constexpr uint16_t MUD             = 24903;
        constexpr uint16_t DEEPSLATE       = 24905; // axis=y default
    }

    // =========================================================================
    // Enum biomi
    // =========================================================================
    enum class Biome {
        Ocean,
        DeepOcean,
        WarmOcean,
        Beach,
        Plains,
        SunflowerPlains,
        Forest,
        BirchForest,
        DarkForest,
        FlowerForest,
        Taiga,
        SnowyTaiga,
        SnowyPlains,
        Desert,
        Savanna,
        Jungle,
        Swamp,
        Mountains,
        MushroomFields
    };

    // =========================================================================
    // Hash pseudo-casuale deterministico
    // =========================================================================
    static uint32_t hash2d(int x, int z) {
        uint32_t h = static_cast<uint32_t>(x * 374761393 + z * 668265263);
        h = (h ^ (h >> 13)) * 1274126177;
        return h ^ (h >> 16);
    }

    // =========================================================================
    // Costruttore / setBlock / getBlock / SkyLight (invariati)
    // =========================================================================

    Chunk::Chunk(int32_t x, int32_t z) : m_chunkX(x), m_chunkZ(z) {
        m_blocks.resize(16 * 384 * 16, 0);
        m_skyLight.resize((16 * 384 * 16) / 2, 0);
    }

    void Chunk::setBlock(int x, int y, int z, uint16_t blockStateId) {
        if (x < 0 || x > 15 || y < -64 || y > 319 || z < 0 || z > 15) return;
        m_blocks[getIndex(x, y, z)] = blockStateId;
    }

    uint16_t Chunk::getBlock(int x, int y, int z) const {
        if (x < 0 || x > 15 || y < -64 || y > 319 || z < 0 || z > 15) return 0;
        return m_blocks[getIndex(x, y, z)];
    }

    void Chunk::setSkyLight(int x, int y, int z, uint8_t light) {
        if (x < 0 || x > 15 || y < -64 || y > 319 || z < 0 || z > 15) return;
        size_t index = getIndex(x, y, z);
        size_t byteIndex = index / 2;
        bool highNibble = (index % 2) != 0;
        if (highNibble) {
            m_skyLight[byteIndex] = (m_skyLight[byteIndex] & 0x0F) | ((light & 0x0F) << 4);
        } else {
            m_skyLight[byteIndex] = (m_skyLight[byteIndex] & 0xF0) | (light & 0x0F);
        }
    }

    uint8_t Chunk::getSkyLight(int x, int y, int z) const {
        if (x < 0 || x > 15 || y < -64 || y > 319 || z < 0 || z > 15) return 0;
        size_t index = getIndex(x, y, z);
        size_t byteIndex = index / 2;
        bool highNibble = (index % 2) != 0;
        if (highNibble) {
            return (m_skyLight[byteIndex] >> 4) & 0x0F;
        } else {
            return m_skyLight[byteIndex] & 0x0F;
        }
    }

    // =========================================================================
    // Sky Light - Invia luce piena (il client Minecraft calcola le ombre da solo)
    // =========================================================================
    void Chunk::calculateLighting() {
        // Il client di Minecraft gestisce internamente il rendering delle ombre
        // basandosi sulla geometria dei blocchi. Inviare skylight=15 ovunque
        // produce il risultato visivo perfetto senza glitch.
        std::fill(m_skyLight.begin(), m_skyLight.end(), 0xFF);
    }

    // =========================================================================
    // Determinazione bioma
    // =========================================================================
    static Biome determineBiome(float continentalness, float temperature, float humidity, float weirdness) {
        // Oceani
        if (continentalness < -0.20f) {
            if (temperature > 0.3f) return Biome::WarmOcean;
            return Biome::DeepOcean;
        }
        if (continentalness < -0.05f) {
            if (temperature > 0.3f) return Biome::WarmOcean;
            return Biome::Ocean;
        }

        // Spiaggia
        if (continentalness < 0.05f) return Biome::Beach;

        // Montagne
        if (weirdness > 0.55f) return Biome::Mountains;

        // Freddo
        if (temperature < -0.45f) {
            if (humidity > 0.0f) return Biome::SnowyTaiga;
            return Biome::SnowyPlains;
        }

        // Temperato freddo
        if (temperature < -0.15f) {
            return Biome::Taiga;
        }

        // Caldo
        if (temperature > 0.55f) {
            if (humidity > 0.3f) return Biome::Jungle;
            return Biome::Desert;
        }

        // Caldo moderato
        if (temperature > 0.25f) {
            if (humidity < -0.2f) return Biome::Savanna;
            if (humidity > 0.3f) return Biome::DarkForest;
            return Biome::Plains;
        }

        // Temperato
        if (humidity > 0.35f) return Biome::Swamp;
        if (humidity > 0.1f) {
            if (weirdness > 0.2f) return Biome::FlowerForest;
            return Biome::Forest;
        }
        if (humidity > -0.15f) return Biome::BirchForest;
        if (weirdness > 0.1f) return Biome::SunflowerPlains;
        return Biome::Plains;
    }

    // =========================================================================
    // Piazza un albero
    // =========================================================================
    void placeTree(Chunk& chunk, int x, int y, int z, uint16_t logId, uint16_t leavesId, int trunkH, int canopyR) {
        // Tronco
        for (int ty = 1; ty <= trunkH; ++ty) {
            chunk.setBlock(x, y + ty, z, logId);
        }
        // Foglie (corona sferica)
        for (int ly = trunkH - 1; ly <= trunkH + 1; ++ly) {
            int r = (ly == trunkH + 1) ? std::max(1, canopyR - 1) : canopyR;
            for (int lx = -r; lx <= r; ++lx) {
                for (int lz = -r; lz <= r; ++lz) {
                    if (lx == 0 && lz == 0 && ly <= trunkH) continue; // tronco
                    int bx = x + lx;
                    int bz = z + lz;
                    if (bx < 0 || bx > 15 || bz < 0 || bz > 15) continue;
                    // Forma arrotondata
                    if (lx * lx + lz * lz <= r * r + 1) {
                        if (chunk.getBlock(bx, y + ly, bz) == 0) {
                            chunk.setBlock(bx, y + ly, bz, leavesId);
                        }
                    }
                }
            }
        }
        // Cima
        chunk.setBlock(x, y + trunkH + 2, z, leavesId);
    }

    // Abete (conico)
    void placeSpruceTree(Chunk& chunk, int x, int y, int z) {
        using namespace BlockId;
        int trunkH = 6 + (hash2d(x + chunk.getX() * 16, z + chunk.getZ() * 16) % 3);
        for (int ty = 1; ty <= trunkH; ++ty) {
            chunk.setBlock(x, y + ty, z, SPRUCE_LOG);
        }
        // Fogliame conico
        for (int ly = 2; ly <= trunkH; ++ly) {
            int r = (trunkH - ly) / 2 + 1;
            if (r > 3) r = 3;
            for (int lx = -r; lx <= r; ++lx) {
                for (int lz = -r; lz <= r; ++lz) {
                    if (lx == 0 && lz == 0) continue;
                    int bx = x + lx, bz = z + lz;
                    if (bx < 0 || bx > 15 || bz < 0 || bz > 15) continue;
                    if (std::abs(lx) + std::abs(lz) <= r + 1) {
                        if (chunk.getBlock(bx, y + ly, bz) == 0)
                            chunk.setBlock(bx, y + ly, bz, SPRUCE_LEAVES);
                    }
                }
            }
        }
        chunk.setBlock(x, y + trunkH + 1, z, SPRUCE_LEAVES);
        chunk.setBlock(x, y + trunkH + 2, z, SPRUCE_LEAVES);
    }

    // Albero della giungla (alto)
    void placeJungleTree(Chunk& chunk, int x, int y, int z) {
        using namespace BlockId;
        int trunkH = 8 + (hash2d(x + chunk.getX() * 16, z + chunk.getZ() * 16 + 77) % 5);
        for (int ty = 1; ty <= trunkH; ++ty) {
            chunk.setBlock(x, y + ty, z, JUNGLE_LOG);
        }
        for (int ly = trunkH - 2; ly <= trunkH + 1; ++ly) {
            int r = (ly >= trunkH) ? 1 : 2;
            for (int lx = -r; lx <= r; ++lx) {
                for (int lz = -r; lz <= r; ++lz) {
                    if (lx == 0 && lz == 0 && ly <= trunkH) continue;
                    int bx = x + lx, bz = z + lz;
                    if (bx < 0 || bx > 15 || bz < 0 || bz > 15) continue;
                    if (chunk.getBlock(bx, y + ly, bz) == 0)
                        chunk.setBlock(bx, y + ly, bz, JUNGLE_LEAVES);
                }
            }
        }
    }

    // Acacia (con rami)
    void placeAcaciaTree(Chunk& chunk, int x, int y, int z) {
        using namespace BlockId;
        int trunkH = 4 + (hash2d(x + chunk.getX() * 16, z + chunk.getZ() * 16 + 99) % 3);
        for (int ty = 1; ty <= trunkH; ++ty) {
            chunk.setBlock(x, y + ty, z, ACACIA_LOG);
        }
        // Chioma piatta e larga
        for (int lx = -3; lx <= 3; ++lx) {
            for (int lz = -3; lz <= 3; ++lz) {
                int bx = x + lx, bz = z + lz;
                if (bx < 0 || bx > 15 || bz < 0 || bz > 15) continue;
                if (std::abs(lx) + std::abs(lz) <= 4) {
                    if (chunk.getBlock(bx, y + trunkH, bz) == 0)
                        chunk.setBlock(bx, y + trunkH, bz, ACACIA_LEAVES);
                    if (std::abs(lx) + std::abs(lz) <= 2) {
                        if (chunk.getBlock(bx, y + trunkH + 1, bz) == 0)
                            chunk.setBlock(bx, y + trunkH + 1, bz, ACACIA_LEAVES);
                    }
                }
            }
        }
    }

    // =========================================================================
    // Fiori casuali per un bioma
    // =========================================================================
    static uint16_t getRandomFlower(uint32_t h) {
        using namespace BlockId;
        static constexpr uint16_t flowers[] = {
            DANDELION, POPPY, BLUE_ORCHID, ALLIUM, AZURE_BLUET,
            RED_TULIP, ORANGE_TULIP, WHITE_TULIP, PINK_TULIP,
            OXEYE_DAISY, CORNFLOWER, LILY_OF_VALLEY
        };
        return flowers[h % 12];
    }

    // =========================================================================
    // Corallo reef per warm ocean
    // =========================================================================
    static uint16_t getRandomCoralBlock(uint32_t h) {
        using namespace BlockId;
        static constexpr uint16_t corals[] = {
            TUBE_CORAL_BLK, BRAIN_CORAL_BLK, BUBBLE_CORAL_BLK,
            FIRE_CORAL_BLK, HORN_CORAL_BLK
        };
        return corals[h % 5];
    }

    static uint16_t getRandomCoralFan(uint32_t h) {
        using namespace BlockId;
        static constexpr uint16_t fans[] = {
            TUBE_CORAL_FAN, BRAIN_CORAL_FAN, BUBBLE_CORAL_FAN,
            FIRE_CORAL_FAN, HORN_CORAL_FAN
        };
        return fans[h % 5];
    }

    // =========================================================================
    // Generazione terreno
    // =========================================================================
    void Chunk::generateTerrain() {
        using namespace BlockId;

        // --- Noise Setup ---
        // Continentalness: determina terra vs oceano
        FastNoiseLite contNoise;
        contNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        contNoise.SetSeed(1337);
        contNoise.SetFrequency(0.003f); // Frequenza ridotta per biomi e continenti più ampi
        contNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        contNoise.SetFractalOctaves(5);
        contNoise.SetFractalLacunarity(2.1f);
        contNoise.SetFractalGain(0.45f);

        // Erosione: modifica l'altezza (montagne vs pianure)
        FastNoiseLite erosionNoise;
        erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        erosionNoise.SetSeed(2718);
        erosionNoise.SetFrequency(0.004f); // Erosione più graduale
        erosionNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        erosionNoise.SetFractalOctaves(4);

        // Temperatura
        FastNoiseLite tempNoise;
        tempNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        tempNoise.SetSeed(4242);
        tempNoise.SetFrequency(0.001f);
        tempNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        tempNoise.SetFractalOctaves(3);

        // Umidità
        FastNoiseLite humidNoise;
        humidNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        humidNoise.SetSeed(9999);
        humidNoise.SetFrequency(0.0012f);
        humidNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        humidNoise.SetFractalOctaves(3);

        // Weirdness: dettaglio extra biomi
        FastNoiseLite weirdNoise;
        weirdNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        weirdNoise.SetSeed(5555);
        weirdNoise.SetFrequency(0.004f);

        // Dettaglio altezza locale
        FastNoiseLite detailNoise;
        detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
        detailNoise.SetSeed(7777);
        detailNoise.SetFrequency(0.02f);
        detailNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        detailNoise.SetFractalOctaves(3);

        // Grotte 3D
        FastNoiseLite caveNoise;
        caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
        caveNoise.SetSeed(3141);
        caveNoise.SetFrequency(0.03f);
        caveNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        caveNoise.SetFractalOctaves(3);

        const int SEA_LEVEL = 63;

        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                float worldX = static_cast<float>(m_chunkX * 16 + x);
                float worldZ = static_cast<float>(m_chunkZ * 16 + z);

                // Campiona i noise
                float cont = contNoise.GetNoise(worldX, worldZ);
                float erosion = erosionNoise.GetNoise(worldX, worldZ);
                float temp = tempNoise.GetNoise(worldX, worldZ);
                float humid = humidNoise.GetNoise(worldX, worldZ);
                float weird = weirdNoise.GetNoise(worldX, worldZ);
                float detail = detailNoise.GetNoise(worldX, worldZ);

                // Determina il bioma
                Biome biome = determineBiome(cont, temp, humid, weird);

                // --- Calcola altezza del terreno ---
                float baseHeight;
                if (cont < -0.15f) {
                    // Deep Ocean: 25-40
                    float t2 = (cont + 0.60f) / 0.45f; 
                    t2 = std::clamp(t2, 0.0f, 1.0f);
                    baseHeight = 25.0f + t2 * 15.0f;
                } else if (cont < 0.0f) {
                    // Shallow Ocean: 40-58
                    float t2 = (cont + 0.15f) / 0.15f;
                    t2 = std::clamp(t2, 0.0f, 1.0f);
                    baseHeight = 40.0f + t2 * 18.0f;
                } else if (cont < 0.35f) {
                    // Beach/Coast/Plains: 63-72 (pianure ampie e piatte)
                    float t2 = (cont) / 0.35f;
                    t2 = std::clamp(t2, 0.0f, 1.0f);
                    t2 = t2 * t2 * (3.0f - 2.0f * t2); // Smooth
                    baseHeight = 63.0f + t2 * 9.0f;
                } else if (cont < 0.70f) {
                    // Inland plains/hills: 72-90
                    float t2 = (cont - 0.35f) / 0.35f;
                    t2 = std::clamp(t2, 0.0f, 1.0f);
                    baseHeight = 72.0f + t2 * 18.0f;
                } else {
                    // Mountains: 90-180, picchi ripidi che partono dal basso
                    float t2 = (cont - 0.70f) / 0.30f;
                    t2 = std::clamp(t2, 0.0f, 1.0f);
                    t2 = t2 * t2 * t2; // Molto più esponenziale per farle crescere piano piano e poi svettare
                    baseHeight = 90.0f + t2 * 90.0f;
                }

                // L'erosione aggiunge variazione dolce
                float erosionFactor = 1.0f;
                if (cont > 0.40f) {
                    erosionFactor = 1.0f + (cont - 0.40f) * 2.0f;
                }
                baseHeight += erosion * 6.0f * erosionFactor;

                // Dettaglio locale (microterreno)
                baseHeight += detail * 2.0f;

                int surfaceY = static_cast<int>(baseHeight);
                surfaceY = std::clamp(surfaceY, -60, 300);

                // --- Scegli blocchi di superficie ---
                uint16_t topBlock = GRASS_BLOCK;
                uint16_t midBlock = DIRT;
                uint16_t stoneBlock = STONE;
                int topDepth = 1;
                int midDepth = 4;

                switch (biome) {
                    case Biome::Desert:
                        topBlock = SAND; midBlock = SAND; stoneBlock = SANDSTONE; midDepth = 6;
                        break;
                    case Biome::Beach:
                        topBlock = SAND; midBlock = SAND; midDepth = 5;
                        break;
                    case Biome::Ocean:
                    case Biome::DeepOcean:
                        topBlock = GRAVEL; midBlock = GRAVEL; midDepth = 3;
                        break;
                    case Biome::WarmOcean:
                        topBlock = SAND; midBlock = SAND; midDepth = 4;
                        break;
                    case Biome::Savanna:
                        topBlock = COARSE_DIRT; midBlock = DIRT;
                        break;
                    case Biome::Taiga:
                        topBlock = PODZOL; midBlock = DIRT;
                        break;
                    case Biome::SnowyTaiga:
                    case Biome::SnowyPlains:
                        topBlock = SNOW_BLOCK; midBlock = DIRT;
                        break;
                    case Biome::Swamp:
                        topBlock = GRASS_BLOCK; midBlock = MUD; midDepth = 5;
                        break;
                    case Biome::MushroomFields:
                        topBlock = MYCELIUM; midBlock = DIRT;
                        break;
                    case Biome::Mountains:
                        if (surfaceY > 120) {
                            topBlock = SNOW_BLOCK; midBlock = STONE; midDepth = 2;
                        } else if (surfaceY > 100) {
                            topBlock = STONE; midBlock = STONE; midDepth = 2;
                        }
                        break;
                    default:
                        break;
                }

                // --- Piazza blocchi colonna ---
                for (int y = -64; y <= 319; ++y) {
                    // Bedrock
                    if (y == -64) {
                        setBlock(x, y, z, BEDROCK);
                        continue;
                    }
                    if (y <= -61 && (hash2d(static_cast<int>(worldX), y * 31 + static_cast<int>(worldZ)) % 4 != 0)) {
                        setBlock(x, y, z, BEDROCK);
                        continue;
                    }

                    if (y > surfaceY) {
                        // Sopra la superficie
                        if (y <= SEA_LEVEL) {
                            setBlock(x, y, z, WATER);
                        }
                        // else AIR (già 0)
                        continue;
                    }

                    // Deepslate sotto y=0
                    if (y < 0) {
                        stoneBlock = DEEPSLATE;
                    } else {
                        // Reset to surface stone
                        if (biome == Biome::Desert) stoneBlock = SANDSTONE;
                        else stoneBlock = STONE;
                    }

                    // Grotte 3D (solo sotto la superficie)
                    if (y < surfaceY - 2 && y > -60) {
                        float cx = caveNoise.GetNoise(worldX, static_cast<float>(y), worldZ);
                        if (cx > 0.55f) {
                            if (y <= SEA_LEVEL) {
                                setBlock(x, y, z, WATER);
                            }
                            // else AIR
                            continue;
                        }
                    }

                    if (y == surfaceY) {
                        setBlock(x, y, z, topBlock);
                    } else if (y > surfaceY - topDepth - midDepth) {
                        setBlock(x, y, z, midBlock);
                    } else {
                        // Pietra con minerali
                        uint32_t oreHash = hash2d(static_cast<int>(worldX) * 3 + y * 17, static_cast<int>(worldZ) * 7 + y);
                        if (y < 16 && (oreHash % 200 == 0)) {
                            setBlock(x, y, z, DIAMOND_ORE);
                        } else if (y < 32 && (oreHash % 120 == 0)) {
                            setBlock(x, y, z, GOLD_ORE);
                        } else if (y < 40 && (oreHash % 80 == 0)) {
                            setBlock(x, y, z, LAPIS_ORE);
                        } else if (y < 64 && (oreHash % 60 == 0)) {
                            setBlock(x, y, z, IRON_ORE);
                        } else if (y < 80 && (oreHash % 45 == 0)) {
                            setBlock(x, y, z, COAL_ORE);
                        } else if (y < 50 && (oreHash % 70 == 0)) {
                            setBlock(x, y, z, COPPER_ORE);
                        } else if (y < 32 && y > -32 && (oreHash % 100 == 0)) {
                            setBlock(x, y, z, EMERALD_ORE);
                        } else {
                            // Variazioni di pietra
                            if (oreHash % 30 == 1) setBlock(x, y, z, GRANITE);
                            else if (oreHash % 30 == 2) setBlock(x, y, z, DIORITE);
                            else if (oreHash % 30 == 3) setBlock(x, y, z, ANDESITE);
                            else if (y < -20 && oreHash % 25 == 4) setBlock(x, y, z, TUFF);
                            else if (y < -30 && oreHash % 30 == 5) setBlock(x, y, z, CALCITE);
                            else setBlock(x, y, z, stoneBlock);
                        }
                    }
                }

                // --- Decorazioni sopra la superficie ---
                if (surfaceY < SEA_LEVEL) {
                    // Sottacqua
                    decorateUnderwater(x, z, surfaceY, biome);
                } else {
                    decorateSurface(x, z, surfaceY, biome);
                }
            }
        }

        calculateLighting();
        setReady(true);
    }

    // =========================================================================
    // Decorazioni sottomarine
    // =========================================================================
    void Chunk::decorateUnderwater(int x, int z, int surfaceY, Biome biome) {
        using namespace BlockId;
        float worldX = static_cast<float>(m_chunkX * 16 + x);
        float worldZ = static_cast<float>(m_chunkZ * 16 + z);
        uint32_t h = hash2d(static_cast<int>(worldX), static_cast<int>(worldZ));

        if (biome == Biome::WarmOcean) {
            // --- Coral Reef ---
            if (surfaceY > 40 && surfaceY < 58) {
                // Blocchi di corallo
                if (h % 8 == 0) {
                    setBlock(x, surfaceY + 1, z, getRandomCoralBlock(h));
                    if (h % 16 == 0) {
                        setBlock(x, surfaceY + 2, z, getRandomCoralBlock(h >> 3));
                    }
                }
                // Fan di corallo sopra i blocchi
                if (h % 6 == 1) {
                    uint16_t below = getBlock(x, surfaceY + 1, z);
                    if (below != AIR && below != WATER) {
                        setBlock(x, surfaceY + 2, z, getRandomCoralFan(h >> 2));
                    } else {
                        setBlock(x, surfaceY + 1, z, getRandomCoralFan(h >> 2));
                    }
                }
                // Sea pickles
                if (h % 12 == 2) {
                    setBlock(x, surfaceY + 1, z, SEA_PICKLE);
                }
            }
            // Seagrass
            if (h % 4 == 0) {
                if (getBlock(x, surfaceY + 1, z) == WATER) {
                    setBlock(x, surfaceY + 1, z, SEAGRASS);
                }
            }
        } else {
            // Oceano freddo/normale
            // Kelp
            if (h % 10 == 0 && surfaceY < 55) {
                int kelpH = 3 + static_cast<int>(h % 8);
                for (int ky = 1; ky <= kelpH && surfaceY + ky < 62; ++ky) {
                    setBlock(x, surfaceY + ky, z, KELP_PLANT);
                }
            }
            // Seagrass
            if (h % 5 == 0) {
                if (getBlock(x, surfaceY + 1, z) == WATER) {
                    setBlock(x, surfaceY + 1, z, SEAGRASS);
                }
            }
        }
    }

    // =========================================================================
    // Decorazioni di superficie
    // =========================================================================
    void Chunk::decorateSurface(int x, int z, int surfaceY, Biome biome) {
        using namespace BlockId;
        float worldX = static_cast<float>(m_chunkX * 16 + x);
        float worldZ = static_cast<float>(m_chunkZ * 16 + z);
        uint32_t h = hash2d(static_cast<int>(worldX), static_cast<int>(worldZ));
        int SEA_LEVEL = 63;

        switch (biome) {
        case Biome::Plains:
        case Biome::SunflowerPlains:
            // Erba corta
            if (h % 3 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            // Fiori
            if (h % 20 == 1) setBlock(x, surfaceY + 1, z, getRandomFlower(h >> 3));
            // Girasoli (SunflowerPlains)
            if (biome == Biome::SunflowerPlains && h % 15 == 2)
                setBlock(x, surfaceY + 1, z, SUNFLOWER_L);
            // Alberi rari
            if (h % 200 == 3)
                placeTree(*this, x, surfaceY, z, OAK_LOG, OAK_LEAVES, 5, 2);
            break;

        case Biome::Forest:
            if (h % 4 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 12 == 1) setBlock(x, surfaceY + 1, z, getRandomFlower(h >> 2));
            if (h % 25 == 2)
                placeTree(*this, x, surfaceY, z, OAK_LOG, OAK_LEAVES, 4 + (h % 3), 2);
            else if (h % 30 == 3)
                placeTree(*this, x, surfaceY, z, BIRCH_LOG, BIRCH_LEAVES, 5 + (h % 2), 2);
            break;

        case Biome::BirchForest:
            if (h % 4 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 15 == 1) setBlock(x, surfaceY + 1, z, LILY_OF_VALLEY);
            if (h % 25 == 2)
                placeTree(*this, x, surfaceY, z, BIRCH_LOG, BIRCH_LEAVES, 5 + (h % 3), 2);
            break;

        case Biome::DarkForest:
            if (h % 5 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 20 == 1) setBlock(x, surfaceY + 1, z, RED_MUSHROOM);
            if (h % 25 == 2) setBlock(x, surfaceY + 1, z, BROWN_MUSHROOM);
            if (h % 18 == 3)
                placeTree(*this, x, surfaceY, z, DARK_OAK_LOG, DARK_OAK_LEAVES, 5 + (h % 2), 3);
            break;

        case Biome::FlowerForest:
            if (h % 3 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 4 == 1) setBlock(x, surfaceY + 1, z, getRandomFlower(h));
            if (h % 30 == 2)
                placeTree(*this, x, surfaceY, z, OAK_LOG, OAK_LEAVES, 4 + (h % 2), 2);
            else if (h % 35 == 3)
                placeTree(*this, x, surfaceY, z, BIRCH_LOG, BIRCH_LEAVES, 5, 2);
            break;

        case Biome::Taiga:
            if (h % 4 == 0) setBlock(x, surfaceY + 1, z, FERN);
            if (h % 22 == 1) placeSpruceTree(*this, x, surfaceY, z);
            if (h % 30 == 2) setBlock(x, surfaceY + 1, z, BROWN_MUSHROOM);
            if (h % 35 == 3) setBlock(x, surfaceY + 1, z, RED_MUSHROOM);
            break;

        case Biome::SnowyTaiga:
            if (h % 5 == 0) setBlock(x, surfaceY + 1, z, FERN);
            if (h % 25 == 1) placeSpruceTree(*this, x, surfaceY, z);
            // Neve sopra il terreno
            if (getBlock(x, surfaceY + 1, z) == AIR)
                setBlock(x, surfaceY + 1, z, SNOW_LAYER);
            break;

        case Biome::SnowyPlains:
            // Neve e pochissimi alberi
            if (h % 300 == 0) placeSpruceTree(*this, x, surfaceY, z);
            if (getBlock(x, surfaceY + 1, z) == AIR)
                setBlock(x, surfaceY + 1, z, SNOW_LAYER);
            break;

        case Biome::Desert:
            if (h % 80 == 0) setBlock(x, surfaceY + 1, z, DEAD_BUSH);
            // Cactus
            if (h % 120 == 1) {
                int cactusH = 1 + (h % 3);
                for (int cy = 1; cy <= cactusH; ++cy)
                    setBlock(x, surfaceY + cy, z, CACTUS);
            }
            // Sugar cane vicino all'acqua (semplificato)
            break;

        case Biome::Savanna:
            if (h % 4 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 35 == 1) placeAcaciaTree(*this, x, surfaceY, z);
            if (h % 50 == 2) setBlock(x, surfaceY + 1, z, DEAD_BUSH);
            break;

        case Biome::Jungle:
            if (h % 3 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            if (h % 4 == 1) setBlock(x, surfaceY + 1, z, FERN);
            if (h % 6 == 2) placeJungleTree(*this, x, surfaceY, z);
            if (h % 10 == 3) setBlock(x, surfaceY + 1, z, getRandomFlower(h >> 1));
            break;

        case Biome::Swamp:
            if (h % 4 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
            // Ninfee sull'acqua
            if (surfaceY == SEA_LEVEL && h % 6 == 1)
                setBlock(x, surfaceY + 1, z, LILY_PAD);
            if (h % 15 == 2) setBlock(x, surfaceY + 1, z, BROWN_MUSHROOM);
            if (h % 12 == 3)
                placeTree(*this, x, surfaceY, z, OAK_LOG, OAK_LEAVES, 4, 2);
            break;

        case Biome::Mountains:
            if (surfaceY < 100) {
                if (h % 5 == 0) setBlock(x, surfaceY + 1, z, SHORT_GRASS);
                if (h % 30 == 1) placeSpruceTree(*this, x, surfaceY, z);
            }
            if (surfaceY > 110 && getBlock(x, surfaceY + 1, z) == AIR)
                setBlock(x, surfaceY + 1, z, SNOW_LAYER);
            break;

        case Biome::MushroomFields:
            if (h % 8 == 0) setBlock(x, surfaceY + 1, z, RED_MUSHROOM);
            if (h % 8 == 1) setBlock(x, surfaceY + 1, z, BROWN_MUSHROOM);
            break;

        case Biome::Beach:
            // Spiaggia pulita, occasionalmente canna da zucchero
            if (h % 200 == 0) setBlock(x, surfaceY + 1, z, SUGAR_CANE);
            break;

        default:
            break;
        }
    }

}

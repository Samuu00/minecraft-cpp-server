#pragma once

#include <cstdint>

class World;

class Entity {
protected:
    int32_t m_id;
    int32_t m_type;
    World* m_world;
    double m_x, m_y, m_z;
    double m_vx, m_vy, m_vz;
    bool m_onGround{false};
    bool m_markedForRemoval{false};
    uint64_t m_age{0};

public:
    Entity(int32_t id, int32_t type, World* world, double x, double y, double z)
        : m_id(id), m_type(type), m_world(world), m_x(x), m_y(y), m_z(z),
          m_vx(0), m_vy(0), m_vz(0) {}

    virtual ~Entity() = default;

    [[nodiscard]] int32_t getId() const { return m_id; }
    [[nodiscard]] int32_t getType() const { return m_type; }

    [[nodiscard]] double getX() const { return m_x; }
    [[nodiscard]] double getY() const { return m_y; }
    [[nodiscard]] double getZ() const { return m_z; }

    void setVelocity(double vx, double vy, double vz) {
        m_vx = vx;
        m_vy = vy;
        m_vz = vz;
    }

    [[nodiscard]] bool isMarkedForRemoval() const { return m_markedForRemoval; }
    void markForRemoval() { m_markedForRemoval = true; }

    virtual void tick(double deltaTime);
};

class MobEntity : public Entity {
public:
    MobEntity(int32_t id, int32_t type, World* world, double x, double y, double z)
        : Entity(id, type, world, x, y, z) {}

    void tick(double deltaTime) override {
        Entity::tick(deltaTime);
        if (m_onGround && m_age % 100 == 0) {
            m_vy = 0.4;
            uint32_t seed = static_cast<uint32_t>((m_id * 31 + m_age) * 1337);
            m_vx = (static_cast<int>(seed % 100) - 50) / 500.0;
            m_vz = (static_cast<int>((seed / 100) % 100) - 50) / 500.0;
        }
    }
};

class ItemEntity : public Entity {
private:
    uint16_t m_itemId;

public:
    ItemEntity(int32_t id, World* world, double x, double y, double z, uint16_t itemId)
        : Entity(id, 55, world, x, y, z), m_itemId(itemId) {} // 55 = Item entity in 1.20.4

    [[nodiscard]] uint16_t getItemId() const { return m_itemId; }

    void tick(double deltaTime) override;
};

#pragma once

#include <string>
#include <cstdint>

struct Vector3 {
    double x{0.0};
    double y{64.0}; 
    double z{0.0};
};

struct Rotation {
    float yaw{0.0f};   // Rotazione orizzontale (0-360)
    float pitch{0.0f}; // Rotazione verticale (-90 a 90)
};

enum class GameMode : uint8_t {
    Survival = 0,
    Creative = 1,
    Adventure = 2,
    Spectator = 3
};

class Player {
public:
    Player(int id, std::string username, std::string uuid);
    ~Player();

    void tick(double deltaTime);

    [[nodiscard]] int getId() const { return m_id; }
    [[nodiscard]] const std::string& getUsername() const { return m_username; }
    [[nodiscard]] const std::string& getUuid() const { return m_uuid; }

    [[nodiscard]] const Vector3& getPosition() const { return m_position; }
    [[nodiscard]] const Rotation& getRotation() const { return m_rotation; }
    
    void setPosition(double x, double y, double z);
    void setRotation(float yaw, float pitch);
    
    [[nodiscard]] float getHealth() const { return m_health; }
    void setHealth(float health);
    void takeDamage(float amount);

    [[nodiscard]] GameMode getGameMode() const { return m_gameMode; }
    void setGameMode(GameMode mode) { m_gameMode = mode; }

    [[nodiscard]] bool isOnGround() const { return m_onGround; }
    void setOnGround(bool onGround) { m_onGround = onGround; }

private:
    int m_id;
    std::string m_username;
    std::string m_uuid;

    // Posizione e Fisica
    Vector3 m_position;
    Vector3 m_velocity{0.0, 0.0, 0.0};
    Rotation m_rotation;
    bool m_onGround{true};

    // Attributi Giocatore
    float m_health{20.0f};      // Max 20.0 (10 cuori)
    int32_t m_foodLevel{20};    // Max 20 (10 cosce)
    GameMode m_gameMode{GameMode::Survival};

    double m_invulnerabilityTimer{0.0};
    double m_regenerationTimer{0.0};
};

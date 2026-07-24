#include "Player.hpp"
#include "../utils/Logger.hpp"
#include <algorithm>
#include <map>

Player::Player(int id, std::string username, std::string uuid)
    : m_id(id), m_username(std::move(username)), m_uuid(std::move(uuid)) {
    LOG_INFO("Player creato: ", m_username, " (UUID: ", m_uuid, ", ID: ", m_id, ")");
}

Player::~Player() {
    LOG_INFO("Player eliminato: ", m_username);
}

void Player::setPosition(double x, double y, double z){
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
}

void Player::setRotation(float yaw, float pitch){
    m_rotation.yaw = yaw;
    m_rotation.pitch = pitch;
}

void Player::setHealth(float health){
    m_health = std::clamp(health, 0.0f, 20.0f);
    m_hasHealthChanged = true;
}

void Player::takeDamage(float amount){
    if(m_gameMode == GameMode::Creative || m_gameMode == GameMode::Spectator) return;

    if(m_invulnerabilityTimer > 0.0) return;

    m_health = std::max(0.0f, m_health - amount);
    m_invulnerabilityTimer = 0.5;
    m_hasDamageEvent = true;
    m_hasHealthChanged = true;

    LOG_INFO(m_username, " ha subito ", amount, " danni! Salute rimanente: ", m_health);

    if(m_health <= 0.0f) LOG_INFO(m_username, " e' morto!");
}

void Player::tick(double deltaTime){
    if(m_invulnerabilityTimer > 0.0) m_invulnerabilityTimer -= deltaTime;

    if (m_gameMode == GameMode::Survival && m_health < 20.0f && m_foodLevel >= 18){
        m_regenerationTimer += deltaTime;

        if(m_regenerationTimer >= 4.0){
            setHealth(m_health + 1.0f);
            m_regenerationTimer = 0.0;
            LOG_INFO("Rigenerazione naturale per ", m_username, ": HP ", m_health);
        }
    }

    if(m_position.y < -64.0) takeDamage(4.0f);
}

int Player::addInventoryItem(uint16_t itemId, uint8_t count) {
    // Cerchiamo uno slot esistente (tra 36 e 44, l'hotbar) con lo stesso item e che non sia pieno (64)
    for (int slot = 36; slot <= 44; ++slot) {
        if (m_inventory.count(slot) > 0) {
            auto& item = m_inventory[slot];
            if (item.first == itemId && item.second < 64) {
                int add = std::min((int)count, 64 - item.second);
                item.second += add;
                return slot;
            }
        }
    }
    // Se non troviamo uno slot compatibile, cerchiamo il primo slot libero
    for (int slot = 36; slot <= 44; ++slot) {
        if (m_inventory.count(slot) == 0) {
            m_inventory[slot] = {itemId, std::min((int)count, 64)};
            return slot;
        }
    }
    return -1; // Inventario pieno
}

bool Player::removeInventoryItem(int slot, uint8_t count) {
    if (m_inventory.count(slot) > 0) {
        auto& item = m_inventory[slot];
        if (item.second > count) {
            item.second -= count;
        } else {
            m_inventory.erase(slot);
        }
        return true;
    }
    return false;
}

std::pair<uint16_t, uint8_t> Player::getInventoryItem(int slot) const {
    auto it = m_inventory.find(slot);
    if (it != m_inventory.end()) {
        return it->second;
    }
    return {0, 0};
}

void Player::setInventoryItem(int slot, uint16_t itemId, uint8_t count) {
    if (count > 0 && itemId != 0) {
        m_inventory[slot] = {itemId, count};
    } else {
        m_inventory.erase(slot);
    }
}


#include "Entity.hpp"
#include "World.hpp"
#include <cmath>

void Entity::tick(double deltaTime) {
    (void)deltaTime;
    m_age++;

    // Applica gravità
    if (!m_onGround) {
        m_vy -= 0.04; // Gravity
    }
    
    // Drag
    m_vx *= 0.98;
    m_vy *= 0.98;
    m_vz *= 0.98;

    // Aggiorna posizione
    m_x += m_vx;
    m_y += m_vy;
    m_z += m_vz;

    // Semplice collisione col terreno (se y cade sotto un blocco solido)
    // Non precisissimo ma fa in modo che gli item non cadano nel vuoto
    if (m_world) {
        uint16_t blockBelow = m_world->getBlock(static_cast<int>(std::floor(m_x)), 
                                                static_cast<int>(std::floor(m_y - 0.1)), 
                                                static_cast<int>(std::floor(m_z)));
        if (blockBelow != 0 && blockBelow != 80) { // Non AIR e non WATER
            m_y = std::floor(m_y);
            m_vy = 0;
            m_onGround = true;
            m_vx *= 0.7; // Attrito col suolo
            m_vz *= 0.7;
        } else {
            m_onGround = false;
        }
    }
}

void ItemEntity::tick(double deltaTime) {
    Entity::tick(deltaTime);
    if (m_pickupDelay > 0) {
        m_pickupDelay--;
    }
    // Semplice gravità e attrito per gli oggetti (6000 tick a 20 TPS)
    if (m_age > 6000) {
        markForRemoval();
    }
}

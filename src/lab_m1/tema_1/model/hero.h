#pragma once

#include "lab_m1/tema_1/config/game_rules.h"

namespace m1
{
    // Ambii jucatori folosesc acelasi erou simplu, fara abilitati speciale.
    class Hero
    {
     public:
        int GetHealth() const { return health; }
        int GetAttack() const { return game_rules::HeroAttack; }
        void TakeDamage(int damage);
        void Heal(int amount);

     private:
        int health = game_rules::InitialHeroHealth;
    };
}

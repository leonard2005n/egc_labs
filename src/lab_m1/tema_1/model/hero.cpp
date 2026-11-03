#include "lab_m1/tema_1/model/hero.h"

#include <algorithm>

using namespace m1;

void Hero::TakeDamage(int damage)
{
    health = std::max(0, health - std::max(0, damage));
}

void Hero::Heal(int amount)
{
    // Lifesteal may raise health above the initial 30 HP.
    health += std::max(0, amount);
}

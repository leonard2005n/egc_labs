#include "lab_m1/tema_1/model/card.h"

#include <algorithm>

using namespace m1;

Card::Card(CardType type, int attack, int health, int manaCost, CardEffect effects)
    : type(type), attack(attack), health(health), manaCost(manaCost), effects(effects)
{
}

bool Card::HasEffect(CardEffect effect) const
{
    return (static_cast<int>(effects) & static_cast<int>(effect)) != 0;
}

void Card::EnterBoard()
{
    summonedThisTurn = true;
    ready = HasEffect(CardEffect::Charge);
}

void Card::Silence()
{
    // Eliminarea Charge anuleaza atacul imediat al unei creaturi nou plasate.
    if (summonedThisTurn && HasEffect(CardEffect::Charge)) {
        ready = false;
    }

    effects = CardEffect::None;
    silenced = true;
}

bool Card::CanAttack() const
{
    return ready && health > 0 && attack > 0;
}

void Card::ReadyForTurn()
{
    ready = true;
    summonedThisTurn = false;
}

void Card::Exhaust()
{
    ready = false;
}

void Card::TakeDamage(int damage)
{
    health = std::max(0, health - std::max(0, damage));
}

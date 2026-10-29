#pragma once

#include <algorithm>

namespace m1
{
    // Ambii jucatori folosesc acelasi erou simplu, fara abilitati speciale.
    class Hero
    {
     public:
        // Intoarce viata ramasa a eroului, initial 30.
        int GetHealth() const { return health; }
        // Eroul nu provoaca damage si nu poate fi selectat ca atacator.
        int GetAttack() const { return 0; }
        // Reduce viata eroului pana la minimum zero.
        void TakeDamage(int damage) { health = std::max(0, health - std::max(0, damage)); }
        // Lifesteal adauga intreaga valoare ATK; viata poate depasi valoarea initiala de 30.
        void Heal(int amount) { health += std::max(0, amount); }

     private:
        int health = 30;
    };
}

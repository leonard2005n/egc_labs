#pragma once

#include <algorithm>

namespace m1
{
    enum class CardType { Triangle, Diamond, Square };
    enum class CardEffect { None = 0, Lifesteal = 1, Charge = 2, Tank = 4, Silence = 8 };

    inline CardEffect operator|(CardEffect first, CardEffect second)
    {
        return static_cast<CardEffect>(static_cast<int>(first) | static_cast<int>(second));
    }

    // Pastreaza datele unei creaturi si dreptul ei de a ataca, fara cod grafic.
    class Card
    {
     public:
        // Creeaza o carte cu atac, viata si cost; initial nu poate ataca.
        Card(CardType type, int attack, int health, int manaCost, CardEffect effects = CardEffect::None)
            : type(type), attack(attack), health(health), manaCost(manaCost), effects(effects) {}

        bool HasEffect(CardEffect effect) const
        {
            return (static_cast<int>(effects) & static_cast<int>(effect)) != 0;
        }
        bool IsSilenced() const { return silenced; }
        void EnterBoard() { summonedThisTurn = true; ready = HasEffect(CardEffect::Charge); }
        void Silence()
        {
            if (summonedThisTurn && HasEffect(CardEffect::Charge)) {
                ready = false;
            }
            effects = CardEffect::None;
            silenced = true;
        }

        // Intoarce tipul folosit pentru culoare si simbol.
        CardType GetType() const { return type; }
        // Intoarce damage-ul provocat intr-o confruntare.
        int GetAttack() const { return attack; }
        // Intoarce viata ramasa dupa confruntari.
        int GetHealth() const { return health; }
        // Intoarce mana necesara pentru a juca aceasta carte.
        int GetManaCost() const { return manaCost; }
        // Permite atacul doar unei creaturi vii, pregatite si cu atac pozitiv.
        bool CanAttack() const { return ready && health > 0 && attack > 0; }
        // Pregateste creatura la inceputul turei proprietarului.
        void ReadyForTurn() { ready = true; summonedThisTurn = false; }
        // Consuma atacul sau impune asteptarea dupa plasarea pe tabla.
        void Exhaust() { ready = false; }
        // Aplica damage fara sa lase viata afisata negativa.
        void TakeDamage(int damage) { health = std::max(0, health - std::max(0, damage)); }

     private:
        CardType type;
        int attack;
        int health;
        int manaCost;
        CardEffect effects;
        bool silenced = false;
        bool summonedThisTurn = false;
        bool ready = false;
    };
}

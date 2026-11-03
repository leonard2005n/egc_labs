#pragma once

namespace m1
{
    enum class CardType
    {
        Triangle,
        Diamond,
        Square
    };

    enum class CardEffect
    {
        None = 0,
        Lifesteal = 1,
        Charge = 2,
        Tank = 4,
        Silence = 8
    };

    inline CardEffect operator|(CardEffect first, CardEffect second)
    {
        return static_cast<CardEffect>(static_cast<int>(first) | static_cast<int>(second));
    }

    // Pastreaza datele unei creaturi si dreptul ei de a ataca, fara cod grafic.
    class Card
    {
     public:
        Card(CardType type, int attack, int health, int manaCost, CardEffect effects = CardEffect::None);

        CardType GetType() const { return type; }
        int GetAttack() const { return attack; }
        int GetHealth() const { return health; }
        int GetManaCost() const { return manaCost; }
        bool IsSilenced() const { return silenced; }

        bool HasEffect(CardEffect effect) const;
        bool CanAttack() const;
        void EnterBoard();
        void Silence();
        void ReadyForTurn();
        void Exhaust();
        void TakeDamage(int damage);

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

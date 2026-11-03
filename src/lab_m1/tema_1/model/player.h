#pragma once

#include "lab_m1/tema_1/config/game_rules.h"

#include <cstddef>
#include <vector>

#include "lab_m1/tema_1/model/card.h"
#include "lab_m1/tema_1/model/hero.h"

namespace m1
{
    // Detine cartile, eroul si resursele unui jucator, fara dependente OpenGL.
    class Player
    {
     public:
        Player(const std::vector<Card>& hand = {}, const std::vector<Card>& deck = {});

        const std::vector<Card>& GetHand() const { return hand; }
        const std::vector<Card>& GetDeck() const { return deck; }
        const std::vector<Card>& GetBoard() const { return board; }
        const Hero& GetHero() const { return hero; }
        int GetMana() const { return mana; }
        int GetMaxMana() const { return maxMana; }
        int GetTurnCount() const { return turnsStarted; }

        void BeginTurn();
        bool DrawCard();
        bool CanPlayCard(std::size_t index) const;
        bool PlayCard(std::size_t index);

        void DamageCard(std::size_t index, int damage);
        void ExhaustCard(std::size_t index);
        void DamageHero(int damage);
        void HealHero(int amount);
        void SilenceCard(std::size_t index);
        void RemoveDeadCards();

     private:
        static bool IsDead(const Card& card);

        std::vector<Card> hand;
        std::vector<Card> deck;
        std::vector<Card> board;
        Hero hero;
        int mana = game_rules::InitialMana;
        int maxMana = game_rules::InitialMana;
        int turnsStarted = 0;
    };
}

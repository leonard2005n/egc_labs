#pragma once

#include <algorithm>
#include <vector>

#include "lab_m1/tema_1/card.h"
#include "lab_m1/tema_1/hero.h"

namespace m1
{
    // Detine cartile, eroul si resursele unui jucator, fara dependente OpenGL.
    class Player
    {
     public:
        // Creeaza un jucator cu listele primite si un erou la viata maxima.
        Player(const std::vector<Card>& hand = {}, const std::vector<Card>& deck = {})
            : hand(hand), deck(deck) {}

        // Ofera mana doar pentru citire.
        const std::vector<Card>& GetHand() const { return hand; }
        // Ofera pachetul doar pentru citire.
        const std::vector<Card>& GetDeck() const { return deck; }
        // Ofera creaturile de pe tabla doar pentru citire.
        const std::vector<Card>& GetBoard() const { return board; }
        // Ofera eroul doar pentru citire.
        const Hero& GetHero() const { return hero; }
        // Intoarce mana disponibila in tura curenta.
        int GetMana() const { return mana; }
        // Intoarce capacitatea de mana a jucatorului.
        int GetMaxMana() const { return maxMana; }
        // Intoarce numarul turei proprii, independent de turele adversarului.
        int GetTurnCount() const { return turnsStarted; }

        // Prima tura foloseste o mana; urmatoarele cresc maximul, reumplu si pregatesc atacurile.
        void BeginTurn()
        {
            if (turnsStarted > 0) {
                maxMana = std::min(10, maxMana + 1);
            }
            ++turnsStarted;
            mana = maxMana;
            for (Card& card : board) {
                card.ReadyForTurn();
            }
        }

        // Muta cartea din varful pachetului in mana; pachetul gol nu produce damage.
        bool DrawCard()
        {
            if (deck.empty()) {
                return false;
            }
            hand.push_back(deck.back());
            deck.pop_back();
            return true;
        }

        // Verifica existenta cartii si costul ei inaintea oricarei modificari.
        bool CanPlayCard(std::size_t index) const
        {
            return index < hand.size() && hand[index].GetManaCost() <= mana;
        }

        // Plateste costul si muta cartea pe tabla; Charge permite atacul imediat.
        bool PlayCard(std::size_t index)
        {
            if (!CanPlayCard(index)) {
                return false;
            }
            mana -= hand[index].GetManaCost();
            board.push_back(hand[index]);
            board.back().EnterBoard();
            hand.erase(hand.begin() + index);
            return true;
        }

        // Aplica damage unei creaturi existente; eliminarea se face dupa ambele lovituri.
        void DamageCard(std::size_t index, int damage) { board.at(index).TakeDamage(damage); }
        // Consuma atacul creaturii alese.
        void ExhaustCard(std::size_t index) { board.at(index).Exhaust(); }
        // Aplica damage eroului.
        void DamageHero(int damage) { hero.TakeDamage(damage); }
        void HealHero(int amount) { hero.Heal(amount); }
        void SilenceCard(std::size_t index) { board.at(index).Silence(); }
        // Elimina toate creaturile care nu mai au viata.
        void RemoveDeadCards()
        {
            board.erase(std::remove_if(board.begin(), board.end(), IsDead), board.end());
        }

     private:
        // Identifica o creatura distrusa pentru operatia de eliminare din vector.
        static bool IsDead(const Card& card) { return card.GetHealth() <= 0; }

        std::vector<Card> hand;
        std::vector<Card> deck;
        std::vector<Card> board;
        Hero hero;
        int mana = 1;
        int maxMana = 1;
        int turnsStarted = 0;
    };
}

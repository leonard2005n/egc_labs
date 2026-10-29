#pragma once

#include <random>

#include "lab_m1/tema_1/player.h"

namespace m1
{
    // Reguli de meci independente de randare; indicele 0 este omul, iar 1 calculatorul.
    class CardGame
    {
     public:
        // Pregateste automat un meci nou.
        CardGame();
        void Reset();
        bool EndTurn();
        bool DrawTurnCard();
        bool PlayCard(std::size_t index);
        bool Attack(std::size_t attackerIndex, int targetIndex);
        bool CanAttack(std::size_t attackerIndex, int targetIndex) const;
        bool ChooseSilenceTarget(std::size_t targetIndex);
        bool NeedsSilenceTarget() const { return silencePending; }

        // Ofera starea unui jucator doar pentru citire.
        const Player& GetPlayer(int index) const { return players[index]; }
        // Intoarce jucatorul care poate actiona acum.
        int GetActivePlayer() const { return activePlayer; }
        // Indica extragerea obligatorie de la inceputul turei.
        bool NeedsTurnDraw() const { return turnDrawPending; }
        // Intoarce -1 in timpul meciului sau indicele castigatorului.
        int GetWinner() const { return winner; }
        // Blocheaza orice actiune dupa moartea unui erou.
        bool IsFinished() const { return winner >= 0; }

     private:
        std::mt19937 randomGenerator{std::random_device{}()};
        Player players[2];
        int activePlayer = 0;
        int winner = -1;
        bool turnDrawPending = false;
        bool silencePending = false;
    };
}

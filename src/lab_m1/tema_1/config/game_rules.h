#pragma once

#include <cstddef>

namespace m1
{
    namespace game_rules
    {
        // Configuratia meciului 1v1; valorile de echilibrare se modifica aici.
        constexpr int PlayerCount = 2;
        constexpr int HumanPlayer = 0;
        constexpr int ComputerPlayerIndex = 1;

        constexpr int InitialHeroHealth = 30;
        constexpr int HeroAttack = 0;
        constexpr int InitialMana = 1;
        constexpr int ManaPerTurn = 1;
        constexpr int MaxMana = 10;
        constexpr std::size_t InitialHandSize = 5;
        constexpr std::size_t CopiesPerCard = 5;
        constexpr float TurnDuration = 30;

        // Indicii negativi nu reprezinta carti: -1 este eroul, -2 lipsa unei tinte.
        constexpr int HeroTarget = -1;
        constexpr int NoTarget = -2;
        constexpr int NoWinner = -1;

        inline int OpponentOf(int owner)
        {
            return PlayerCount - 1 - owner;
        }
    }
}

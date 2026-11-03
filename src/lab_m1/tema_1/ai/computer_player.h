#pragma once

#include "lab_m1/tema_1/model/card_game.h"

namespace m1
{
    struct ComputerAction
    {
        enum class Type
        {
            PlayCard,
            Attack,
            EndTurn
        };

        ComputerAction(Type type, std::size_t cardIndex = 0, int targetIndex = game_rules::HeroTarget)
            : type(type), cardIndex(cardIndex), targetIndex(targetIndex) {}

        Type type;
        std::size_t cardIndex;
        int targetIndex;
    };

    // Alege actiunile calculatorului fara sa modifice meciul sau animatiile.
    class ComputerPlayer
    {
     public:
        static ComputerAction ChooseAction(const CardGame& game);
        static std::size_t ChooseSilenceTarget(const CardGame& game);
    };
}

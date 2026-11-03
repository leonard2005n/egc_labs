#pragma once

#include <vector>

#include "lab_m1/tema_1/model/card.h"

namespace m1
{
    struct CardDefinition
    {
        CardType type;
        int attack;
        int health;
        int manaCost;
        CardEffect effects;
        std::size_t copies;
    };

    struct EffectDefinition
    {
        CardEffect effect;
        const char* label;
        int silencePriority;
    };

    class CardCatalog
    {
     public:
        static const std::vector<CardDefinition>& Cards();
        static const std::vector<EffectDefinition>& Effects();
        static std::vector<Card> CreateDeck(const std::vector<CardDefinition>& definitions = Cards());
        static int SilencePriority(const Card& card);
    };
}

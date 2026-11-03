#include "lab_m1/tema_1/model/card_catalog.h"
#include "lab_m1/tema_1/config/game_rules.h"

#include <algorithm>

using namespace m1;
using namespace m1::game_rules;

const std::vector<CardDefinition>& CardCatalog::Cards()
{
    // Date de echilibrare, separate de algoritmul de construire a pachetului.
    static const std::vector<CardDefinition> definitions = {
        {CardType::Triangle, 1, 2, 1, CardEffect::None, CopiesPerCard},
        {CardType::Diamond,  2, 3, 2, CardEffect::Charge, CopiesPerCard},
        {CardType::Square,   3, 4, 3, CardEffect::Tank, CopiesPerCard},
        {CardType::Triangle, 4, 4, 4, CardEffect::Lifesteal, CopiesPerCard},
        {CardType::Diamond,  5, 5, 5, CardEffect::Silence, CopiesPerCard},
        {CardType::Square,   6, 6, 6, CardEffect::Tank | CardEffect::Lifesteal, CopiesPerCard}
    };

    return definitions;
}

const std::vector<EffectDefinition>& CardCatalog::Effects()
{
    // Ordinea controleaza afisarea; ponderile pastreaza prioritatea AI-ului existent.
    static const std::vector<EffectDefinition> definitions = {
        {CardEffect::Lifesteal, "LIFESTEAL", 50},
        {CardEffect::Charge,   "CHARGE",   20},
        {CardEffect::Tank,     "TANK",    100},
        {CardEffect::Silence,  "SILENCE",   0}
    };

    return definitions;
}

std::vector<Card> CardCatalog::CreateDeck(const std::vector<CardDefinition>& definitions)
{
    std::size_t totalCards = 0;
    std::size_t maxCopies = 0;

    for (const CardDefinition& definition : definitions) {
        totalCards += definition.copies;
        maxCopies = std::max(maxCopies, definition.copies);
    }

    std::vector<Card> cards;
    cards.reserve(totalCards);

    // Intercalarea pastreaza ordinea initiala folosita inainte de shuffle.
    // Numarul de modele si numarul exemplarelor pot varia independent.
    for (std::size_t copy = 0; copy < maxCopies; ++copy) {
        for (const CardDefinition& definition : definitions) {
            if (copy < definition.copies) {
                cards.emplace_back(definition.type, definition.attack, definition.health,
                                   definition.manaCost, definition.effects);
            }
        }
    }

    return cards;
}

int CardCatalog::SilencePriority(const Card& card)
{
    int score = 0;

    for (const EffectDefinition& definition : Effects()) {
        if (card.HasEffect(definition.effect)) {
            score += definition.silencePriority;
        }
    }

    return score;
}

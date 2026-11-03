#pragma once

#include <memory>
#include <string>

#include "lab_m1/tema_1/model/card_game.h"
#include "lab_m1/tema_1/config/animation_config.h"
#include "lab_m1/tema_1/presentation/row_animation.h"

namespace m1
{
    struct AttackAnimation
    {
        std::unique_ptr<Card> card;
        std::unique_ptr<Card> defender;
        int owner = game_rules::HumanPlayer;
        std::size_t index = 0;
        int target = game_rules::HeroTarget;
        float elapsed = 0;
        float initialScale = 1;
        bool impacted = false;
        bool survived = true;
        bool defenderSurvived = true;
        float defenderScale = 1;
    };

    // Starea vizuala este modificata de controller si citita de renderer.
    // Regulile meciului raman exclusiv in CardGame.
    class GamePresentation
    {
     public:
        enum class CardMotion
        {
            Idle,
            Dragging,
            Returning,
            Drawing
        };

        void Reset(const CardGame& game);
        float UpdateOpeningSpinner(float deltaTimeSeconds);
        void UpdateCardScales(const CardGame& game, float deltaTimeSeconds);
        void UpdateCardReturn(float deltaTimeSeconds);
        bool IsLayoutAnimating() const;
        bool CanHumanInteract(const CardGame& game) const;
        void ShowMessage(const std::string& text);
        glm::vec2 GetHoveredCardPosition() const;
        glm::vec2 GetHandVisualPosition(std::size_t index) const;
        int FindHandCard(const CardGame& game, const glm::vec2& mouse) const;
        int FindBoardCard(int ownerIndex, const glm::vec2& mouse) const;

        RowAnimation handAnimations[game_rules::PlayerCount];
        RowAnimation boardAnimations[game_rules::PlayerCount];
        int drawingPlayer = game_rules::HumanPlayer;
        CardMotion cardMotion = CardMotion::Idle;

        // Indicii de selectie si hover se refera la randurile curente de carti.
        int selectedCardIndex = -1;
        int hoveredCardIndex = -1;
        int hoveredBoardOwner = -1;
        int hoveredBoardIndex = -1;
        int selectedAttackerIndex = -1;

        // Cartea trasa are o pozitie independenta pana la plasare sau revenire.
        glm::vec2 cardPosition = glm::vec2(0);
        glm::vec2 initialCardPosition = glm::vec2(0);
        glm::vec2 grabOffset = glm::vec2(0);
        glm::vec2 returnStartPosition = glm::vec2(0);
        float returnElapsed = 0;

        float computerDelay = 0;
        float turnTimeRemaining = game_rules::TurnDuration;
        bool openingSpinnerActive = true;
        float openingSpinnerElapsed = 0;

        std::string message;
        float messageTime = 0;

        glm::vec2 attackStart = glm::vec2(0);
        glm::vec2 attackEnd = glm::vec2(0);
        float attackEffectTime = 0;
        AttackAnimation attackAnimation;
    };
}

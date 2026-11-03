#include "lab_m1/tema_1/presentation/game_presentation.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

#include <algorithm>
#include <cmath>

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;

void GamePresentation::Reset(const CardGame& game)
{
    // Tura si secventa de inceput de meci.
    openingSpinnerActive = true;
    openingSpinnerElapsed = 0;
    turnTimeRemaining = TurnDuration;
    computerDelay = 0;

    // Selectiile nu se transfera de la un meci la altul.
    cardMotion = CardMotion::Idle;
    selectedCardIndex = hoveredCardIndex = selectedAttackerIndex = -1;
    hoveredBoardOwner = hoveredBoardIndex = -1;

    // Oprim animatiile si mesajele, apoi reconstruim randurile vizuale.
    returnElapsed = messageTime = attackEffectTime = 0;
    message.clear();
    attackAnimation = AttackAnimation();

    for (int owner = 0; owner < PlayerCount; ++owner) {
        handAnimations[owner] = RowAnimation();
        boardAnimations[owner] = RowAnimation();
        const auto& hand = game.GetPlayer(owner).GetHand();

        for (std::size_t i = 0; i < hand.size(); ++i) {
            handAnimations[owner].positions.push_back(GameLayout::GetHandCardPosition(i, hand.size(), owner));
            handAnimations[owner].scales.push_back(1);
        }
    }
}

float GamePresentation::UpdateOpeningSpinner(float deltaTimeSeconds)
{
    if (!openingSpinnerActive) {
        return deltaTimeSeconds;
    }

    const float duration = SpinnerSpinDuration + SpinnerRevealDuration;
    const float nextElapsed = openingSpinnerElapsed + deltaTimeSeconds;
    openingSpinnerElapsed = std::min(duration, nextElapsed);

    if (openingSpinnerElapsed < duration) {
        return 0;
    }

    openingSpinnerActive = false;
    computerDelay = ComputerActionDelay;

    return std::max(0.0f, nextElapsed - duration);
}

void GamePresentation::UpdateCardScales(const CardGame& game, float deltaTimeSeconds)
{
    const float blend = 1 - std::exp(-ScaleResponse * deltaTimeSeconds);

    for (int owner = 0; owner < PlayerCount; ++owner) {
        for (std::size_t i = 0; i < handAnimations[owner].scales.size(); ++i) {
            float target = owner == HumanPlayer && static_cast<int>(i) == hoveredCardIndex ? HoverScale : 1;

            if (owner == HumanPlayer && static_cast<int>(i) == selectedCardIndex && cardMotion == CardMotion::Dragging) {
                target = SelectedScale;
            }

            float& scale = handAnimations[owner].scales[i];
            scale += (target - scale) * blend;
        }

        for (std::size_t i = 0; i < boardAnimations[owner].scales.size(); ++i) {
            float target = owner == hoveredBoardOwner && static_cast<int>(i) == hoveredBoardIndex ? HoverScale : 1;

            if ((owner == HumanPlayer && static_cast<int>(i) == selectedAttackerIndex) ||
                (game.NeedsSilenceTarget() && owner == game.GetActivePlayer() &&
                 i + 1 == boardAnimations[owner].scales.size())) {
                target = SelectedScale;
            }

            float& scale = boardAnimations[owner].scales[i];
            scale += (target - scale) * blend;
        }
    }
}

void GamePresentation::UpdateCardReturn(float deltaTimeSeconds)
{
    if (cardMotion != CardMotion::Returning) {
        return;
    }

    returnElapsed += deltaTimeSeconds;
    const float progress = std::min(returnElapsed / ReturnDuration, 1.0f);
    cardPosition = returnStartPosition + progress * (initialCardPosition - returnStartPosition);

    if (progress >= 1) {
        selectedCardIndex = -1;
        cardMotion = CardMotion::Idle;
    }
}

bool GamePresentation::IsLayoutAnimating() const
{
    for (int owner = 0; owner < PlayerCount; ++owner) {
        if (handAnimations[owner].active || boardAnimations[owner].active) {
            return true;
        }
    }

    return false;
}

glm::vec2 GamePresentation::GetHoveredCardPosition() const
{
    return GetHandVisualPosition(hoveredCardIndex);
}

glm::vec2 GamePresentation::GetHandVisualPosition(std::size_t index) const
{
    const float lift = std::min(1.0f, (handAnimations[HumanPlayer].scales[index] - 1) / (HoverScale - 1));

    return handAnimations[HumanPlayer].positions[index] + glm::vec2(0, HoverLift * lift);
}

int GamePresentation::FindHandCard(const CardGame& game, const glm::vec2& mouse) const
{
    if (hoveredCardIndex >= 0 && GameLayout::IsInsideScaledCard(mouse, GetHoveredCardPosition(),
                                                            handAnimations[HumanPlayer].scales[hoveredCardIndex])) {
        return hoveredCardIndex;
    }

    for (int i = static_cast<int>(game.GetPlayer(HumanPlayer).GetHand().size()) - 1; i >= 0; --i) {
        if (GameLayout::IsInsideScaledCard(mouse, GetHandVisualPosition(i), handAnimations[HumanPlayer].scales[i])) {
            return i;
        }
    }

    return -1;
}

int GamePresentation::FindBoardCard(int ownerIndex, const glm::vec2& mouse) const
{
    const auto& positions = boardAnimations[ownerIndex].positions;
    const int raised = ownerIndex == HumanPlayer && selectedAttackerIndex >= 0 ? selectedAttackerIndex :
        (ownerIndex == hoveredBoardOwner ? hoveredBoardIndex : -1);

    if (raised >= 0 && static_cast<std::size_t>(raised) < positions.size() &&
        GameLayout::IsInsideScaledCard(mouse, positions[raised], boardAnimations[ownerIndex].scales[raised])) {
        return raised;
    }

    for (int i = static_cast<int>(positions.size()) - 1; i >= 0; --i) {
        if (GameLayout::IsInsideScaledCard(mouse, positions[i], boardAnimations[ownerIndex].scales[i])) {
            return i;
        }
    }

    return -1;
}

bool GamePresentation::CanHumanInteract(const CardGame& game) const
{
    return !openingSpinnerActive && !game.IsFinished() && game.GetActivePlayer() == HumanPlayer && !game.NeedsTurnDraw() &&
           cardMotion == CardMotion::Idle && !IsLayoutAnimating() && attackEffectTime <= 0 && !attackAnimation.card;
}

void GamePresentation::ShowMessage(const std::string& text)
{
    message = text;
    messageTime = MessageDuration;
}

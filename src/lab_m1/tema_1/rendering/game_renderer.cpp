#include "lab_m1/tema_1/rendering/game_renderer.h"
#include "lab_m1/tema_1/presentation/game_layout.h"
#include "lab_m1/tema_1/rendering/game_mesh_factory.h"
#include "core/managers/resource_path.h"
#include "utils/text_utils.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;
using namespace m1::game_style;

GameRenderer::GameRenderer(const CardGame& game, const GamePresentation& view, MeshDrawer drawMesh)
    : game(game), view(view), drawMesh(std::move(drawMesh))
{
}

void GameRenderer::Init(const std::string& resourceDirectory)
{
    GameMeshFactory::Create([this](Mesh* mesh) {
        meshes[mesh->GetMeshID()].reset(mesh);
    });
    textRenderer.reset(new gfxc::TextRenderer(resourceDirectory, SceneWidth, SceneHeight));
    textRenderer->Load(PATH_JOIN(resourceDirectory, RESOURCE_PATH::FONTS, FontFile), FontPixelSize);
    // Seteaza explicit proiectia textului pe shaderul lui, folosind coordonate de sus in jos.
    textRenderer->m_textShader->Use();
    const glm::mat4 projection = glm::ortho(0.0f, SceneWidth, SceneHeight, 0.0f);
    glUniformMatrix4fv(glGetUniformLocation(textRenderer->m_textShader->program, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));
}

void GameRenderer::DrawFrame(const glm::vec2* cursorPosition)
{
    const glm::ivec2 resolution = GameLayout::GetFramebufferResolution();

    if (resolution.x <= 0 || resolution.y <= 0) {
        return;
    }

    const GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    DrawBoard();
    DrawHUD();

    for (int owner = 0; owner < PlayerCount; ++owner) {
        DrawHand(owner);
        DrawDeck(owner, DeckPositions[owner].x, DeckPositions[owner].y);
        DrawPlayedCards(owner);
    }

    if (view.hoveredCardIndex >= 0) {
        const glm::vec2 position = view.GetHoveredCardPosition();
        DrawCardVisual(game.GetPlayer(HumanPlayer).GetHand()[view.hoveredCardIndex], position, true,
                       view.handAnimations[HumanPlayer].scales[view.hoveredCardIndex],
                       HandCardOutline(HumanPlayer, view.hoveredCardIndex));
    }

    if (view.selectedCardIndex >= 0) {
        DrawCardVisual(game.GetPlayer(HumanPlayer).GetHand()[view.selectedCardIndex], view.cardPosition, true,
                       view.handAnimations[HumanPlayer].scales[view.selectedCardIndex], &AttackColor);
    }

    if (view.cardMotion == GamePresentation::CardMotion::Drawing) {
        const glm::vec2 position = view.handAnimations[view.drawingPlayer].positions.back();
        DrawCard(game.GetPlayer(view.drawingPlayer).GetHand().back(), position.x, position.y,
                 view.drawingPlayer == HumanPlayer);
    }

    DrawAttackAnimation();
    DrawAttackEffect();
    DrawSelectionArrow(cursorPosition);

    if (game.IsFinished() && !view.attackAnimation.card) {
        DrawGameOver();
    }

    if (view.openingSpinnerActive) {
        DrawOpeningSpinner();
    }

    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

void GameRenderer::DrawBoard()
{
    DrawArtwork(BoardArt(), glm::vec4(0, 0, SceneWidth, SceneHeight));
}

void GameRenderer::DrawHand(int ownerIndex)
{
    const auto& hand = game.GetPlayer(ownerIndex).GetHand();

    for (std::size_t i = 0; i < hand.size(); ++i) {
        if (ownerIndex == HumanPlayer &&
            (static_cast<int>(i) == view.selectedCardIndex || static_cast<int>(i) == view.hoveredCardIndex)) {
            continue;
        }

        if (view.cardMotion == GamePresentation::CardMotion::Drawing &&
            ownerIndex == view.drawingPlayer && i + 1 == hand.size()) {
            continue;
        }

        const glm::vec2 position = ownerIndex == HumanPlayer ?
            view.GetHandVisualPosition(i) : view.handAnimations[ownerIndex].positions[i];
        DrawCardVisual(hand[i], position, ownerIndex == HumanPlayer,
                       view.handAnimations[ownerIndex].scales[i], HandCardOutline(ownerIndex, i));
    }
}

const glm::vec3* GameRenderer::HandCardOutline(int ownerIndex, std::size_t index) const
{
    // Evidentiem si cartile care pot fi plasate, nu doar creaturile care pot ataca.
    const bool playable = ownerIndex == HumanPlayer && view.CanHumanInteract(game) &&
                          !game.NeedsSilenceTarget() && game.GetPlayer(ownerIndex).CanPlayCard(index);

    return playable ? &ReadyColor : nullptr;
}

void GameRenderer::DrawPlayedCards(int ownerIndex)
{
    const auto& cards = game.GetPlayer(ownerIndex).GetBoard();
    auto draw = [this, ownerIndex, &cards](std::size_t i) {
        if (view.attackAnimation.card && ownerIndex == view.attackAnimation.owner && i == view.attackAnimation.index &&
            (!view.attackAnimation.impacted || view.attackAnimation.survived)) {
            return;
        }

        if (view.attackAnimation.defender && ownerIndex == OpponentOf(view.attackAnimation.owner) &&
            static_cast<int>(i) == view.attackAnimation.target &&
            (!view.attackAnimation.impacted || view.attackAnimation.defenderSurvived)) {
            return;
        }

        const glm::vec3* outline = nullptr;

        if (game.NeedsSilenceTarget() && ownerIndex == OpponentOf(game.GetActivePlayer())) {
            outline = &ManaColor;
        } else if (game.NeedsSilenceTarget() && ownerIndex == game.GetActivePlayer() && i + 1 == cards.size()) {
            outline = &ManaColor;
        } else if (ownerIndex == HumanPlayer && static_cast<int>(i) == view.selectedAttackerIndex) {
            outline = &AttackColor;
        } else if (!game.IsFinished() && ownerIndex == game.GetActivePlayer() && cards[i].CanAttack()) {
            outline = &ReadyColor;
        }

        DrawCardVisual(cards[i], view.boardAnimations[ownerIndex].positions[i], true,
                       view.boardAnimations[ownerIndex].scales[i], outline);
    };
    int raised = ownerIndex == view.hoveredBoardOwner ? view.hoveredBoardIndex : -1;

    if (ownerIndex == HumanPlayer && view.selectedAttackerIndex >= 0) {
        raised = view.selectedAttackerIndex;
    }

    for (std::size_t i = 0; i < cards.size(); ++i) {
        if (static_cast<int>(i) != raised) {
            draw(i);
        }
    }

    if (raised >= 0 && static_cast<std::size_t>(raised) < cards.size()) {
        draw(raised);
    }
}

void GameRenderer::DrawHUD()
{
    for (int owner = 0; owner < PlayerCount; ++owner) {
        DrawHero(owner, HeroBounds[owner]);
    }

    for (int owner = 0; owner < PlayerCount; ++owner) {
        DrawMana(owner);
    }

    const bool enabled = view.CanHumanInteract(game) && !game.NeedsSilenceTarget();
    DrawRect(GameLayout::Inset(EndTurnBounds, -ButtonBorder), Border);
    DrawRect(EndTurnBounds, enabled ? ButtonEnabled : ButtonDisabled);
    DrawCenteredText(view.openingSpinnerActive ? "SE ALEGE..." :
                     (game.NeedsSilenceTarget() ? "ALEGE TINTA" :
                     (game.GetActivePlayer() == HumanPlayer ? "SFARSIT TURA" : "TURA ADVERSAR")),
                     GameLayout::Center(EndTurnBounds), ButtonTextSize, Paper);
    DrawTurnTimer();
    const std::string turn = view.openingSpinnerActive ? "CINE INCEPE?" :
        (game.GetActivePlayer() == HumanPlayer ? "TURA TA" : "TURA CALCULATORULUI");
    DrawText(view.openingSpinnerActive ? turn : turn + " | Runda " +
             std::to_string(game.GetPlayer(game.GetActivePlayer()).GetTurnCount()),
             HudLeft, HudTurnY, TurnTextSize, Paper);

    for (const TextLabel& hint : HudHints()) {
        DrawText(hint.text, hint.position.x, hint.position.y, hint.size, hint.color);
    }

    if (game.NeedsSilenceTarget() && game.GetActivePlayer() == HumanPlayer) {
        DrawText("SILENCE: click pe cartea adversa careia ii anulezi efectele.", HudLeft, HudMessageY, BodyTextSize, ManaColor);
    } else if (view.messageTime > 0) {
        DrawText(view.message, HudLeft, HudMessageY, BodyTextSize, AttackColor);
    }

    DrawText("Carti adverse: " + std::to_string(game.GetPlayer(ComputerPlayerIndex).GetHand().size()),
             HudLeft, SceneHeight - OpponentHandTopInset, BodyTextSize, Paper);

    for (int owner = 0; owner < PlayerCount; ++owner) {
        const glm::vec2 position = DeckPositions[owner] + DeckLabelOffsets[owner];
        DrawText("PACHET: " + std::to_string(game.GetPlayer(owner).GetDeck().size()),
                 position.x, position.y, BodyTextSize, Paper);
    }
}

void GameRenderer::DrawTurnTimer()
{
    const glm::vec2 center = TimerCenter;
    const float radius = TimerRadius;

    DrawCircle(center, radius + TimerBorder, TimerBorderColor);
    DrawCircle(center, radius, TimerTrack);
    DrawCircle(center, radius * TimerInnerRatio, TimerInside);

    const float remaining = std::max(0.0f, std::min(1.0f, view.turnTimeRemaining / TurnDuration));
    const float step = 2 * Pi / TimerSegments;
    const float segmentsLeft = remaining * TimerSegments;
    const glm::vec3 color = view.turnTimeRemaining <= TimerWarningSeconds ? HealthColor : AttackColor;

    for (int i = 0; i < static_cast<int>(std::ceil(segmentsLeft)); ++i) {
        const float portion = std::min(1.0f, segmentsLeft - i);
        const float angle = -((i + portion / 2) * step);
        const float c = std::cos(angle), sn = std::sin(angle);
        const glm::mat3 model(radius * portion * c, radius * portion * sn, 0,
                              -radius * sn, radius * c, 0, center.x, center.y, 1);
        drawMesh(meshes.at("tema1_timer_segment").get(), model, color);
    }

    DrawCenteredText(std::to_string(static_cast<int>(std::ceil(view.turnTimeRemaining))),
                     center + TimerNumberOffset, TimerNumberSize, Paper);
    DrawCenteredText("SEC", center + TimerUnitOffset, TimerUnitSize, Border);
}

void GameRenderer::DrawHero(int ownerIndex, const glm::vec4& bounds)
{
    const Hero& hero = game.GetPlayer(ownerIndex).GetHero();
    const Artwork& artwork = HeroArt();

    DrawArtwork(artwork, bounds);
    DrawCenteredText("ATAC " + std::to_string(hero.GetAttack()),
                     GameLayout::PlacePoint(HeroAttackOffset, artwork.designSize, bounds), HeroAttackTextSize, Paper);
    DrawStatBadge(hero.GetHealth(), GameLayout::PlacePoint(HeroBadgeOffset, artwork.designSize, bounds),
                  HeroBadgeRadius, HealthColor, "tema1_heart");
}

void GameRenderer::DrawMana(int ownerIndex)
{
    const Player& owner = game.GetPlayer(ownerIndex);
    const glm::vec2 origin = ManaOrigins[ownerIndex];

    for (int i = 0; i < owner.GetMaxMana(); ++i) {
        const glm::vec2 position = origin + glm::vec2(i * ManaGemSpacing, 0);
        DrawManaCrystal(glm::vec4(position, ManaGemSize), i < owner.GetMana() ? ManaColor : EmptyMana);
    }

    DrawText("MANA " + std::to_string(owner.GetMana()) + " / " + std::to_string(owner.GetMaxMana()),
             origin.x, origin.y + ManaLabelOffset, ManaTextSize, ManaColor);
}

void GameRenderer::DrawGameOver()
{
    const glm::vec2 origin(GameOverBounds);
    const glm::vec2 resultPosition = origin + game_over::ResultOffset;
    const glm::vec2 detailPosition = origin + game_over::DetailOffset;
    const glm::vec2 restartPosition = glm::vec2(RestartBounds) + game_over::RestartTextOffset;
    const bool won = game.GetWinner() == game_rules::HumanPlayer;

    DrawArtwork(GameOverArt(), GameOverBounds);
    DrawText(won ? "VICTORIE!" : "INFRANGERE", resultPosition.x, resultPosition.y, game_over::ResultSize, Paper);
    DrawText(won ? "Eroul advers a cazut." : "Eroul tau a cazut.",
             detailPosition.x, detailPosition.y, game_over::DetailSize, Paper);

    DrawRect(RestartBounds, RestartButton);
    DrawText("JOC NOU (R)", restartPosition.x, restartPosition.y, game_over::RestartTextSize, Paper);
}

#include "lab_m1/tema_1/rendering/game_renderer.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;
using namespace m1::game_style;

void GameRenderer::DrawOpeningSpinner()
{
    const glm::vec2 origin(SpinnerBounds);
    const glm::vec2 center = origin + spinner::CenterOffset;
    const glm::vec2 title = origin + spinner::TitleOffset;
    const float radius = spinner::Radius;

    DrawArtwork(SpinnerArt(), SpinnerBounds);
    DrawCenteredText("CINE INCEPE MECIUL?", title, spinner::TitleSize, Paper);
    DrawCircle(center, radius + spinner::BorderWidth, Border);

    const char* sectorMeshes[] = {"tema1_spinner_human", "tema1_spinner_ai"};
    const char* sectorLabels[] = {"TU", "AI"};

    for (int owner = 0; owner < PlayerCount; ++owner) {
        DrawShape(sectorMeshes[owner], center.x, center.y, radius, radius, spinner::SectorColors[owner]);
    }

    DrawRectangle(center.x - radius, center.y - spinner::DividerWidth / 2,
                  2 * radius, spinner::DividerWidth, Border);

    for (int owner = 0; owner < PlayerCount; ++owner) {
        DrawCenteredText(sectorLabels[owner], center + spinner::SectorLabelOffsets[owner],
                         spinner::SectorLabelSize, Paper);
    }

    const float progress = std::min(1.0f, view.openingSpinnerElapsed / SpinnerSpinDuration);
    const float ease = 1 - std::pow(1 - progress, SpinnerEasePower);
    const float travel = 2 * SpinnerTurns * Pi + game.GetActivePlayer() * Pi;
    const float angle = Pi / 2 - travel * ease;
    const glm::vec2 direction(std::cos(angle), std::sin(angle));
    const glm::vec2 normal(-direction.y, direction.x);
    const glm::vec2 needleStart = center - direction * spinner::NeedleBack - normal * (spinner::NeedleWidth / 2);
    const glm::mat3 needle(spinner::NeedleLength * direction.x, spinner::NeedleLength * direction.y, 0,
                           spinner::NeedleWidth * normal.x, spinner::NeedleWidth * normal.y, 0, needleStart.x, needleStart.y, 1);
    drawMesh(meshes.at("tema1_rectangle").get(), needle, AttackColor);
    const glm::vec2 tip = center + direction * spinner::TipDistance;
    const glm::mat3 head(spinner::HeadSize.x * direction.x, spinner::HeadSize.x * direction.y, 0,
                         spinner::HeadSize.y * normal.x, spinner::HeadSize.y * normal.y, 0, tip.x, tip.y, 1);
    drawMesh(meshes.at("tema1_arrowhead").get(), head, Paper);
    DrawCircle(center, spinner::HubOuterRadius, Border);
    DrawCircle(center, spinner::HubInnerRadius, AttackColor);

    const bool revealed = view.openingSpinnerElapsed >= SpinnerSpinDuration;
    DrawCenteredText(revealed ? (game.GetActivePlayer() == HumanPlayer ? "TU INCEPI!" : "AI-UL INCEPE!") : "SE ALEGE PRIMUL JUCATOR...",
                     origin + spinner::ResultOffset, revealed ? spinner::ResultSize : spinner::PendingSize, revealed ? AttackColor : Paper);
    DrawCenteredText(revealed ? "Meciul incepe imediat" : "Sanse egale: 50% TU / 50% AI",
                     origin + spinner::HintOffset, BodyTextSize, Border);
}

void GameRenderer::DrawAttackAnimation()
{
    const AttackAnimation& animation = view.attackAnimation;

    if (!animation.card) {
        return;
    }

    const float elapsed = animation.elapsed;
    const glm::vec2 direction = glm::normalize(view.attackEnd - view.attackStart);
    const glm::vec2 contact = view.attackEnd - direction * AttackContactInset;
    const float afterImpact = std::max(0.0f, elapsed - AttackImpactTime);
    const float shakeProgress = std::min(1.0f, afterImpact / (AttackDuration - AttackImpactTime));
    const float rotation = ShakeAngle * std::sin(2 * ShakeCycles * Pi * shakeProgress) * (1 - ShakeDamping * shakeProgress);
    const float deathScale = 1 - GameLayout::SmoothStep((elapsed - (AttackDuration - DeathShrinkDuration)) / DeathShrinkDuration);

    if (animation.defender) {
        glm::vec2 targetCenter = view.attackEnd;

        if (animation.impacted && animation.defenderSurvived) {
            targetCenter = GameLayout::CardCenter(view.boardAnimations[OpponentOf(animation.owner)].positions[animation.target]);
        }

        const float targetScale = animation.defenderScale *
            (animation.impacted && !animation.defenderSurvived ? deathScale : 1);
        DrawCardVisual(*animation.defender, targetCenter - glm::vec2(CardWidth, CardHeight) / 2.0f,
                       true, targetScale, nullptr, -rotation);
    }

    glm::vec2 center;
    float scale;

    if (elapsed < AttackWindupDuration) {
        const float t = GameLayout::SmoothStep(elapsed / AttackWindupDuration);
        center = view.attackStart - direction * (AttackPullback * t);
        scale = animation.initialScale + (SelectedScale - animation.initialScale) * t;
    } else if (elapsed < AttackImpactTime) {
        const float t = GameLayout::SmoothStep((elapsed - AttackWindupDuration) / (AttackImpactTime - AttackWindupDuration));
        center = glm::mix(view.attackStart - direction * AttackPullback, contact, t);
        scale = SelectedScale;
    } else if (animation.survived) {
        const float t = GameLayout::SmoothStep(shakeProgress);
        const glm::vec2 home = GameLayout::CardCenter(view.boardAnimations[animation.owner].positions[animation.index]);
        center = glm::mix(contact, home, t);
        scale = SelectedScale + (view.boardAnimations[animation.owner].scales[animation.index] - SelectedScale) * t;
    } else {
        center = contact;
        scale = SelectedScale * deathScale;
    }

    DrawCardVisual(*animation.card, center - glm::vec2(CardWidth, CardHeight) / 2.0f,
                   true, scale, &AttackColor, rotation);
}

void GameRenderer::DrawAttackEffect()
{
    if (view.attackEffectTime <= 0) {
        return;
    }

    const float progress = 1 - view.attackEffectTime / ImpactDuration;
    const float distance = SparkStartRadius + SparkTravel * progress;
    const float length = SparkLength * (1 - progress);

    for (int i = 0; i < SparkCount; ++i) {
        const float angle = 2 * Pi * i / SparkCount;
        const float c = std::cos(angle), sn = std::sin(angle);
        const glm::vec2 position = view.attackEnd + glm::vec2(c, sn) * distance;
        const glm::mat3 model(length * c, length * sn, 0, -SparkWidth * sn, SparkWidth * c, 0,
                              position.x + (SparkWidth / 2) * sn, position.y - (SparkWidth / 2) * c, 1);
        drawMesh(meshes.at("tema1_rectangle").get(), model, i % 2 ? Paper : AttackColor);
    }
}

void GameRenderer::DrawSelectionArrow(const glm::vec2* cursorPosition)
{
    const bool silence = game.NeedsSilenceTarget();

    if ((!silence && view.selectedAttackerIndex < 0) || !view.CanHumanInteract(game) ||
        !cursorPosition) {
        return;
    }

    const glm::vec2 target = *cursorPosition;
    const std::size_t sourceIndex = silence ? view.boardAnimations[HumanPlayer].positions.size() - 1 : view.selectedAttackerIndex;
    const glm::vec2 source = GameLayout::CardCenter(view.boardAnimations[HumanPlayer].positions[sourceIndex]);
    const glm::vec2 delta = target - source;
    const float distance = glm::length(delta);

    if (distance < arrow::MinDistance) {
        return;
    }

    const glm::vec2 direction = delta / distance;
    const glm::vec2 normal(-direction.y, direction.x);
    const float inset = std::min(arrow::MaxInset, distance * arrow::InsetRatio);
    const glm::vec2 start = source + inset * direction;
    const float headLength = std::min(arrow::MaxHeadLength, (distance - inset) * arrow::HeadRatio);
    const int targetIndex = view.FindBoardCard(ComputerPlayerIndex, target);
    const bool onHero = GameLayout::IsInside(target, HeroBounds[ComputerPlayerIndex]);
    const bool validTarget = silence ? targetIndex >= 0 :
        ((targetIndex >= 0 && game.CanAttack(view.selectedAttackerIndex, targetIndex)) ||
         (onHero && game.CanAttack(view.selectedAttackerIndex, HeroTarget)));
    const glm::vec3 color = validTarget ? ReadyColor :
        ((targetIndex >= 0 || onHero) ? HealthColor : (silence ? ManaColor : AttackColor));
    // Rama inchisa pastreaza sageata vizibila si peste carti deschise la culoare.
    for (int border = 1; border >= 0; --border) {
        const float thickness = border ? arrow::BorderWidth : arrow::Width;
        const float shaftLength = distance - inset - headLength + arrow::ShaftOverlap;
        const glm::vec2 origin = start - normal * (thickness / 2);
        const glm::mat3 shaft(shaftLength * direction.x, shaftLength * direction.y, 0,
                              thickness * normal.x, thickness * normal.y, 0, origin.x, origin.y, 1);
        const glm::vec3 tint = border ? SymbolShadow : color;
        drawMesh(meshes.at("tema1_rectangle").get(), shaft, tint);
        const float width = headLength + (border ? arrow::HeadBorder : 0);
        const float height = border ? arrow::BorderHeadHeight : arrow::HeadHeight;
        const glm::mat3 head(width * direction.x, width * direction.y, 0,
                             height * normal.x, height * normal.y, 0, target.x, target.y, 1);
        drawMesh(meshes.at("tema1_arrowhead").get(), head, tint);
    }
}

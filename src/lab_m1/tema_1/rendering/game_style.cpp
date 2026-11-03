#include "lab_m1/tema_1/rendering/game_style.h"
#include "lab_m1/tema_1/presentation/game_layout.h"
#include "lab_m1/tema_1/rendering/mesh_data.h"

#include <stdexcept>

using namespace m1;
using namespace m1::game_view;

glm::vec2 CardBadge::Center(const glm::vec4& cardBounds) const
{
    return glm::vec2(cardBounds) + anchor * glm::vec2(cardBounds.z, cardBounds.w) + offset;
}

bool CardBadge::Contains(const glm::vec2& point, const glm::vec4& cardBounds) const
{
    const glm::vec2 center = Center(cardBounds);

    if (hitShape == BadgeHitShape::Circle) {
        return glm::length(point - center) <= hitRadius;
    }

    // Transformarea inversa foloseste inclusiv bordura desenata in DrawStatBadge.
    const float diameter = 2 * (radius + game_style::BadgeBorder);
    const glm::vec2 local = (point - center) / diameter + glm::vec2(0.5f);
    const auto& outline = ManaCrystalOutline();
    constexpr float edgeTolerance = 0.00001f;

    for (std::size_t i = 0; i < outline.size(); ++i) {
        const glm::vec2 edge = outline[(i + 1) % outline.size()] - outline[i];
        const glm::vec2 offset = local - outline[i];

        // Conturul este convex si ordonat trigonometric; marginile sunt selectabile.
        if (edge.x * offset.y - edge.y * offset.x < -edgeTolerance) {
            return false;
        }
    }

    return true;
}

const CardAppearance& game_style::Appearance(CardType type)
{
    static const std::vector<CardAppearance> appearances = {
        {CardType::Triangle, "tema1_triangle", "LUPTATOR", glm::vec3(0.61f, 0.20f, 0.19f)},
        {CardType::Diamond, "tema1_diamond", "MAG",      glm::vec3(0.16f, 0.35f, 0.57f)},
        {CardType::Square, "tema1_rectangle", "GARDA",   glm::vec3(0.20f, 0.43f, 0.30f)}
    };

    for (const CardAppearance& appearance : appearances) {
        if (appearance.type == type) {
            return appearance;
        }
    }

    throw std::out_of_range("Tip de carte fara aspect definit");
}

const std::vector<CardBadge>& game_style::CardBadges()
{
    // Aceeasi definitie este folosita de randare si de hit-test, inclusiv zona iesita din carte.
    static const std::vector<CardBadge> badges = {
        {{0, 0}, {10, 16}, 17, 23, "tema1_sword", AttackColor, &Card::GetAttack, BadgeHitShape::Circle},
        {{1, 0}, {-10, 16}, 17, 18, "tema1_heart", HealthColor, &Card::GetHealth, BadgeHitShape::Circle},
        {{0, 1}, {10, -12}, 14, 0, "tema1_mana_crystal", ManaColor, &Card::GetManaCost, BadgeHitShape::Crystal}
    };

    return badges;
}

const Artwork& game_style::BoardArt()
{
    static const Artwork artwork = {
        glm::vec2(SceneWidth, SceneHeight),
        {
            {"tema1_rectangle", glm::vec4(0, 0, SceneWidth, SceneHeight), glm::vec3(0.15f, 0.09f, 0.06f)},
            {"tema1_rectangle", glm::vec4(0, 0, SceneWidth, 166), glm::vec3(0.23f, 0.14f, 0.10f)},
            {"tema1_rectangle", glm::vec4(0, SceneHeight - 164, SceneWidth, 164), glm::vec3(0.23f, 0.14f, 0.10f)},
            {"tema1_board", BoardFrameBounds, glm::vec3(0.09f, 0.055f, 0.035f)},
            {"tema1_board", GameLayout::Inset(BoardFrameBounds, 6), Border},
            {"tema1_board", GameLayout::Inset(BoardFrameBounds, 13), glm::vec3(0.73f, 0.60f, 0.38f)},
            {"tema1_board", GameLayout::Inset(BoardFrameBounds, 21), glm::vec3(0.80f, 0.69f, 0.47f)},
            {"tema1_rectangle", glm::vec4(PlayerBoard.x - 8, SceneHeight / 2 - 2, PlayerBoard.z - 10, 2), glm::vec3(0.65f, 0.53f, 0.33f)}
        },
        {
            {"TABLA TA", glm::vec2(PlayerBoard.x + 3, PlayerBoard.y + 2), 10, glm::vec3(0.37f, 0.27f, 0.14f), false},
            {"ADVERSAR", glm::vec2(PlayerBoard.x + 3, BoardY[game_rules::ComputerPlayerIndex] + CardHeight + 6),
             10, glm::vec3(0.37f, 0.27f, 0.14f), false}
        }
    };

    return artwork;
}

const Artwork& game_style::HeroArt()
{
    static const Artwork artwork = {
        HeroSize,
        {
            {"tema1_circle", glm::vec4(8, 0, HeroSize.x - 16, HeroSize.y), glm::vec3(0.09f, 0.055f, 0.035f)},
            {"tema1_circle", glm::vec4(12, 4, HeroSize.x - 24, HeroSize.y - 8), Border},
            {"tema1_circle", glm::vec4(17, 9, HeroSize.x - 34, HeroSize.y - 18), CardBack},
            {"tema1_circle", glm::vec4(31, 39, 88, 76), glm::vec3(0.21f, 0.29f, 0.36f)},
            {"tema1_diamond", glm::vec4(50, 49, 50, 60), Border},
            {"tema1_diamond", glm::vec4(59, 60, 32, 38), Paper}
        },
        {
            {"PAZNIC", glm::vec2(HeroSize.x / 2, HeroSize.y - 20), 13, Paper, true}
        }
    };

    return artwork;
}

const Artwork& game_style::CardBackArt()
{
    static const Artwork artwork = {
        glm::vec2(CardWidth, CardHeight),
        {
            {"tema1_rectangle", glm::vec4(0, 0, CardWidth, CardHeight), Border},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, CardWidth, CardHeight), CardFrameInset), glm::vec3(0.30f, 0.14f, 0.11f)},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, CardWidth, CardHeight), 10), Border},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, CardWidth, CardHeight), 12), CardBack},
            {"tema1_diamond", glm::vec4((CardWidth - 50) / 2, (CardHeight - 68) / 2, 50, 68), Border},
            {"tema1_diamond", glm::vec4((CardWidth - 34) / 2, (CardHeight - 46) / 2, 34, 46), glm::vec3(0.25f, 0.36f, 0.40f)}
        },
        {}
    };

    return artwork;
}

const Artwork& game_style::SpinnerArt()
{
    static const Artwork artwork = {
        SpinnerSize,
        {
            {"tema1_rectangle", glm::vec4(0, 0, SpinnerSize), glm::vec3(0.06f, 0.04f, 0.03f)},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, SpinnerSize), 6), Border},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, SpinnerSize), 11), CardBack}
        },
        {}
    };

    return artwork;
}

const Artwork& game_style::GameOverArt()
{
    static const Artwork artwork = {
        GameOverSize,
        {
            {"tema1_rectangle", glm::vec4(0, 0, GameOverSize), Border},
            {"tema1_rectangle", GameLayout::Inset(glm::vec4(0, 0, GameOverSize), 4), CardBack}
        },
        {}
    };

    return artwork;
}

const std::vector<TextLabel>& game_style::HudHints()
{
    static const std::vector<TextLabel> hints = {
        {"Drag: joaca o carte", {HudLeft, 72}, BodyTextSize, Paper, false},
        {"Atac: click creatura, apoi tinta", {HudLeft, 50}, BodyTextSize, Paper, false},
        {"R: joc nou | Verde: atac disponibil", {HudLeft, 28}, BodyTextSize, Border, false}
    };

    return hints;
}

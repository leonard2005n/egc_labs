#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "lab_m1/tema_1/model/card.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

namespace m1
{
    // Date vizuale locale. Rendererul le poate desena in orice dreptunghi destinatie.
    struct ShapeLayer
    {
        const char* mesh;
        glm::vec4 bounds;
        glm::vec3 color;
    };

    struct TextLabel
    {
        const char* text;
        glm::vec2 position;
        float size;
        glm::vec3 color;
        bool centered;
    };

    struct Artwork
    {
        glm::vec2 designSize;
        std::vector<ShapeLayer> layers;
        std::vector<TextLabel> labels;
    };

    struct CardAppearance
    {
        CardType type;
        const char* symbol;
        const char* title;
        glm::vec3 color;
    };

    enum class BadgeHitShape
    {
        Circle,
        Crystal
    };

    struct CardBadge
    {
        // anchor alege coltul; offset este deplasarea fata de acel colt.
        glm::vec2 anchor;
        glm::vec2 offset;
        float radius;
        float hitRadius;  // Folosit doar pentru selectia formelor circulare.
        const char* symbol;
        glm::vec3 color;
        int (Card::*value)() const;
        BadgeHitShape hitShape;

        glm::vec2 Center(const glm::vec4& cardBounds) const;
        bool Contains(const glm::vec2& point, const glm::vec4& cardBounds) const;
    };

    namespace game_style
    {
        const CardAppearance& Appearance(CardType type);
        const std::vector<CardBadge>& CardBadges();
        const Artwork& BoardArt();
        const Artwork& HeroArt();
        const Artwork& CardBackArt();
        const Artwork& SpinnerArt();
        const Artwork& GameOverArt();

        // Geometria comuna a fetei unei carti.
        constexpr float CardFrameInset = 3;
        constexpr float CardPaperInset = 7;
        constexpr float PortraitInset = 4;
        const glm::vec4 PortraitBounds(10, 40, game_view::CardWidth - 20, 82);
        const glm::vec4 SymbolBounds((game_view::CardWidth - 40) / 2, 62, 40, 40);
        const glm::vec2 CardTitleOffset(game_view::CardWidth / 2 + 9, game_view::CardHeight - 12);
        constexpr float CardTitleSize = 9;
        const glm::vec4 EffectStrip(CardPaperInset, 28, game_view::CardWidth - 2 * CardPaperInset, 15);
        const glm::vec2 EffectTextOffset(game_view::CardWidth / 2, 36);
        constexpr std::size_t CompactEffectThreshold = 12;
        constexpr float EffectTextSize = 8;
        constexpr float CompactEffectTextSize = 6.5f;

        // Fontul si badge-urile folosesc aceiasi parametri la toate dimensiunile.
        constexpr unsigned int FontPixelSize = 18;
        const char* const FontFile = "Hack-Bold.ttf";
        constexpr int FontAdvanceShift = 6;  // FreeType stocheaza avansul in unitati 1/64 px.
        constexpr float TextBaselineOffset = 3;
        constexpr float BadgeBorder = 2;
        constexpr float BadgeTint = 0.85f;
        constexpr float CrystalBorder = 1.5f;
        constexpr float CrystalCoreTint = 0.9f;
        constexpr float CrystalGlintLightness = 0.4f;
        constexpr float SingleDigitScale = 1.35f;
        constexpr float MultiDigitScale = 1.05f;
        constexpr float NumberOutlineOffset = 1.5f;
        constexpr float SwordSizeScale = 2.35f;
        constexpr float SwordAngle = 0.62f;
        const glm::vec4 HeartHighlight(-0.52f, 0.37f, 0.22f, 0.15f);

        constexpr float ButtonBorder = 3;
        constexpr float CardOutline = 3;
        constexpr float HeroBadgeRadius = 20;
        const glm::vec2 HeroBadgeOffset(130, 26);
        const glm::vec2 HeroAttackOffset(75, 24);
        constexpr float HeroAttackTextSize = 11;

        // Asezarea textelor si a resurselor fata de zona HUD-ului.
        constexpr float HudLeft = 24;
        constexpr float HudTurnY = 100;
        constexpr float HudMessageY = 145;
        constexpr float OpponentHandTopInset = 43;
        constexpr float BodyTextSize = 12;
        constexpr float TurnTextSize = 15;
        constexpr float ButtonTextSize = 14;
        const glm::vec2 ManaGemSize(12, 16);
        constexpr float ManaGemSpacing = 15;
        constexpr float ManaLabelOffset = 25;
        constexpr float ManaTextSize = 14;
        const std::vector<TextLabel>& HudHints();

        // Timerul circular: mesh-ul si randarea folosesc aceeasi grosime a inelului.
        const glm::vec2 TimerCenter(60, 360);
        constexpr float TimerRadius = 40;
        constexpr float TimerBorder = 3;
        constexpr float TimerInnerRatio = 0.77f;
        constexpr float TimerWarningSeconds = 5;
        constexpr float TimerNumberSize = 25;
        constexpr float TimerUnitSize = 9;
        const glm::vec2 TimerNumberOffset(0, 3);
        const glm::vec2 TimerUnitOffset(0, -16);
        const glm::vec3 TimerBorderColor(0.07f, 0.08f, 0.12f);
        const glm::vec3 TimerTrack(0.14f, 0.19f, 0.29f);
        const glm::vec3 TimerInside(0.055f, 0.075f, 0.12f);

        namespace spinner
        {
            const glm::vec2 CenterOffset(game_view::SpinnerSize.x / 2, game_view::SpinnerSize.y / 2 - 3);
            const glm::vec2 TitleOffset(game_view::SpinnerSize.x / 2, game_view::SpinnerSize.y - 52);
            const glm::vec2 ResultOffset(game_view::SpinnerSize.x / 2, 76);
            const glm::vec2 HintOffset(game_view::SpinnerSize.x / 2, 38);
            const glm::vec2 SectorLabelOffsets[] = {glm::vec2(-63, 43), glm::vec2(63, -43)};
            const glm::vec3 SectorColors[] = {glm::vec3(0.12f, 0.38f, 0.62f),
                                              glm::vec3(0.57f, 0.17f, 0.20f)};
            constexpr float Radius = 122;
            constexpr float BorderWidth = 5;
            constexpr float DividerWidth = 3;
            constexpr float NeedleBack = 18;
            constexpr float NeedleLength = 105;
            constexpr float NeedleWidth = 8;
            constexpr float TipDistance = 109;
            const glm::vec2 HeadSize(26, 30);
            constexpr float HubOuterRadius = 13;
            constexpr float HubInnerRadius = 8;
            constexpr float TitleSize = 25;
            constexpr float SectorLabelSize = 22;
            constexpr float ResultSize = 25;
            constexpr float PendingSize = 17;
        }

        namespace arrow
        {
            constexpr float MinDistance = 24;
            constexpr float MaxInset = 48;
            constexpr float InsetRatio = 0.3f;
            constexpr float MaxHeadLength = 20;
            constexpr float HeadRatio = 0.45f;
            constexpr float ShaftOverlap = 2;
            constexpr float BorderWidth = 9;
            constexpr float Width = 5;
            constexpr float HeadBorder = 4;
            constexpr float HeadHeight = 20;
            constexpr float BorderHeadHeight = 26;
        }

        namespace game_over
        {
            const glm::vec2 ResultOffset(202, 171);
            const glm::vec2 DetailOffset(183, 127);
            const glm::vec2 RestartTextOffset(78, 13);
            constexpr float ResultSize = 34;
            constexpr float DetailSize = 18;
            constexpr float RestartTextSize = 20;
        }

        constexpr float CardFrameShade = 0.55f;
        constexpr float CardFaceLightness = 0.35f;
        const glm::vec3 PortraitShadow(0.21f, 0.14f, 0.09f);
        const glm::vec3 CardTitleColor(0.19f, 0.12f, 0.07f);
        const glm::vec3 SymbolShadow(0.12f, 0.08f, 0.05f);
        const glm::vec3 NumberShadow(0.10f, 0.07f, 0.05f);
        const glm::vec3 HeartHighlightColor(1.0f, 0.62f, 0.61f);
        const glm::vec3 ButtonEnabled(0.66f, 0.43f, 0.14f);
        const glm::vec3 ButtonDisabled(0.24f, 0.21f, 0.17f);
        const glm::vec3 RestartButton(0.42f, 0.34f, 0.18f);
        const glm::vec3 EmptyMana(0.18f, 0.20f, 0.22f);
        const glm::vec3 WindowBackground(0.025f, 0.035f, 0.055f);
    }
}

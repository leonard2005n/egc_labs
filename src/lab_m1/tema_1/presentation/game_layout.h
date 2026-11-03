#pragma once

#include <cstddef>
#include <glm/glm.hpp>

#include "lab_m1/tema_1/config/game_rules.h"
#include "lab_m1/tema_1/config/animation_config.h"

namespace m1
{
    // Asezarea si dimensiunile comune pastreaza desenarea sincronizata cu selectia.
    namespace game_view
    {
        constexpr float SceneWidth = 1280;
        constexpr float SceneHeight = 720;
        constexpr float CardWidth = 104;
        constexpr float CardHeight = 140;
        constexpr float CameraNearPlane = 0.01f;
        constexpr float CameraFarPlane = 400;
        constexpr float CameraDistance = 50;
        using game_rules::TurnDuration;
        using game_rules::PlayerCount;

        // Dimensiunile scenei sunt logice; viewport-ul le adapteaza oricarei ferestre.
        constexpr int TimerSegments = 180;
        constexpr float Pi = 3.14159265f;

        // Zona de joc este definita prin margini; celelalte elemente se ancoreaza de ea.
        constexpr float BoardLeftMargin = 140;
        constexpr float BoardRightMargin = 180;
        constexpr float BoardRowInset = 21;
        const glm::vec4 PlayerBoard(BoardLeftMargin, SceneHeight / 4,
                                     SceneWidth - BoardLeftMargin - BoardRightMargin, SceneHeight / 4);
        const glm::vec4 BoardFrameBounds(86, 158, 1038, 404);

        constexpr float BoardMaxSpacing = 130;
        constexpr float HandMaxSpacing = 70;
        constexpr float HandWidth = 370;
        constexpr float HandArcHeight = 12;
        constexpr float HandEdgeInset = 6;
        constexpr float OpponentHandOverhang = 20;
        const glm::vec2 HandOrigins[PlayerCount] = {
            glm::vec2(PlayerBoard.x + PlayerBoard.z - HandWidth, HandEdgeInset),
            glm::vec2(PlayerBoard.x + 10, SceneHeight - CardHeight + OpponentHandOverhang)
        };
        const float HandArcDirections[PlayerCount] = {1, -1};
        const float BoardY[PlayerCount] = {
            PlayerBoard.y + BoardRowInset,
            SceneHeight - (PlayerBoard.y + BoardRowInset) - CardHeight
        };

        // Coloana laterala: butonul este centrat, iar pachetele sunt de o parte si de alta.
        constexpr float RightColumnX = 1140;
        constexpr float TurnButtonLeftOffset = 26;
        constexpr float DeckBottomGap = 49;
        constexpr float DeckTopGap = 50;
        constexpr std::size_t DeckVisibleLayers = 3;
        constexpr float DeckLayerStep = 5;
        const glm::vec2 TurnButtonSize(148, 46);
        const glm::vec4 EndTurnBounds(RightColumnX - TurnButtonLeftOffset,
                                      (SceneHeight - TurnButtonSize.y) / 2,
                                      TurnButtonSize.x, TurnButtonSize.y);
        const glm::vec2 DeckPositions[PlayerCount] = {
            glm::vec2(RightColumnX, EndTurnBounds.y - CardHeight - DeckBottomGap),
            glm::vec2(RightColumnX, EndTurnBounds.y + EndTurnBounds.w + DeckTopGap)
        };
        const glm::vec2 DeckLabelOffsets[PlayerCount] = {glm::vec2(-10, -24), glm::vec2(-10, -23)};
        const glm::vec2 ManaOrigins[PlayerCount] = {
            glm::vec2(PlayerBoard.x + PlayerBoard.z + 6, 40), glm::vec2(24, SceneHeight - 155)
        };

        // Ambii eroi folosesc aceeasi dimensiune si aceeasi axa a tablei.
        const glm::vec2 HeroSize(150, 145);
        const float HeroLeft = PlayerBoard.x + (PlayerBoard.z - HeroSize.x) / 2;
        const glm::vec4 HeroBounds[PlayerCount] = {
            glm::vec4(HeroLeft, 17, HeroSize.x, HeroSize.y),
            glm::vec4(HeroLeft, SceneHeight - HeroSize.y - 12, HeroSize.x, HeroSize.y)
        };

        // Panourile se centreaza in scena; butonul de restart se ancoreaza de rezultat.
        const glm::vec2 SpinnerSize(560, 470);
        const glm::vec2 GameOverSize(724, 244);
        const glm::vec2 RestartSize(320, 44);
        const glm::vec4 SpinnerBounds((SceneWidth - SpinnerSize.x) / 2, (SceneHeight - SpinnerSize.y) / 2,
                                      SpinnerSize.x, SpinnerSize.y);
        const glm::vec4 GameOverBounds((SceneWidth - GameOverSize.x) / 2, (SceneHeight - GameOverSize.y) / 2,
                                       GameOverSize.x, GameOverSize.y);
        const glm::vec4 RestartBounds((SceneWidth - RestartSize.x) / 2, GameOverBounds.y + 30,
                                      RestartSize.x, RestartSize.y);

        const glm::vec3 Border(0.66f, 0.60f, 0.43f);
        const glm::vec3 Paper(0.94f, 0.90f, 0.77f);
        const glm::vec3 CardBack(0.14f, 0.20f, 0.31f);
        const glm::vec3 HealthColor(0.95f, 0.34f, 0.35f);
        const glm::vec3 ManaColor(0.26f, 0.67f, 0.98f);
        const glm::vec3 AttackColor(1.0f, 0.72f, 0.24f);
        const glm::vec3 ReadyColor(0.42f, 0.94f, 0.55f);
    }

    class GameLayout
    {
     public:
        // Operatii comune: art local -> dreptunghi destinatie, centrare si margini.
        static glm::vec2 Center(const glm::vec4& bounds);
        static glm::vec2 CardCenter(const glm::vec2& position);
        static glm::vec4 Inset(const glm::vec4& bounds, float margin);
        static glm::vec4 PlaceRect(const glm::vec4& local, const glm::vec2& designSize,
                                   const glm::vec4& destination);
        static glm::vec2 PlacePoint(const glm::vec2& local, const glm::vec2& designSize,
                                    const glm::vec4& destination);
        static glm::vec4 DeckBounds(int ownerIndex);
        static float DeckTopOffset(std::size_t count);

        static float SmoothStep(float t);
        static glm::ivec2 GetFramebufferResolution();
        static glm::ivec4 GetSceneViewport(const glm::ivec2& resolution);
        static bool MouseToScene(int mouseX, int mouseY, glm::vec2& position);
        static bool IsInside(const glm::vec2& point, const glm::vec4& rectangle);
        static glm::vec2 GetRowCardPosition(std::size_t index, std::size_t count, float y);
        static glm::vec2 GetHandCardPosition(std::size_t index, std::size_t count, int ownerIndex);
        static bool IsInsideCard(const glm::vec2& point, const glm::vec2& position);
        static glm::vec2 CenteredRowPosition(std::size_t index, std::size_t count,
                                             const glm::vec4& bounds, float maxSpacing);
        static bool IsInsideScaledCard(const glm::vec2& point, const glm::vec2& position, float scale);
    };
}

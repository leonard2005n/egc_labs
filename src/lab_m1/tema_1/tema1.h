#pragma once

#include <memory>

#include "components/simple_scene.h"
#include "lab_m1/tema_1/model/card_game.h"
#include "lab_m1/tema_1/presentation/game_presentation.h"
#include "lab_m1/tema_1/rendering/game_renderer.h"

namespace m1
{
    // Conecteaza evenimentele framework-ului cu regulile, prezentarea si randarea.
    class Tema1 : public gfxc::SimpleScene
    {
     public:
        Tema1() = default;
        ~Tema1() override = default;
        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void OnWindowResize(int width, int height) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnKeyPress(int key, int mods) override;

        void ResetGame();
        void AdvanceTurn();
        void StartTurnDraw();
        void UpdateComputer(float deltaTimeSeconds);
        void AutoChooseSilenceTarget();
        bool PlayCardOnBoard(std::size_t index, const glm::vec2& position);
        void ResolveAttack(std::size_t attackerIndex, int targetIndex);
        void ApplyAttackImpact();
        void UpdateAttackAnimation(float deltaTimeSeconds);
        void UpdateHoveredCard(int mouseX, int mouseY);

        CardGame game;
        GamePresentation view;
        std::unique_ptr<GameRenderer> renderer;
    };
}

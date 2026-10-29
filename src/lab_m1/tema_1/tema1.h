#pragma once

#include <memory>

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include "lab_m1/tema_1/card_game.h"

namespace m1
{
    // Scena gestioneaza prezentarea si input-ul; CardGame aplica regulile meciului.
    class Tema1 : public gfxc::SimpleScene
    {
     public:
        // Creeaza scena; resursele grafice sunt pregatite in Init.
        Tema1() = default;
        // Elibereaza obiectele detinute de scena prin membrii lor RAII.
        ~Tema1() override = default;
        void Init() override;

     private:
        // Pozitiile vizuale raman separate de datele cartilor.
        struct RowAnimation
        {
            std::vector<glm::vec2> positions;
            std::vector<glm::vec2> starts;
            std::vector<glm::vec2> targets;
            std::vector<float> scales;
            float elapsed = 0;
            bool active = false;
        };

        struct AttackAnimation
        {
            std::unique_ptr<Card> card;
            std::unique_ptr<Card> defender;
            int owner = 0;
            std::size_t index = 0;
            int target = -1;
            float elapsed = 0;
            float initialScale = 1;
            bool impacted = false;
            bool survived = true;
            bool defenderSurvived = true;
            float defenderScale = 1;
        };

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void OnWindowResize(int width, int height) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnKeyPress(int key, int mods) override;

        void CreateMeshes();
        void ResetGame();
        float UpdateOpeningSpinner(float deltaTimeSeconds);
        void DrawOpeningSpinner();
        void AdvanceTurn();
        void UpdateComputer(float deltaTimeSeconds);
        void AutoChooseSilenceTarget();
        bool PlayCardOnBoard(std::size_t index, const glm::vec2& position);
        void ResolveAttack(std::size_t attackerIndex, int targetIndex);
        void ApplyAttackImpact();
        void UpdateAttackAnimation(float deltaTimeSeconds);
        void UpdateCardScales(float deltaTimeSeconds);
        bool CanHumanInteract() const;
        void ShowMessage(const std::string& text);

        void StartRowAnimation(RowAnimation& row, int ownerIndex, bool inHand);
        void UpdateRowAnimation(RowAnimation& row, float deltaTimeSeconds);
        void UpdateCardReturn(float deltaTimeSeconds);
        bool IsLayoutAnimating() const;
        void StartTurnDraw();
        bool MouseToScene(int mouseX, int mouseY, glm::vec2& position) const;
        void UpdateHoveredCard(int mouseX, int mouseY);
        int FindHandCard(const glm::vec2& mouse) const;
        int FindBoardCard(int ownerIndex, const glm::vec2& mouse) const;
        glm::vec2 GetHoveredCardPosition() const;
        glm::vec2 GetHandVisualPosition(std::size_t index) const;

        void DrawBoard();
        void DrawHand(int ownerIndex);
        void DrawPlayedCards(int ownerIndex);
        void DrawHUD();
        void DrawTurnTimer();
        void DrawHero(int ownerIndex, const glm::vec4& bounds);
        void DrawMana(int ownerIndex);
        void DrawGameOver();
        void DrawAttackEffect();
        void DrawAttackAnimation();
        void DrawSelectionArrow();
        void DrawCardVisual(const Card& card, const glm::vec2& position, bool faceUp,
                            float scale, const glm::vec3* outline = nullptr, float rotation = 0);
        void DrawText(const std::string& text, float x, float y, float size, const glm::vec3& color);
        void DrawCenteredText(const std::string& text, const glm::vec2& center, float size, const glm::vec3& color);
        void DrawStatBadge(int value, const glm::vec2& center, float radius, const glm::vec3& color,
                           const std::string& symbol);
        void DrawCard(const Card& card, float x, float y, bool faceUp);
        void DrawProperties(const Card& card, float x, float y);
        void DrawDeck(int ownerIndex, float x, float y);
        void DrawCardBack(float x, float y);
        void DrawOutline(const glm::vec2& position, const glm::vec3& color);
        void DrawRectangle(float x, float y, float width, float height, const glm::vec3& color);
        void DrawShape(const std::string& meshName, float x, float y,
                       float width, float height, const glm::vec3& color);

        CardGame game;
        RowAnimation handAnimations[2];
        RowAnimation boardAnimations[2];
        std::unique_ptr<gfxc::TextRenderer> textRenderer;
        int drawingPlayer = 0;
        enum class CardMotion { Idle, Dragging, Returning, Drawing };
        CardMotion cardMotion = CardMotion::Idle;
        int selectedCardIndex = -1;
        int hoveredCardIndex = -1;
        int hoveredBoardOwner = -1;
        int hoveredBoardIndex = -1;
        int selectedAttackerIndex = -1;
        glm::vec2 cardPosition = glm::vec2(0);
        glm::vec2 initialCardPosition = glm::vec2(0);
        glm::vec2 grabOffset = glm::vec2(0);
        glm::vec2 returnStartPosition = glm::vec2(0);
        float returnElapsed = 0;
        float computerDelay = 0;
        float turnTimeRemaining = 30;
        bool openingSpinnerActive = true;
        float openingSpinnerElapsed = 0;
        std::string message;
        float messageTime = 0;
        glm::vec2 attackStart = glm::vec2(0);
        glm::vec2 attackEnd = glm::vec2(0);
        float attackEffectTime = 0;
        AttackAnimation attackAnimation;
        glm::mat3 cardTransform = glm::mat3(1);
    };
}

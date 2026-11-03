#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "components/text_renderer.h"
#include "lab_m1/tema_1/presentation/game_presentation.h"
#include "lab_m1/tema_1/rendering/game_style.h"

namespace m1
{
    class GameRenderer
    {
     public:
        using MeshDrawer = std::function<void(Mesh*, const glm::mat3&, const glm::vec3&)>;

        GameRenderer(const CardGame& game, const GamePresentation& view, MeshDrawer drawMesh);
        void Init(const std::string& resourceDirectory);
        void DrawFrame(const glm::vec2* cursorPosition);

     private:
        void DrawOpeningSpinner();
        void DrawBoard();
        void DrawHand(int ownerIndex);
        const glm::vec3* HandCardOutline(int ownerIndex, std::size_t index) const;
        void DrawPlayedCards(int ownerIndex);
        void DrawHUD();
        void DrawTurnTimer();
        void DrawHero(int ownerIndex, const glm::vec4& bounds);
        void DrawMana(int ownerIndex);
        void DrawManaCrystal(const glm::vec4& bounds, const glm::vec3& color);
        void DrawGameOver();
        void DrawAttackEffect();
        void DrawAttackAnimation();
        void DrawSelectionArrow(const glm::vec2* cursorPosition);
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
        void DrawArtwork(const Artwork& artwork, const glm::vec4& destination);
        void DrawRectWithMesh(const std::string& mesh, const glm::vec4& bounds, const glm::vec3& color);
        void DrawRect(const glm::vec4& bounds, const glm::vec3& color);
        void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color);
        void DrawRectangle(float x, float y, float width, float height, const glm::vec3& color);
        void DrawShape(const std::string& meshName, float x, float y,
                       float width, float height, const glm::vec3& color);

        const CardGame& game;
        const GamePresentation& view;
        MeshDrawer drawMesh;
        std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
        std::unique_ptr<gfxc::TextRenderer> textRenderer;
        glm::mat3 cardTransform = glm::mat3(1);
    };
}

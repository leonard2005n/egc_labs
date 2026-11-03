#include "lab_m1/tema_1/rendering/game_renderer.h"
#include "lab_m1/tema_1/presentation/game_layout.h"
#include "lab_m1/tema_1/model/card_catalog.h"
#include "lab_m1/tema_1/rendering/mesh_data.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

using namespace m1;
using namespace m1::game_view;
using namespace m1::game_style;

void GameRenderer::DrawCardVisual(const Card& card, const glm::vec2& position, bool faceUp,
                          float scale, const glm::vec3* outline, float rotation)
{
    const glm::vec2 center = GameLayout::CardCenter(position);
    const float c = scale * std::cos(rotation), sn = scale * std::sin(rotation);
    cardTransform = glm::mat3(c, sn, 0, -sn, c, 0,
                              center.x - c * center.x + sn * center.y,
                              center.y - sn * center.x - c * center.y, 1);
    DrawCard(card, position.x, position.y, faceUp);

    if (outline) {
        DrawOutline(position, *outline);
    }

    cardTransform = glm::mat3(1);
}

void GameRenderer::DrawText(const std::string& text, float x, float y, float size, const glm::vec3& color)
{
    // Transforma proiectia, astfel incat si fiecare litera sa se roteasca impreuna cu cartea.
    const glm::mat4 model(cardTransform[0].x, cardTransform[0].y, 0, 0,
                          cardTransform[1].x, cardTransform[1].y, 0, 0,
                          0, 0, 1, 0, cardTransform[2].x, cardTransform[2].y, 0, 1);
    const glm::mat4 flip(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, SceneHeight, 0, 1);
    const glm::mat4 projection = glm::ortho(0.0f, SceneWidth, SceneHeight, 0.0f) * flip * model * flip;
    textRenderer->m_textShader->Use();
    glUniformMatrix4fv(glGetUniformLocation(textRenderer->m_textShader->program, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));
    textRenderer->RenderText(text, x, SceneHeight - y - size, size / FontPixelSize, color);
}

void GameRenderer::DrawCenteredText(const std::string& text, const glm::vec2& center, float size, const glm::vec3& color)
{
    float width = 0;

    for (char character : text) {
        width += (textRenderer->Characters.at(character).Advance >> FontAdvanceShift) * size / FontPixelSize;
    }

    DrawText(text, center.x - width / 2, center.y - size / 2 - TextBaselineOffset, size, color);
}

void GameRenderer::DrawStatBadge(int value, const glm::vec2& center, float radius, const glm::vec3& color,
                          const std::string& symbol)
{
    if (symbol == "tema1_sword") {
        // Doua sabii incrucisate, cu lama argintie si maner bronz, ca in referinta.
        const float size = radius * SwordSizeScale;

        for (float angle : {-SwordAngle, SwordAngle}) {
            const float c = std::cos(angle), sn = std::sin(angle);
            const glm::mat3 model(size * c, size * sn, 0, -size * sn, size * c, 0,
                                  center.x, center.y, 1);

            for (const SwordPart& part : SwordParts()) {
                drawMesh(meshes.at(part.mesh).get(), cardTransform * model, part.color);
            }
        }

    } else if (symbol == "tema1_mana_crystal") {
        const float outerRadius = radius + BadgeBorder;
        DrawManaCrystal(glm::vec4(center - glm::vec2(outerRadius), glm::vec2(2 * outerRadius)), color);
    } else {
        DrawRectWithMesh(symbol, glm::vec4(center - glm::vec2(radius + BadgeBorder),
                                          glm::vec2(2 * (radius + BadgeBorder))), SymbolShadow);
        DrawRectWithMesh(symbol, glm::vec4(center - glm::vec2(radius), glm::vec2(2 * radius)), Border);
        DrawRectWithMesh(symbol, glm::vec4(center - glm::vec2(radius - BadgeBorder),
                                          glm::vec2(2 * (radius - BadgeBorder))), color * BadgeTint);

        if (symbol == "tema1_heart") {
            DrawRectWithMesh("tema1_circle", glm::vec4(center + radius * glm::vec2(HeartHighlight),
                                                       radius * glm::vec2(HeartHighlight.z, HeartHighlight.w)),
                              HeartHighlightColor);
        }
    }

    const std::string number = std::to_string(value);
    const float size = radius * (number.size() > 1 ? MultiDigitScale : SingleDigitScale);

    for (const glm::vec2& offset : {glm::vec2(-NumberOutlineOffset, 0), glm::vec2(NumberOutlineOffset, 0),
                                    glm::vec2(0, -NumberOutlineOffset), glm::vec2(0, NumberOutlineOffset)}) {
        DrawCenteredText(number, center + offset, size, NumberShadow);
    }

    DrawCenteredText(number, center, size, glm::vec3(1));
}

void GameRenderer::DrawCard(const Card& card, float x, float y, bool faceUp)
{
    if (!faceUp) {
        DrawCardBack(x, y);
        return;
    }

    const glm::vec4 bounds(x, y, CardWidth, CardHeight);
    const glm::vec2 position(x, y);
    const CardAppearance& appearance = Appearance(card.GetType());

    // Tipul coloreaza intreaga suprafata dreptunghiulara, nu doar portretul.
    DrawRect(bounds, Border);
    DrawRect(GameLayout::Inset(bounds, CardFrameInset), appearance.color * CardFrameShade);
    DrawRect(GameLayout::Inset(bounds, CardPaperInset), glm::mix(appearance.color, Paper, CardFaceLightness));

    const glm::vec4 portrait(position + glm::vec2(PortraitBounds), PortraitBounds.z, PortraitBounds.w);
    DrawRectWithMesh("tema1_circle", portrait, PortraitShadow);
    DrawRectWithMesh("tema1_circle", GameLayout::Inset(portrait, PortraitInset), appearance.color);
    DrawRectWithMesh(appearance.symbol,
                     glm::vec4(position + glm::vec2(SymbolBounds), SymbolBounds.z, SymbolBounds.w), Paper);
    DrawCenteredText(appearance.title, position + CardTitleOffset, CardTitleSize, CardTitleColor);

    // Efectele sunt date de catalog, nu cazuri speciale bazate pe cost sau tip.
    std::string effects;

    for (const EffectDefinition& definition : CardCatalog::Effects()) {
        if (card.HasEffect(definition.effect)) {
            effects += std::string(definition.label) + " ";
        }
    }

    if (card.IsSilenced()) {
        effects = "SILENCED";
    }

    if (!effects.empty()) {
        const float textSize = effects.size() > CompactEffectThreshold ? CompactEffectTextSize : EffectTextSize;
        DrawRect(glm::vec4(position + glm::vec2(EffectStrip), EffectStrip.z, EffectStrip.w), CardBack);
        DrawCenteredText(effects, position + EffectTextOffset, textSize, card.IsSilenced() ? Border : Paper);
    }

    DrawProperties(card, x, y);
}

void GameRenderer::DrawProperties(const Card& card, float x, float y)
{
    const glm::vec4 bounds(x, y, CardWidth, CardHeight);

    for (const CardBadge& badge : CardBadges()) {
        DrawStatBadge((card.*badge.value)(), badge.Center(bounds), badge.radius, badge.color, badge.symbol);
    }
}

void GameRenderer::DrawManaCrystal(const glm::vec4& bounds, const glm::vec3& color)
{
    const glm::vec4 interior = GameLayout::Inset(bounds, CrystalBorder);

    DrawRectWithMesh("tema1_mana_crystal", bounds, SymbolShadow);
    DrawRectWithMesh("tema1_mana_crystal", interior, color * CrystalCoreTint);

    for (const CrystalFacet& facet : ManaCrystalFacets()) {
        const glm::vec3 tint = glm::clamp(color * facet.brightness, glm::vec3(0), glm::vec3(1));
        DrawRectWithMesh(facet.mesh, interior, tint);
    }

    DrawRectWithMesh("tema1_mana_glint", interior, glm::mix(color, glm::vec3(1), CrystalGlintLightness));
}

void GameRenderer::DrawDeck(int ownerIndex, float x, float y)
{
    const std::size_t layers = std::min(game.GetPlayer(ownerIndex).GetDeck().size(), DeckVisibleLayers);

    for (std::size_t i = 0; i < layers; ++i) {
        DrawCardBack(x + i * DeckLayerStep, y + i * DeckLayerStep);
    }
}

void GameRenderer::DrawOutline(const glm::vec2& position, const glm::vec3& color)
{
    const float thickness = CardOutline;

    // Patru muchii calculate din dimensiunile curente, fara un al doilea contur fix.
    DrawRectangle(position.x, position.y, CardWidth, thickness, color);
    DrawRectangle(position.x, position.y + CardHeight - thickness, CardWidth, thickness, color);
    DrawRectangle(position.x, position.y, thickness, CardHeight, color);
    DrawRectangle(position.x + CardWidth - thickness, position.y, thickness, CardHeight, color);
}

void GameRenderer::DrawCardBack(float x, float y)
{
    DrawArtwork(CardBackArt(), glm::vec4(x, y, CardWidth, CardHeight));
}

void GameRenderer::DrawRectangle(float x, float y, float width, float height, const glm::vec3& color)
{
    DrawShape("tema1_rectangle", x, y, width, height, color);
}

void GameRenderer::DrawShape(const std::string& meshName, float x, float y,
                      float width, float height, const glm::vec3& color)
{
    const glm::mat3 modelMatrix(
        width, 0, 0,
        0, height, 0,
        x, y, 1);
    drawMesh(meshes.at(meshName).get(), cardTransform * modelMatrix, color);
}

void GameRenderer::DrawRectWithMesh(const std::string& mesh, const glm::vec4& bounds, const glm::vec3& color)
{
    DrawShape(mesh, bounds.x, bounds.y, bounds.z, bounds.w, color);
}

void GameRenderer::DrawRect(const glm::vec4& bounds, const glm::vec3& color)
{
    DrawRectWithMesh("tema1_rectangle", bounds, color);
}

void GameRenderer::DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color)
{
    DrawShape("tema1_circle", center.x - radius, center.y - radius, 2 * radius, 2 * radius, color);
}

void GameRenderer::DrawArtwork(const Artwork& artwork, const glm::vec4& destination)
{
    // Coordonatele locale ale desenului sunt proiectate in dreptunghiul destinatie.
    for (const ShapeLayer& layer : artwork.layers) {
        DrawRectWithMesh(layer.mesh, GameLayout::PlaceRect(layer.bounds, artwork.designSize, destination), layer.color);
    }

    const float textScale = std::min(destination.z / artwork.designSize.x, destination.w / artwork.designSize.y);

    for (const TextLabel& label : artwork.labels) {
        const glm::vec2 position = GameLayout::PlacePoint(label.position, artwork.designSize, destination);

        if (label.centered) {
            DrawCenteredText(label.text, position, label.size * textScale, label.color);
        } else {
            DrawText(label.text, position.x, position.y, label.size * textScale, label.color);
        }
    }
}

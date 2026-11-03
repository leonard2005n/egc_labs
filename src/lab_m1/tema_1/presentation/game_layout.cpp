#include "lab_m1/tema_1/presentation/game_layout.h"
#include "lab_m1/tema_1/rendering/game_style.h"

#include <algorithm>
#include <GLFW/glfw3.h>

using namespace m1;
using namespace m1::game_view;

float GameLayout::SmoothStep(float t)
{
    t = std::max(0.0f, std::min(1.0f, t));

    return t * t * (3 - 2 * t);
}

glm::ivec2 GameLayout::GetFramebufferResolution()
{
    glm::ivec2 resolution(0);
    glfwGetFramebufferSize(glfwGetCurrentContext(), &resolution.x, &resolution.y);

    return resolution;
}

glm::ivec4 GameLayout::GetSceneViewport(const glm::ivec2& resolution)
{
    const float scale = std::min(resolution.x / SceneWidth, resolution.y / SceneHeight);
    const int width = static_cast<int>(SceneWidth * scale);
    const int height = static_cast<int>(SceneHeight * scale);

    return glm::ivec4((resolution.x - width) / 2, (resolution.y - height) / 2, width, height);
}

bool GameLayout::IsInside(const glm::vec2& point, const glm::vec4& rectangle)
{
    return point.x >= rectangle.x && point.x <= rectangle.x + rectangle.z &&
           point.y >= rectangle.y && point.y <= rectangle.y + rectangle.w;
}

glm::vec2 GameLayout::CenteredRowPosition(std::size_t index, std::size_t count,
                                          const glm::vec4& bounds, float maxSpacing)
{
    const float spacing = count > 1 ? std::min(maxSpacing, (bounds.z - CardWidth) / (count - 1)) : 0;
    const float totalWidth = CardWidth + (count > 0 ? count - 1 : 0) * spacing;

    return glm::vec2(bounds.x + (bounds.z - totalWidth) / 2 + index * spacing, bounds.y);
}

glm::vec2 GameLayout::GetRowCardPosition(std::size_t index, std::size_t count, float y)
{
    const glm::vec4 row(PlayerBoard.x, y, PlayerBoard.z, CardHeight);

    return CenteredRowPosition(index, count, row, BoardMaxSpacing);
}

glm::vec2 GameLayout::GetHandCardPosition(std::size_t index, std::size_t count, int ownerIndex)
{
    const glm::vec4 row(HandOrigins[ownerIndex], HandWidth, CardHeight);
    const float offset = count > 1 ? 2.0f * index / (count - 1) - 1.0f : 0;
    const float arc = HandArcHeight * (1 - offset * offset);

    return CenteredRowPosition(index, count, row, HandMaxSpacing) +
           glm::vec2(0, HandArcDirections[ownerIndex] * arc);
}

bool GameLayout::IsInsideCard(const glm::vec2& point, const glm::vec2& position)
{
    if (IsInside(point, glm::vec4(position, CardWidth, CardHeight))) {
        return true;
    }

    // Folosim exact ancorele badge-urilor desenate, fara coordonate duplicate.
    const glm::vec4 bounds(position, CardWidth, CardHeight);

    for (const CardBadge& badge : game_style::CardBadges()) {
        if (badge.Contains(point, bounds)) {
            return true;
        }
    }

    return false;
}

bool GameLayout::IsInsideScaledCard(const glm::vec2& point, const glm::vec2& position, float scale)
{
    const glm::vec2 center = CardCenter(position);

    return IsInsideCard(center + (point - center) / scale, position);
}

bool GameLayout::MouseToScene(int mouseX, int mouseY, glm::vec2& position)
{
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    const glm::ivec2 resolution = GetFramebufferResolution();
    const glm::ivec4 viewport = GetSceneViewport(resolution);

    if (width <= 0 || height <= 0 || viewport.z <= 0 || viewport.w <= 0) {
        return false;
    }

    const float pixelX = static_cast<float>(mouseX) * resolution.x / width;
    const float pixelY = resolution.y - static_cast<float>(mouseY) * resolution.y / height;
    position = glm::vec2((pixelX - viewport.x) * SceneWidth / viewport.z,
                         (pixelY - viewport.y) * SceneHeight / viewport.w);

    return true;
}

glm::vec2 GameLayout::Center(const glm::vec4& bounds)
{
    return glm::vec2(bounds) + glm::vec2(bounds.z, bounds.w) / 2.0f;
}

glm::vec2 GameLayout::CardCenter(const glm::vec2& position)
{
    return Center(glm::vec4(position, CardWidth, CardHeight));
}

glm::vec4 GameLayout::Inset(const glm::vec4& bounds, float margin)
{
    return glm::vec4(bounds.x + margin, bounds.y + margin,
                     bounds.z - 2 * margin, bounds.w - 2 * margin);
}

glm::vec4 GameLayout::PlaceRect(const glm::vec4& local, const glm::vec2& designSize,
                               const glm::vec4& destination)
{
    const glm::vec2 scale = glm::vec2(destination.z, destination.w) / designSize;

    return glm::vec4(glm::vec2(destination) + glm::vec2(local) * scale,
                     glm::vec2(local.z, local.w) * scale);
}

glm::vec2 GameLayout::PlacePoint(const glm::vec2& local, const glm::vec2& designSize,
                                const glm::vec4& destination)
{
    return glm::vec2(destination) + local * glm::vec2(destination.z, destination.w) / designSize;
}

glm::vec4 GameLayout::DeckBounds(int ownerIndex)
{
    const float overhang = (DeckVisibleLayers - 1) * DeckLayerStep;

    return glm::vec4(DeckPositions[ownerIndex], CardWidth + overhang, CardHeight + overhang);
}

float GameLayout::DeckTopOffset(std::size_t count)
{
    return count > 0 ? (std::min(count, DeckVisibleLayers) - 1) * DeckLayerStep : 0;
}

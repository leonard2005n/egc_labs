#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace m1
{
    struct SwordPart
    {
        const char* mesh;
        glm::vec3 color;
        std::vector<glm::vec2> vertices;
    };

    // Contururi artistice in coordonate locale, independente de pozitiile cartilor.
    const std::vector<SwordPart>& SwordParts();

    struct CrystalFacet
    {
        std::string mesh;
        std::vector<glm::vec2> vertices;
        float brightness;
    };

    // Conturul unitar este comun mesh-ului si selectiei badge-ului de mana.
    const std::vector<glm::vec2>& ManaCrystalOutline();
    const std::vector<CrystalFacet>& ManaCrystalFacets();
    const std::vector<glm::vec2>& ManaCrystalGlint();

    namespace mesh_config
    {
        constexpr unsigned int CircleSegments = 96;
        constexpr unsigned int HeartSegments = 128;
        constexpr int SpinnerSectorSegments = 80;
        constexpr float BoardBevelX = 0.06f;
        constexpr float BoardBevelY = 0.10f;
        constexpr float CrystalFacetInset = 0.28f;
    }
}

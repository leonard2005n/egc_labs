#include "lab_m1/tema_1/rendering/mesh_data.h"

using namespace m1;

const std::vector<glm::vec2>& m1::ManaCrystalOutline()
{
    static const std::vector<glm::vec2> outline = {
        {0.5f, 0}, {1, 0.28f}, {1, 0.72f}, {0.5f, 1}, {0, 0.72f}, {0, 0.28f}
    };

    return outline;
}

const std::vector<CrystalFacet>& m1::ManaCrystalFacets()
{
    static const std::vector<CrystalFacet> facets = [] {
        const auto& outline = ManaCrystalOutline();
        const std::vector<float> lighting = {0.55f, 0.72f, 1.05f, 1.25f, 1.10f, 0.65f};
        const glm::vec2 center(0.5f);
        std::vector<CrystalFacet> result;

        // Fiecare fateta leaga o muchie exterioara de conturul interior concentric.
        for (std::size_t i = 0; i < outline.size(); ++i) {
            const std::size_t next = (i + 1) % outline.size();
            const glm::vec2 innerFirst = glm::mix(outline[i], center, mesh_config::CrystalFacetInset);
            const glm::vec2 innerNext = glm::mix(outline[next], center, mesh_config::CrystalFacetInset);

            result.push_back({"tema1_mana_facet_" + std::to_string(i),
                              {outline[i], outline[next], innerNext, innerFirst}, lighting.at(i)});
        }

        return result;
    }();

    return facets;
}

const std::vector<glm::vec2>& m1::ManaCrystalGlint()
{
    static const std::vector<glm::vec2> glint = {{0.24f, 0.68f}, {0.36f, 0.83f}, {0.44f, 0.72f}};

    return glint;
}

const std::vector<SwordPart>& m1::SwordParts()
{
    static const std::vector<SwordPart> parts = {
        {"tema1_sword", glm::vec3(0.10f, 0.09f, 0.075f),
            {{-0.12f, -0.18f}, {0.12f, -0.18f}, {0.11f, 0.34f},
             {0, 0.53f}, {-0.11f, 0.34f}}},

        {"tema1_sword_blade", glm::vec3(0.72f, 0.76f, 0.77f),
            {{-0.08f, -0.13f}, {0.08f, -0.13f}, {0.075f, 0.33f},
             {0, 0.47f}, {-0.075f, 0.33f}}},

        {"tema1_sword_edge", glm::vec3(0.94f, 0.94f, 0.87f),
            {{-0.08f, -0.13f}, {0, -0.13f}, {0, 0.47f}, {-0.075f, 0.33f}}},

        {"tema1_sword_hilt", glm::vec3(0.62f, 0.46f, 0.23f),
            {{-0.24f, -0.23f}, {-0.20f, -0.15f}, {0.20f, -0.15f},
             {0.24f, -0.23f}, {0.18f, -0.27f}, {-0.18f, -0.27f}}},

        {"tema1_sword_pommel", glm::vec3(0.62f, 0.46f, 0.23f),
            {{-0.10f, -0.47f}, {0, -0.53f},
             {0.10f, -0.47f}, {0.075f, -0.40f}, {-0.075f, -0.40f}}},

        {"tema1_sword_grip", glm::vec3(0.26f, 0.20f, 0.14f),
            {{-0.065f, -0.44f}, {0.065f, -0.44f},
             {0.065f, -0.23f}, {-0.065f, -0.23f}}}
    };

    return parts;
}

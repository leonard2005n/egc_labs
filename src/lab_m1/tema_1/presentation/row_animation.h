#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace m1
{
    class RowAnimation
    {
     public:
        void Start(int ownerIndex, bool inHand);
        void Update(float deltaTimeSeconds);

        std::vector<glm::vec2> positions;
        std::vector<glm::vec2> starts;
        std::vector<glm::vec2> targets;
        std::vector<float> scales;
        float elapsed = 0;
        bool active = false;
    };
}

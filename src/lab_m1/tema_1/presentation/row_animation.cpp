#include "lab_m1/tema_1/presentation/row_animation.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

#include <algorithm>

using namespace m1;
using namespace m1::game_view;

void RowAnimation::Start(int ownerIndex, bool inHand)
{
    starts = positions;
    targets.clear();

    for (std::size_t i = 0; i < positions.size(); ++i) {
        targets.push_back(inHand ? GameLayout::GetHandCardPosition(i, positions.size(), ownerIndex) :
            GameLayout::GetRowCardPosition(i, positions.size(), BoardY[ownerIndex]));
    }

    elapsed = 0;
    active = !positions.empty();
}

void RowAnimation::Update(float deltaTimeSeconds)
{
    if (!active) {
        return;
    }

    elapsed += deltaTimeSeconds;
    const float progress = std::min(elapsed / LayoutDuration, 1.0f);

    for (std::size_t i = 0; i < positions.size(); ++i) {
        positions[i] = starts[i] + progress * (targets[i] - starts[i]);
    }

    active = progress < 1;
}

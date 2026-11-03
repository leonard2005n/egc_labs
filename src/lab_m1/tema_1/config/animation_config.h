#pragma once

namespace m1
{
    namespace game_view
    {
        // Durate in secunde; timpii de contact si de final sunt folositi si de controller.
        constexpr float ReturnDuration = 0.3f;
        constexpr float LayoutDuration = 0.35f;
        constexpr float AttackImpactTime = 0.30f;
        constexpr float AttackDuration = 0.90f;
        constexpr float ImpactDuration = 0.28f;
        constexpr float SpinnerSpinDuration = 3.2f;
        constexpr float SpinnerRevealDuration = 1.0f;

        constexpr float HoverLift = 28;
        constexpr float HoverScale = 1.10f;
        constexpr float SelectedScale = 1.18f;
        constexpr float ScaleResponse = 14;
        constexpr float ComputerActionDelay = 0.65f;
        constexpr float MessageDuration = 2.5f;

        // Fazele atacului: pregatire, deplasare, contact, scuturare si disparitie.
        constexpr float AttackWindupDuration = 0.10f;
        constexpr float AttackPullback = 16;
        constexpr float AttackContactInset = 70;
        constexpr float ShakeAngle = 0.105f;
        constexpr int ShakeCycles = 3;
        constexpr float ShakeDamping = 0.3f;
        constexpr float DeathShrinkDuration = 0.18f;

        constexpr int SparkCount = 8;
        constexpr float SparkStartRadius = 12;
        constexpr float SparkTravel = 42;
        constexpr float SparkLength = 20;
        constexpr float SparkWidth = 4;

        // Ruleta face cinci rotatii complete si apoi se opreste pe sectorul castigator.
        constexpr int SpinnerTurns = 5;
        constexpr float SpinnerEasePower = 3;
    }
}

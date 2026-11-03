#include "lab_m1/tema_1/tema1.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;

void Tema1::ResolveAttack(std::size_t attackerIndex, int targetIndex)
{
    // Copiile vizuale raman active pana la contact, cand CardGame aplica damage-ul simultan.
    AttackAnimation& animation = view.attackAnimation;
    const int owner = game.GetActivePlayer();
    const int opponent = OpponentOf(owner);
    const auto& board = game.GetPlayer(owner).GetBoard();

    if (view.openingSpinnerActive || animation.card || !game.CanAttack(attackerIndex, targetIndex)) {
        if (owner == HumanPlayer && !game.IsFinished() && !game.NeedsSilenceTarget()) {
            view.ShowMessage("Distruge mai intai creaturile TANK adverse.");
        }

        return;
    }

    animation = AttackAnimation();
    animation.card.reset(new Card(board[attackerIndex]));
    animation.owner = owner;
    animation.index = attackerIndex;
    animation.target = targetIndex;
    animation.initialScale = view.boardAnimations[owner].scales[attackerIndex];

    if (targetIndex >= 0) {
        animation.defender.reset(new Card(game.GetPlayer(opponent).GetBoard()[targetIndex]));
        animation.defenderScale = view.boardAnimations[opponent].scales[targetIndex];
    }

    view.attackStart = GameLayout::CardCenter(view.boardAnimations[owner].positions[attackerIndex]);
    view.attackEnd = targetIndex < 0 ? GameLayout::Center(HeroBounds[opponent]) :
        GameLayout::CardCenter(view.boardAnimations[opponent].positions[targetIndex]);
    view.messageTime = 0;
    view.selectedAttackerIndex = view.hoveredCardIndex = -1;
    view.hoveredBoardOwner = view.hoveredBoardIndex = -1;
}

void Tema1::ApplyAttackImpact()
{
    AttackAnimation& animation = view.attackAnimation;

    if (!animation.card || animation.impacted) {
        return;
    }

    animation.impacted = true;
    const int owner = animation.owner;
    const int opponent = OpponentOf(owner);
    const std::size_t attackerIndex = animation.index;
    const int targetIndex = animation.target;
    RowAnimation& attackers = view.boardAnimations[owner];
    RowAnimation& defenders = view.boardAnimations[opponent];

    if (!game.Attack(attackerIndex, targetIndex)) {
        animation.survived = false;
        return;
    }

    view.attackEffectTime = ImpactDuration;
    animation.survived = game.GetPlayer(owner).GetBoard().size() == attackers.positions.size();

    if (!animation.survived) {
        animation.card->TakeDamage(animation.card->GetHealth());
        attackers.positions.erase(attackers.positions.begin() + attackerIndex);
        attackers.scales.erase(attackers.scales.begin() + attackerIndex);
    } else {
        *animation.card = game.GetPlayer(owner).GetBoard()[attackerIndex];
    }

    if (targetIndex >= 0) {
        animation.defenderSurvived = game.GetPlayer(opponent).GetBoard().size() == defenders.positions.size();

        if (!animation.defenderSurvived) {
            animation.defender->TakeDamage(animation.defender->GetHealth());
            defenders.positions.erase(defenders.positions.begin() + targetIndex);
            defenders.scales.erase(defenders.scales.begin() + targetIndex);
        } else {
            *animation.defender = game.GetPlayer(opponent).GetBoard()[targetIndex];
        }
    }

    attackers.Start(owner, false);
    defenders.Start(opponent, false);
}

void Tema1::UpdateAttackAnimation(float deltaTimeSeconds)
{
    AttackAnimation& animation = view.attackAnimation;

    if (!animation.card) {
        return;
    }

    animation.elapsed += deltaTimeSeconds;
    // Marcajul de contact previne aplicarea dubla a damage-ului la expirarea turei.
    if (animation.elapsed >= AttackImpactTime) {
        ApplyAttackImpact();
    }

    if (animation.elapsed >= AttackDuration) {
        animation = AttackAnimation();
    }
}

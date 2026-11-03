#include "lab_m1/tema_1/ai/computer_player.h"
#include "lab_m1/tema_1/model/card_catalog.h"

using namespace m1;

using namespace m1::game_rules;

ComputerAction ComputerPlayer::ChooseAction(const CardGame& game)
{
    const Player& computer = game.GetPlayer(ComputerPlayerIndex);

    for (std::size_t i = 0; i < computer.GetHand().size(); ++i) {
        if (computer.CanPlayCard(i)) {
            return ComputerAction(ComputerAction::Type::PlayCard, i);
        }
    }

    for (std::size_t i = 0; i < computer.GetBoard().size(); ++i) {
        const Card& attacker = computer.GetBoard()[i];

        if (!attacker.CanAttack()) {
            continue;
        }

        int target = NoTarget;
        const auto& enemies = game.GetPlayer(HumanPlayer).GetBoard();
        // Prioritati: victorie imediata, creatura eliminabila, erou, apoi orice tinta legala.
        if (game.CanAttack(i, HeroTarget) && attacker.GetAttack() >= game.GetPlayer(HumanPlayer).GetHero().GetHealth()) {
            target = HeroTarget;
        }

        for (std::size_t j = 0; target == NoTarget && j < enemies.size(); ++j) {
            if (game.CanAttack(i, static_cast<int>(j)) && enemies[j].GetHealth() <= attacker.GetAttack()) {
                target = static_cast<int>(j);
            }
        }

        if (target == NoTarget && game.CanAttack(i, HeroTarget)) {
            target = HeroTarget;
        }

        for (std::size_t j = 0; target == NoTarget && j < enemies.size(); ++j) {
            if (game.CanAttack(i, static_cast<int>(j))) {
                target = static_cast<int>(j);
            }
        }

        if (target != NoTarget) {
            return ComputerAction(ComputerAction::Type::Attack, i, target);
        }
    }

    return ComputerAction(ComputerAction::Type::EndTurn);
}

std::size_t ComputerPlayer::ChooseSilenceTarget(const CardGame& game)
{
    const auto& enemies = game.GetPlayer(OpponentOf(game.GetActivePlayer())).GetBoard();
    std::size_t best = 0;
    int bestScore = -1;

    for (std::size_t i = 0; i < enemies.size(); ++i) {
        const int score = CardCatalog::SilencePriority(enemies[i]);

        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }

    return best;
}

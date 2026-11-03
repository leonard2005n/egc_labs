#include "lab_m1/tema_1/model/card_game.h"
#include "lab_m1/tema_1/model/card_catalog.h"

#include <algorithm>

using namespace m1;
using namespace m1::game_rules;

// Construieste acelasi meci initial ca la apasarea butonului de restart.
CardGame::CardGame()
{
    Reset();
}

// Amesteca independent pachetul din catalog inainte de impartirea fiecarei maini.
void CardGame::Reset()
{
    std::vector<Card> cards = CardCatalog::CreateDeck();

    for (int owner = 0; owner < PlayerCount; ++owner) {
        std::shuffle(cards.begin(), cards.end(), randomGenerator);

        // Impartirea ramane valida si daca definitiile unui pachet sunt mai putine.
        const auto handEnd = cards.begin() + std::min(InitialHandSize, cards.size());
        players[owner] = Player(std::vector<Card>(cards.begin(), handEnd), std::vector<Card>(handEnd, cards.end()));
    }

    activePlayer = std::uniform_int_distribution<int>(0, PlayerCount - 1)(randomGenerator);
    winner = NoWinner;
    turnDrawPending = false;
    silencePending = false;
    players[activePlayer].BeginTurn();
}

// Schimba jucatorul, reumple mana si marcheaza extragerea unica pentru noua tura.
bool CardGame::EndTurn()
{
    if (IsFinished() || turnDrawPending || silencePending) {
        return false;
    }

    activePlayer = OpponentOf(activePlayer);
    players[activePlayer].BeginTurn();
    turnDrawPending = true;

    return true;
}

// Consuma extragerea turei chiar si la pachet gol; nu permite extrageri repetate.
bool CardGame::DrawTurnCard()
{
    if (IsFinished() || !turnDrawPending) {
        return false;
    }

    turnDrawPending = false;

    return players[activePlayer].DrawCard();
}

// Permite plasarea doar jucatorului activ, dupa extragerea de inceput de tura.
bool CardGame::PlayCard(std::size_t index)
{
    if (IsFinished() || turnDrawPending || silencePending || !players[activePlayer].PlayCard(index)) {
        return false;
    }

    silencePending = players[activePlayer].GetBoard().back().HasEffect(CardEffect::Silence) &&
                     !players[OpponentOf(activePlayer)].GetBoard().empty();

    return true;
}

// Silence este o alegere obligatorie la plasare, fara un cost suplimentar.
bool CardGame::ChooseSilenceTarget(std::size_t targetIndex)
{
    if (IsFinished() || !silencePending || targetIndex >= players[OpponentOf(activePlayer)].GetBoard().size()) {
        return false;
    }

    players[OpponentOf(activePlayer)].SilenceCard(targetIndex);
    silencePending = false;

    return true;
}

// Tank protejeaza toate celelalte creaturi si eroul pana cand toate tank-urile dispar.
bool CardGame::CanAttack(std::size_t attackerIndex, int targetIndex) const
{
    if (IsFinished() || turnDrawPending || silencePending || targetIndex < HeroTarget) {
        return false;
    }

    const auto& attackers = players[activePlayer].GetBoard();
    const auto& defenders = players[OpponentOf(activePlayer)].GetBoard();

    if (attackerIndex >= attackers.size() || !attackers[attackerIndex].CanAttack() ||
        (targetIndex >= 0 && static_cast<std::size_t>(targetIndex) >= defenders.size())) {
        return false;
    }

    const bool hasTank = std::any_of(defenders.begin(), defenders.end(), [](const Card& card) {
        return card.HasEffect(CardEffect::Tank);
    });

    return !hasTank || (targetIndex >= 0 && defenders[targetIndex].HasEffect(CardEffect::Tank));
}

// Rezolva un atac; tinta -1 inseamna eroul advers.
bool CardGame::Attack(std::size_t attackerIndex, int targetIndex)
{
    if (!CanAttack(attackerIndex, targetIndex)) {
        return false;
    }

    Player& attacker = players[activePlayer];
    Player& defender = players[OpponentOf(activePlayer)];
    const bool lifesteal = attacker.GetBoard()[attackerIndex].HasEffect(CardEffect::Lifesteal);
    const int attackDamage = attacker.GetBoard()[attackerIndex].GetAttack();
    attacker.ExhaustCard(attackerIndex);

    if (targetIndex == HeroTarget) {
        defender.DamageHero(attackDamage);
    } else {
        // Memoreaza ambele atacuri inainte de damage, astfel incat loviturile sa fie simultane.
        const int counterDamage = defender.GetBoard()[targetIndex].GetAttack();
        defender.DamageCard(targetIndex, attackDamage);
        attacker.DamageCard(attackerIndex, counterDamage);
        attacker.RemoveDeadCards();
        defender.RemoveDeadCards();
    }

    if (lifesteal) {
        attacker.HealHero(attackDamage);
    }

    if (defender.GetHero().GetHealth() == 0) {
        winner = activePlayer;
    }

    return true;
}

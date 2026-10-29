#include "lab_m1/tema_1/card_game.h"

using namespace m1;

// Construieste acelasi meci initial ca la apasarea butonului de restart.
CardGame::CardGame()
{
    Reset();
}

// Amesteca independent cele 30 de carti inainte de impartirea fiecarei maini.
void CardGame::Reset()
{
    const std::vector<Card> types = {
        Card(CardType::Triangle, 1, 2, 1),
        Card(CardType::Diamond, 2, 3, 2, CardEffect::Charge),
        Card(CardType::Square, 3, 4, 3, CardEffect::Tank),
        Card(CardType::Triangle, 4, 4, 4, CardEffect::Lifesteal),
        Card(CardType::Diamond, 5, 5, 5, CardEffect::Silence),
        Card(CardType::Square, 6, 6, 6, CardEffect::Tank | CardEffect::Lifesteal)
    };
    std::vector<Card> cards;
    for (int i = 0; i < 30; ++i) {
        cards.push_back(types[i % types.size()]);
    }
    for (int owner = 0; owner < 2; ++owner) {
        std::shuffle(cards.begin(), cards.end(), randomGenerator);
        players[owner] = Player(std::vector<Card>(cards.begin(), cards.begin() + 5),
                                std::vector<Card>(cards.begin() + 5, cards.end()));
    }
    activePlayer = std::uniform_int_distribution<int>(0, 1)(randomGenerator);
    winner = -1;
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
    activePlayer = 1 - activePlayer;
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
                     !players[1 - activePlayer].GetBoard().empty();
    return true;
}

// Silence este o alegere obligatorie la plasare, fara un cost suplimentar.
bool CardGame::ChooseSilenceTarget(std::size_t targetIndex)
{
    if (IsFinished() || !silencePending || targetIndex >= players[1 - activePlayer].GetBoard().size()) {
        return false;
    }
    players[1 - activePlayer].SilenceCard(targetIndex);
    silencePending = false;
    return true;
}

// Tank protejeaza toate celelalte creaturi si eroul pana cand toate tank-urile dispar.
bool CardGame::CanAttack(std::size_t attackerIndex, int targetIndex) const
{
    if (IsFinished() || turnDrawPending || silencePending || targetIndex < -1) {
        return false;
    }
    const auto& attackers = players[activePlayer].GetBoard();
    const auto& defenders = players[1 - activePlayer].GetBoard();
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
    Player& defender = players[1 - activePlayer];
    const bool lifesteal = attacker.GetBoard()[attackerIndex].HasEffect(CardEffect::Lifesteal);
    const int attackDamage = attacker.GetBoard()[attackerIndex].GetAttack();
    attacker.ExhaustCard(attackerIndex);
    if (targetIndex == -1) {
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

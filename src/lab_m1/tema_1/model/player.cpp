#include "lab_m1/tema_1/model/player.h"

#include <algorithm>

using namespace m1;

Player::Player(const std::vector<Card>& hand, const std::vector<Card>& deck)
    : hand(hand), deck(deck)
{
}

void Player::BeginTurn()
{
    // Prima tura proprie foloseste mana initiala; cresterea incepe din tura urmatoare.
    if (turnsStarted > 0) {
        maxMana = std::min(game_rules::MaxMana, maxMana + game_rules::ManaPerTurn);
    }

    ++turnsStarted;
    mana = maxMana;

    for (Card& card : board) {
        card.ReadyForTurn();
    }
}

bool Player::DrawCard()
{
    if (deck.empty()) {
        return false;
    }

    hand.push_back(deck.back());
    deck.pop_back();

    return true;
}

bool Player::CanPlayCard(std::size_t index) const
{
    return index < hand.size() && hand[index].GetManaCost() <= mana;
}

bool Player::PlayCard(std::size_t index)
{
    if (!CanPlayCard(index)) {
        return false;
    }

    mana -= hand[index].GetManaCost();
    board.push_back(hand[index]);
    board.back().EnterBoard();
    hand.erase(hand.begin() + index);

    return true;
}

void Player::DamageCard(std::size_t index, int damage)
{
    board.at(index).TakeDamage(damage);
}

void Player::ExhaustCard(std::size_t index)
{
    board.at(index).Exhaust();
}

void Player::DamageHero(int damage)
{
    hero.TakeDamage(damage);
}

void Player::HealHero(int amount)
{
    hero.Heal(amount);
}

void Player::SilenceCard(std::size_t index)
{
    board.at(index).Silence();
}

void Player::RemoveDeadCards()
{
    board.erase(std::remove_if(board.begin(), board.end(), IsDead), board.end());
}

bool Player::IsDead(const Card& card)
{
    return card.GetHealth() <= 0;
}

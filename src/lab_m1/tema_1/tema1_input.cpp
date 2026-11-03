#include "lab_m1/tema_1/tema1.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

#include <GLFW/glfw3.h>

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;

void Tema1::OnWindowResize(int width, int height)
{
    const glm::ivec2 cursor = window->GetCursorPosition();
    OnMouseMove(cursor.x, cursor.y, 0, 0);
}

void Tema1::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_R) {
        ResetGame();
    }
}

void Tema1::UpdateHoveredCard(int mouseX, int mouseY)
{
    if (!view.CanHumanInteract(game)) {
        view.hoveredCardIndex = -1;
        view.hoveredBoardOwner = view.hoveredBoardIndex = -1;
        return;
    }

    glm::vec2 mouse;

    if (!GameLayout::MouseToScene(mouseX, mouseY, mouse)) {
        view.hoveredCardIndex = -1;
        view.hoveredBoardOwner = view.hoveredBoardIndex = -1;
        return;
    }

    view.hoveredCardIndex = view.FindHandCard(game, mouse);
    int newOwner = -1, newIndex = -1;

    for (int owner = 0; owner < PlayerCount; ++owner) {
        const int index = view.FindBoardCard(owner, mouse);

        if (index >= 0) {
            newOwner = owner;
            newIndex = index;
        }
    }

    view.hoveredBoardOwner = newOwner;
    view.hoveredBoardIndex = newIndex;
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    glm::vec2 mouse;

    if (!GameLayout::MouseToScene(mouseX, mouseY, mouse)) {
        return;
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) && game.IsFinished() && GameLayout::IsInside(mouse, RestartBounds)) {
        ResetGame();
        return;
    }

    if (!view.CanHumanInteract(game)) {
        return;
    }

    // Silence trebuie rezolvat inaintea oricarei alte actiuni de joc.
    if (game.NeedsSilenceTarget()) {
        if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
            const int target = view.FindBoardCard(ComputerPlayerIndex, mouse);

            if (target >= 0 && game.ChooseSilenceTarget(static_cast<std::size_t>(target))) {
                view.ShowMessage("SILENCE: efectele cartii alese au fost eliminate.");
            } else {
                view.ShowMessage("SILENCE: alege o carte adversa de pe tabla.");
            }
        }

        return;
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT)) {
        view.selectedAttackerIndex = -1;
        view.messageTime = 0;
        return;
    }

    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        return;
    }

    if (GameLayout::IsInside(mouse, EndTurnBounds)) {
        AdvanceTurn();
        return;
    }

    // Un click pe adversar confirma tinta atacatorului deja selectat.
    if (view.selectedAttackerIndex >= 0) {
        if (GameLayout::IsInside(mouse, HeroBounds[ComputerPlayerIndex])) {
            ResolveAttack(static_cast<std::size_t>(view.selectedAttackerIndex), HeroTarget);
            return;
        }

        const int target = view.FindBoardCard(ComputerPlayerIndex, mouse);

        if (target >= 0) {
            ResolveAttack(static_cast<std::size_t>(view.selectedAttackerIndex), target);
            return;
        }
    }

    const int attacker = view.FindBoardCard(HumanPlayer, mouse);

    if (attacker >= 0) {
        if (game.GetPlayer(HumanPlayer).GetBoard()[attacker].CanAttack()) {
            view.selectedAttackerIndex = attacker;
            view.ShowMessage("Alege o creatura adversa sau eroul advers. Click dreapta: anuleaza.");
        } else {
            view.ShowMessage("Aceasta creatura nu mai poate ataca in tura curenta.");
        }

        return;
    }

    if (GameLayout::IsInside(mouse, GameLayout::DeckBounds(HumanPlayer)) ||
        GameLayout::IsInside(mouse, GameLayout::DeckBounds(ComputerPlayerIndex))) {
        view.ShowMessage("Extragerea este automata la inceputul fiecarei ture.");
        return;
    }

    // Daca nu am ales un atac, incercam prinderea unei carti din mana.
    UpdateHoveredCard(mouseX, mouseY);

    if (view.hoveredCardIndex >= 0) {
        view.selectedAttackerIndex = -1;
        view.selectedCardIndex = view.hoveredCardIndex;
        view.initialCardPosition = view.handAnimations[HumanPlayer].positions[view.selectedCardIndex];
        view.cardPosition = view.GetHoveredCardPosition();
        view.grabOffset = mouse - view.cardPosition;
        view.cardMotion = GamePresentation::CardMotion::Dragging;
        view.hoveredCardIndex = -1;
    }
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    glm::vec2 mouse;

    if (view.cardMotion == GamePresentation::CardMotion::Dragging && GameLayout::MouseToScene(mouseX, mouseY, mouse)) {
        view.cardPosition = mouse - view.grabOffset;
    }

    UpdateHoveredCard(mouseX, mouseY);
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) || view.cardMotion != GamePresentation::CardMotion::Dragging) {
        return;
    }

    glm::vec2 mouse;
    const bool hasPosition = GameLayout::MouseToScene(mouseX, mouseY, mouse);

    if (hasPosition) {
        view.cardPosition = mouse - view.grabOffset;
    }

    if (hasPosition && GameLayout::IsInside(mouse, PlayerBoard) &&
        PlayCardOnBoard(static_cast<std::size_t>(view.selectedCardIndex), view.cardPosition)) {
        view.selectedCardIndex = -1;
        view.cardMotion = GamePresentation::CardMotion::Idle;
        view.messageTime = 0;
    } else {
        if (hasPosition && GameLayout::IsInside(mouse, PlayerBoard)) {
            view.ShowMessage("Mana insuficienta pentru aceasta carte.");
        }

        view.returnStartPosition = view.cardPosition;
        view.returnElapsed = 0;
        view.cardMotion = GamePresentation::CardMotion::Returning;
    }
}

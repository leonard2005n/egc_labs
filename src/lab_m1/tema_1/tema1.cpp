#include "lab_m1/tema_1/tema1.h"
#include "lab_m1/tema_1/ai/computer_player.h"
#include "lab_m1/tema_1/presentation/game_layout.h"

#include <algorithm>
#include <GLFW/glfw3.h>

using namespace m1;
using namespace m1::game_rules;
using namespace m1::game_view;

void Tema1::Init()
{
    // Camera foloseste spatiul logic; redimensionarea este rezolvata de viewport.
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, SceneWidth, 0, SceneHeight, CameraNearPlane, CameraFarPlane);
    camera->SetPosition(glm::vec3(0, 0, CameraDistance));
    camera->SetRotation(glm::vec3(0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // Resursele se creeaza o singura data; restartul reseteaza doar starea meciului.
    renderer.reset(new GameRenderer(game, view,
        [this](Mesh* mesh, const glm::mat3& transform, const glm::vec3& color) {
            RenderMesh2D(mesh, transform, color);
        }));
    renderer->Init(window->props.selfDir);
    ResetGame();
}

void Tema1::ResetGame()
{
    game.Reset();
    view.Reset(game);
}

void Tema1::FrameStart()
{
    const glm::vec3 background = game_style::WindowBackground;
    glClearColor(background.r, background.g, background.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const glm::ivec4 viewport = GameLayout::GetSceneViewport(GameLayout::GetFramebufferResolution());
    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void Tema1::Update(float deltaTimeSeconds)
{
    // Pastram ordinea: ceasuri, animatii, expirarea turei, AI, input vizual, desenare.
    const float dt = std::max(0.0f, deltaTimeSeconds);
    const float gameDt = view.UpdateOpeningSpinner(dt);
    view.messageTime = std::max(0.0f, view.messageTime - dt);
    view.attackEffectTime = std::max(0.0f, view.attackEffectTime - dt);
    UpdateAttackAnimation(dt);
    view.UpdateCardReturn(dt);

    for (int owner = 0; owner < PlayerCount; ++owner) {
        view.handAnimations[owner].Update(dt);
        view.boardAnimations[owner].Update(dt);
    }

    if (view.cardMotion == GamePresentation::CardMotion::Drawing && !view.IsLayoutAnimating()) {
        view.cardMotion = GamePresentation::CardMotion::Idle;
    }

    // Timpul curge inclusiv in timpul animatiilor si al drag-ului.
    bool turnExpired = false;

    if (!view.openingSpinnerActive && !game.IsFinished()) {
        view.turnTimeRemaining = std::max(0.0f, view.turnTimeRemaining - gameDt);

        if (view.turnTimeRemaining <= 0) {
            AdvanceTurn();
            turnExpired = true;
        }
    }

    if (!turnExpired) {
        UpdateComputer(gameDt);
    }

    // Hover-ul si desenarea citesc aceleasi pozitii actualizate ale cartilor.
    const glm::ivec2 cursor = window->GetCursorPosition();
    UpdateHoveredCard(cursor.x, cursor.y);

    if (!glfwGetWindowAttrib(glfwGetCurrentContext(), GLFW_HOVERED)) {
        view.hoveredCardIndex = -1;
        view.hoveredBoardOwner = view.hoveredBoardIndex = -1;
    }

    view.UpdateCardScales(game, dt);
    glm::vec2 cursorPosition;
    const bool hasCursor = glfwGetWindowAttrib(glfwGetCurrentContext(), GLFW_HOVERED) &&
                           GameLayout::MouseToScene(cursor.x, cursor.y, cursorPosition);
    renderer->DrawFrame(hasCursor ? &cursorPosition : nullptr);
}

void Tema1::FrameEnd()
{
}

void Tema1::AdvanceTurn()
{
    if (view.openingSpinnerActive) {
        return;
    }

    AutoChooseSilenceTarget();
    // Un atac deja confirmat isi aplica damage-ul o singura data chiar la expirarea timerului.
    if (view.attackAnimation.card) {
        ApplyAttackImpact();
        view.attackAnimation = AttackAnimation();
    }

    if (!game.EndTurn()) {
        return;
    }

    view.selectedCardIndex = view.selectedAttackerIndex = view.hoveredCardIndex = -1;
    view.cardMotion = GamePresentation::CardMotion::Idle;
    view.turnTimeRemaining = TurnDuration;
    view.messageTime = 0;
    view.computerDelay = ComputerActionDelay;
    StartTurnDraw();
}

void Tema1::UpdateComputer(float deltaTimeSeconds)
{
    if (view.openingSpinnerActive || game.IsFinished() || game.GetActivePlayer() != ComputerPlayerIndex ||
        view.cardMotion != GamePresentation::CardMotion::Idle || view.IsLayoutAnimating() || view.attackAnimation.card) {
        return;
    }

    view.computerDelay -= deltaTimeSeconds;

    if (view.computerDelay > 0 || view.attackEffectTime > 0) {
        return;
    }

    view.computerDelay = ComputerActionDelay;
    const ComputerAction action = ComputerPlayer::ChooseAction(game);

    switch (action.type) {
        case ComputerAction::Type::PlayCard:
            PlayCardOnBoard(action.cardIndex, view.handAnimations[ComputerPlayerIndex].positions[action.cardIndex]);
            break;
        case ComputerAction::Type::Attack:
            ResolveAttack(action.cardIndex, action.targetIndex);
            break;
        case ComputerAction::Type::EndTurn:
            AdvanceTurn();
            break;
    }
}

void Tema1::AutoChooseSilenceTarget()
{
    if (game.NeedsSilenceTarget()) {
        game.ChooseSilenceTarget(ComputerPlayer::ChooseSilenceTarget(game));
    }
}

bool Tema1::PlayCardOnBoard(std::size_t index, const glm::vec2& position)
{
    const int owner = game.GetActivePlayer();

    if (view.openingSpinnerActive || !game.PlayCard(index)) {
        return false;
    }

    RowAnimation& hand = view.handAnimations[owner];
    hand.positions.erase(hand.positions.begin() + index);
    hand.scales.erase(hand.scales.begin() + index);
    view.boardAnimations[owner].positions.push_back(position);
    view.boardAnimations[owner].scales.push_back(SelectedScale);
    hand.Start(owner, true);
    view.boardAnimations[owner].Start(owner, false);

    if (owner == ComputerPlayerIndex) {
        AutoChooseSilenceTarget();
    }

    return true;
}

void Tema1::StartTurnDraw()
{
    view.drawingPlayer = game.GetActivePlayer();
    const std::size_t deckSize = game.GetPlayer(view.drawingPlayer).GetDeck().size();

    if (!game.DrawTurnCard()) {
        return;
    }

    const float topOffset = GameLayout::DeckTopOffset(deckSize);
    RowAnimation& hand = view.handAnimations[view.drawingPlayer];
    hand.positions.push_back(DeckPositions[view.drawingPlayer] + glm::vec2(topOffset));
    hand.scales.push_back(1);
    hand.Start(view.drawingPlayer, true);
    view.cardMotion = GamePresentation::CardMotion::Drawing;
}

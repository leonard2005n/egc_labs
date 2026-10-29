#include "lab_m1/tema_1/tema1.h"

#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    constexpr float SceneWidth = 1280;
    constexpr float SceneHeight = 720;
    constexpr float CardWidth = 104;
    constexpr float CardHeight = 140;
    constexpr float ReturnDuration = 0.3f;
    constexpr float LayoutDuration = 0.35f;
    constexpr float HoverLift = 28;
    constexpr float HoverScale = 1.10f;
    constexpr float SelectedScale = 1.18f;
    constexpr float AttackImpactTime = 0.30f;
    constexpr float AttackDuration = 0.90f;
    constexpr float ImpactDuration = 0.28f;
    constexpr float SpinnerSpinDuration = 3.2f;
    constexpr float SpinnerRevealDuration = 1.0f;
    constexpr float TurnDuration = 30;
    constexpr int TimerSegments = 180;
    constexpr float Pi = 3.14159265f;
    const float BoardY[2] = {201, 379};
    const glm::vec2 DeckPositions[2] = {glm::vec2(1140, 148), glm::vec2(1140, 433)};
    const glm::vec4 PlayerBoard(140, 180, 960, 180);
    const glm::vec4 HeroBounds[2] = {glm::vec4(545, 17, 150, 145), glm::vec4(545, 563, 150, 145)};
    const glm::vec4 EndTurnBounds(1114, 337, 148, 46);
    const glm::vec4 RestartBounds(480, 268, 320, 44);
    const glm::vec3 Border(0.66f, 0.60f, 0.43f);
    const glm::vec3 Paper(0.94f, 0.90f, 0.77f);
    const glm::vec3 CardBack(0.14f, 0.20f, 0.31f);
    const glm::vec3 HealthColor(0.95f, 0.34f, 0.35f);
    const glm::vec3 ManaColor(0.26f, 0.67f, 0.98f);
    const glm::vec3 AttackColor(1.0f, 0.72f, 0.24f);
    const glm::vec3 ReadyColor(0.42f, 0.94f, 0.55f);

    float SmoothStep(float t)
    {
        t = std::max(0.0f, std::min(1.0f, t));
        return t * t * (3 - 2 * t);
    }

    // Citeste pixelii reali ai ferestrei, inclusiv pe ecrane cu scalare DPI.
    glm::ivec2 GetFramebufferResolution()
    {
        glm::ivec2 resolution(0);
        glfwGetFramebufferSize(glfwGetCurrentContext(), &resolution.x, &resolution.y);
        return resolution;
    }

    // Calculeaza acelasi viewport proportional pentru desenare si mouse.
    glm::ivec4 GetSceneViewport(const glm::ivec2& resolution)
    {
        const float scale = std::min(resolution.x / SceneWidth, resolution.y / SceneHeight);
        const int width = static_cast<int>(SceneWidth * scale);
        const int height = static_cast<int>(SceneHeight * scale);
        return glm::ivec4((resolution.x - width) / 2, (resolution.y - height) / 2, width, height);
    }

    // Include si marginile dreptunghiului in testul de selectie.
    bool IsInside(const glm::vec2& point, const glm::vec4& rectangle)
    {
        return point.x >= rectangle.x && point.x <= rectangle.x + rectangle.z &&
               point.y >= rectangle.y && point.y <= rectangle.y + rectangle.w;
    }

    // Centreaza creaturile pe tabla, pastrand un spatiu mai mic intre carti.
    glm::vec2 GetRowCardPosition(std::size_t index, std::size_t count, float y)
    {
        const float spacing = count > 1 ? std::min(130.0f, (960 - CardWidth) / (count - 1)) : 0;
        const float totalWidth = CardWidth + (count > 0 ? count - 1 : 0) * spacing;
        return glm::vec2(140 + (960 - totalWidth) / 2 + index * spacing, y);
    }

    // Grupeaza mainile in colturi opuse, pe un arc discret, fara rotatii complicate.
    glm::vec2 GetHandCardPosition(std::size_t index, std::size_t count, int ownerIndex)
    {
        const float width = 370;
        const float left = ownerIndex == 0 ? 730 : 150;
        const float spacing = count > 1 ? std::min(70.0f, (width - CardWidth) / (count - 1)) : 0;
        const float totalWidth = CardWidth + (count > 0 ? count - 1 : 0) * spacing;
        const float offset = count > 1 ? 2.0f * index / (count - 1) - 1.0f : 0;
        const float arc = 12 * (1 - offset * offset);
        return glm::vec2(left + (width - totalWidth) / 2 + index * spacing,
                         ownerIndex == 0 ? 6 + arc : 600 - arc);
    }

    // Include bulinele iesite din contur in zona pe care cartea poate fi prinsa.
    bool IsInsideCard(const glm::vec2& point, const glm::vec2& position)
    {
        if (IsInside(point, glm::vec4(position, CardWidth, CardHeight))) {
            return true;
        }
        const glm::vec2 attackCenter = position + glm::vec2(10, 16);
        const glm::vec2 healthCenter = position + glm::vec2(CardWidth - 10, 16);
        const glm::vec2 manaCenter = position + glm::vec2(10, CardHeight - 12);
        return glm::length(point - attackCenter) <= 23 || glm::length(point - healthCenter) <= 18 ||
               glm::length(point - manaCenter) <= 15;
    }
    // Selectia foloseste aceeasi scalare in jurul centrului ca desenarea.
    bool IsInsideScaledCard(const glm::vec2& point, const glm::vec2& position, float scale)
    {
        const glm::vec2 center = position + glm::vec2(CardWidth, CardHeight) / 2.0f;
        return IsInsideCard(center + (point - center) / scale, position);
    }

}

using namespace m1;

// Pregateste camera, geometria si fontul existent in proiect, apoi porneste meciul.
void Tema1::Init()
{
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, SceneWidth, 0, SceneHeight, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0));
    camera->Update();
    GetCameraInput()->SetActive(false);
    CreateMeshes();
    textRenderer.reset(new gfxc::TextRenderer(window->props.selfDir, 1280, 720));
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 18);
    // Seteaza explicit proiectia textului pe shaderul lui, folosind coordonate de sus in jos.
    textRenderer->m_textShader->Use();
    const glm::mat4 projection = glm::ortho(0.0f, SceneWidth, SceneHeight, 0.0f);
    glUniformMatrix4fv(glGetUniformLocation(textRenderer->m_textShader->program, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));
    ResetGame();
}

// Reseteaza regulile si toate selectiile/animatiile, fara a recrea resursele grafice.
void Tema1::ResetGame()
{
    game.Reset();
    openingSpinnerActive = true;
    openingSpinnerElapsed = 0;
    turnTimeRemaining = TurnDuration;
    cardMotion = CardMotion::Idle;
    selectedCardIndex = hoveredCardIndex = selectedAttackerIndex = -1;
    computerDelay = returnElapsed = messageTime = attackEffectTime = 0;
    message.clear();
    attackAnimation = AttackAnimation();
    hoveredBoardOwner = hoveredBoardIndex = -1;
    for (int owner = 0; owner < 2; ++owner) {
        handAnimations[owner] = RowAnimation();
        boardAnimations[owner] = RowAnimation();
        const auto& hand = game.GetPlayer(owner).GetHand();
        for (std::size_t i = 0; i < hand.size(); ++i) {
            handAnimations[owner].positions.push_back(GetHandCardPosition(i, hand.size(), owner));
            handAnimations[owner].scales.push_back(1);
        }
    }
}

// Timpul turei si AI-ul pornesc abia dupa rotire si afisarea rezultatului.
float Tema1::UpdateOpeningSpinner(float deltaTimeSeconds)
{
    if (!openingSpinnerActive) {
        return deltaTimeSeconds;
    }
    const float duration = SpinnerSpinDuration + SpinnerRevealDuration;
    const float nextElapsed = openingSpinnerElapsed + deltaTimeSeconds;
    openingSpinnerElapsed = std::min(duration, nextElapsed);
    if (openingSpinnerElapsed < duration) {
        return 0;
    }
    openingSpinnerActive = false;
    computerDelay = 0.65f;
    return std::max(0.0f, nextElapsed - duration);
}

// Doua sectoare egale; acul incetineste si se opreste la jucatorul ales de reguli.
void Tema1::DrawOpeningSpinner()
{
    DrawRectangle(360, 125, 560, 470, glm::vec3(0.06f, 0.04f, 0.03f));
    DrawRectangle(366, 131, 548, 458, Border);
    DrawRectangle(371, 136, 538, 448, CardBack);
    DrawCenteredText("CINE INCEPE MECIUL?", glm::vec2(640, 543), 25, Paper);
    const glm::vec2 center(640, 357);
    const float radius = 122;
    DrawShape("tema1_circle", center.x - radius - 5, center.y - radius - 5,
              2 * radius + 10, 2 * radius + 10, Border);
    DrawShape("tema1_spinner_human", center.x, center.y, radius, radius, glm::vec3(0.12f, 0.38f, 0.62f));
    DrawShape("tema1_spinner_ai", center.x, center.y, radius, radius, glm::vec3(0.57f, 0.17f, 0.20f));
    DrawRectangle(center.x - radius, center.y - 1.5f, 2 * radius, 3, Border);
    DrawCenteredText("TU", center + glm::vec2(-63, 43), 22, Paper);
    DrawCenteredText("AI", center + glm::vec2(63, -43), 22, Paper);

    const float progress = std::min(1.0f, openingSpinnerElapsed / SpinnerSpinDuration);
    const float ease = 1 - std::pow(1 - progress, 3);
    const float travel = 10 * Pi + (game.GetActivePlayer() == 1 ? Pi : 0);
    const float angle = Pi / 2 - travel * ease;
    const glm::vec2 direction(std::cos(angle), std::sin(angle));
    const glm::vec2 normal(-direction.y, direction.x);
    const glm::vec2 needleStart = center - direction * 18.0f - normal * 4.0f;
    const glm::mat3 needle(105 * direction.x, 105 * direction.y, 0,
                           8 * normal.x, 8 * normal.y, 0, needleStart.x, needleStart.y, 1);
    RenderMesh2D(meshes.at("tema1_rectangle"), needle, AttackColor);
    const glm::vec2 tip = center + direction * 109.0f;
    const glm::mat3 head(26 * direction.x, 26 * direction.y, 0,
                         30 * normal.x, 30 * normal.y, 0, tip.x, tip.y, 1);
    RenderMesh2D(meshes.at("tema1_arrowhead"), head, Paper);
    DrawShape("tema1_circle", center.x - 13, center.y - 13, 26, 26, Border);
    DrawShape("tema1_circle", center.x - 8, center.y - 8, 16, 16, AttackColor);

    const bool revealed = openingSpinnerElapsed >= SpinnerSpinDuration;
    DrawCenteredText(revealed ? (game.GetActivePlayer() == 0 ? "TU INCEPI!" : "AI-UL INCEPE!") : "SE ALEGE PRIMUL JUCATOR...",
                     glm::vec2(640, 201), revealed ? 25 : 17, revealed ? AttackColor : Paper);
    DrawCenteredText(revealed ? "Meciul incepe imediat" : "Sanse egale: 50% TU / 50% AI",
                     glm::vec2(640, 163), 12, Border);
}

// Curata tot ecranul si centreaza scena, cu benzi la redimensionare.
void Tema1::FrameStart()
{
    glClearColor(0.025f, 0.035f, 0.055f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const glm::ivec4 viewport = GetSceneViewport(GetFramebufferResolution());
    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

// Actualizeaza animatiile si calculatorul, apoi deseneaza de la fundal spre cartea activa.
void Tema1::Update(float deltaTimeSeconds)
{
    const float dt = std::max(0.0f, deltaTimeSeconds);
    const float gameDt = UpdateOpeningSpinner(dt);
    messageTime = std::max(0.0f, messageTime - dt);
    attackEffectTime = std::max(0.0f, attackEffectTime - dt);
    UpdateAttackAnimation(dt);
    UpdateCardReturn(dt);
    for (int owner = 0; owner < 2; ++owner) {
        UpdateRowAnimation(handAnimations[owner], dt);
        UpdateRowAnimation(boardAnimations[owner], dt);
    }
    if (cardMotion == CardMotion::Drawing && !IsLayoutAnimating()) {
        cardMotion = CardMotion::Idle;
    }
    // Timpul curge inclusiv in timpul animatiilor si al drag-ului.
    bool turnExpired = false;
    if (!openingSpinnerActive && !game.IsFinished()) {
        turnTimeRemaining = std::max(0.0f, turnTimeRemaining - gameDt);
        if (turnTimeRemaining <= 0) {
            AdvanceTurn();
            turnExpired = true;
        }
    }
    if (!turnExpired) {
        UpdateComputer(gameDt);
    }
    const glm::ivec2 cursor = window->GetCursorPosition();
    UpdateHoveredCard(cursor.x, cursor.y);
    if (!glfwGetWindowAttrib(glfwGetCurrentContext(), GLFW_HOVERED)) {
        hoveredCardIndex = -1;
        hoveredBoardOwner = hoveredBoardIndex = -1;
    }
    UpdateCardScales(dt);
    const glm::ivec2 resolution = GetFramebufferResolution();
    if (resolution.x <= 0 || resolution.y <= 0) {
        return;
    }
    const GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    DrawBoard();
    DrawHUD();
    for (int owner = 0; owner < 2; ++owner) {
        DrawHand(owner);
        DrawDeck(owner, DeckPositions[owner].x, DeckPositions[owner].y);
        DrawPlayedCards(owner);
    }
    if (hoveredCardIndex >= 0) {
        const glm::vec2 position = GetHoveredCardPosition();
        DrawCardVisual(game.GetPlayer(0).GetHand()[hoveredCardIndex], position, true,
                       handAnimations[0].scales[hoveredCardIndex]);
    }
    if (selectedCardIndex >= 0) {
        DrawCardVisual(game.GetPlayer(0).GetHand()[selectedCardIndex], cardPosition, true,
                       handAnimations[0].scales[selectedCardIndex], &AttackColor);
    }
    if (cardMotion == CardMotion::Drawing) {
        const glm::vec2 position = handAnimations[drawingPlayer].positions.back();
        DrawCard(game.GetPlayer(drawingPlayer).GetHand().back(), position.x, position.y, drawingPlayer == 0);
    }
    DrawAttackAnimation();
    DrawAttackEffect();
    DrawSelectionArrow();
    if (game.IsFinished() && !attackAnimation.card) {
        DrawGameOver();
    }
    if (openingSpinnerActive) {
        DrawOpeningSpinner();
    }
    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

// Nu adauga axe de laborator peste interfata jocului.
void Tema1::FrameEnd()
{
}

// Reface pozitia cursorului in spatiul logic dupa schimbarea dimensiunilor ferestrei.
void Tema1::OnWindowResize(int width, int height)
{
    const glm::ivec2 cursor = window->GetCursorPosition();
    OnMouseMove(cursor.x, cursor.y, 0, 0);
}

// Permite reluarea meciului in orice moment, fara reinstantierea scenei.
void Tema1::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_R) {
        ResetGame();
    }
}

// Accepta input de joc doar in tura omului si dupa terminarea miscarilor.
bool Tema1::CanHumanInteract() const
{
    return !openingSpinnerActive && !game.IsFinished() && game.GetActivePlayer() == 0 && !game.NeedsTurnDraw() &&
           cardMotion == CardMotion::Idle && !IsLayoutAnimating() && attackEffectTime <= 0 && !attackAnimation.card;
}

// Pastreaza un mesaj scurt pe ecran pentru o actiune care nu poate fi efectuata.
void Tema1::ShowMessage(const std::string& text)
{
    message = text;
    messageTime = 2.5f;
}

// Schimba tura si porneste extragerea obligatorie; selectiile nu trec la adversar.
void Tema1::AdvanceTurn()
{
    if (openingSpinnerActive) {
        return;
    }
    AutoChooseSilenceTarget();
    // Un atac deja confirmat isi aplica damage-ul o singura data chiar la expirarea timerului.
    if (attackAnimation.card) {
        ApplyAttackImpact();
        attackAnimation = AttackAnimation();
    }
    if (!game.EndTurn()) {
        return;
    }
    selectedCardIndex = selectedAttackerIndex = hoveredCardIndex = -1;
    cardMotion = CardMotion::Idle;
    turnTimeRemaining = TurnDuration;
    messageTime = 0;
    computerDelay = 0.65f;
    StartTurnDraw();
}

// Calculatorul joaca prima carte accesibila, apoi ataca o tinta eliminabila sau eroul.
void Tema1::UpdateComputer(float deltaTimeSeconds)
{
    if (openingSpinnerActive || game.IsFinished() || game.GetActivePlayer() != 1 || cardMotion != CardMotion::Idle || IsLayoutAnimating() || attackAnimation.card) {
        return;
    }
    computerDelay -= deltaTimeSeconds;
    if (computerDelay > 0 || attackEffectTime > 0) {
        return;
    }
    computerDelay = 0.65f;
    const Player& computer = game.GetPlayer(1);
    for (std::size_t i = 0; i < computer.GetHand().size(); ++i) {
        if (computer.CanPlayCard(i)) {
            PlayCardOnBoard(i, handAnimations[1].positions[i]);
            return;
        }
    }
    for (std::size_t i = 0; i < computer.GetBoard().size(); ++i) {
        const Card& attacker = computer.GetBoard()[i];
        if (!attacker.CanAttack()) {
            continue;
        }
        int target = -2;
        const auto& enemies = game.GetPlayer(0).GetBoard();
        if (game.CanAttack(i, -1) && attacker.GetAttack() >= game.GetPlayer(0).GetHero().GetHealth()) {
            target = -1;
        }
        for (std::size_t j = 0; target == -2 && j < enemies.size(); ++j) {
            if (game.CanAttack(i, static_cast<int>(j)) && enemies[j].GetHealth() <= attacker.GetAttack()) {
                target = static_cast<int>(j);
            }
        }
        if (target == -2 && game.CanAttack(i, -1)) {
            target = -1;
        }
        for (std::size_t j = 0; target == -2 && j < enemies.size(); ++j) {
            if (game.CanAttack(i, static_cast<int>(j))) {
                target = static_cast<int>(j);
            }
        }
        if (target != -2) {
            ResolveAttack(i, target);
            return;
        }
    }
    AdvanceTurn();
}

// AI-ul si expirarea timerului aleg o tinta utila pentru efectul ramas in asteptare.
void Tema1::AutoChooseSilenceTarget()
{
    if (!game.NeedsSilenceTarget()) {
        return;
    }
    const auto& enemies = game.GetPlayer(1 - game.GetActivePlayer()).GetBoard();
    std::size_t best = 0;
    int bestScore = -1;
    for (std::size_t i = 0; i < enemies.size(); ++i) {
        const int score = (enemies[i].HasEffect(CardEffect::Tank) ? 100 : 0) +
                          (enemies[i].HasEffect(CardEffect::Lifesteal) ? 50 : 0) +
                          (enemies[i].HasEffect(CardEffect::Charge) ? 20 : 0);
        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }
    game.ChooseSilenceTarget(best);
}

// Muta datele si pozitiile impreuna, apoi anima aranjarea mainii si a tablei proprietarului.
bool Tema1::PlayCardOnBoard(std::size_t index, const glm::vec2& position)
{
    const int owner = game.GetActivePlayer();
    if (openingSpinnerActive || !game.PlayCard(index)) {
        return false;
    }
    RowAnimation& hand = handAnimations[owner];
    hand.positions.erase(hand.positions.begin() + index);
    hand.scales.erase(hand.scales.begin() + index);
    boardAnimations[owner].positions.push_back(position);
    boardAnimations[owner].scales.push_back(SelectedScale);
    StartRowAnimation(hand, owner, true);
    StartRowAnimation(boardAnimations[owner], owner, false);
    if (owner == 1) {
        AutoChooseSilenceTarget();
    }
    return true;
}

// Memoreaza atacul; regulile se aplica la contact, dupa deplasarea spre tinta.
void Tema1::ResolveAttack(std::size_t attackerIndex, int targetIndex)
{
    const int owner = game.GetActivePlayer();
    const int opponent = 1 - owner;
    const auto& board = game.GetPlayer(owner).GetBoard();
    if (openingSpinnerActive || attackAnimation.card || !game.CanAttack(attackerIndex, targetIndex)) {
        if (owner == 0 && !game.IsFinished() && !game.NeedsSilenceTarget()) {
            ShowMessage("Distruge mai intai creaturile TANK adverse.");
        }
        return;
    }
    attackAnimation = AttackAnimation();
    attackAnimation.card.reset(new Card(board[attackerIndex]));
    attackAnimation.owner = owner;
    attackAnimation.index = attackerIndex;
    attackAnimation.target = targetIndex;
    attackAnimation.initialScale = boardAnimations[owner].scales[attackerIndex];
    if (targetIndex >= 0) {
        attackAnimation.defender.reset(new Card(game.GetPlayer(opponent).GetBoard()[targetIndex]));
        attackAnimation.defenderScale = boardAnimations[opponent].scales[targetIndex];
    }
    attackStart = boardAnimations[owner].positions[attackerIndex] + glm::vec2(CardWidth, CardHeight) / 2.0f;
    attackEnd = targetIndex < 0 ? glm::vec2(HeroBounds[opponent]) +
        glm::vec2(HeroBounds[opponent].z, HeroBounds[opponent].w) / 2.0f :
        boardAnimations[opponent].positions[targetIndex] + glm::vec2(CardWidth, CardHeight) / 2.0f;
    messageTime = 0;
    selectedAttackerIndex = hoveredCardIndex = -1;
    hoveredBoardOwner = hoveredBoardIndex = -1;
}

// Aplica loviturile simultane exact o data si sincronizeaza randurile dupa eliminari.
void Tema1::ApplyAttackImpact()
{
    if (!attackAnimation.card || attackAnimation.impacted) {
        return;
    }
    attackAnimation.impacted = true;
    const int owner = attackAnimation.owner;
    const int opponent = 1 - owner;
    const std::size_t attackerIndex = attackAnimation.index;
    const int targetIndex = attackAnimation.target;
    if (!game.Attack(attackerIndex, targetIndex)) {
        attackAnimation.survived = false;
        return;
    }
    attackEffectTime = ImpactDuration;
    attackAnimation.survived = game.GetPlayer(owner).GetBoard().size() == boardAnimations[owner].positions.size();
    if (!attackAnimation.survived) {
        attackAnimation.card->TakeDamage(attackAnimation.card->GetHealth());
        boardAnimations[owner].positions.erase(boardAnimations[owner].positions.begin() + attackerIndex);
        boardAnimations[owner].scales.erase(boardAnimations[owner].scales.begin() + attackerIndex);
    } else {
        *attackAnimation.card = game.GetPlayer(owner).GetBoard()[attackerIndex];
    }
    if (targetIndex >= 0) {
        attackAnimation.defenderSurvived = game.GetPlayer(opponent).GetBoard().size() ==
                                            boardAnimations[opponent].positions.size();
        if (!attackAnimation.defenderSurvived) {
            attackAnimation.defender->TakeDamage(attackAnimation.defender->GetHealth());
            boardAnimations[opponent].positions.erase(boardAnimations[opponent].positions.begin() + targetIndex);
            boardAnimations[opponent].scales.erase(boardAnimations[opponent].scales.begin() + targetIndex);
        } else {
            *attackAnimation.defender = game.GetPlayer(opponent).GetBoard()[targetIndex];
        }
    }
    StartRowAnimation(boardAnimations[owner], owner, false);
    StartRowAnimation(boardAnimations[opponent], opponent, false);
}

void Tema1::UpdateAttackAnimation(float deltaTimeSeconds)
{
    if (!attackAnimation.card) {
        return;
    }
    attackAnimation.elapsed += deltaTimeSeconds;
    if (attackAnimation.elapsed >= AttackImpactTime) {
        ApplyAttackImpact();
    }
    if (attackAnimation.elapsed >= AttackDuration) {
        attackAnimation = AttackAnimation();
    }
}

// Interpolare exponentiala independenta de framerate, pentru hover, selectie si revenire.
void Tema1::UpdateCardScales(float deltaTimeSeconds)
{
    const float blend = 1 - std::exp(-14 * deltaTimeSeconds);
    for (int owner = 0; owner < 2; ++owner) {
        for (std::size_t i = 0; i < handAnimations[owner].scales.size(); ++i) {
            float target = owner == 0 && static_cast<int>(i) == hoveredCardIndex ? HoverScale : 1;
            if (owner == 0 && static_cast<int>(i) == selectedCardIndex && cardMotion == CardMotion::Dragging) {
                target = SelectedScale;
            }
            float& scale = handAnimations[owner].scales[i];
            scale += (target - scale) * blend;
        }
        for (std::size_t i = 0; i < boardAnimations[owner].scales.size(); ++i) {
            float target = owner == hoveredBoardOwner && static_cast<int>(i) == hoveredBoardIndex ? HoverScale : 1;
            if ((owner == 0 && static_cast<int>(i) == selectedAttackerIndex) ||
                (game.NeedsSilenceTarget() && owner == game.GetActivePlayer() &&
                 i + 1 == boardAnimations[owner].scales.size())) {
                target = SelectedScale;
            }
            float& scale = boardAnimations[owner].scales[i];
            scale += (target - scale) * blend;
        }
    }
}

// Converteste pixelii mouse-ului in coordonatele logice, inclusiv benzile si scalarea DPI.
bool Tema1::MouseToScene(int mouseX, int mouseY, glm::vec2& position) const
{
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    const glm::ivec2 resolution = GetFramebufferResolution();
    const glm::ivec4 viewport = GetSceneViewport(resolution);
    if (width <= 0 || height <= 0 || viewport.z <= 0 || viewport.w <= 0) {
        return false;
    }
    const float pixelX = static_cast<float>(mouseX) * resolution.x / width;
    const float pixelY = resolution.y - static_cast<float>(mouseY) * resolution.y / height;
    position = glm::vec2((pixelX - viewport.x) * SceneWidth / viewport.z,
                         (pixelY - viewport.y) * SceneHeight / viewport.w);
    return true;
}

// Calculeaza ridicarea vizuala fara sa schimbe locul cartii in mana.
glm::vec2 Tema1::GetHoveredCardPosition() const
{
    return GetHandVisualPosition(hoveredCardIndex);
}

glm::vec2 Tema1::GetHandVisualPosition(std::size_t index) const
{
    const float lift = std::min(1.0f, (handAnimations[0].scales[index] - 1) / (HoverScale - 1));
    return handAnimations[0].positions[index] + glm::vec2(0, HoverLift * lift);
}

// Cauta de la ultima carte desenata si pastreaza accesibila portiunea ridicata.
int Tema1::FindHandCard(const glm::vec2& mouse) const
{
    if (hoveredCardIndex >= 0 && IsInsideScaledCard(mouse, GetHoveredCardPosition(),
                                                  handAnimations[0].scales[hoveredCardIndex])) {
        return hoveredCardIndex;
    }
    for (int i = static_cast<int>(game.GetPlayer(0).GetHand().size()) - 1; i >= 0; --i) {
        if (IsInsideScaledCard(mouse, GetHandVisualPosition(i), handAnimations[0].scales[i])) {
            return i;
        }
    }
    return -1;
}

// Cauta o creatura pe jumatatea de tabla indicata, in ordinea vizuala a suprapunerii.
int Tema1::FindBoardCard(int ownerIndex, const glm::vec2& mouse) const
{
    const auto& positions = boardAnimations[ownerIndex].positions;
    const int raised = ownerIndex == 0 && selectedAttackerIndex >= 0 ? selectedAttackerIndex :
        (ownerIndex == hoveredBoardOwner ? hoveredBoardIndex : -1);
    if (raised >= 0 && static_cast<std::size_t>(raised) < positions.size() &&
        IsInsideScaledCard(mouse, positions[raised], boardAnimations[ownerIndex].scales[raised])) {
        return raised;
    }
    for (int i = static_cast<int>(positions.size()) - 1; i >= 0; --i) {
        if (IsInsideScaledCard(mouse, positions[i], boardAnimations[ownerIndex].scales[i])) {
            return i;
        }
    }
    return -1;
}

// Hover pe mana proprie si cartile vizibile de pe tabla, in timpul turei omului.
void Tema1::UpdateHoveredCard(int mouseX, int mouseY)
{
    if (!CanHumanInteract()) {
        hoveredCardIndex = -1;
        hoveredBoardOwner = hoveredBoardIndex = -1;
        return;
    }
    glm::vec2 mouse;
    if (!MouseToScene(mouseX, mouseY, mouse)) {
        hoveredCardIndex = -1;
        hoveredBoardOwner = hoveredBoardIndex = -1;
        return;
    }
    hoveredCardIndex = FindHandCard(mouse);
    int newOwner = -1, newIndex = -1;
    for (int owner = 0; owner < 2; ++owner) {
        const int index = FindBoardCard(owner, mouse);
        if (index >= 0) {
            newOwner = owner;
            newIndex = index;
        }
    }
    hoveredBoardOwner = newOwner;
    hoveredBoardIndex = newIndex;
}

// Gestioneaza sfarsitul turei, alegerea atacatorului/tintei si prinderea unei carti din mana.
void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    glm::vec2 mouse;
    if (!MouseToScene(mouseX, mouseY, mouse)) {
        return;
    }
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) && game.IsFinished() && IsInside(mouse, RestartBounds)) {
        ResetGame();
        return;
    }
    if (!CanHumanInteract()) {
        return;
    }
    if (game.NeedsSilenceTarget()) {
        if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
            const int target = FindBoardCard(1, mouse);
            if (target >= 0 && game.ChooseSilenceTarget(static_cast<std::size_t>(target))) {
                ShowMessage("SILENCE: efectele cartii alese au fost eliminate.");
            } else {
                ShowMessage("SILENCE: alege o carte adversa de pe tabla.");
            }
        }
        return;
    }
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT)) {
        selectedAttackerIndex = -1;
        messageTime = 0;
        return;
    }
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        return;
    }
    if (IsInside(mouse, EndTurnBounds)) {
        AdvanceTurn();
        return;
    }
    if (selectedAttackerIndex >= 0) {
        if (IsInside(mouse, HeroBounds[1])) {
            ResolveAttack(static_cast<std::size_t>(selectedAttackerIndex), -1);
            return;
        }
        const int target = FindBoardCard(1, mouse);
        if (target >= 0) {
            ResolveAttack(static_cast<std::size_t>(selectedAttackerIndex), target);
            return;
        }
    }
    const int attacker = FindBoardCard(0, mouse);
    if (attacker >= 0) {
        if (game.GetPlayer(0).GetBoard()[attacker].CanAttack()) {
            selectedAttackerIndex = attacker;
            ShowMessage("Alege o creatura adversa sau eroul advers. Click dreapta: anuleaza.");
        } else {
            ShowMessage("Aceasta creatura nu mai poate ataca in tura curenta.");
        }
        return;
    }
    if (IsInside(mouse, glm::vec4(DeckPositions[0], CardWidth + 10, CardHeight + 10)) ||
        IsInside(mouse, glm::vec4(DeckPositions[1], CardWidth + 10, CardHeight + 10))) {
        ShowMessage("Extragerea este automata la inceputul fiecarei ture.");
        return;
    }
    UpdateHoveredCard(mouseX, mouseY);
    if (hoveredCardIndex >= 0) {
        selectedAttackerIndex = -1;
        selectedCardIndex = hoveredCardIndex;
        initialCardPosition = handAnimations[0].positions[selectedCardIndex];
        cardPosition = GetHoveredCardPosition();
        grabOffset = mouse - cardPosition;
        cardMotion = CardMotion::Dragging;
        hoveredCardIndex = -1;
    }
}

// Pastreaza punctul de prindere sub cursor si actualizeaza hover-ul cand nu exista drag.
void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    glm::vec2 mouse;
    if (cardMotion == CardMotion::Dragging && MouseToScene(mouseX, mouseY, mouse)) {
        cardPosition = mouse - grabOffset;
    }
    UpdateHoveredCard(mouseX, mouseY);
}

// Valideaza zona si mana; o plasare refuzata revine animat fara sa consume resurse.
void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) || cardMotion != CardMotion::Dragging) {
        return;
    }
    glm::vec2 mouse;
    const bool hasPosition = MouseToScene(mouseX, mouseY, mouse);
    if (hasPosition) {
        cardPosition = mouse - grabOffset;
    }
    if (hasPosition && IsInside(mouse, PlayerBoard) &&
        PlayCardOnBoard(static_cast<std::size_t>(selectedCardIndex), cardPosition)) {
        selectedCardIndex = -1;
        cardMotion = CardMotion::Idle;
        messageTime = 0;
    } else {
        if (hasPosition && IsInside(mouse, PlayerBoard)) {
            ShowMessage("Mana insuficienta pentru aceasta carte.");
        }
        returnStartPosition = cardPosition;
        returnElapsed = 0;
        cardMotion = CardMotion::Returning;
    }
}

// Memoreaza pozitiile actuale si noile destinatii centrate ale unui rand.
void Tema1::StartRowAnimation(RowAnimation& row, int ownerIndex, bool inHand)
{
    row.starts = row.positions;
    row.targets.clear();
    for (std::size_t i = 0; i < row.positions.size(); ++i) {
        row.targets.push_back(inHand ? GetHandCardPosition(i, row.positions.size(), ownerIndex) :
            GetRowCardPosition(i, row.positions.size(), BoardY[ownerIndex]));
    }
    row.elapsed = 0;
    row.active = !row.positions.empty();
}

// Interpoleaza toate pozitiile folosind timpul, nu numarul de cadre.
void Tema1::UpdateRowAnimation(RowAnimation& row, float deltaTimeSeconds)
{
    if (!row.active) {
        return;
    }
    row.elapsed += deltaTimeSeconds;
    const float progress = std::min(row.elapsed / LayoutDuration, 1.0f);
    for (std::size_t i = 0; i < row.positions.size(); ++i) {
        row.positions[i] = row.starts[i] + progress * (row.targets[i] - row.starts[i]);
    }
    row.active = progress < 1;
}

// Verifica simultan animatiile ambelor maini si ambelor jumatati de tabla.
bool Tema1::IsLayoutAnimating() const
{
    return handAnimations[0].active || handAnimations[1].active ||
           boardAnimations[0].active || boardAnimations[1].active;
}

// Extrage exact o carte la schimbarea turei si porneste deplasarea ei din varful pachetului.
void Tema1::StartTurnDraw()
{
    drawingPlayer = game.GetActivePlayer();
    const std::size_t deckSize = game.GetPlayer(drawingPlayer).GetDeck().size();
    if (!game.DrawTurnCard()) {
        return;
    }
    const float topOffset = (std::min(deckSize, std::size_t(3)) - 1) * 5.0f;
    handAnimations[drawingPlayer].positions.push_back(DeckPositions[drawingPlayer] + glm::vec2(topOffset));
    handAnimations[drawingPlayer].scales.push_back(1);
    StartRowAnimation(handAnimations[drawingPlayer], drawingPlayer, true);
    cardMotion = CardMotion::Drawing;
}

// Readuce cartea refuzata la pozitia initiala din mana si elibereaza selectia la final.
void Tema1::UpdateCardReturn(float deltaTimeSeconds)
{
    if (cardMotion != CardMotion::Returning) {
        return;
    }
    returnElapsed += deltaTimeSeconds;
    const float progress = std::min(returnElapsed / ReturnDuration, 1.0f);
    cardPosition = returnStartPosition + progress * (initialCardPosition - returnStartPosition);
    if (progress >= 1) {
        selectedCardIndex = -1;
        cardMotion = CardMotion::Idle;
    }
}

// Compune o tabla centrala de culoarea pergamentului, inconjurata de o rama de lemn.
void Tema1::DrawBoard()
{
    DrawRectangle(0, 0, SceneWidth, SceneHeight, glm::vec3(0.15f, 0.09f, 0.06f));
    DrawRectangle(0, 0, SceneWidth, 166, glm::vec3(0.23f, 0.14f, 0.10f));
    DrawRectangle(0, 556, SceneWidth, 164, glm::vec3(0.23f, 0.14f, 0.10f));
    DrawShape("tema1_board", 86, 158, 1038, 404, glm::vec3(0.09f, 0.055f, 0.035f));
    DrawShape("tema1_board", 92, 164, 1026, 392, Border);
    DrawShape("tema1_board", 99, 171, 1012, 378, glm::vec3(0.73f, 0.60f, 0.38f));
    DrawShape("tema1_board", 107, 179, 996, 362, glm::vec3(0.80f, 0.69f, 0.47f));
    DrawRectangle(132, 358, 950, 2, glm::vec3(0.65f, 0.53f, 0.33f));
    DrawText("TABLA TA", 143, 182, 10, glm::vec3(0.37f, 0.27f, 0.14f));
    DrawText("ADVERSAR", 143, 525, 10, glm::vec3(0.37f, 0.27f, 0.14f));
}

// Deseneaza mana, sarind cartile care apar separat la hover, drag sau extragere.
void Tema1::DrawHand(int ownerIndex)
{
    const auto& hand = game.GetPlayer(ownerIndex).GetHand();
    for (std::size_t i = 0; i < hand.size(); ++i) {
        if (ownerIndex == 0 && (static_cast<int>(i) == selectedCardIndex || static_cast<int>(i) == hoveredCardIndex)) {
            continue;
        }
        /* Cărți cu efecte speciale:

        Lifesteal – după atac, valoarea de ATK a cărții care atacă este adăugată la HP-ul eroului
        Charge – cartea poate ataca încă din prima rundă când este plasată
        Tank – înainte de a ataca o altă carte, inamicul trebuie să prioritizeze distrugerea acestei cărți
        Silence – anulează efectele de lifesteal și tank ale unei cărți la alegere când este plasată (sau orice alte efecte implementați la bonus)
        */
        if (cardMotion == CardMotion::Drawing && ownerIndex == drawingPlayer && i + 1 == hand.size()) {
            continue;
        }
        const glm::vec2 position = ownerIndex == 0 ? GetHandVisualPosition(i) : handAnimations[ownerIndex].positions[i];
        DrawCardVisual(hand[i], position, ownerIndex == 0, handAnimations[ownerIndex].scales[i]);
    }
}

// Arata creaturile de pe ambele table; verde indica un atac disponibil, galben atacatorul ales.
void Tema1::DrawPlayedCards(int ownerIndex)
{
    const auto& cards = game.GetPlayer(ownerIndex).GetBoard();
    auto draw = [this, ownerIndex, &cards](std::size_t i) {
        if (attackAnimation.card && ownerIndex == attackAnimation.owner && i == attackAnimation.index &&
            (!attackAnimation.impacted || attackAnimation.survived)) {
            return;
        }
        if (attackAnimation.defender && ownerIndex == 1 - attackAnimation.owner &&
            static_cast<int>(i) == attackAnimation.target &&
            (!attackAnimation.impacted || attackAnimation.defenderSurvived)) {
            return;
        }
        const glm::vec3* outline = nullptr;
        if (game.NeedsSilenceTarget() && ownerIndex == 1 - game.GetActivePlayer()) {
            outline = &ManaColor;
        } else if (game.NeedsSilenceTarget() && ownerIndex == game.GetActivePlayer() && i + 1 == cards.size()) {
            outline = &ManaColor;
        } else if (ownerIndex == 0 && static_cast<int>(i) == selectedAttackerIndex) {
            outline = &AttackColor;
        } else if (!game.IsFinished() && ownerIndex == game.GetActivePlayer() && cards[i].CanAttack()) {
            outline = &ReadyColor;
        }
        DrawCardVisual(cards[i], boardAnimations[ownerIndex].positions[i], true,
                       boardAnimations[ownerIndex].scales[i], outline);
    };
    int raised = ownerIndex == hoveredBoardOwner ? hoveredBoardIndex : -1;
    if (ownerIndex == 0 && selectedAttackerIndex >= 0) {
        raised = selectedAttackerIndex;
    }
    for (std::size_t i = 0; i < cards.size(); ++i) {
        if (static_cast<int>(i) != raised) {
            draw(i);
        }
    }
    if (raised >= 0 && static_cast<std::size_t>(raised) < cards.size()) {
        draw(raised);
    }
}

// Pune eroii pe axa centrala, mana in colturi si butonul de tura langa tabla.
void Tema1::DrawHUD()
{
    DrawHero(0, HeroBounds[0]);
    DrawHero(1, HeroBounds[1]);
    DrawMana(0);
    DrawMana(1);
    const bool enabled = CanHumanInteract() && !game.NeedsSilenceTarget();
    DrawRectangle(EndTurnBounds.x - 3, EndTurnBounds.y - 3, EndTurnBounds.z + 6, EndTurnBounds.w + 6, Border);
    DrawRectangle(EndTurnBounds.x, EndTurnBounds.y, EndTurnBounds.z, EndTurnBounds.w,
                  enabled ? glm::vec3(0.66f, 0.43f, 0.14f) : glm::vec3(0.24f, 0.21f, 0.17f));
    DrawCenteredText(openingSpinnerActive ? "SE ALEGE..." :
                     (game.NeedsSilenceTarget() ? "ALEGE TINTA" :
                     (game.GetActivePlayer() == 0 ? "SFARSIT TURA" : "TURA ADVERSAR")),
                     glm::vec2(EndTurnBounds.x + EndTurnBounds.z / 2, EndTurnBounds.y + EndTurnBounds.w / 2), 14, Paper);
    DrawTurnTimer();
    const std::string turn = openingSpinnerActive ? "CINE INCEPE?" :
        (game.GetActivePlayer() == 0 ? "TURA TA" : "TURA CALCULATORULUI");
    DrawText(openingSpinnerActive ? turn : turn + " | Runda " +
             std::to_string(game.GetPlayer(game.GetActivePlayer()).GetTurnCount()),
             24, 100, 15, Paper);
    DrawText("Drag: joaca o carte", 24, 72, 12, Paper);
    DrawText("Atac: click creatura, apoi tinta", 24, 50, 12, Paper);
    DrawText("R: joc nou | Verde: atac disponibil", 24, 28, 12, Border);
    if (game.NeedsSilenceTarget() && game.GetActivePlayer() == 0) {
        DrawText("SILENCE: click pe cartea adversa careia ii anulezi efectele.", 24, 145, 12, ManaColor);
    } else if (messageTime > 0) {
        DrawText(message, 24, 145, 12, AttackColor);
    }
    DrawText("Carti adverse: " + std::to_string(game.GetPlayer(1).GetHand().size()), 24, 677, 12, Paper);
    for (int owner = 0; owner < 2; ++owner) {
        DrawText("PACHET: " + std::to_string(game.GetPlayer(owner).GetDeck().size()),
                 1130, owner == 0 ? 124 : 410, 12, Paper);
    }
}

// Inelul se goleste in sens orar, pornind de sus; ultimul segment scade continuu.
void Tema1::DrawTurnTimer()
{
    const glm::vec2 center(60, 360);
    const float radius = 40;
    const glm::vec3 track(0.14f, 0.19f, 0.29f);
    DrawShape("tema1_circle", center.x - radius - 3, center.y - radius - 3,
              2 * radius + 6, 2 * radius + 6, glm::vec3(0.07f, 0.08f, 0.12f));
    DrawShape("tema1_circle", center.x - radius, center.y - radius,
              2 * radius, 2 * radius, track);
    DrawShape("tema1_circle", center.x - radius * 0.77f, center.y - radius * 0.77f,
              radius * 1.54f, radius * 1.54f, glm::vec3(0.055f, 0.075f, 0.12f));
    const float remaining = std::max(0.0f, std::min(1.0f, turnTimeRemaining / TurnDuration));
    const float step = 2 * Pi / TimerSegments;
    const float segmentsLeft = remaining * TimerSegments;
    const glm::vec3 color = turnTimeRemaining <= 5 ? HealthColor : AttackColor;
    for (int i = 0; i < static_cast<int>(std::ceil(segmentsLeft)); ++i) {
        const float portion = std::min(1.0f, segmentsLeft - i);
        const float angle = -((i + portion / 2) * step);
        const float c = std::cos(angle), sn = std::sin(angle);
        const glm::mat3 model(radius * portion * c, radius * portion * sn, 0,
                              -radius * sn, radius * c, 0, center.x, center.y, 1);
        RenderMesh2D(meshes.at("tema1_timer_segment"), model, color);
    }
    DrawCenteredText(std::to_string(static_cast<int>(std::ceil(turnTimeRemaining))),
                     center + glm::vec2(0, 3), 25, Paper);
    DrawCenteredText("SEC", center + glm::vec2(0, -16), 9, Border);
}

// Deseneaza portretul eroului si viata intr-o inima rosie.
void Tema1::DrawHero(int ownerIndex, const glm::vec4& bounds)
{
    const Hero& hero = game.GetPlayer(ownerIndex).GetHero();
    DrawShape("tema1_circle", bounds.x + 8, bounds.y, bounds.z - 16, bounds.w, glm::vec3(0.09f, 0.055f, 0.035f));
    DrawShape("tema1_circle", bounds.x + 12, bounds.y + 4, bounds.z - 24, bounds.w - 8, Border);
    DrawShape("tema1_circle", bounds.x + 17, bounds.y + 9, bounds.z - 34, bounds.w - 18, CardBack);
    DrawShape("tema1_circle", bounds.x + 31, bounds.y + 39, 88, 76, glm::vec3(0.21f, 0.29f, 0.36f));
    DrawShape("tema1_diamond", bounds.x + 50, bounds.y + 49, 50, 60, Border);
    DrawShape("tema1_diamond", bounds.x + 59, bounds.y + 60, 32, 38, Paper);
    DrawCenteredText("PAZNIC", glm::vec2(bounds.x + 75, bounds.y + 125), 13, Paper);
    DrawCenteredText("ATAC 0", glm::vec2(bounds.x + 75, bounds.y + 24), 11, Paper);
    DrawStatBadge(hero.GetHealth(), glm::vec2(bounds.x + 130, bounds.y + 26), 20, HealthColor, "tema1_heart");
}

// Pastreaza resursele langa mana fiecarui jucator, in colturi opuse.
void Tema1::DrawMana(int ownerIndex)
{
    const Player& owner = game.GetPlayer(ownerIndex);
    const float x = ownerIndex == 0 ? 1106 : 24;
    const float y = ownerIndex == 0 ? 40 : 565;
    for (int i = 0; i < owner.GetMaxMana(); ++i) {
        DrawShape("tema1_circle", x + i * 15, y, 12, 12,
                  i < owner.GetMana() ? ManaColor : glm::vec3(0.18f, 0.20f, 0.22f));
    }
    DrawText("MANA " + std::to_string(owner.GetMana()) + " / " + std::to_string(owner.GetMaxMana()),
             x, y + 25, 14, ManaColor);
}

// Suprapune rezultatul peste scena si permite restart doar prin buton sau tasta R.
void Tema1::DrawGameOver()
{
    DrawRectangle(278, 238, 724, 244, Border);
    DrawRectangle(282, 242, 716, 236, CardBack);
    DrawText(game.GetWinner() == 0 ? "VICTORIE!" : "INFRANGERE", 480, 409, 34, Paper);
    DrawText(game.GetWinner() == 0 ? "Eroul advers a cazut." : "Eroul tau a cazut.", 461, 365, 18, Paper);
    DrawRectangle(RestartBounds.x, RestartBounds.y, RestartBounds.z, RestartBounds.w, glm::vec3(0.42f, 0.34f, 0.18f));
    DrawText("JOC NOU (R)", 558, 281, 20, Paper);
}

// Dupa contact, ambele carti oscileaza de trei ori intre aproximativ -6 si +6 grade.
void Tema1::DrawAttackAnimation()
{
    if (!attackAnimation.card) {
        return;
    }
    const float elapsed = attackAnimation.elapsed;
    const glm::vec2 direction = glm::normalize(attackEnd - attackStart);
    const glm::vec2 contact = attackEnd - direction * 70.0f;
    const float afterImpact = std::max(0.0f, elapsed - AttackImpactTime);
    const float shakeProgress = std::min(1.0f, afterImpact / (AttackDuration - AttackImpactTime));
    const float rotation = 0.105f * std::sin(6 * Pi * shakeProgress) * (1 - 0.3f * shakeProgress);
    const float deathScale = 1 - SmoothStep((elapsed - (AttackDuration - 0.18f)) / 0.18f);
    if (attackAnimation.defender) {
        glm::vec2 targetCenter = attackEnd;
        if (attackAnimation.impacted && attackAnimation.defenderSurvived) {
            targetCenter = boardAnimations[1 - attackAnimation.owner].positions[attackAnimation.target] +
                           glm::vec2(CardWidth, CardHeight) / 2.0f;
        }
        const float targetScale = attackAnimation.defenderScale *
            (attackAnimation.impacted && !attackAnimation.defenderSurvived ? deathScale : 1);
        DrawCardVisual(*attackAnimation.defender, targetCenter - glm::vec2(CardWidth, CardHeight) / 2.0f,
                       true, targetScale, nullptr, -rotation);
    }
    glm::vec2 center;
    float scale;
    if (elapsed < 0.10f) {
        const float t = SmoothStep(elapsed / 0.10f);
        center = attackStart - direction * (16 * t);
        scale = attackAnimation.initialScale + (SelectedScale - attackAnimation.initialScale) * t;
    } else if (elapsed < AttackImpactTime) {
        const float t = SmoothStep((elapsed - 0.10f) / (AttackImpactTime - 0.10f));
        center = glm::mix(attackStart - direction * 16.0f, contact, t);
        scale = SelectedScale;
    } else if (attackAnimation.survived) {
        const float t = SmoothStep(shakeProgress);
        const glm::vec2 home = boardAnimations[attackAnimation.owner].positions[attackAnimation.index] +
                              glm::vec2(CardWidth, CardHeight) / 2.0f;
        center = glm::mix(contact, home, t);
        scale = SelectedScale + (boardAnimations[attackAnimation.owner].scales[attackAnimation.index] - SelectedScale) * t;
    } else {
        center = contact;
        scale = SelectedScale * deathScale;
    }
    DrawCardVisual(*attackAnimation.card, center - glm::vec2(CardWidth, CardHeight) / 2.0f,
                   true, scale, &AttackColor, rotation);
}

// Scantei scurte la contact; damage-ul devine vizibil in acelasi moment.
void Tema1::DrawAttackEffect()
{
    if (attackEffectTime <= 0) {
        return;
    }
    const float progress = 1 - attackEffectTime / ImpactDuration;
    const float distance = 12 + 42 * progress;
    const float length = 20 * (1 - progress);
    for (int i = 0; i < 8; ++i) {
        const float angle = 2 * Pi * i / 8;
        const float c = std::cos(angle), sn = std::sin(angle);
        const glm::vec2 position = attackEnd + glm::vec2(c, sn) * distance;
        const glm::mat3 model(length * c, length * sn, 0, -4 * sn, 4 * c, 0,
                              position.x + 2 * sn, position.y - 2 * c, 1);
        RenderMesh2D(meshes.at("tema1_rectangle"), model, i % 2 ? Paper : AttackColor);
    }
}

// Sageata porneste din cartea aleasa si urmareste cursorul in coordonatele scenei.
void Tema1::DrawSelectionArrow()
{
    const bool silence = game.NeedsSilenceTarget();
    if ((!silence && selectedAttackerIndex < 0) || !CanHumanInteract() ||
        !glfwGetWindowAttrib(glfwGetCurrentContext(), GLFW_HOVERED)) {
        return;
    }
    const glm::ivec2 cursor = window->GetCursorPosition();
    glm::vec2 target;
    if (!MouseToScene(cursor.x, cursor.y, target)) {
        return;
    }
    const std::size_t sourceIndex = silence ? boardAnimations[0].positions.size() - 1 : selectedAttackerIndex;
    const glm::vec2 source = boardAnimations[0].positions[sourceIndex] +
                             glm::vec2(CardWidth, CardHeight) / 2.0f;
    const glm::vec2 delta = target - source;
    const float distance = glm::length(delta);
    if (distance < 24) {
        return;
    }
    const glm::vec2 direction = delta / distance;
    const glm::vec2 normal(-direction.y, direction.x);
    const float inset = std::min(48.0f, distance * 0.3f);
    const glm::vec2 start = source + inset * direction;
    const float headLength = std::min(20.0f, (distance - inset) * 0.45f);
    const int targetIndex = FindBoardCard(1, target);
    const bool onHero = IsInside(target, HeroBounds[1]);
    const bool validTarget = silence ? targetIndex >= 0 :
        ((targetIndex >= 0 && game.CanAttack(selectedAttackerIndex, targetIndex)) ||
         (onHero && game.CanAttack(selectedAttackerIndex, -1)));
    const glm::vec3 color = validTarget ? ReadyColor :
        ((targetIndex >= 0 || onHero) ? HealthColor : (silence ? ManaColor : AttackColor));
    // Rama inchisa pastreaza sageata vizibila si peste carti deschise la culoare.
    for (int border = 1; border >= 0; --border) {
        const float thickness = border ? 9.0f : 5.0f;
        const float shaftLength = distance - inset - headLength + 2;
        const glm::vec2 origin = start - normal * (thickness / 2);
        const glm::mat3 shaft(shaftLength * direction.x, shaftLength * direction.y, 0,
                              thickness * normal.x, thickness * normal.y, 0, origin.x, origin.y, 1);
        const glm::vec3 tint = border ? glm::vec3(0.12f, 0.08f, 0.05f) : color;
        RenderMesh2D(meshes.at("tema1_rectangle"), shaft, tint);
        const float width = headLength + (border ? 4 : 0);
        const float height = border ? 26.0f : 20.0f;
        const glm::mat3 head(width * direction.x, width * direction.y, 0,
                             height * normal.x, height * normal.y, 0, target.x, target.y, 1);
        RenderMesh2D(meshes.at("tema1_arrowhead"), head, tint);
    }
}

// Aplica aceeasi scalare tuturor pieselor cartii, inclusiv textului si conturului.
void Tema1::DrawCardVisual(const Card& card, const glm::vec2& position, bool faceUp,
                          float scale, const glm::vec3* outline, float rotation)
{
    const glm::vec2 center = position + glm::vec2(CardWidth, CardHeight) / 2.0f;
    const float c = scale * std::cos(rotation), sn = scale * std::sin(rotation);
    cardTransform = glm::mat3(c, sn, 0, -sn, c, 0,
                              center.x - c * center.x + sn * center.y,
                              center.y - sn * center.x - c * center.y, 1);
    DrawCard(card, position.x, position.y, faceUp);
    if (outline) {
        DrawOutline(position, *outline);
    }
    cardTransform = glm::mat3(1);
}

// Converteste pozitia textului din sistemul nostru cu originea jos in sistemul fontului.
void Tema1::DrawText(const std::string& text, float x, float y, float size, const glm::vec3& color)
{
    // Transforma proiectia, astfel incat si fiecare litera sa se roteasca impreuna cu cartea.
    const glm::mat4 model(cardTransform[0].x, cardTransform[0].y, 0, 0,
                          cardTransform[1].x, cardTransform[1].y, 0, 0,
                          0, 0, 1, 0, cardTransform[2].x, cardTransform[2].y, 0, 1);
    const glm::mat4 flip(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, SceneHeight, 0, 1);
    const glm::mat4 projection = glm::ortho(0.0f, SceneWidth, SceneHeight, 0.0f) * flip * model * flip;
    textRenderer->m_textShader->Use();
    glUniformMatrix4fv(glGetUniformLocation(textRenderer->m_textShader->program, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));
    textRenderer->RenderText(text, x, SceneHeight - y - size, size / 18.0f, color);
}

// Centreaza textul folosind avansul real al caracterelor din fontul incarcat.
void Tema1::DrawCenteredText(const std::string& text, const glm::vec2& center, float size, const glm::vec3& color)
{
    float width = 0;
    for (char character : text) {
        width += (textRenderer->Characters.at(character).Advance >> 6) * size / 18.0f;
    }
    DrawText(text, center.x - width / 2, center.y - size / 2 - 3, size, color);
}

// Deseneaza simbolul statisticii cu margine si valoare numerica lizibila.
void Tema1::DrawStatBadge(int value, const glm::vec2& center, float radius, const glm::vec3& color,
                          const std::string& symbol)
{
    if (symbol == "tema1_sword") {
        // Doua sabii incrucisate, cu lama argintie si maner bronz, ca in referinta.
        const float size = radius * 2.35f;
        for (float angle : {-0.62f, 0.62f}) {
            const float c = std::cos(angle), sn = std::sin(angle);
            const glm::mat3 model(size * c, size * sn, 0, -size * sn, size * c, 0,
                                  center.x, center.y, 1);
            RenderMesh2D(meshes.at("tema1_sword"), cardTransform * model, glm::vec3(0.10f, 0.09f, 0.075f));
            RenderMesh2D(meshes.at("tema1_sword_blade"), cardTransform * model, glm::vec3(0.72f, 0.76f, 0.77f));
            RenderMesh2D(meshes.at("tema1_sword_edge"), cardTransform * model, glm::vec3(0.94f, 0.94f, 0.87f));
            RenderMesh2D(meshes.at("tema1_sword_hilt"), cardTransform * model, glm::vec3(0.62f, 0.46f, 0.23f));
            RenderMesh2D(meshes.at("tema1_sword_pommel"), cardTransform * model, glm::vec3(0.62f, 0.46f, 0.23f));
            RenderMesh2D(meshes.at("tema1_sword_grip"), cardTransform * model, glm::vec3(0.26f, 0.20f, 0.14f));
        }
    } else {
        DrawShape(symbol, center.x - radius - 2, center.y - radius - 2,
                  2 * radius + 4, 2 * radius + 4, glm::vec3(0.12f, 0.08f, 0.05f));
        DrawShape(symbol, center.x - radius, center.y - radius, 2 * radius, 2 * radius, Border);
        DrawShape(symbol, center.x - radius + 2, center.y - radius + 2,
                  2 * radius - 4, 2 * radius - 4, color * 0.85f);
        if (symbol == "tema1_heart") {
            DrawShape("tema1_circle", center.x - radius * 0.52f, center.y + radius * 0.37f,
                      radius * 0.22f, radius * 0.15f, glm::vec3(1.0f, 0.62f, 0.61f));
        }
    }
    const std::string number = std::to_string(value);
    const float size = radius * (number.size() > 1 ? 1.05f : 1.35f);
    for (const glm::vec2& offset : {glm::vec2(-1.5f, 0), glm::vec2(1.5f, 0),
                                    glm::vec2(0, -1.5f), glm::vec2(0, 1.5f)}) {
        DrawCenteredText(number, center + offset, size, glm::vec3(0.10f, 0.07f, 0.05f));
    }
    DrawCenteredText(number, center, size, glm::vec3(1));
}

// Construieste o carte compacta, cu simbol central si costul de mana in coltul superior.
void Tema1::DrawCard(const Card& card, float x, float y, bool faceUp)
{
    if (!faceUp) {
        DrawCardBack(x, y);
        return;
    }
    glm::vec3 color;
    std::string symbol;
    std::string title;
    switch (card.GetType()) {
        case CardType::Triangle:
            color = glm::vec3(0.61f, 0.20f, 0.19f); symbol = "tema1_triangle"; title = "LUPTATOR"; break;
        case CardType::Diamond:
            color = glm::vec3(0.16f, 0.35f, 0.57f); symbol = "tema1_diamond"; title = "MAG"; break;
        case CardType::Square:
            color = glm::vec3(0.20f, 0.43f, 0.30f); symbol = "tema1_rectangle"; title = "GARDA"; break;
    }
    DrawRectangle(x, y, CardWidth, CardHeight, Border);
    DrawRectangle(x + 3, y + 3, CardWidth - 6, CardHeight - 6, glm::vec3(0.28f, 0.19f, 0.12f));
    DrawRectangle(x + 7, y + 7, CardWidth - 14, CardHeight - 14, glm::vec3(0.74f, 0.61f, 0.40f));
    DrawShape("tema1_circle", x + 10, y + 40, 84, 82, glm::vec3(0.21f, 0.14f, 0.09f));
    DrawShape("tema1_circle", x + 14, y + 44, 76, 74, color);
    DrawShape(symbol, x + 32, y + 62, 40, 40, Paper);
    DrawCenteredText(title, glm::vec2(x + 61, y + 128), 9, glm::vec3(0.19f, 0.12f, 0.07f));
    std::string effects;
    if (card.HasEffect(CardEffect::Lifesteal)) effects += "LIFESTEAL ";
    if (card.HasEffect(CardEffect::Charge)) effects += "CHARGE ";
    if (card.HasEffect(CardEffect::Tank)) effects += "TANK ";
    if (card.HasEffect(CardEffect::Silence)) effects += "SILENCE ";
    if (card.IsSilenced()) effects = "SILENCED";
    if (!effects.empty()) {
        DrawRectangle(x + 7, y + 28, CardWidth - 14, 15, CardBack);
        DrawCenteredText(effects, glm::vec2(x + CardWidth / 2, y + 36),
                         effects.size() > 12 ? 6.5f : 8, card.IsSilenced() ? Border : Paper);
    }
    DrawProperties(card, x, y);
}

// Atacul este in stanga-jos, viata in dreapta-jos, iar mana in stanga-sus.
void Tema1::DrawProperties(const Card& card, float x, float y)
{
    DrawStatBadge(card.GetAttack(), glm::vec2(x + 10, y + 16), 17, AttackColor, "tema1_sword");
    DrawStatBadge(card.GetHealth(), glm::vec2(x + CardWidth - 10, y + 16), 17, HealthColor, "tema1_heart");
    DrawStatBadge(card.GetManaCost(), glm::vec2(x + 10, y + CardHeight - 12), 14, ManaColor, "tema1_circle");
}

// Numarul straturilor scade la ultimele carti, iar pachetul gol nu mai deseneaza carti.
void Tema1::DrawDeck(int ownerIndex, float x, float y)
{
    const std::size_t layers = std::min(game.GetPlayer(ownerIndex).GetDeck().size(), std::size_t(3));
    for (std::size_t i = 0; i < layers; ++i) {
        DrawCardBack(x + i * 5, y + i * 5);
    }
}

// Marcheaza o carte fara a modifica mesh-ul sau datele ei.
void Tema1::DrawOutline(const glm::vec2& position, const glm::vec3& color)
{
    DrawRectangle(position.x, position.y, CardWidth, 3, color);
    DrawRectangle(position.x, position.y + CardHeight - 3, CardWidth, 3, color);
    DrawRectangle(position.x, position.y, 3, CardHeight, color);
    DrawRectangle(position.x + CardWidth - 3, position.y, 3, CardHeight, color);
}

// Creeaza forme unitare albe, colorate si dimensionate ulterior la desenare
void Tema1::CreateMeshes()
{
    const glm::vec3 white(1);
    Mesh* rectangle = new Mesh("tema1_rectangle");
    rectangle->InitFromData({
        VertexFormat(glm::vec3(0, 0, 0), white),
        VertexFormat(glm::vec3(1, 0, 0), white),
        VertexFormat(glm::vec3(1, 1, 0), white),
        VertexFormat(glm::vec3(0, 1, 0), white)
    }, {0, 1, 2, 0, 2, 3});
    AddMeshToList(rectangle);

    Mesh* triangle = new Mesh("tema1_triangle");
    triangle->InitFromData({
        VertexFormat(glm::vec3(0, 0, 0), white),
        VertexFormat(glm::vec3(1, 0, 0), white),
        VertexFormat(glm::vec3(0.5f, 1, 0), white)
    }, {0, 1, 2});
    AddMeshToList(triangle);

    Mesh* diamond = new Mesh("tema1_diamond");
    diamond->InitFromData({
        VertexFormat(glm::vec3(0.5f, 0, 0), white),
        VertexFormat(glm::vec3(1, 0.5f, 0), white),
        VertexFormat(glm::vec3(0.5f, 1, 0), white),
        VertexFormat(glm::vec3(0, 0.5f, 0), white)
    }, {0, 1, 2, 0, 2, 3});
    AddMeshToList(diamond);

    // Un singur cerc unitar este reutilizat pentru toate bulinele si portretele.
    std::vector<VertexFormat> circleVertices = {VertexFormat(glm::vec3(0.5f, 0.5f, 0), white)};
    std::vector<unsigned int> circleIndices;
    const unsigned int segments = 96;
    for (unsigned int i = 0; i < segments; ++i) {
        const float angle = 2 * 3.14159265f * i / segments;
        circleVertices.push_back(VertexFormat(glm::vec3(0.5f + 0.5f * std::cos(angle),
                                                        0.5f + 0.5f * std::sin(angle), 0), white));
        circleIndices.push_back(0);
        circleIndices.push_back(i + 1);
        circleIndices.push_back((i + 1) % segments + 1);
    }
    Mesh* circle = new Mesh("tema1_circle");
    circle->InitFromData(circleVertices, circleIndices);
    AddMeshToList(circle);

    // Inima clasica parametrica: doi lobi rotunzi si un varf ascutit.
    // x = 16 sin^3(t); y = 13 cos(t)-5 cos(2t)-2 cos(3t)-cos(4t).
    std::vector<VertexFormat> heartVertices = {VertexFormat(glm::vec3(0.5f, 0.48f, 0), white)};
    std::vector<unsigned int> heartIndices;
    const unsigned int heartSegments = 128;
    for (unsigned int i = 0; i <= heartSegments; ++i) {
        const float t = 2 * Pi * i / heartSegments;
        const float x = 16 * std::pow(std::sin(t), 3);
        const float y = 13 * std::cos(t) - 5 * std::cos(2 * t) -
                        2 * std::cos(3 * t) - std::cos(4 * t);
        heartVertices.push_back(VertexFormat(glm::vec3(0.5f + x / 32, (y + 17) / 29, 0), white));
        if (i > 0) {
            heartIndices.insert(heartIndices.end(), {0, i + 1, i});
        }
    }
    Mesh* heart = new Mesh("tema1_heart");
    heart->InitFromData(heartVertices, heartIndices);
    AddMeshToList(heart);

    // Piese convexe ale sabiei, in coordonate locale centrate pentru rotatie.
    auto addPolygon = [this, &white](const std::string& name, const std::vector<glm::vec2>& points) {
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;
        for (const glm::vec2& point : points) {
            vertices.push_back(VertexFormat(glm::vec3(point, 0), white));
        }
        float area = 0;
        for (std::size_t i = 0; i < points.size(); ++i) {
            const glm::vec2& a = points[i];
            const glm::vec2& b = points[(i + 1) % points.size()];
            area += a.x * b.y - b.x * a.y;
        }
        for (unsigned int i = 1; i + 1 < points.size(); ++i) {
            indices.insert(indices.end(), {0, area > 0 ? i : i + 1, area > 0 ? i + 1 : i});
        }
        Mesh* mesh = new Mesh(name);
        mesh->InitFromData(vertices, indices);
        AddMeshToList(mesh);
    };
    addPolygon("tema1_arrowhead", {{0, 0}, {-1, 0.5f}, {-1, -0.5f}});
    // Semicercuri cu centrul in origine pentru ruleta de inceput de meci.
    for (int owner = 0; owner < 2; ++owner) {
        std::vector<glm::vec2> sector;
        for (int i = 0; i <= 80; ++i) {
            const float angle = owner * Pi + Pi * i / 80;
            sector.push_back(glm::vec2(std::cos(angle), std::sin(angle)));
        }
        addPolygon(owner == 0 ? "tema1_spinner_human" : "tema1_spinner_ai", sector);
    }
    // Conturul este compus din aceeasi lama si o garda lata; piesele interioare il pastreaza vizibil.
    addPolygon("tema1_sword", {{-0.12f, -0.18f}, {0.12f, -0.18f}, {0.11f, 0.34f},
                               {0, 0.53f}, {-0.11f, 0.34f}});
    addPolygon("tema1_sword_blade", {{-0.08f, -0.13f}, {0.08f, -0.13f}, {0.075f, 0.33f},
                                     {0, 0.47f}, {-0.075f, 0.33f}});
    addPolygon("tema1_sword_edge", {{-0.08f, -0.13f}, {0, -0.13f}, {0, 0.47f}, {-0.075f, 0.33f}});
    addPolygon("tema1_sword_hilt", {{-0.24f, -0.23f}, {-0.20f, -0.15f}, {0.20f, -0.15f},
                                    {0.24f, -0.23f}, {0.18f, -0.27f}, {-0.18f, -0.27f}});
    addPolygon("tema1_sword_grip", {{-0.065f, -0.44f}, {0.065f, -0.44f},
                                    {0.065f, -0.23f}, {-0.065f, -0.23f}});

    addPolygon("tema1_sword_pommel", {{-0.10f, -0.47f}, {0, -0.53f},
                                      {0.10f, -0.47f}, {0.075f, -0.40f}, {-0.075f, -0.40f}});

    // Un segment de inel reutilizat prin rotatie; nu realoca buffere in fiecare cadru.
    const float halfStep = Pi / TimerSegments;
    const float sx = std::sin(halfStep), sy = std::cos(halfStep);
    addPolygon("tema1_timer_segment", {{-sx, sy}, {-0.77f * sx, 0.77f * sy},
                                       {0.77f * sx, 0.77f * sy}, {sx, sy}});

    // Colturi tesite pentru rama tablei, fara texturi sau mesh-uri externe.
    Mesh* board = new Mesh("tema1_board");
    board->InitFromData({
        VertexFormat(glm::vec3(0.5f, 0.5f, 0), white),
        VertexFormat(glm::vec3(0.06f, 0, 0), white),
        VertexFormat(glm::vec3(0.94f, 0, 0), white),
        VertexFormat(glm::vec3(1, 0.10f, 0), white),
        VertexFormat(glm::vec3(1, 0.90f, 0), white),
        VertexFormat(glm::vec3(0.94f, 1, 0), white),
        VertexFormat(glm::vec3(0.06f, 1, 0), white),
        VertexFormat(glm::vec3(0, 0.90f, 0), white),
        VertexFormat(glm::vec3(0, 0.10f, 0), white)
    }, {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1});
    AddMeshToList(board);
}


// Pastreaza toate cartile ascunse identice si adapteaza modelul la noua dimensiune.
void Tema1::DrawCardBack(float x, float y)
{
    DrawRectangle(x, y, CardWidth, CardHeight, Border);
    DrawRectangle(x + 3, y + 3, CardWidth - 6, CardHeight - 6, glm::vec3(0.30f, 0.14f, 0.11f));
    DrawRectangle(x + 10, y + 10, CardWidth - 20, CardHeight - 20, Border);
    DrawRectangle(x + 12, y + 12, CardWidth - 24, CardHeight - 24, CardBack);
    DrawShape("tema1_diamond", x + 27, y + 36, 50, 68, Border);
    DrawShape("tema1_diamond", x + 35, y + 47, 34, 46, glm::vec3(0.25f, 0.36f, 0.40f));
}

// Simplifica desenarea unui dreptunghi prin reutilizarea mesh-ului unitar
void Tema1::DrawRectangle(float x, float y, float width, float height, const glm::vec3& color)
{
    DrawShape("tema1_rectangle", x, y, width, height, color);
}

// Aplica scalarea si translatia formei unitare, apoi foloseste shaderul Color existent
void Tema1::DrawShape(const std::string& meshName, float x, float y,
                      float width, float height, const glm::vec3& color)
{
    const glm::mat3 modelMatrix(
        width, 0, 0,
        0, height, 0,
        x, y, 1);
    RenderMesh2D(meshes.at(meshName), cardTransform * modelMatrix, color);
}

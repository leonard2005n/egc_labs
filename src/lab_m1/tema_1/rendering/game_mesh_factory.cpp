#include "lab_m1/tema_1/rendering/game_mesh_factory.h"
#include "lab_m1/tema_1/presentation/game_layout.h"
#include "lab_m1/tema_1/rendering/game_style.h"
#include "lab_m1/tema_1/rendering/mesh_data.h"

#include <cmath>
#include <vector>

using namespace m1;
using namespace m1::game_view;

void GameMeshFactory::Create(const MeshReceiver& addMesh)
{
    CreateBasicShapes(addMesh);
    CreateCircle(addMesh);
    CreateHeart(addMesh);
    CreateSymbols(addMesh);
    CreateBoard(addMesh);
}

void GameMeshFactory::CreateBasicShapes(const MeshReceiver& addMesh)
{
    const glm::vec3 white(1);
    Mesh* rectangle = new Mesh("tema1_rectangle");
    rectangle->InitFromData({
        VertexFormat(glm::vec3(0, 0, 0), white),
        VertexFormat(glm::vec3(1, 0, 0), white),
        VertexFormat(glm::vec3(1, 1, 0), white),
        VertexFormat(glm::vec3(0, 1, 0), white)
    }, {0, 1, 2, 0, 2, 3});
    addMesh(rectangle);

    Mesh* triangle = new Mesh("tema1_triangle");
    triangle->InitFromData({
        VertexFormat(glm::vec3(0, 0, 0), white),
        VertexFormat(glm::vec3(1, 0, 0), white),
        VertexFormat(glm::vec3(0.5f, 1, 0), white)
    }, {0, 1, 2});
    addMesh(triangle);

    Mesh* diamond = new Mesh("tema1_diamond");
    diamond->InitFromData({
        VertexFormat(glm::vec3(0.5f, 0, 0), white),
        VertexFormat(glm::vec3(1, 0.5f, 0), white),
        VertexFormat(glm::vec3(0.5f, 1, 0), white),
        VertexFormat(glm::vec3(0, 0.5f, 0), white)
    }, {0, 1, 2, 0, 2, 3});
    addMesh(diamond);
}

void GameMeshFactory::CreateCircle(const MeshReceiver& addMesh)
{
    const glm::vec3 white(1);
    // Un singur cerc unitar este reutilizat pentru toate bulinele si portretele.
    std::vector<VertexFormat> circleVertices = {VertexFormat(glm::vec3(0.5f, 0.5f, 0), white)};
    std::vector<unsigned int> circleIndices;
    const unsigned int segments = mesh_config::CircleSegments;

    for (unsigned int i = 0; i < segments; ++i) {
        const float angle = 2 * Pi * i / segments;
        circleVertices.push_back(VertexFormat(glm::vec3(0.5f + 0.5f * std::cos(angle),
                                                        0.5f + 0.5f * std::sin(angle), 0), white));
        circleIndices.push_back(0);
        circleIndices.push_back(i + 1);
        circleIndices.push_back((i + 1) % segments + 1);
    }

    Mesh* circle = new Mesh("tema1_circle");
    circle->InitFromData(circleVertices, circleIndices);
    addMesh(circle);
}

void GameMeshFactory::CreateHeart(const MeshReceiver& addMesh)
{
    const glm::vec3 white(1);
    // Inima clasica parametrica: doi lobi rotunzi si un varf ascutit.
    // x = 16 sin^3(t); y = 13 cos(t)-5 cos(2t)-2 cos(3t)-cos(4t).
    std::vector<VertexFormat> heartVertices = {VertexFormat(glm::vec3(0.5f, 0.48f, 0), white)};
    std::vector<unsigned int> heartIndices;
    const unsigned int heartSegments = mesh_config::HeartSegments;

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
    addMesh(heart);
}

void GameMeshFactory::CreateSymbols(const MeshReceiver& addMesh)
{
    const glm::vec3 white(1);
    // Piese convexe ale sabiei, in coordonate locale centrate pentru rotatie.
    auto addPolygon = [&addMesh, &white](const std::string& name, const std::vector<glm::vec2>& points) {
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
        addMesh(mesh);
    };
    addPolygon("tema1_arrowhead", {{0, 0}, {-1, 0.5f}, {-1, -0.5f}});
    // Semicercuri cu centrul in origine pentru ruleta de inceput de meci.
    for (int owner = 0; owner < 2; ++owner) {
        std::vector<glm::vec2> sector;

        for (int i = 0; i <= mesh_config::SpinnerSectorSegments; ++i) {
            const float angle = owner * Pi + Pi * i / mesh_config::SpinnerSectorSegments;
            sector.push_back(glm::vec2(std::cos(angle), std::sin(angle)));
        }

        addPolygon(owner == 0 ? "tema1_spinner_human" : "tema1_spinner_ai", sector);
    }

    // Datele pieselor sunt folosite si la randare, in aceeasi ordine.
    for (const SwordPart& part : SwordParts()) {
        addPolygon(part.mesh, part.vertices);
    }

    // Acelasi cristal fatetat reprezinta costul cartii si resursele ambilor eroi.
    addPolygon("tema1_mana_crystal", ManaCrystalOutline());

    for (const CrystalFacet& facet : ManaCrystalFacets()) {
        addPolygon(facet.mesh, facet.vertices);
    }

    addPolygon("tema1_mana_glint", ManaCrystalGlint());

    // Un segment de inel reutilizat prin rotatie; nu realoca buffere in fiecare cadru.
    const float halfStep = Pi / TimerSegments;
    const float sx = std::sin(halfStep), sy = std::cos(halfStep);
    addPolygon("tema1_timer_segment", {{-sx, sy}, {-game_style::TimerInnerRatio * sx, game_style::TimerInnerRatio * sy},
                                       {game_style::TimerInnerRatio * sx, game_style::TimerInnerRatio * sy}, {sx, sy}});
}

void GameMeshFactory::CreateBoard(const MeshReceiver& addMesh)
{
    const glm::vec3 white(1);
    // Colturi tesite pentru rama tablei, fara texturi sau mesh-uri externe.
    Mesh* board = new Mesh("tema1_board");
    board->InitFromData({
        VertexFormat(glm::vec3(0.5f, 0.5f, 0), white),
        VertexFormat(glm::vec3(mesh_config::BoardBevelX, 0, 0), white),
        VertexFormat(glm::vec3(1 - mesh_config::BoardBevelX, 0, 0), white),
        VertexFormat(glm::vec3(1, mesh_config::BoardBevelY, 0), white),
        VertexFormat(glm::vec3(1, 1 - mesh_config::BoardBevelY, 0), white),
        VertexFormat(glm::vec3(1 - mesh_config::BoardBevelX, 1, 0), white),
        VertexFormat(glm::vec3(mesh_config::BoardBevelX, 1, 0), white),
        VertexFormat(glm::vec3(0, 1 - mesh_config::BoardBevelY, 0), white),
        VertexFormat(glm::vec3(0, mesh_config::BoardBevelY, 0), white)
    }, {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1});
    addMesh(board);
}

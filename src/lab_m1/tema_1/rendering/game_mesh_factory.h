#pragma once

#include <functional>

#include "core/gpu/mesh.h"

namespace m1
{
    // Creeaza mesh-uri unitare reutilizabile; receptorul preia proprietatea lor.
    class GameMeshFactory
    {
     public:
        using MeshReceiver = std::function<void(Mesh*)>;

        static void Create(const MeshReceiver& addMesh);

     private:
        static void CreateBasicShapes(const MeshReceiver& addMesh);
        static void CreateCircle(const MeshReceiver& addMesh);
        static void CreateHeart(const MeshReceiver& addMesh);
        static void CreateSymbols(const MeshReceiver& addMesh);
        static void CreateBoard(const MeshReceiver& addMesh);
    };
}

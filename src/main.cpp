

/*******************************************************************************************
*
*   raylib [models] example - mesh generation
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.8, last time updated with raylib 4.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "base/include/box.h"
#include "base/include/implicits.h"
#include "base/include/mesh.h"

#define NUM_MODELS  9               // Parametric 3d shapes to generate

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static Mesh GenMeshCustom(TMesh& tmesh);    // Generate a simple triangle mesh from code

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");

    // We generate a checked image for texturing
    Image checked = GenImageChecked(2, 2, 1, 1, RED, GREEN);
    Texture2D texture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    AnalyticScalarField implicit;
    TMesh mesh;
    implicit.Polygonize(31, mesh, Box(2.0));
    Model models = LoadModelFromMesh(GenMeshCustom(mesh));

    // NOTE: Generated meshes could be exported using ExportMesh()

    // Set checked texture as default diffuse component for all models material
    models.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Define the camera to look into our 3d world
    Camera camera = { { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };

    int currentModel = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

               DrawModel(models, position, 1.0f, WHITE);
               DrawGrid(10, 1.0);

            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture

    // Unload models data (GPU VRAM)
    UnloadModel(models);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Generate a simple triangle mesh from code
static Mesh GenMeshCustom(TMesh& tmesh)
{
    Mesh mesh = { 0 };

    // Expand the indexed TMesh into one raylib vertex for each triangle corner.
    // This keeps the position and normal for each corner aligned and avoids
    // needing to convert TMesh's separate vertex/normal indices to one index.
    mesh.triangleCount = tmesh.Triangles();
    mesh.vertexCount = mesh.triangleCount * 3;

    if (mesh.vertexCount == 0) return mesh;

    mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    for (int tri = 0; tri < mesh.triangleCount; ++tri)
    {
        for (int corner = 0; corner < 3; ++corner)
        {
            const int dst = tri * 3 + corner;
            const Vector p = tmesh.Vertex(tri, corner);
            const Vector n = tmesh.Normal(tmesh.NormalIndex(tri, corner));

            // Positions: xyz
            mesh.vertices[dst * 3 + 0] = (float)p[0];
            mesh.vertices[dst * 3 + 1] = (float)p[2];
            mesh.vertices[dst * 3 + 2] = (float)p[1];

            // Normals: xyz
            mesh.normals[dst * 3 + 0] = (float)n[0];
            mesh.normals[dst * 3 + 1] = (float)n[2];
            mesh.normals[dst * 3 + 2] = (float)n[1];

            // Basic planar UVs. Replace this mapping if the mesh needs a
            // specific texture layout.
            mesh.texcoords[dst * 2 + 0] = (float)p[0] * 0.5f + 0.5f;
            mesh.texcoords[dst * 2 + 1] = (float)p[2] * 0.5f + 0.5f;
        }
    }

    // No index buffer is needed because each triangle has its own 3 vertices.
    // Upload the CPU-side arrays to the GPU.
    UploadMesh(&mesh, false);

    return mesh;
}



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
#include "rcamera.h"
#include "../include/rlights.h"
#include "raymath.h"

#define NUM_MODELS  9               // Parametric 3d shapes to generate
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

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
    Image checked = GenImageChecked(2, 2, 1, 1, BLUE, PURPLE);
    Texture2D texture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    AnalyticScalarField implicit;
    TMesh mesh;
    implicit.Polygonize(31, mesh, Box(2.0));
    Model models = LoadModelFromMesh(GenMeshCustom(mesh));
    models.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // NOTE: Generated meshes could be exported using ExportMesh()

    
    // // Load lighting shader
    // Shader shader = LoadShader(TextFormat("res/shaders/sahder2BG_vert.glsl", GLSL_VERSION),
    //                            TextFormat("res/shaders/sahder2BG_frag.glsl", GLSL_VERSION));
    // // Get shader locations
    // shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvpMatrix");
    // shader.locs[SHADER_LOC_VERTEX_NORMAL] = GetShaderLocation(shader, "v_normal");
    // shader.locs[SHADER_LOC_VERTEX_COLOR] = GetShaderLocation(shader, "color");

    // //Set shader value: light source
    // int lightDirLoc = GetShaderLocation(shader, "lightDir");
    // SetShaderValue(shader, lightDirLoc, (float[4]){ 0.0f, 0.0f, -1.0f, 0.0f }, SHADER_UNIFORM_VEC4);


    // // Load default material (using raylib intenral default shader) for non-instanced mesh drawing
    // // WARNING: Default shader enables vertex color attribute BUT GenMeshCube() does not generate vertex colors, so,
    // // when drawing the color attribute is disabled and a default color value is provided as input for thevertex attribute
    // models.materials[0].shader = shader;
    // // Set checked texture as default diffuse component for all models material
  
    // models.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;

    DisableCursor();                    // Limit cursor to relative movement inside the window
    
    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE))
        {
            cameraMode = CAMERA_THIRD_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            cameraMode = CAMERA_ORBITAL;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        // Switch camera projection
        if (IsKeyPressed(KEY_P))
        {
            if (camera.projection == CAMERA_PERSPECTIVE)
            {
                // Create isometric view
                cameraMode = CAMERA_THIRD_PERSON;
                // Note: The target distance is related to the render distance in the orthographic projection
                camera.position = (Vector3){ 0.0f, 2.0f, -100.0f };
                camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                CameraYaw(&camera, -135*DEG2RAD, true);
                CameraPitch(&camera, -45*DEG2RAD, true, true, false);
            }
            else if (camera.projection == CAMERA_ORTHOGRAPHIC)
            {
                // Reset to default view
                cameraMode = CAMERA_THIRD_PERSON;
                camera.position = (Vector3){ 0.0f, 2.0f, 10.0f };
                camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera.projection = CAMERA_PERSPECTIVE;
                camera.fovy = 60.0f;
            }
        }

        // Update camera computes movement internally depending on the camera mode
        // Some default standard keyboard/mouse inputs are hardcoded to simplify use
        // For advanced camera controls, it's recommended to compute camera movement manually
        UpdateCamera(&camera, cameraMode);                  // Update camera

       
    

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
            mesh.vertices[dst * 3 + 1] = (float)p[1];
            mesh.vertices[dst * 3 + 2] = (float)p[2];

            // Normals: xyz
            mesh.normals[dst * 3 + 0] = (float)n[0];
            mesh.normals[dst * 3 + 1] = (float)n[1];
            mesh.normals[dst * 3 + 2] = (float)n[2];

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

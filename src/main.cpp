

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
#include "base/include/mathematics.h"
#include "blobs/include/OperatorBlob.h"
#include "raylib.h"
#include "base/include/box.h"
#include "base/include/implicits.h"
#include "base/include/mesh.h"
#include "rcamera.h"
#include "../include/rlights.h"
#include "raymath.h"
#include "blobs/include/Blob.h"
#include "blobs/include/BlobParser.h"
#include "gui/include/TreeEditor.h"
#include <exception>
#include <string>


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
    SetTraceLogLevel(LOG_ALL);
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1600;
    const int screenHeight = 900;

    // HIGHDPI renders at the display's native resolution (sharp on Retina),
    // MSAA smooths mesh edges, RESIZABLE lets you drag the window bigger.
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");

    // We generate a checked image for texturing
    Image checked = GenImageChecked(2, 2, 1, 1, BLUE, PURPLE);
    Texture2D texture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    Shader shader = LoadShader("res/shaders/shader2BG_vert.glsl", "res/shaders/shader2BG_frag.glsl");
    printf("Shader ID: %u\n", shader.id);


    // Vertex attributes
    shader.locs[SHADER_LOC_VERTEX_POSITION] =
        GetShaderLocationAttrib(shader, "position");

    shader.locs[SHADER_LOC_VERTEX_NORMAL] =
        GetShaderLocationAttrib(shader, "normal");

    // Matrices
    shader.locs[SHADER_LOC_MATRIX_MVP] =
        GetShaderLocation(shader, "mvpMatrix");

    shader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocation(shader, "matModel");

    // Material color
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] =
        GetShaderLocation(shader, "colDiffuse");

    // Blob tree editor: the text is parsed into a tree and polygonized on "Generate"
    TreeEditor editor(
        "union(\n"
        "  capsule(1 0 0, 1 1.5 0, 0.7),\n"
        "  sphere(0 1 0, 0.7)\n"
        ")\n");

    Model model = { 0 };    // empty until the first successful generate

    auto generate = [&]() {
        try {
            std::unique_ptr<Node> root = ParseBlobTree(editor.GetText());

            AnalyticScalarField implicit(root.get());
            TMesh mesh;
            implicit.Polygonize(64, mesh, Box(4.0));

            if (mesh.Triangles() == 0) {
                editor.SetStatus("No surface inside the box [-2, 2]^3", true);
                return;
            }

            // UnloadModel frees the mesh and material maps but not the shared shader
            UnloadModel(model);
            model = LoadModelFromMesh(GenMeshCustom(mesh));
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = PURPLE;
            model.materials[0].shader = shader;

            editor.SetStatus("Generated " + std::to_string(mesh.Triangles()) + " triangles", false);
        }
        catch (const std::exception& e) {
            editor.SetStatus(e.what(), true);
        }
        catch (const char* e) {     // unimplemented operator methods throw C strings
            editor.SetStatus(e, true);
        }
    };
    editor.LoadFont();
    generate();


    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;

    // The editor starts focused, so the cursor stays free until the user clicks the 3D view
    
    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Two input modes:
        //  - editing: cursor is free, keys go to the text box, camera is still
        //  - camera:  cursor is captured, WASD + mouse move the camera
        // Clicking the 3D view (or TAB) enters camera mode; TAB returns to the editor.
        bool toCamera = editor.focused &&
            (IsKeyPressed(KEY_TAB) ||
             (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !editor.Contains(GetMousePosition())));
        bool toEditor = !editor.focused && IsKeyPressed(KEY_TAB);

        if (toCamera)
        {
            editor.focused = false;
            DisableCursor();
        }
        else if (toEditor)
        {
            editor.focused = true;
            EnableCursor();
        }

        if (editor.focused)
        {
            if (editor.Update()) generate();
        }
        else
        {
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
        }

       
    

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

               DrawModel(model, position, 1.0f, WHITE);
               DrawGrid(10, 1.0);

            EndMode3D();

            editor.Draw();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture

    // Unload models data (GPU VRAM)
    UnloadModel(model);
    UnloadShader(shader);
    editor.UnloadFont();

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

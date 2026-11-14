

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
#include <fstream>
#include <sstream>


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

// The editor text is saved here on exit and restored on the next start
// (relative to the working directory, i.e. the project root when run as ./app).
static const char* kSavedTreePath = "saved_tree.txt";

// Expands a leading "~/" to the home directory.
static std::string ExpandPath(const std::string& path)
{
    if (path.rfind("~/", 0) == 0)
        if (const char* home = getenv("HOME")) return std::string(home) + path.substr(1);
    return path;
}

static bool LoadTextFile(const std::string& path, std::string& text)
{
    std::ifstream in(ExpandPath(path));
    if (!in) return false;
    std::stringstream ss;
    ss << in.rdbuf();
    text = ss.str();
    return true;
}

static bool SaveTextFile(const std::string& path, const std::string& text)
{
    std::ofstream out(ExpandPath(path));
    if (!out) return false;
    out << text;
    return (bool)out;
}

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

    // ESC toggles between the editor and the camera instead of quitting;
    // close the window (or Cmd+Q) to quit.
    SetExitKey(KEY_NULL);

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
    // Restore the text from the last session, or start from a default tree
    std::string initialTree =
        "union(\n"
        "  capsule(1 0 0, 1 1.5 0, 0.7),\n"
        "  sphere(0 1 0, 0.7)\n"
        ")\n";
    LoadTextFile(kSavedTreePath, initialTree);
    TreeEditor editor(initialTree);

    Model model = { 0 };    // empty until the first successful generate

    // Returns true if a new mesh was built
    auto generate = [&]() -> bool {
        try {
            std::unique_ptr<Node> root = ParseBlobTree(editor.GetText());

            AnalyticScalarField implicit(root.get());
            TMesh mesh;
            implicit.Polygonize(64, mesh, Box(4.0));

            if (mesh.Triangles() == 0) {
                editor.SetStatus("No surface inside the box [-4, 4]^3", true);
                return false;
            }

            // UnloadModel frees the mesh and material maps but not the shared shader
            UnloadModel(model);
            model = LoadModelFromMesh(GenMeshCustom(mesh));
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = PURPLE;
            model.materials[0].shader = shader;

            editor.SetStatus("Generated " + std::to_string(mesh.Triangles()) + " triangles", false);
            return true;
        }
        catch (const std::exception& e) {
            editor.SetStatus(e.what(), true);
        }
        catch (const char* e) {     // unimplemented operator methods throw C strings
            editor.SetStatus(e, true);
        }
        return false;
    };

    // Loads a tree file into the editor and generates it
    auto loadFile = [&](const std::string& path) {
        std::string text;
        if (!LoadTextFile(path, text)) {
            editor.SetStatus("Could not open " + path, true);
            return;
        }
        editor.SetText(text);
        editor.SetFilePath(path);
        // Keep generate's error message if the file doesn't parse
        if (generate()) editor.SetStatus(std::string("Loaded ") + GetFileName(path.c_str()), false);
    };

    auto saveFile = [&](const std::string& path) {
        if (SaveTextFile(path, editor.GetText())) {
            editor.SetFilePath(path);
            editor.SetStatus(std::string("Saved ") + GetFileName(path.c_str()), false);
        } else {
            editor.SetStatus("Could not save to " + path, true);
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
    while (!WindowShouldClose())    // Detect window close button (ESC is not the exit key)
    {
        // Update
        //----------------------------------------------------------------------------------
        // Two input modes:
        //  - editing: cursor is free, keys go to the text box, camera is still
        //  - camera:  cursor is captured, WASD + mouse move the camera
        // Clicking the 3D view (or ESC) enters camera mode; ESC returns to the editor.
        // While the file prompt is open, ESC belongs to it (cancel).
        bool toCamera = editor.focused &&
            ((IsKeyPressed(KEY_ESCAPE) && !editor.PromptOpen()) ||
             (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !editor.Contains(GetMousePosition())));
        bool toEditor = !editor.focused && IsKeyPressed(KEY_ESCAPE);

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

        // A file dropped on the window is loaded, whatever the mode
        if (IsFileDropped())
        {
            FilePathList dropped = LoadDroppedFiles();
            if (dropped.count > 0) loadFile(dropped.paths[0]);
            UnloadDroppedFiles(dropped);
        }

        if (editor.focused)
        {
            switch (editor.Update())
            {
                case EditorAction::Generate: generate(); break;
                case EditorAction::Load:     loadFile(editor.ActionPath()); break;
                case EditorAction::Save:     saveFile(editor.ActionPath()); break;
                case EditorAction::None:     break;
            }
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
    if (!SaveTextFile(kSavedTreePath, editor.GetText()))
        TraceLog(LOG_WARNING, "Could not save the tree to %s", kSavedTreePath);

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

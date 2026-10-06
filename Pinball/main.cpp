#include "raylib.h"
#include <box2d/box2d.h>

int main()
{
    InitWindow(800, 600, "Pinball");
    SetTargetFPS(60);

    b2Vec2 gravity(0.0f, 10.0f);
    b2World world(gravity);

    while (!WindowShouldClose())
    {
        world.Step(1.0f / 60.0f, 8, 3);
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
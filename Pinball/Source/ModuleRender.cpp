#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include <math.h>

ModuleRender::ModuleRender(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    background = RAYWHITE;
}

// Destructor
ModuleRender::~ModuleRender()
{}

// Called before render is available
bool ModuleRender::Init()
{
	LOG("Creating Renderer context");
	bool ret = true;

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRender::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Update: debug camera
update_status ModuleRender::Update()
{
    ClearBackground(background);

    // NOTE: This function setups render batching system for
    // maximum performance, all consecutive Draw() calls are
    // not processed until EndDrawing() is called
    BeginDrawing();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRender::PostUpdate()
{
    // Draw everything in our batch!
    DrawFPS(10, 10);

    EndDrawing();

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRender::CleanUp()
{
	return true;
}

void ModuleRender::SetBackgroundColor(Color color)
{
	background = color;
}

// Draw a texture (or a section of it) with its pivot at (x, y), rotated "angle" degrees around that pivot
bool ModuleRender::Draw(Texture2D texture, int x, int y, const Rectangle* section, double angle, int pivot_x, int pivot_y) const
{
	Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
	if (section != NULL)
		source = *section;

	// Where the pivot ends up on screen
	Rectangle dest = { (float)x + camera.x, (float)y + camera.y, source.width, source.height };

	// Point of the texture we rotate around (relative to its top-left corner)
	Vector2 origin = { (float)pivot_x, (float)pivot_y };

	DrawTexturePro(texture, source, dest, origin, (float)angle, WHITE);

	return true;
}

bool ModuleRender::DrawText(const char * text, int x, int y, Font font, int spacing, Color tint) const
{
    bool ret = true;

    Vector2 position = { (float)x, (float)y };

    DrawTextEx(font, text, position, (float)font.baseSize, (float)spacing, tint);

    return ret;
}

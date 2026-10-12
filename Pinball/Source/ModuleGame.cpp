#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "Entities/Entity.h"
#include "Entities/Wall.h"
#include "TableShapes.h"
#include "Entities/Ball.h"
#include "Entities/Flipper.h"


#include <algorithm>


ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleGame::~ModuleGame()
{
}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading game scene");

	background = LoadTexture("Assets/background.png");
	ball_texture = LoadTexture("Assets/ball.png");
	flipper_left_texture = LoadTexture("Assets/left_flipper.png");
	flipper_right_texture = LoadTexture("Assets/right_flipper.png");

	SetTextureFilter(flipper_left_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(flipper_right_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(ball_texture, TEXTURE_FILTER_BILINEAR);

	// Static walls of the table (size = number of ints, not of points)
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, table_outer, sizeof(table_outer) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, left_rail, sizeof(left_rail) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, inverted_v, sizeof(inverted_v) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, triangle, sizeof(triangle) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, left_flipper_guide, sizeof(left_flipper_guide) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, right_flipper_guide, sizeof(right_flipper_guide) / sizeof(int))));

	// Flippers: hinge (x, y), shape, limits [-25, 25] degrees, start at rest
	b2RevoluteJoint* joint = NULL;

	PhysBody* body = App->physics->CreateFlipper(145, 770, flipper_left_shape, sizeof(flipper_left_shape) / sizeof(int), -25.0f, 25.0f, 25.0f, &joint);
	entities.emplace_back(new Flipper(body, joint, App, flipper_left_texture, 145, 770, 8, 9, KEY_LEFT, true));


	body = App->physics->CreateFlipper(320, 770, flipper_right_shape, sizeof(flipper_right_shape) / sizeof(int), -25.0f, 25.0f, -25.0f, &joint);
	entities.emplace_back(new Flipper(body, joint, App, flipper_right_texture, 320, 770, 71, 9, KEY_RIGHT, false));
	App->renderer->camera.x = App->renderer->camera.y = 0;

	return true;
}

// Unload assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading game scene");

	DestroyAllEntities();
	UnloadTexture(ball_texture);
	UnloadTexture(flipper_left_texture);
	UnloadTexture(flipper_right_texture);
	UnloadTexture(background);


	return true;
}

update_status ModuleGame::Update()
{
	DrawTexture(background, 0, 0, WHITE);

	// Debug: right click spawns a ball at the mouse
	if (App->physics->IsDebug() && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		entities.emplace_back(new Ball(App->physics->CreateCircle(GetMouseX(), GetMouseY(), 10), App, ball_texture));
	}

	for (PhysicEntity* entity : entities)
	{
		entity->Update();
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	// TODO: react to the collisions of the bodies that have this module as listener
}

void ModuleGame::DestroyEntity(PhysicEntity* entity)
{
	entities.erase(std::find(entities.begin(), entities.end(), entity));
	App->physics->DestroyBody(entity->body);
	delete entity;
}

void ModuleGame::DestroyAllEntities()
{
	for (PhysicEntity* entity : entities)
	{
		App->physics->DestroyBody(entity->body);
		delete entity;
	}
	entities.clear();
}
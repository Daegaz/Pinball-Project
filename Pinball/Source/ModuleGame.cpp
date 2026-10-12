#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "Entities/Entity.h"
#include "Entities/Wall.h"
#include "TableShapes.h"

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

	// Static walls of the table (size = number of ints, not of points)
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, table_outer, sizeof(table_outer) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, left_rail, sizeof(left_rail) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, inverted_v, sizeof(inverted_v) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, triangle, sizeof(triangle) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, left_flipper_guide, sizeof(left_flipper_guide) / sizeof(int))));
	entities.emplace_back(new Wall(App->physics->CreateChain(0, 0, right_flipper_guide, sizeof(right_flipper_guide) / sizeof(int))));

	App->renderer->camera.x = App->renderer->camera.y = 0;

	return true;
}

// Unload assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading game scene");

	DestroyAllEntities();
	UnloadTexture(background);

	return true;
}

update_status ModuleGame::Update()
{
	DrawTexture(background, 0, 0, WHITE);

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
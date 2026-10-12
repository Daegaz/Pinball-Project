#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "Entities/Entity.h"

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
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
#include "Entities/Kicker.h"
#include "Entities/DeathZone.h"

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

	App->renderer->camera.x = App->renderer->camera.y = 0;

	// Textures ------------------------------------------------------------
	background = LoadTexture("Assets/background.png");
	ball_texture = LoadTexture("Assets/ball.png");
	flipper_left_texture = LoadTexture("Assets/left_flipper.png");
	flipper_right_texture = LoadTexture("Assets/right_flipper.png");
	kicker_plunger_texture = LoadTexture("Assets/kicker_plunger.png");
	kicker_spring_texture = LoadTexture("Assets/kicker_3.png");

	// Smooth filtering: small sprites look better when rotated or squashed
	SetTextureFilter(ball_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(flipper_left_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(flipper_right_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(kicker_spring_texture, TEXTURE_FILTER_BILINEAR);

	// Static walls of the table (size = number of ints, not of points) -----
	AddEntity(new Wall(App->physics->CreateChain(0, 0, table_outer, sizeof(table_outer) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, left_rail, sizeof(left_rail) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, inverted_v, sizeof(inverted_v) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, triangle, sizeof(triangle) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, left_flipper_guide, sizeof(left_flipper_guide) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, right_flipper_guide, sizeof(right_flipper_guide) / sizeof(int))));

	// Lost ball sensor: the drain between the flippers (x 150..310, bottom of the screen)
	AddEntity(new DeathZone(App->physics->CreateRectangleSensor(230, 885, 80, 10)));

	// Flippers: hinge (x, y), shape, limits [-25, 25] degrees, start at rest
	b2RevoluteJoint* joint = NULL;

	PhysBody* body = App->physics->CreateFlipper(145, 770, flipper_left_shape, sizeof(flipper_left_shape) / sizeof(int), -25.0f, 25.0f, 25.0f, &joint);
	AddEntity(new Flipper(body, joint, App, flipper_left_texture, 145, 770, 8, 9, KEY_LEFT, true));

	body = App->physics->CreateFlipper(320, 770, flipper_right_shape, sizeof(flipper_right_shape) / sizeof(int), -25.0f, 25.0f, -25.0f, &joint);
	AddEntity(new Flipper(body, joint, App, flipper_right_texture, 320, 770, 71, 9, KEY_RIGHT, false));

	// Kicker: plunger 30x8 centered in the lane (x 485), resting at y 636, can go 126 px down
	b2PrismaticJoint* kicker_joint = NULL;
	PhysBody* kicker_body = App->physics->CreateKicker(485, 636, 15, 4, 126, &kicker_joint);
	AddEntity(new Kicker(kicker_body, kicker_joint, App, kicker_plunger_texture, kicker_spring_texture, 485, 840));

	// First ball, resting on the plunger
	SpawnBall();

	return true;
}

// Unload assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading game scene");

	DestroyAllEntities();
	UnloadTexture(kicker_plunger_texture);
	UnloadTexture(kicker_spring_texture);
	UnloadTexture(ball_texture);
	UnloadTexture(flipper_left_texture);
	UnloadTexture(flipper_right_texture);
	UnloadTexture(background);


	return true;
}

update_status ModuleGame::Update()
{
	DrawTexture(background, 0, 0, WHITE);

	// Remove the lost balls here, safely outside the physics step
	if (!lost_balls.empty())
	{
		for (PhysicEntity* ball : lost_balls)
			DestroyEntity(ball);
		lost_balls.clear();

		// No balls left on the table: a new one on the kicker
		if (CountBalls() == 0)
			SpawnBall();
	}

	// Debug: right click spawns a ball at the mouse
	if (App->physics->IsDebug() && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		AddEntity(new Ball(App->physics->CreateCircle(GetMouseX(), GetMouseY(), 10), App, ball_texture));
	}

	for (PhysicEntity* entity : entities)
	{
		entity->Update();
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	// Bodies created without AddEntity have no entity (e.g. the ground)
	if (bodyA->entity == NULL || bodyB->entity == NULL)
		return;

	EntityType type_a = bodyA->entity->GetType();
	EntityType type_b = bodyB->entity->GetType();

	// Ball fell into the drain (only react when A is the ball, so it counts once)
	if (type_a == EntityType::BALL && type_b == EntityType::DEATH_ZONE)
	{
		LOG("Ball lost!");

		// Only mark it: we can't destroy bodies while Box2D is stepping
		if (std::find(lost_balls.begin(), lost_balls.end(), bodyA->entity) == lost_balls.end())
			lost_balls.push_back(bodyA->entity);
	}
}

// Every entity goes through here: link body <-> entity and listen to its collisions
void ModuleGame::AddEntity(PhysicEntity* entity)
{
	entity->body->entity = entity;
	entity->body->listener = this;
	entities.emplace_back(entity);
}

// New ball resting on the kicker
void ModuleGame::SpawnBall()
{
	AddEntity(new Ball(App->physics->CreateCircle(485, 620, 10), App, ball_texture));
}

// How many balls are on the table right now
int ModuleGame::CountBalls() const
{
	int count = 0;
	for (PhysicEntity* entity : entities)
	{
		if (entity->GetType() == EntityType::BALL)
			++count;
	}
	return count;
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
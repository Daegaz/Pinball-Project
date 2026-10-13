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
#include "Entities/Bumper.h"
#include "Entities/Slingshot.h"

#include <algorithm>


ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	state = GameState::PLAYING;
	balls_left = BALLS_PER_ROUND;
	score = 0;
	previous_score = 0;
	highest_score = 0;
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
	slingshot_left_texture = LoadTexture("Assets/slingshot_left.png");
	slingshot_left_hit_texture = LoadTexture("Assets/slingshot_left_hit.png");
	slingshot_right_texture = LoadTexture("Assets/slingshot_right.png");
	slingshot_right_hit_texture = LoadTexture("Assets/slingshot_right_hit.png");

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

	// Bumpers: textures (same order everywhere: pink, blue, orange)
	const char* bumper_colors[3] = { "pink", "blue", "orange" };
	for (int i = 0; i < 3; ++i)
	{
		bumper_textures[i] = LoadTexture(TextFormat("Assets/bumper_%s.png", bumper_colors[i]));
		bumper_light_textures[i] = LoadTexture(TextFormat("Assets/bumper_%s_light.png", bumper_colors[i]));
		bumper_dark_textures[i] = LoadTexture(TextFormat("Assets/bumper_%s_dark.png", bumper_colors[i]));
		SetTextureFilter(bumper_textures[i], TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(bumper_light_textures[i], TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(bumper_dark_textures[i], TEXTURE_FILTER_BILINEAR);
	}

	// Bumpers: static circles at the positions of the reference table
	const int bumper_pos[3][2] = { { 350, 202 }, { 240, 203 }, { 303, 310 } };
	for (int i = 0; i < 3; ++i)
	{
		PhysBody* bumper_body = App->physics->CreateCircle(bumper_pos[i][0], bumper_pos[i][1], BUMPER_RADIUS, false);
		AddEntity(new Bumper(bumper_body, App, bumper_textures[i], bumper_light_textures[i], bumper_dark_textures[i]));
	}

	// Slingshots: chain of their face + the rubber line (the diagonal), ordered so its outer side is on the left of a -> b
	AddEntity(new Slingshot(App->physics->CreateChain(0, 0, slingshot_left_shape, sizeof(slingshot_left_shape) / sizeof(int)),
		App, slingshot_left_texture, slingshot_left_hit_texture, 92, 592, false, 106, 605, 144, 687));
	AddEntity(new Slingshot(App->physics->CreateChain(0, 0, slingshot_right_shape, sizeof(slingshot_right_shape) / sizeof(int)),
		App, slingshot_right_texture, slingshot_right_hit_texture, 321, 592, true, 321, 687, 360, 603));

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
	StartRound();

	return true;
}

// Unload assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading game scene");

	DestroyAllEntities();
	for (int i = 0; i < 3; ++i)
	{
		UnloadTexture(bumper_textures[i]);
		UnloadTexture(bumper_light_textures[i]);
		UnloadTexture(bumper_dark_textures[i]);
	}
	UnloadTexture(slingshot_left_texture);
	UnloadTexture(slingshot_left_hit_texture);
	UnloadTexture(slingshot_right_texture);
	UnloadTexture(slingshot_right_hit_texture);
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

	// Game over: wait a few seconds and start the next round automatically
	if (state == GameState::GAME_OVER && game_over_timer.ReadSec() >= GAME_OVER_SECONDS)
		StartRound();

	// Kicks from bumpers and slingshots (outside the physics step)
	for (const Kick& kick : kicks)
	{
		b2Body* ball = kick.ball->body->body;
		b2Vec2 direction(kick.dir_x, kick.dir_y);
		ball->ApplyLinearImpulseToCenter(kick.speed * ball->GetMass() * direction, true);
	}
	kicks.clear();

	// Remove the lost balls here, safely outside the physics step
	if (!lost_balls.empty())
	{
		for (PhysicEntity* ball : lost_balls)
			DestroyEntity(ball);
		lost_balls.clear();

		// No balls left on the table: one less for this round (only while playing)
		if (CountBalls() == 0 && state == GameState::PLAYING)
		{
			--balls_left;

			if (balls_left > 0)
				SpawnBall();
			else
				EndRound();
		}
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

	DrawUI();

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
	// Ball hits a bumper: points, flash and a kick away from its center
	if (type_a == EntityType::BALL && type_b == EntityType::BUMPER)
	{
		score += BUMPER_POINTS;
		bodyB->entity->OnHit(bodyA->entity);

		b2Vec2 direction = bodyA->body->GetPosition() - bodyB->body->GetPosition();
		direction.Normalize();
		kicks.push_back({ bodyA->entity, direction.x, direction.y, BUMPER_KICK });
	}
	// Ball hits a slingshot: only the rubber (the diagonal) kicks, the straight sides just bounce
	if (type_a == EntityType::BALL && type_b == EntityType::SLINGSHOT)
	{
		Slingshot* slingshot = (Slingshot*)bodyB->entity;
		b2Vec2 ball_pos = bodyA->body->GetPosition();

		if (slingshot->IsInFrontOfRubber(METERS_TO_PIXELS(ball_pos.x), METERS_TO_PIXELS(ball_pos.y)))
		{
			score += SLINGSHOT_POINTS;
			slingshot->OnHit(bodyA->entity);
			kicks.push_back({ bodyA->entity, slingshot->normal_x, slingshot->normal_y, SLINGSHOT_KICK });
		}
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

// Round finished: save the scores and wait for the next round
void ModuleGame::EndRound()
{
	previous_score = score;
	if (score > highest_score)
		highest_score = score;

	state = GameState::GAME_OVER;
	LOG("Game over! Score: %d", score);
	game_over_timer.Start();
}

// Draws "text" right-aligned so it ends at x_right, vertically centered on center_y
static void DrawTextRight(const char* text, int x_right, int center_y, int size, Color color)
{
	DrawText(text, x_right - MeasureText(text, size), center_y - size / 2, size, color);
}

// Numbers inside the panels of the background (the labels are already in the image)
void ModuleGame::DrawUI() const
{
	DrawTextRight(TextFormat("%d", score), 116, 27, 20, WHITE);
	DrawTextRight(TextFormat("%d", balls_left), 494, 27, 20, WHITE);
	DrawTextRight(TextFormat("%d", previous_score), 122, 837, 14, WHITE);
	DrawTextRight(TextFormat("%d", highest_score), 450, 837, 14, WHITE);

	// Game over banner in the middle of the screen
	if (state == GameState::GAME_OVER)
	{
		DrawRectangle(0, SCREEN_HEIGHT / 2 - 50, SCREEN_WIDTH, 100, Fade(BLACK, 0.7f));

		const char* title = "GAME OVER";
		DrawText(title, (SCREEN_WIDTH - MeasureText(title, 40)) / 2, SCREEN_HEIGHT / 2 - 32, 40, WHITE);

		const char* result = TextFormat("Score: %d", previous_score);
		DrawText(result, (SCREEN_WIDTH - MeasureText(result, 20)) / 2, SCREEN_HEIGHT / 2 + 14, 20, LIGHTGRAY);
	}
}

// New round: reset the round values and put a ball on the kicker
void ModuleGame::StartRound()
{
	score = 0;
	balls_left = BALLS_PER_ROUND;
	state = GameState::PLAYING;
	SpawnBall();

	LOG("New round");
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
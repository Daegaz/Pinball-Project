#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "Entities/Entity.h"
#include "Entities/Wall.h"
#include "TableLayout.h"
#include "Entities/Ball.h"
#include "Entities/Flipper.h"
#include "Entities/Kicker.h"
#include "Entities/DeathZone.h"
#include "Entities/Bumper.h"
#include "Entities/Slingshot.h"
#include "Entities/LightTarget.h"
#include "Entities/Crystal.h"
#include "Entities/RailLight.h"
#include <ctime> 
#include <algorithm>

#define CHASE_STEP 0.3 // seconds each arrow stays lit in the chase

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	state = GameState::INTRO;
	balls_left = BALLS_PER_ROUND;
	score = 0;
	previous_score = 0;
	highest_score = 0;
	crystals_respawning = false;
	rail_combo_showing = false;
	rail_run_active = false;
}

ModuleGame::~ModuleGame()
{
}

// Loads a texture with smooth filtering (small sprites look better when rotated or scaled)
static Texture2D LoadSmooth(const char* path)
{
	Texture2D texture = LoadTexture(path);
	SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
	return texture;
}

// Load assets and build the table
bool ModuleGame::Start()
{

	LOG("Loading game scene");
	if (state == GameState::INTRO)
	{
		App->renderer->camera.x = App->renderer->camera.y = 0;
		DrawUI();
		
		timerIntro.Start();
		LoadTextures();
		LoadSounds();
		App->audio->PlayFx(16);
	}
	
		
	

	return true;
}

void ModuleGame::LoadTextures()
{
	background = LoadTexture("Assets/background.png"); // drawn 1:1, no filter needed
	ball_texture = LoadSmooth("Assets/ball.png");
	titlescreen = LoadTexture("Assets/titlescreen.png");
	flipper_left_texture = LoadSmooth("Assets/left_flipper.png");
	flipper_right_texture = LoadSmooth("Assets/right_flipper.png");
	start = LoadTexture("Assets/start.png");
	kicker_plunger_texture = LoadTexture("Assets/kicker_plunger.png");
	kicker_spring_texture = LoadSmooth("Assets/kicker_3.png");

	slingshot_left_texture = LoadTexture("Assets/slingshot_left.png");
	slingshot_left_hit_texture = LoadTexture("Assets/slingshot_left_hit.png");
	slingshot_right_texture = LoadTexture("Assets/slingshot_right.png");
	slingshot_right_hit_texture = LoadTexture("Assets/slingshot_right_hit.png");

	crystal_texture = LoadSmooth("Assets/crystal.png");

	arrow_off_texture = LoadSmooth("Assets/arrow_orange_off.png");
	arrow_on_texture = LoadSmooth("Assets/arrow_orange_on.png");

	arrow_blue_off_texture = LoadSmooth("Assets/arrow_blue_off.png");
	arrow_blue_on_texture = LoadSmooth("Assets/arrow_blue_on.png");

	rail_light_dark_texture = LoadSmooth("Assets/bumper_orange_dark.png");
	rail_light_light_texture = LoadSmooth("Assets/bumper_orange_light.png");

	arrow_pink_off_texture = LoadSmooth("Assets/arrow_pink_off.png");
	arrow_pink_on_texture = LoadSmooth("Assets/arrow_pink_on.png");

	arrow_up_orange_off_texture = LoadSmooth("Assets/arrow_up_orange_off.png");
	arrow_up_orange_on_texture = LoadSmooth("Assets/arrow_up_orange_on.png");
	triangle_orange_off_texture = LoadSmooth("Assets/triangle_orange_off.png");
	triangle_orange_on_texture = LoadSmooth("Assets/triangle_orange_on.png");

	triangle_pink_off_texture = LoadSmooth("Assets/triangle_pink_off.png");
	triangle_pink_on_texture = LoadSmooth("Assets/triangle_pink_on.png");

	// Bumpers (pink, blue, orange)
	const char* bumper_colors[3] = { "pink", "blue", "orange" };
	for (int i = 0; i < 3; ++i)
	{
		bumper_textures[i] = LoadSmooth(TextFormat("Assets/bumper_%s.png", bumper_colors[i]));
		bumper_light_textures[i] = LoadSmooth(TextFormat("Assets/bumper_%s_light.png", bumper_colors[i]));
		bumper_dark_textures[i] = LoadSmooth(TextFormat("Assets/bumper_%s_dark.png", bumper_colors[i]));
	}

	// Side light targets (top to bottom: orange, blue, pink)
	const char* target_colors[3] = { "orange", "blue", "pink" };
	for (int i = 0; i < 3; ++i)
	{
		target_off_textures[i] = LoadTexture(TextFormat("Assets/target_%s_off.png", target_colors[i]));
		target_on_textures[i] = LoadTexture(TextFormat("Assets/target_%s_on.png", target_colors[i]));
	}
}

// Every entity of the table, created from the data in TableLayout.h
void ModuleGame::CreateTable()
{
	// Walls (chains; size = number of ints, not of points)
	AddEntity(new Wall(App->physics->CreateChain(0, 0, table_outer, sizeof(table_outer) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, left_rail, sizeof(left_rail) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, inverted_v, sizeof(inverted_v) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, triangle, sizeof(triangle) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, left_flipper_guide, sizeof(left_flipper_guide) / sizeof(int))));
	AddEntity(new Wall(App->physics->CreateChain(0, 0, right_flipper_guide, sizeof(right_flipper_guide) / sizeof(int))));

	// Bumpers: static circles
	for (int i = 0; i < 3; ++i)
	{
		PhysBody* bumper_body = App->physics->CreateCircle(bumper_pos[i][0], bumper_pos[i][1], BUMPER_RADIUS, false);
		AddEntity(new Bumper(bumper_body, App, bumper_textures[i], bumper_light_textures[i], bumper_dark_textures[i]));
	}

	// Slingshots: chain of their face + the rubber line that kicks
	AddEntity(new Slingshot(App->physics->CreateChain(0, 0, slingshot_left_shape, sizeof(slingshot_left_shape) / sizeof(int)),
		App, slingshot_left_texture, slingshot_left_hit_texture, slingshot_left_sprite[0], slingshot_left_sprite[1], false,
		slingshot_left_rubber[0], slingshot_left_rubber[1], slingshot_left_rubber[2], slingshot_left_rubber[3]));
	AddEntity(new Slingshot(App->physics->CreateChain(0, 0, slingshot_right_shape, sizeof(slingshot_right_shape) / sizeof(int)),
		App, slingshot_right_texture, slingshot_right_hit_texture, slingshot_right_sprite[0], slingshot_right_sprite[1], true,
		slingshot_right_rubber[0], slingshot_right_rubber[1], slingshot_right_rubber[2], slingshot_right_rubber[3]));

	// Side light targets: static boxes 7x34 (center = sprite top-left + half size)
	for (int i = 0; i < 3; ++i)
	{
		AddEntity(new LightTarget(App->physics->CreateRectangle(target_left_x + 3, target_left_y[i] + 18, 3, 17, false),
			App, target_off_textures[i], target_on_textures[i], target_left_x, target_left_y[i], 1.0f));
		AddEntity(new LightTarget(App->physics->CreateRectangle(target_right_x + 3, target_right_y[i] + 18, 3, 17, false),
			App, target_off_textures[i], target_on_textures[i], target_right_x, target_right_y[i], -1.0f));
	}

	// Floating wave: left first, then the top one, then the right one (more phase = hops earlier)
	const float crystal_phase[3] = { 0.3f, 0.0f, 0.6f }; // top, right, left (same order as crystal_pos)

	// Crystals: collectible sensors (also kept in their own list to check the combo)
	for (int i = 0; i < 3; ++i)
	{
		PhysBody* crystal_body = App->physics->CreateRectangleSensor(crystal_pos[i][0], crystal_pos[i][1], 10, 14);
		Crystal* crystal = new Crystal(crystal_body, App, crystal_texture, crystal_pos[i][0], crystal_pos[i][1], crystal_phase[i]);		
		crystals.push_back(crystal);
		AddEntity(crystal);
	}

	// Rail lights: small sensors in the lane between the rail and the V
	for (int i = 0; i < 5; ++i)
	{
		PhysBody* light_body = App->physics->CreateRectangleSensor(rail_light_pos[i][0], rail_light_pos[i][1], 9, 9);
		RailLight* light = new RailLight(light_body, App, rail_light_dark_texture, rail_light_light_texture,
			rail_light_pos[i][0], rail_light_pos[i][1], i);
		rail_lights.push_back(light);
		AddEntity(light);
	}

	// Lost ball sensor
	AddEntity(new DeathZone(App->physics->CreateRectangleSensor(death_zone[0], death_zone[1], death_zone[2], death_zone[3])));

	// Flippers: hinge + shape, limits [-25, 25] degrees, start at rest
	b2RevoluteJoint* joint = NULL;

	PhysBody* body = App->physics->CreateFlipper(flipper_left_hinge[0], flipper_left_hinge[1],
		flipper_left_shape, sizeof(flipper_left_shape) / sizeof(int), -25.0f, 25.0f, 25.0f, &joint);
	AddEntity(new Flipper(body, joint, App, flipper_left_texture, flipper_left_hinge[0], flipper_left_hinge[1],
		flipper_left_pivot[0], flipper_left_pivot[1], KEY_LEFT, true));

	body = App->physics->CreateFlipper(flipper_right_hinge[0], flipper_right_hinge[1],
		flipper_right_shape, sizeof(flipper_right_shape) / sizeof(int), -25.0f, 25.0f, -25.0f, &joint);
	AddEntity(new Flipper(body, joint, App, flipper_right_texture, flipper_right_hinge[0], flipper_right_hinge[1],
		flipper_right_pivot[0], flipper_right_pivot[1], KEY_RIGHT, false));

	// Kicker: plunger 30x8 sliding down the lane
	b2PrismaticJoint* kicker_joint = NULL;
	PhysBody* kicker_body = App->physics->CreateKicker(kicker_x, kicker_rest_y, 15, 4, kicker_travel, &kicker_joint);
	AddEntity(new Kicker(kicker_body, kicker_joint, App, kicker_plunger_texture, kicker_spring_texture, kicker_x, kicker_bottom_y));
}

void ModuleGame::LoadSounds() 
{
	
	App->audio->LoadFx("Fx/Ballsound.wav");
	App->audio->LoadFx("Fx/Ballsound2.wav");
	App->audio->LoadFx("Fx/Ballsound3.wav");
	App->audio->LoadFx("Fx/Bumpelectric.wav");
	App->audio->LoadFx("Fx/Bumper.wav");
	App->audio->LoadFx("Fx/Bumper2.wav");
	App->audio->LoadFx("Fx/Chargecoil2.wav");
	App->audio->LoadFx("Fx/Chargecoil.wav");
	App->audio->LoadFx("Fx/Hit.wav");
	App->audio->LoadFx("Fx/Gameover.wav");
	App->audio->LoadFx("Fx/Musiclevel.wav");
	App->audio->LoadFx("Fx/Loseball.wav");
	App->audio->LoadFx("Fx/Newscore.wav");
	App->audio->LoadFx("Fx/Diamond.wav");
	App->audio->LoadFx("Fx/Bonus2.wav");
	App->audio->LoadFx("Fx/Bumper3.wav");
	App->audio->LoadFx("Fx/kurwa.wav");

}
// Unload assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading game scene");

	crystals.clear(); // the crystals are deleted with the rest of the entities
	rail_lights.clear();

	DestroyAllEntities();
	for (int i = 0; i < 3; ++i)
	{
		UnloadTexture(bumper_textures[i]);
		UnloadTexture(bumper_light_textures[i]);
		UnloadTexture(bumper_dark_textures[i]);
	}
	for (int i = 0; i < 3; ++i)
	{
		UnloadTexture(target_off_textures[i]);
		UnloadTexture(target_on_textures[i]);
	}
	UnloadTexture(triangle_pink_off_texture);
	UnloadTexture(triangle_pink_on_texture);
	UnloadTexture(triangle_orange_off_texture);
	UnloadTexture(triangle_orange_on_texture);
	UnloadTexture(arrow_up_orange_off_texture);
	UnloadTexture(arrow_up_orange_on_texture);
	UnloadTexture(arrow_pink_off_texture);
	UnloadTexture(arrow_pink_on_texture);
	UnloadTexture(rail_light_dark_texture);
	UnloadTexture(rail_light_light_texture);
	UnloadTexture(arrow_blue_off_texture);
	UnloadTexture(arrow_blue_on_texture);
	UnloadTexture(arrow_off_texture);
	UnloadTexture(arrow_on_texture);
	UnloadTexture(crystal_texture);
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
	if (state == GameState::PLAYING) {
		DrawTexture(background, 0, 0, WHITE);
		DrawDecorations();
		
	}
	if (state == GameState::INTRO && IsKeyPressed(KEY_ENTER)) {
		state = GameState::PLAYING;
		

		CreateTable();
		StartRound();
		

	}


	// Game over: wait a few seconds and start the next round automatically
	if (state == GameState::GAME_OVER && game_over_timer.ReadSec() >= GAME_OVER_SECONDS)
		StartRound();
	
	// Crystals come back a few seconds after the combo
	if (crystals_respawning && crystal_respawn_timer.ReadSec() >= CRYSTAL_RESPAWN_TIME)
		RespawnCrystals();

	// Rail lights: after showing the 5 lit, back to blinking
	if (rail_combo_showing && rail_combo_timer.ReadSec() >= RAIL_COMBO_SHOW_TIME)
		ResetRailLights();

	// Rail lights: run not finished in time -> back to blinking
	if (rail_run_active && rail_run_timer.ReadSec() >= RAIL_RUN_TIME)
		ResetRailLights();

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
	if (type_a == EntityType::BALL) {
		std::srand(std::time(nullptr));
		int numero_aleatorio = std::rand() % (2 - 0 + 1);
		
		App->audio->PlayFx(numero_aleatorio);
	}
	if (type_a == EntityType::BALL && type_b == EntityType::KICKER && IsKeyDown(KEY_DOWN))
	{
		std::srand(std::time(nullptr));
		
		App->audio->PlayFx(6);
		App->audio->PlayFx(7);
	}
	// Ball fell into the drain (only react when A is the ball, so it counts once)
	if (type_a == EntityType::BALL && type_b == EntityType::DEATH_ZONE)
	{
		LOG("Ball lost!");
		App->audio->PlayFx(11);
		// Only mark it: we can't destroy bodies while Box2D is stepping
		if (std::find(lost_balls.begin(), lost_balls.end(), bodyA->entity) == lost_balls.end())
			lost_balls.push_back(bodyA->entity);
	}
	// Ball hits a bumper: points, flash and a kick away from its center
	if (type_a == EntityType::BALL && type_b == EntityType::BUMPER)
	{
		score += BUMPER_POINTS;
		bodyB->entity->OnHit(bodyA->entity);
		App->audio->PlayFx(15);
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
			App->audio->PlayFx(4);
			slingshot->OnHit(bodyA->entity);
			kicks.push_back({ bodyA->entity, slingshot->normal_x, slingshot->normal_y, SLINGSHOT_KICK });
		}
	}
	// Ball hits a side target: points, it lights up and the ball bounces off the wall
	if (type_a == EntityType::BALL && type_b == EntityType::LIGHT_TARGET)
	{
		LightTarget* target = (LightTarget*)bodyB->entity;
		score += TARGET_POINTS;
		App->audio->PlayFx(8);
		target->OnHit(bodyA->entity);
		kicks.push_back({ bodyA->entity, target->kick_x, 0.0f, TARGET_KICK });
	}
	// Ball collects a crystal. Collect the 3 -> extra ball + bonus, and they come back after a delay
	if (type_a == EntityType::BALL && type_b == EntityType::CRYSTAL)
	{
		Crystal* crystal = (Crystal*)bodyB->entity;
		if (!crystal->IsCollected())
		{
			crystal->OnHit(bodyA->entity);
			score += CRYSTAL_POINTS;
			App->audio->PlayFx(13);
			bool all_collected = true;
			for (Crystal* c : crystals)
			{
				if (!c->IsCollected())
					all_collected = false;
			}

			if (all_collected)
			{
				++balls_left;
				score += CRYSTAL_COMBO_POINTS;
				crystals_respawning = true;
				crystal_respawn_timer.Start();
				App->audio->PlayFx(14);
				LOG("Combo! Extra ball");
			}
		}
	}
	// Ball touches a rail light: the first one starts a run (all black), each one touched stays lit
	if (type_a == EntityType::BALL && type_b == EntityType::RAIL_LIGHT && !rail_combo_showing)
	{
		RailLight* touched = (RailLight*)bodyB->entity;

		if (touched->mode != RailLightMode::LIT)
		{
			// First touch of a run: switch all off
			if (touched->mode == RailLightMode::BLINK)
			{
				for (RailLight* light : rail_lights)
					light->mode = RailLightMode::OFF;
		
				rail_run_active = true;
				rail_run_timer.Start();
			}

			touched->mode = RailLightMode::LIT;

			// All lit: bonus, show them a moment and back to blinking
			bool all_lit = true;
			for (RailLight* light : rail_lights)
			{
				if (light->mode != RailLightMode::LIT)
					all_lit = false;
			}

			if (all_lit)
			{
				score += RAIL_COMBO_POINTS;
				App->audio->PlayFx(14);
				rail_run_active = false;
				rail_combo_showing = true;
				rail_combo_timer.Start();
				LOG("Rail lights complete! +%d", RAIL_COMBO_POINTS);
			}
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
	AddEntity(new Ball(App->physics->CreateCircle(ball_spawn[0], ball_spawn[1], 10), App, ball_texture ));
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

// The 3 crystals come back
void ModuleGame::RespawnCrystals()
{
	for (Crystal* crystal : crystals)
		crystal->Respawn();

	crystals_respawning = false;
}

// Back to the idle blinking
void ModuleGame::ResetRailLights()
{
	for (RailLight* light : rail_lights)
		light->mode = RailLightMode::BLINK;

	rail_combo_showing = false;
	rail_run_active = false;
}

// Round finished: save the scores and wait for the next round
void ModuleGame::EndRound()
{
	previous_score = score;
	if (score > highest_score)
		highest_score = score;

	state = GameState::GAME_OVER;
	LOG("Game over! Score: %d", score);
	App->audio->PlayFx(9);
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
	else if(state == GameState::INTRO)
	{
		
		int counter = 0;
		counter += (int)timerIntro.ReadSec();
		if (counter < 7.0f) {
			DrawTexturePro(titlescreen,
				{ 0, 0, (float)titlescreen.width, (float)titlescreen.height },
				{ 0, 0, (float)SCREEN_WIDTH + 400, (float)SCREEN_HEIGHT },
				{ 200, 0 },
				0, WHITE);
			
		}
		else
		{
			DrawTexturePro(start,
				{ 0, 0, (float)titlescreen.width, (float)titlescreen.height },
				{ 0, 0, (float)SCREEN_WIDTH + 400, (float)SCREEN_HEIGHT },
				{ 200, 0 },
				0, WHITE);
		
		}
		
	}
}

// A group of arrows lit one step at a time ("chase"): "steps" = how many different steps the group has
void ModuleGame::DrawArrowChase(const DecoArrow* arrows, int count, int steps, Texture2D off, Texture2D on, Color tint) const
{
	int lit_step = (int)(GetTime() / CHASE_STEP) % steps;

	for (int i = 0; i < count; ++i)
	{
		const DecoArrow& arrow = arrows[i];
		Texture2D texture = (arrow.step == lit_step) ? on : off;
		App->renderer->Draw(texture, arrow.x, arrow.y, NULL, arrow.angle, texture.width / 2, texture.height / 2, 1.0f, tint);
	}
}

// Decorative lights painted on the table (under the ball)
void ModuleGame::DrawDecorations() const
{
	DrawArrowChase(side_arrows, sizeof(side_arrows) / sizeof(DecoArrow), 3, arrow_off_texture, arrow_on_texture);
	DrawArrowChase(center_arrows, sizeof(center_arrows) / sizeof(DecoArrow), 4, arrow_blue_off_texture, arrow_blue_on_texture);

	DrawArrowChase(v_arrows, sizeof(v_arrows) / sizeof(DecoArrow), 2, arrow_pink_off_texture, arrow_pink_on_texture);

	DrawArrowChase(right_lane_arrow, 1, 4, arrow_up_orange_off_texture, arrow_up_orange_on_texture);
	DrawArrowChase(right_lane_triangles, 3, 4, triangle_orange_off_texture, triangle_orange_on_texture);

	DrawArrowChase(tube_triangles, 8, 8, triangle_pink_off_texture, triangle_pink_on_texture, Color{ 255, 255, 255, 150 });
}

// New round: reset the round values and put a ball on the kicker
void ModuleGame::StartRound()
{
	score = 0;
	balls_left = BALLS_PER_ROUND;
	state = GameState::PLAYING;
	
	SpawnBall();
	RespawnCrystals();
	ResetRailLights();

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
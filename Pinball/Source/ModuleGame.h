#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>
#include "Timer.h"

#define BALLS_PER_ROUND 3
#define GAME_OVER_SECONDS 3.0  // time showing "game over" before the next round starts
#define BUMPER_KICK 6.0f  // m/s added to the ball when it hits a bumper

enum class GameState
{
	PLAYING,
	GAME_OVER
};

class PhysBody;
class PhysicEntity;
class Crystal;
class RailLight;

struct DecoArrow;

// A ball that has to be kicked (applied in Update, outside the physics step)
struct Kick
{
	PhysicEntity* ball;
	float dir_x;   // direction (normalized)
	float dir_y;
	float speed;   // m/s added to the ball
};

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	Texture2D background;
	Texture2D ball_texture;
	Texture2D flipper_left_texture;
	Texture2D flipper_right_texture;
	Texture2D kicker_plunger_texture;
	Texture2D kicker_spring_texture;
	Texture2D bumper_textures[3];       // pink, blue, orange
	Texture2D bumper_light_textures[3];
	Texture2D bumper_dark_textures[3];
	Texture2D slingshot_left_texture;
	Texture2D slingshot_left_hit_texture;
	Texture2D slingshot_right_texture;
	Texture2D slingshot_right_hit_texture;
	Texture2D target_off_textures[3]; // orange, blue, pink (top to bottom)
	Texture2D target_on_textures[3];
	Texture2D crystal_texture;
	Texture2D arrow_off_texture;
	Texture2D arrow_on_texture;
	Texture2D arrow_blue_off_texture;
	Texture2D arrow_blue_on_texture;
	Texture2D rail_light_dark_texture;
	Texture2D rail_light_light_texture;
	Texture2D arrow_pink_off_texture;
	Texture2D arrow_pink_on_texture;
	Texture2D arrow_up_orange_off_texture;
	Texture2D arrow_up_orange_on_texture;
	Texture2D triangle_orange_off_texture;
	Texture2D triangle_orange_on_texture;
	Texture2D triangle_pink_off_texture;
	Texture2D triangle_pink_on_texture;

	void DrawUI() const;
	void DrawDecorations() const;
	void DrawArrowChase(const DecoArrow* arrows, int count, int steps, Texture2D off, Texture2D on, Color tint = WHITE) const;
	bool Start();
	update_status Update();
	bool CleanUp();

	void LoadTextures();
	void CreateTable();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;
	void AddEntity(PhysicEntity* entity);

	void DestroyEntity(PhysicEntity* entity);
	void DestroyAllEntities();

	void SpawnBall();
	int CountBalls() const;

	void RespawnCrystals();
	bool crystals_respawning;
	Timer crystal_respawn_timer;

	void ResetRailLights();
	bool rail_combo_showing;             // the 5 are lit, waiting to go back to blinking
	Timer rail_combo_timer;
	bool rail_run_active; // a run started (first light touched) and the clock is running
	Timer rail_run_timer;

	void EndRound();
	Timer game_over_timer;

	GameState state;
	int balls_left;      // balls still to play in this round (including the one on the table)
	int score;           // current round
	int previous_score;  // last finished round
	int highest_score;   // best round of this session

	void StartRound();



public:

	std::vector<PhysicEntity*> entities;
	std::vector<PhysicEntity*> lost_balls; // balls that fell, removed in Update (never inside the physics step)
	std::vector<Kick> kicks;
	std::vector<Crystal*> crystals;
	std::vector<RailLight*> rail_lights; // also in entities: only to handle the group

};

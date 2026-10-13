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

// A ball that hit a bumper, waiting for its kick (applied in Update, outside the physics step)
struct BumperHit
{
	PhysicEntity* ball;
	PhysicEntity* bumper;
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

	void DrawUI() const;
	bool Start();
	update_status Update();
	bool CleanUp();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;
	void AddEntity(PhysicEntity* entity);

	void DestroyEntity(PhysicEntity* entity);
	void DestroyAllEntities();

	void SpawnBall();
	int CountBalls() const;

	void EndRound();

	GameState state;
	int balls_left;      // balls still to play in this round (including the one on the table)
	int score;           // current round
	int previous_score;  // last finished round
	int highest_score;   // best round of this session

	void StartRound();

	Timer game_over_timer;

public:

	std::vector<PhysicEntity*> entities;
	std::vector<PhysicEntity*> lost_balls; // balls that fell, removed in Update (never inside the physics step)
	std::vector<BumperHit> bumper_hits;

};

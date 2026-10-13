#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

class PhysBody;
class PhysicEntity;


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

	bool Start();
	update_status Update();
	bool CleanUp();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;
	void AddEntity(PhysicEntity* entity);

	void DestroyEntity(PhysicEntity* entity);
	void DestroyAllEntities();

	void SpawnBall();
	int CountBalls() const;

public:

	std::vector<PhysicEntity*> entities;
	std::vector<PhysicEntity*> lost_balls; // balls that fell, removed in Update (never inside the physics step)

};

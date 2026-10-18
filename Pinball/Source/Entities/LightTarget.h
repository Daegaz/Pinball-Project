#pragma once
#include "Entities/Entity.h"
#include "Timer.h"
#include "raylib.h"

#define TARGET_POINTS 50
#define TARGET_KICK 1.0f // m/s, away from the wall

class Application;

// Small light on the side walls: bounces, scores and stays lit (combo when all are lit)
class LightTarget : public PhysicEntity
{
public:
	// (x, y) = top-left of the sprite; kick_x = +1 on the left wall, -1 on the right wall
	LightTarget(PhysBody* body, Application* app, Texture2D off_texture, Texture2D on_texture, int x, int y, float kick_x);

	void Update() override;
	void OnHit(PhysicEntity* other) override; // lights it

	bool IsLit() const { return lit; }
	void TurnOff() { lit = false; }

	float kick_x; // direction of the bounce (horizontal, out of the wall)

private:
	Application* app;
	Texture2D off_texture;
	Texture2D on_texture;
	int x, y;
	bool lit;
	Timer lit_timer;
}; 

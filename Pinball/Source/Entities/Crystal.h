#pragma once
#include "Entities/Entity.h"
#include "raylib.h"
#include "Timer.h"

#define CRYSTAL_POINTS 250
#define CRYSTAL_COMBO_POINTS 1000 // bonus for collecting the 3
#define CRYSTAL_RESPAWN_TIME 3.0  // seconds until the 3 crystals come back after the combo

class Application;

// Collectible: a sensor the ball goes through. Collect the 3 -> extra ball
class Crystal : public PhysicEntity
{
public:
	Crystal(PhysBody* body, Application* app, Texture2D texture, int x, int y, float phase);

	void Update() override;
	void OnHit(PhysicEntity* other) override; // collected

	bool IsCollected() const { return collected; }
	void Respawn();

private:
	Application* app;
	Texture2D texture;
	int x, y;      // center
	float phase;   // so the 3 crystals don't float in sync
	bool collected;
	Timer appear_timer;
	bool appearing;
};
#pragma once
#include "Entities/Entity.h"
#include "Timer.h"
#include "raylib.h"

#define BUMPER_RADIUS 31   // physics circle, in pixels
#define BUMPER_POINTS 100

class Application;

class Bumper : public PhysicEntity
{
public:
	Bumper(PhysBody* body, Application* app, Texture2D texture, Texture2D light_texture, Texture2D dark_texture);

	void Update() override;
	void OnHit(PhysicEntity* other) override; // starts the flash

private:
	Application* app;
	Texture2D texture;       // the bumper itself
	Texture2D light_texture; // flash frames over the center
	Texture2D dark_texture;
	Timer flash_timer;
	bool flashing;
};
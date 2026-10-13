#pragma once
#include "Entities/Entity.h"
#include "raylib.h"

#define RAIL_LIGHT_POINTS 10     // each light touched
#define RAIL_COMBO_POINTS 500    // the 5 lit
#define RAIL_COMBO_SHOW_TIME 1.0 // seconds all lit before blinking again

class Application;

enum class RailLightMode
{
	BLINK, // idle: alternating dark / light
	OFF,   // a run started and this one is not touched yet (black)
	LIT    // touched in this run
};

class RailLight : public PhysicEntity
{
public:
	RailLight(PhysBody* body, Application* app, Texture2D dark_texture, Texture2D light_texture, int x, int y, int index);

	void Update() override;

	RailLightMode mode;

private:
	Application* app;
	Texture2D dark_texture;
	Texture2D light_texture;
	int x, y;
	int index; // position in the row: even / odd alternate while blinking
};
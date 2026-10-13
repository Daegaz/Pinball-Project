#include "Entities/RailLight.h"
#include "Application.h"
#include "ModuleRender.h"

#define BLINK_STEP 0.4 // seconds between swaps while blinking
#define RAIL_LIGHT_SCALE 0.8f // 23 px sprites drawn at ~18 px

RailLight::RailLight(PhysBody* body, Application* app, Texture2D dark_texture, Texture2D light_texture, int x, int y, int index)
	: PhysicEntity(body, EntityType::RAIL_LIGHT), mode(RailLightMode::BLINK), app(app),
	dark_texture(dark_texture), light_texture(light_texture), x(x), y(y), index(index)
{
}

void RailLight::Update()
{
	Texture2D texture = dark_texture;
	Color tint = WHITE;

	switch (mode)
	{
	case RailLightMode::BLINK:
	{
		// Even and odd lights take turns: dark-light-dark-light-dark, then swapped
		int swap = (int)(GetTime() / BLINK_STEP) % 2;
		texture = ((index + swap) % 2 == 1) ? light_texture : dark_texture;
	}
	break;

	case RailLightMode::OFF:
		texture = dark_texture; // not touched yet in this run
		break;

	case RailLightMode::LIT:
		texture = light_texture;
		break;
	}

	app->renderer->Draw(texture, x, y, NULL, 0.0, texture.width / 2, texture.height / 2, RAIL_LIGHT_SCALE);
}
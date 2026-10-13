#include "Entities/Bumper.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#define FLASH_TIME 0.3   // seconds the light blinks after a hit
#define FLASH_FRAME 0.1  // seconds per frame (light -> dark -> light)
#define FLASH_SCALE 0.15f // extra size at the moment of the hit (+15%)


Bumper::Bumper(PhysBody* body, Application* app, Texture2D texture, Texture2D light_texture, Texture2D dark_texture)
	: PhysicEntity(body, EntityType::BUMPER), app(app), texture(texture),
	light_texture(light_texture), dark_texture(dark_texture), flashing(false)
{
	// Bouncy surface (the extra "kick" is added by ModuleGame on hit)
	body->body->GetFixtureList()->SetRestitution(0.8f);
}

void Bumper::OnHit(PhysicEntity* other)
{
	flashing = true;
	flash_timer.Start();
}

void Bumper::Update()
{
	int x, y;
	body->GetPosition(x, y); // center of the top face of the bumper

	float scale = 1.0f;
	Texture2D* light = NULL;

	if (flashing)
	{
		double t = flash_timer.ReadSec();
		if (t >= FLASH_TIME)
		{
			flashing = false;
		}
		else
		{
			// Pop: +15% at the hit, shrinking back to normal size during the flash
			scale = 1.0f + FLASH_SCALE * (1.0f - (float)(t / FLASH_TIME));

			// Blink: light, dark, light...
			int frame = (int)(t / FLASH_FRAME);
			light = (frame % 2 == 0) ? &light_texture : &dark_texture;
		}
	}

	// Bumper sprite: (32, 32) is the center of its top face
	app->renderer->Draw(texture, x, y, NULL, 0.0, 32, 32, scale);

	// Light over the center, growing with the bumper
	if (light != NULL)
		app->renderer->Draw(*light, x, y, NULL, 0.0, light->width / 2, light->height / 2, scale);
}
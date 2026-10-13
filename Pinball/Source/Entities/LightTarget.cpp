#include "Entities/LightTarget.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#define LIT_TIME 0.3 // seconds the light stays on after a hit


LightTarget::LightTarget(PhysBody* body, Application* app, Texture2D off_texture, Texture2D on_texture, int x, int y, float kick_x)
	: PhysicEntity(body, EntityType::LIGHT_TARGET), kick_x(kick_x), app(app),
	off_texture(off_texture), on_texture(on_texture), x(x), y(y), lit(false)
{
	body->body->GetFixtureList()->SetRestitution(0.6f);
}

void LightTarget::OnHit(PhysicEntity* other)
{
	lit = true;
	lit_timer.Start(); // a new hit restarts the time
}

void LightTarget::Update()
{
	// Back to dark after LIT_TIME
	if (lit && lit_timer.ReadSec() >= LIT_TIME)
		lit = false;

	app->renderer->Draw(lit ? on_texture : off_texture, x, y);
}
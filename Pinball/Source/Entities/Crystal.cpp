#include "Entities/Crystal.h"
#include "Application.h"
#include "ModuleRender.h"

#include <math.h>

#define FLOAT_HEIGHT 4.0f  // pixels it rises
#define FLOAT_MOVE 0.6f    // seconds of the quick up-and-down
#define FLOAT_CYCLE 2.0f   // seconds of a full cycle (move + rest)
#define APPEAR_TIME 0.4f // seconds of the "pop-in" when it comes back

Crystal::Crystal(PhysBody* body, Application* app, Texture2D texture, int x, int y, float phase)
	: PhysicEntity(body, EntityType::CRYSTAL), app(app), texture(texture), x(x), y(y), phase(phase), collected(false)
{
}

void Crystal::OnHit(PhysicEntity* other)
{
	collected = true;
}

void Crystal::Update()
{
	if (collected)
		return; // invisible until it respawns

	// Floating: a quick hop up and down, then it rests until the next cycle
	float t = fmodf((float)GetTime() + phase, FLOAT_CYCLE);
	float offset = 0.0f;
	if (t < FLOAT_MOVE)
		offset = -FLOAT_HEIGHT * (1.0f - cosf(2.0f * PI * t / FLOAT_MOVE)) * 0.5f;

	// Pop-in when it respawns: grows with a small overshoot and fades in
	float scale = 1.0f;
	Color tint = WHITE;
	if (appearing)
	{
		float p = (float)appear_timer.ReadSec() / APPEAR_TIME; // 0 -> 1
		if (p >= 1.0f)
		{
			appearing = false;
		}
		else
		{
			// "Ease out back": goes a bit past 1 and settles (c = how much it overshoots)
			const float c = 1.7f;
			float q = p - 1.0f;
			scale = 1.0f + (c + 1.0f) * q * q * q + c * q * q;
			tint = Fade(WHITE, p);
		}
	}

	app->renderer->Draw(texture, x, y + (int)offset, NULL, 0.0, texture.width / 2, texture.height / 2, scale, tint);
}

void Crystal::Respawn()
{
	if (!collected)
		return; // already visible: no effect

	collected = false;
	appearing = true;
	appear_timer.Start();
}
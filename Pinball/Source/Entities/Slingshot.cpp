#include "Entities/Slingshot.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include <math.h>

#define HIT_TIME 0.15 // seconds showing the stretched rubber

Slingshot::Slingshot(PhysBody* body, Application* app, Texture2D texture, Texture2D hit_texture, int x, int y, bool align_right,
	int ax, int ay, int bx, int by)
	: PhysicEntity(body, EntityType::SLINGSHOT), app(app), texture(texture), hit_texture(hit_texture),
	x(x), y(y), align_right(align_right), ax(ax), ay(ay), hit(false)
{
	// Normal of the rubber: perpendicular to a -> b, pointing out
	float dx = (float)(bx - ax);
	float dy = (float)(by - ay);
	float length = sqrtf(dx * dx + dy * dy);
	normal_x = dy / length;
	normal_y = -dx / length;

	body->body->GetFixtureList()->SetRestitution(0.5f);
}

bool Slingshot::IsInFrontOfRubber(float px, float py) const
{
	// Dot product with the normal: positive = the point is on the outer side of the rubber line
	return (px - ax) * normal_x + (py - ay) * normal_y > 0.0f;
}

void Slingshot::OnHit(PhysicEntity* other)
{
	hit = true;
	hit_timer.Start();
}

void Slingshot::Update()
{
	if (hit && hit_timer.ReadSec() >= HIT_TIME)
		hit = false;

	if (hit)
	{
		// The hit sprite is wider: on the right slingshot, shift it so both share the right edge
		int draw_x = align_right ? x + texture.width - hit_texture.width : x;
		app->renderer->Draw(hit_texture, draw_x, y);
	}
	else
	{
		app->renderer->Draw(texture, x, y);
	}
}
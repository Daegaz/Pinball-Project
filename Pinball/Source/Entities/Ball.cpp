#include "Entities/Ball.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

Ball::Ball(PhysBody* body, Application* app, Texture2D texture)
	: PhysicEntity(body, EntityType::BALL), app(app), texture(texture)
{
	// Continuous collision: a fast ball never goes through thin walls
	body->body->SetBullet(true);

	b2Fixture* fixture = body->body->GetFixtureList();
	fixture->SetRestitution(0.4f); // bounciness
	fixture->SetFriction(0.1f);
}

void Ball::Update()
{
	int x, y;
	body->GetPosition(x, y); // center of the body, in pixels

	// Pivot at the center of the sprite (rotation is ignored for now: pending fix n.1)
	app->renderer->Draw(texture, x, y, NULL, body->GetRotation() * RADTODEG, texture.width / 2, texture.height / 2);
}
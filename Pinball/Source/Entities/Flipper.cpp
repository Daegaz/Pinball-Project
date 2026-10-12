#include "Entities/Flipper.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#define FLIPPER_SPEED 20.0f // motor speed in rad/s

Flipper::Flipper(PhysBody* body, b2RevoluteJoint* joint, Application* app, Texture2D texture, int x, int y, int pivot_x, int pivot_y, KeyboardKey key, bool is_left)
	: PhysicEntity(body, EntityType::FLIPPER), joint(joint), app(app), texture(texture),
	x(x), y(y), pivot_x(pivot_x), pivot_y(pivot_y), key(key), is_left(is_left)
{
}

void Flipper::Update()
{
	// Up: the left flipper turns counter-clockwise (negative), the right one clockwise (positive)
	float up_speed = is_left ? -FLIPPER_SPEED : FLIPPER_SPEED;

	// Key held: motor pushes up. Released: motor pushes down to the resting position
	joint->SetMotorSpeed(IsKeyDown(key) ? up_speed : -up_speed);

	// The hinge never moves: draw at its fixed position (body position jitters 1px due to rounding)
	app->renderer->Draw(texture, x, y, NULL, body->GetRotation() * RADTODEG, pivot_x, pivot_y);
}
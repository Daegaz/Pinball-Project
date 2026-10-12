#include "Entities/Kicker.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include <stdlib.h>

#define KICKER_CHARGE_SPEED 6.0f  // m/s the plunger goes down while holding DOWN
#define KICKER_MAX_SPEED 35.0f    // m/s launch speed at full charge
#define KICKER_IDLE_SPEED 2.0f    // m/s to keep the plunger up at rest

Kicker::Kicker(PhysBody* body, b2PrismaticJoint* joint, Application* app, Texture2D plunger_texture, Texture2D spring_texture, int x, int bottom_y)
	: PhysicEntity(body, EntityType::KICKER), joint(joint), app(app), plunger_texture(plunger_texture),
	spring_texture(spring_texture), x(x), bottom_y(bottom_y), launch_speed(0.0f) {}

void Kicker::Update()
{
	float travel = joint->GetJointTranslation(); // meters pulled down (0 = rest)
	float max_travel = joint->GetUpperLimit();

	if (IsKeyDown(KEY_DOWN))
	{
		// Charging: pull the plunger down slowly
		joint->SetMotorSpeed(KICKER_CHARGE_SPEED);
		launch_speed = 0.0f;
	}
	else
	{
		// Just released: the further down, the stronger the launch
		if (IsKeyReleased(KEY_DOWN))
			launch_speed = KICKER_MAX_SPEED * travel / max_travel;

		// Back at the top: launch finished
		if (travel <= 0.01f)
			launch_speed = 0.0f;

		// Push up (fast while launching, gently at rest)
		joint->SetMotorSpeed(-MAX(launch_speed, KICKER_IDLE_SPEED));
	}

	// Draw ------------------------------------------------------------
	int body_x, body_y;
	body->GetPosition(body_x, body_y); // plunger center (only Y matters, X is fixed)

	// The spring goes from just under the plunger down to the bottom of the lane
	int spring_top = body_y + plunger_texture.height / 2;
	int spring_height = bottom_y - spring_top;

	// Squash the whole spring image into that height
	Rectangle source = { 0.0f, 0.0f, (float)spring_texture.width, (float)spring_texture.height };
	Rectangle dest = { (float)(x - spring_texture.width / 2), (float)spring_top, (float)spring_texture.width, (float)spring_height };
	DrawTexturePro(spring_texture, source, dest, { 0.0f, 0.0f }, 0.0f, WHITE);

	// Plunger on top of the spring
	app->renderer->Draw(plunger_texture, x, body_y, NULL, 0.0, plunger_texture.width / 2, plunger_texture.height / 2);
}
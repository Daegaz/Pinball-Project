#pragma once
#include "Entities/Entity.h"
#include "raylib.h"
#include "Application.h"



class Application;
class b2PrismaticJoint;

class Kicker : public PhysicEntity
{
public:
	Kicker(PhysBody* body, b2PrismaticJoint* joint, Application* app, Texture2D plunger_texture, Texture2D spring_texture, int x, int bottom_y);

	void Update() override;

private:
	b2PrismaticJoint* joint;
	Application* app;
	Texture2D plunger_texture;
	Texture2D spring_texture; // squashed to fit between the plunger and the bottom
	int x;                    // center of the lane (never changes)
	int bottom_y;             // where the spring is attached (bottom of the lane)
	float launch_speed;       // m/s, set when DOWN is released
};
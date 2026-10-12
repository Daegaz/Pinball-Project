#pragma once
#include "Entities/Entity.h"
#include "raylib.h"

class Application;
class b2RevoluteJoint;

class Flipper : public PhysicEntity
{
public:
	Flipper(PhysBody* body, b2RevoluteJoint* joint, Application* app, Texture2D texture, int x, int y, int pivot_x, int pivot_y, KeyboardKey key, bool is_left);
	void Update() override;

private:
	b2RevoluteJoint* joint; // hinge with motor (destroyed by Box2D with the body)
	Application* app;
	Texture2D texture;
	int pivot_x;            // hinge position inside the sprite
	int pivot_y;
	int x;                  // hinge position on screen (it never moves)
	int y;
	KeyboardKey key;
	bool is_left;
}; 

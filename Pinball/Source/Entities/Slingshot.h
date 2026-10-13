#pragma once
#include "Entities/Entity.h"
#include "Timer.h"
#include "raylib.h"

#define SLINGSHOT_POINTS 50
#define SLINGSHOT_KICK 5.0f // m/s

class Application;

class Slingshot : public PhysicEntity
{
public:
	// (x, y) = top-left of the sprite. The rubber is the line a -> b, ordered so its outer side is on the LEFT of a -> b
	Slingshot(PhysBody* body, Application* app, Texture2D texture, Texture2D hit_texture, int x, int y, bool align_right,
		int ax, int ay, int bx, int by);

	void Update() override;
	void OnHit(PhysicEntity* other) override;

	// True if (px, py) is on the rubber side (in front of the diagonal)
	bool IsInFrontOfRubber(float px, float py) const;

	float normal_x; // direction of the kick (normalized, pointing out of the rubber)
	float normal_y;

private:
	Application* app;
	Texture2D texture;
	Texture2D hit_texture; // with the rubber stretched out
	int x, y;
	bool align_right;      // the hit sprite is wider: keep its right edge in place (right slingshot)
	int ax, ay;            // a point of the rubber line
	Timer hit_timer;
	bool hit;
};
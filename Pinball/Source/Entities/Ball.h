#pragma once

#include "Entities/Entity.h"
#include "raylib.h"

class Application;

class Ball : public PhysicEntity
{
public:
	Ball(PhysBody* body, Application* app, Texture2D texture);

	void Update() override;

private:
	Application* app;   // to reach the renderer
	Texture2D texture;  // loaded once by ModuleGame
};
#pragma once

class PhysBody;

// Every kind of thing on the table, so the game knows what it hit
enum class EntityType
{
	WALL,
	BALL,
	FLIPPER,
	KICKER,
	BUMPER,
	SLINGSHOT,
	LIGHT_TARGET,
	DEATH_ZONE,
	CRYSTAL
};

// Base class for everything in the scene that has a physics body
class PhysicEntity
{
protected:
	// Protected: nobody creates a "generic" entity, only its children
	PhysicEntity(PhysBody* body, EntityType type) : body(body), type(type) {}

public:
	virtual ~PhysicEntity() = default;

	// Logic + draw every frame (empty by default: a wall does nothing)
	virtual void Update() {}

	// Called when something touches this entity (bumper gives points, etc.)
	virtual void OnHit(PhysicEntity* other) {}

	EntityType GetType() const { return type; }

public:
	PhysBody* body;

protected:
	EntityType type;
};
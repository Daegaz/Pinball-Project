#pragma once
#include "Entities/Entity.h"

// Static chain: the outer border and every obstacle of the table
class Wall : public PhysicEntity
{
public:
	Wall(PhysBody* body) : PhysicEntity(body, EntityType::WALL){}
};
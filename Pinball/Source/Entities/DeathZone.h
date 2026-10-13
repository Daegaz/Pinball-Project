#pragma once
#include "Entities/Entity.h"

// Invisible sensor at the bottom: the ball touching it is a lost ball
class DeathZone : public PhysicEntity
{
public:
	DeathZone(PhysBody* body) : PhysicEntity(body, EntityType::DEATH_ZONE){}
};
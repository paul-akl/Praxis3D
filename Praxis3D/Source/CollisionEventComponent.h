#pragma once

#include "CommonDefinitions.h"
#include "EngineDefinitions.h"
#include "Math.h"

struct CollisionEvent
{
	CollisionEvent() : m_entityID(NULL_ENTITY_ID), m_position(0.0f), m_appliedImpulse(0.0f), m_firstObjInCollisionPair(false) { }

	EntityID m_entityID;
	glm::vec3 m_position;
	glm::vec3 m_velocity;
	btTransform m_worldTransform;
	float m_appliedImpulse;
	bool m_firstObjInCollisionPair;
};

struct CollisionEventComponent
{
	CollisionEventComponent(EntityID p_entityID) : m_entityID(p_entityID), m_numOfDynamicCollisions{ 0, 0 }, m_numOfStaticCollisions{ 0, 0 } { }
	~CollisionEventComponent() { }

	CollisionEvent m_dynamicCollisions[2][NUM_DYNAMIC_COLLISION_EVENTS];
	CollisionEvent m_staticCollisions[2][NUM_STATIC_COLLISION_EVENTS];

	size_t m_numOfDynamicCollisions[2];
	size_t m_numOfStaticCollisions[2];

	EntityID m_entityID;
};
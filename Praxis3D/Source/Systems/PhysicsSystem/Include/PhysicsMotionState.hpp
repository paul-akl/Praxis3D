#pragma once

#include <bullet3/btBulletDynamicsCommon.h>

#include "Definitions/Include/InheritanceObjects.hpp"
#include "Math/Include/Math.hpp"

class PhysicsMotionState : public btMotionState//, public SpatialDataManagerObject
{
public:
	PhysicsMotionState()
	{
		m_motionStateDirty = false;
		m_graphicsWorldTransUpToDate = false;
		m_centerOfMassWorldTrans = btTransform::getIdentity();
		m_graphicsWorldTrans = glm::mat4(1.0f);
	}
	~PhysicsMotionState()
	{

	}

	inline void updateMotionStateTrans()
	{		
		// Update the graphics world transform (from btTransform) if it is not up to date
		if(!m_graphicsWorldTransUpToDate)
		{
			m_graphicsWorldTrans = Math::toGlmMat4(m_centerOfMassWorldTrans);
			m_graphicsWorldTransUpToDate = true;
		}
	}

	// Get the world transform in the form of btTransform
	void getWorldTransform(btTransform &p_worldTrans) const 
	{ 
		p_worldTrans = m_centerOfMassWorldTrans; 
	}

	// Get the world transform in the form of glm::mat4
	const inline glm::mat4 &getWorldTransform() const { return m_graphicsWorldTrans; }

	const glm::vec3 &getPosition() const
	{ 
		m_tempPosition = Math::toGlmVec3(m_centerOfMassWorldTrans.getOrigin());
		return m_tempPosition;
	}
	const glm::quat &getRotation() const
	{ 
		m_tempRotation = Math::toGlmQuat(m_centerOfMassWorldTrans.getRotation());
		return m_tempRotation;
	}

	void setPosition(const glm::vec3 &p_position)
	{
		m_centerOfMassWorldTrans.setOrigin(Math::toBtVector3(p_position));
		m_graphicsWorldTransUpToDate = false;
		m_motionStateDirty = true;
	}	
	void setPosition(const glm::mat4 &p_worldTrans)
	{
		m_centerOfMassWorldTrans.setOrigin(Math::toBtVector3(p_worldTrans[3]));
		m_graphicsWorldTransUpToDate = false;
		m_motionStateDirty = true;
	}
	void setRotation(const glm::quat &p_rotation)
	{
		m_centerOfMassWorldTrans.setRotation(Math::toBtQuaternion(p_rotation));
		m_graphicsWorldTransUpToDate = false;
		m_motionStateDirty = true;
	}

	//Bullet only calls the update of world transform for active objects
	void setWorldTransform(const btTransform &p_worldTrans) 
	{
		m_centerOfMassWorldTrans = p_worldTrans;
		m_graphicsWorldTransUpToDate = false;
		m_motionStateDirty = true;
	}
	void setWorldTransform(const glm::mat4 &p_worldTrans)
	{
		//setPosition(glm::vec3(p_worldTrans[3][0], p_worldTrans[3][1], p_worldTrans[3][2]));
		auto test = p_worldTrans[0];
		m_graphicsWorldTrans = p_worldTrans;
		m_centerOfMassWorldTrans = Math::toBtTransform(p_worldTrans);
		m_graphicsWorldTransUpToDate = true;
		m_motionStateDirty = true;
	}

	const inline bool getMotionStateDirtyFlag() const { return m_motionStateDirty; }
	const inline bool getMotionStateDirtyFlagAndReset()
	{ 
		if(m_motionStateDirty)
		{
			resetMotionStateDirtyFlag();
			return true;
		}
		else
		{
			resetMotionStateDirtyFlag();
			return false;
		}
	}

	const inline void resetMotionStateDirtyFlag() { m_motionStateDirty = false; }

private:
	// Mutable spatial data used in const getters required by on observer
	mutable glm::vec3 m_tempPosition;
	mutable glm::quat m_tempRotation;

	btTransform m_centerOfMassWorldTrans;
	glm::mat4 m_graphicsWorldTrans;
	bool m_graphicsWorldTransUpToDate;
	bool m_motionStateDirty;
};
#include "Systems/PhysicsSystem/Include/PhysicsScene.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"

void RigidBodyComponent::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	// Track what data has been modified
	BitMask newChanges = Systems::Changes::None;

	// Track whether any spatial data was modified, so that the transform matrix can be recreated
	bool transformModifed = false;

	if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
	{
		// Get the active flag from the subject and set the active flag accordingly
		setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
	}

	// Consider ignoring LocalTransform change, as Bullet can only accept a transform matrix that does not have scale applied to it. LocalTransform however includes scaling.
	// To avoid scaled transform, only the position is retrieved from the LocalTransform, and the rotation is retrieved by getting a LocalRotation quaternion.
	// This might cause a problem of getting an out-of-date rotation, as it is not certain if the LocalRotation quaternion has been updated.
	if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransform))
	{
		m_motionState.setPosition(p_subject->getMat4(this, Systems::Changes::Spatial::LocalTransform));
		m_motionState.setRotation(p_subject->getQuaternion(this, Systems::Changes::Spatial::LocalRotation));

		transformModifed = true;

		newChanges |= Systems::Changes::Spatial::LocalTransformNoScale;
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransformNoScale))
	{
		m_motionState.setWorldTransform(p_subject->getMat4(this, Systems::Changes::Spatial::LocalTransformNoScale));

		transformModifed = true;

		newChanges |= Systems::Changes::Spatial::LocalTransformNoScale;
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalPosition))
	{
		m_motionState.setPosition(p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition));

		transformModifed = true;

		newChanges |= Systems::Changes::Spatial::LocalPosition;
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalRotation))
	{
		m_motionState.setRotation(p_subject->getQuaternion(this, Systems::Changes::Spatial::LocalRotation));

		transformModifed = true;

		newChanges |= Systems::Changes::Spatial::LocalRotation;
	}

	if(transformModifed)
	{
		btTransform transform;
		m_motionState.getWorldTransform(transform);

		m_rigidBody->setWorldTransform(transform);
		m_rigidBody->getMotionState()->setWorldTransform(transform);

		m_rigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
		m_rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
		m_rigidBody->clearForces();
		m_rigidBody->activate(true);
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Spatial::Velocity))
	{
		m_rigidBody->setLinearVelocity(Math::toBtVector3(p_subject->getVec3(this, Systems::Changes::Spatial::Velocity)));

		newChanges |= Systems::Changes::Spatial::Velocity;
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::CollisionShapeSize))
	{
		switch(m_collisionShapeType)
		{
		case CollisionShapeType::CollisionShapeType_Box:
			m_collisionShape.m_boxShape->setImplicitShapeDimensions(Math::toBtVector3(
				p_subject->getVec3(this, Systems::Changes::Physics::CollisionShapeSize) - glm::vec3(m_collisionShape.m_boxShape->getMargin())));
			static_cast<PhysicsScene *>(m_systemScene)->cleanProxyFromPairs(*m_rigidBody);
			break;
		case CollisionShapeType::CollisionShapeType_Capsule:
			break;
		case CollisionShapeType::CollisionShapeType_Cone:
			break;
		case CollisionShapeType::CollisionShapeType_ConvexHull:
			break;
		case CollisionShapeType::CollisionShapeType_Cylinder:
			m_collisionShape.m_cylinderShape->setImplicitShapeDimensions(Math::toBtVector3(
				p_subject->getVec3(this, Systems::Changes::Physics::CollisionShapeSize) - glm::vec3(m_collisionShape.m_cylinderShape->getMargin())));
			static_cast<PhysicsScene *>(m_systemScene)->cleanProxyFromPairs(*m_rigidBody);
			break;
		case CollisionShapeType::CollisionShapeType_Sphere:
			m_collisionShape.m_sphereShape->setImplicitShapeDimensions(Math::toBtVector3(
				p_subject->getVec3(this, Systems::Changes::Physics::CollisionShapeSize)));
			static_cast<PhysicsScene *>(m_systemScene)->cleanProxyFromPairs(*m_rigidBody);
			break;
		}
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::CollisionShapeType))
	{
		auto collisionShapeTypeNumber = p_subject->getUnsignedInt(this, Systems::Changes::Physics::CollisionShapeType);
		if(collisionShapeTypeNumber >= 0 && collisionShapeTypeNumber < CollisionShapeType::CollisionShapeType_NumOfTypes)
		{
			// Save the current collision shape size for the new shape
			const auto oldCollisionShapeSize = getCollisionShapeSize();

			// Get the new collision shape type
			m_collisionShapeType = static_cast<CollisionShapeType>(collisionShapeTypeNumber);

			// Delete the old collision shape
			delete m_rigidBody->getCollisionShape();

			// Create the new collision shape
			switch(m_collisionShapeType)
			{
				case CollisionShapeType::CollisionShapeType_Box:
					m_collisionShape.m_boxShape = new btBoxShape(Math::toBtVector3(oldCollisionShapeSize));
					m_rigidBody->setCollisionShape(m_collisionShape.m_boxShape);
					break;
				case CollisionShapeType::CollisionShapeType_Capsule:
					break;
				case CollisionShapeType::CollisionShapeType_Cone:
					break;
				case CollisionShapeType::CollisionShapeType_ConvexHull:
					break;
				case CollisionShapeType::CollisionShapeType_Cylinder:
					m_collisionShape.m_cylinderShape = new btCylinderShape(Math::toBtVector3(oldCollisionShapeSize));
					m_rigidBody->setCollisionShape(m_collisionShape.m_cylinderShape);
					break;
				case CollisionShapeType::CollisionShapeType_Sphere:
					m_collisionShape.m_sphereShape = new btSphereShape(oldCollisionShapeSize.x);
					m_rigidBody->setCollisionShape(m_collisionShape.m_sphereShape);
					break;
			}

			// Reload physics scene pairs
			static_cast<PhysicsScene *>(m_systemScene)->cleanProxyFromPairs(*m_rigidBody);
		}
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::Friction))
	{
		m_rigidBody->setFriction(p_subject->getFloat(this, Systems::Changes::Physics::Friction));
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::RollingFriction))
	{
		m_rigidBody->setRollingFriction(p_subject->getFloat(this, Systems::Changes::Physics::RollingFriction));
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::SpinningFriction))
	{
		m_rigidBody->setSpinningFriction(p_subject->getFloat(this, Systems::Changes::Physics::SpinningFriction));
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::Mass))
	{
		const auto mass = p_subject->getFloat(this, Systems::Changes::Physics::Mass);
		getCollisionShape()->calculateLocalInertia(mass, m_constructionInfo->m_localInertia);
		m_rigidBody->setMassProps(mass, m_constructionInfo->m_localInertia);
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::Restitution))
	{
		m_rigidBody->setRestitution(p_subject->getFloat(this, Systems::Changes::Physics::Restitution));
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Physics::Kinematic))
	{
		bool kinematic = p_subject->getBool(this, Systems::Changes::Physics::Kinematic);

		if(kinematic)
		{
			m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT);
			m_rigidBody->setActivationState(DISABLE_DEACTIVATION);

			//Globals::phyWorld->removeRigidBody(Obj->BodyRigid);
			//Obj->BodyRigid->setMassProps(0, btVector3(0, 0, 0));
			//Obj->BodyRigid->setCollisionFlags(Obj->BodyRigid->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			//Obj->BodyRigid->setLinearVelocity(btVector3(0, 0, 0));
			//Obj->BodyRigid->setAngularVelocity(btVector3(0, 0, 0));
			//Obj->BodyRigid->setActivationState(DISABLE_DEACTIVATION);
			//Globals::phyWorld->addRigidBody(Obj->BodyRigid);
		}
		else
		{
			m_rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_DYNAMIC_OBJECT);
			m_rigidBody->setActivationState(WANTS_DEACTIVATION);

			//Globals::phySoftWorld->removeRigidBody(Obj->BodyRigid);
			//btVector3 inertia(0, 0, 0);
			//Obj->BodyShape->calculateLocalInertia(Obj->Behavior.ObjTraits->Mass, inertia);
			//Obj->BodyRigid->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
			//Obj->BodyRigid->setMassProps(Obj->Behavior.ObjTraits->Mass, inertia);
			//Obj->BodyRigid->updateInertiaTensor();
			//Obj->BodyRigid->setLinearVelocity(btVector3(0, 0, 0));
			//Obj->BodyRigid->setAngularVelocity(btVector3(0, 0, 0));
			//Obj->BodyRigid->setActivationState(WANTS_DEACTIVATION);
			//Globals::phySoftWorld->addRigidBody(Obj->BodyRigid);
		}
	}


	//if(CheckBitmask(p_changeType, Systems::Changes::Physics::AllDynamic))
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Physics::Impulse))
		{

		}

		if(CheckBitmask(p_changeType, Systems::Changes::Physics::Torque))
		{
			m_rigidBody->applyTorqueImpulse(Math::toBtVector3(p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition)));
			m_rigidBody->activate(true);
		}
	}

	postChanges(newChanges);
}

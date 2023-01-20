#pragma once

#include "InheritanceObjects.h"
#include "PhysicsMotionState.h"

class RigidBodyComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class PhysicsScene;
public:
	enum CollisionShapeType : unsigned int
	{
		CollisionShapeType_Null = 0,
		CollisionShapeType_Box,
		CollisionShapeType_Capsule,
		CollisionShapeType_Cone,
		CollisionShapeType_ConvexHull,
		CollisionShapeType_Cylinder,
		CollisionShapeType_Sphere
	};

	struct RigidBodyComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		RigidBodyComponentConstructionInfo()
		{
			m_friction = 0.5f;
			m_mass = 0.0f;
			m_restitution = 0.0f;
			m_kinematic = false;
			m_collisionShapeType = CollisionShapeType::CollisionShapeType_Null;
			m_collisionShapeSize = glm::vec3(0.5f);
		}

		float m_friction;
		float m_mass;
		float m_restitution;
		bool m_kinematic;
		CollisionShapeType m_collisionShapeType;
		glm::vec3 m_collisionShapeSize;
	};

	RigidBodyComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::RigidBodyComponent, p_entityID)
	{
		m_collisionShapeType = CollisionShapeType::CollisionShapeType_Null;
		m_collisionShape.m_boxShape = nullptr;
		m_constructionInfo = nullptr;
		m_rigidBody = nullptr;
		m_kinematic = false;
	}
	~RigidBodyComponent()
	{

	}
	ErrorCode init() final override
	{
		// Mark the object as loaded, because there is nothing to be specifically loaded, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		auto luaError = ErrorCode::Success;// m_luaScript.init();

		if(luaError != ErrorCode::Success)
			ErrHandlerLoc().get().log(luaError, ErrorSource::Source_LuaComponent, m_name);
		//else
		//	m_luaScriptLoaded = true;

		setActive(true);
	}

	void update(const float p_deltaTime)
	{
		if(m_motionState.getMotionStateDirtyFlagAndReset())
		{
			m_motionState.updateMotionStateTrans();

			postChanges(Systems::Changes::Spatial::LocalTransformNoScale);
		}

		//std::cout << m_motionState.getWorldTransform()[3].x << " : " << m_motionState.getWorldTransform()[3].y << " : " << m_motionState.getWorldTransform()[3].z << std::endl;

		/*/ Perform updates only the if the script is loaded
		if(m_luaScriptLoaded)
		{
			// Get the current spatial data
			m_luaSpatialData.setSpatialData(*m_spatialData);

			// Update the lua script
			m_luaScript.update(p_deltaTime);

			// Get the changes from the lua script
			auto changes = m_luaScript.getChanges();

			// Post the new changes
			postChanges(changes);
		}*/
	}

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			if(p_properties.getPropertyID() == Properties::RigidBodyComponent)
			{
				// --------------------
				// Load collision shape
				// --------------------
				auto const &collisionShapeProperty = p_properties.getPropertySetByID(Properties::CollisionShape);
				if(collisionShapeProperty)
				{
					// Get the type of the collision shape and load the data based on it
					auto const &typeProperty = collisionShapeProperty.getPropertyByID(Properties::Type);
					if(typeProperty)
					{
						switch(typeProperty.getID())
						{
						case Properties::Box:
						{
							// Get the size of the box by half-extents
							auto const &sizeProperty = collisionShapeProperty.getPropertyByID(Properties::Size);
							btVector3 boxHalfExtents(0.5f, 0.5f, 0.5f);

							// If the size was not given, leave it to a default 0.5f all around (so it gets to the final 1.0/1.0/1.0 dimension)
							if(sizeProperty)
								boxHalfExtents = Math::toBtVector3(sizeProperty.getVec3f());
							else
								ErrHandlerLoc().get().log(ErrorCode::Property_missing_size, m_name, ErrorSource::Source_RigidBodyComponent);

							m_collisionShape.m_boxShape = new btBoxShape(boxHalfExtents);
							m_collisionShapeType = CollisionShapeType::CollisionShapeType_Box;
						}
						break;

						case Properties::Sphere:
						{
							// Get the size of the sphere in its radius
							auto const &radiusProperty = collisionShapeProperty.getPropertyByID(Properties::Radius);
							float radius(0.5f);

							// If the size was not given, leave it to a default radius of 0.5f (which makes the sphere diameter equal to 1.0)
							if(radiusProperty)
								radius = radiusProperty.getFloat();
							else
								ErrHandlerLoc().get().log(ErrorCode::Property_missing_radius, m_name, ErrorSource::Source_RigidBodyComponent);

							m_collisionShape.m_sphereShape = new btSphereShape(radius);
							m_collisionShapeType = CollisionShapeType::CollisionShapeType_Sphere;
						}
						break;

						default:
							
							// If this is reached, the collision shape type was not valid
							importError = ErrorCode::Failure;
							ErrHandlerLoc().get().log(ErrorCode::Collision_invalid, m_name, ErrorSource::Source_RigidBodyComponent);
							break;
						}

						// Success on the loaded collision shape
						importError = ErrorCode::Success;
						ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_RigidBodyComponent, m_name + " - Collision shape loaded");

					}
					else
					{
						// Missing the Type property entirely
						importError = ErrorCode::Failure;
						ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, m_name, ErrorSource::Source_RigidBodyComponent);
					}
				}

				// Continue only if collision shape was created
				if(m_collisionShapeType != CollisionShapeType::CollisionShapeType_Null)
				{
					// Create the struct that holds all the required information for constructing a rigid body
					m_constructionInfo = new btRigidBody::btRigidBodyConstructionInfo(0.0f, &m_motionState, getCollisionShape());

					// -----------------------------
					// Load individual property data
					// -----------------------------
					for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
					{
						switch(p_properties[i].getPropertyID())
						{
						case Properties::Friction:
							m_constructionInfo->m_friction = p_properties[i].getFloat();
							break;
						case Properties::Kinematic:
							m_kinematic = p_properties[i].getBool();
							break;
						case Properties::Mass:
							m_constructionInfo->m_mass = p_properties[i].getFloat();
							break;
						case Properties::Restitution:
							m_constructionInfo->m_restitution = p_properties[i].getFloat();
							break;
						}
					}

					// If mass is not zero, rigid body is dynamic; in that case, calculate the local inertia 
					if(m_constructionInfo->m_mass != 0.0f)
					{
						// Kinematic objects must have a mass of zero
						if(m_kinematic)
						{
							m_constructionInfo->m_mass = 0.0f;
							ErrHandlerLoc().get().log(ErrorCode::Kinematic_has_mass, m_name, ErrorSource::Source_RigidBodyComponent);
						}
						else
							getCollisionShape()->calculateLocalInertia(m_constructionInfo->m_mass, m_constructionInfo->m_localInertia);
					}
				}
				else
				{
					// Missing the collision shape
					importError = ErrorCode::Failure;
					ErrHandlerLoc().get().log(ErrorCode::Collision_missing, m_name, ErrorSource::Source_RigidBodyComponent);
				}


			}

			if(importError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(true);
			}
		}

		return importError;
	}

	PropertySet exportObject() final override
	{
		// Create the root Camera property set
		PropertySet propertySet(Properties::Camera);

		return propertySet;
	}

	// System type is Physics
	BitMask getSystemType() final override { return Systems::Physics; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Spatial::AllLocal; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::Spatial::AllLocal; }

	const inline CollisionShapeType getCollisionShapeType() const { return m_collisionShapeType; }
	inline btCollisionShape *getCollisionShape()
	{
		btCollisionShape *collisionShape = nullptr;

		switch(m_collisionShapeType)
		{
		case CollisionShapeType::CollisionShapeType_Box:
			collisionShape = m_collisionShape.m_boxShape;
			break;
		case CollisionShapeType::CollisionShapeType_Capsule:
			collisionShape = m_collisionShape.m_capsuleShape;
			break;
		case CollisionShapeType::CollisionShapeType_Cone:
			collisionShape = m_collisionShape.m_coneShape;
			break;
		case CollisionShapeType::CollisionShapeType_ConvexHull:
			collisionShape = m_collisionShape.m_convexHullShape;
			break;
		case CollisionShapeType::CollisionShapeType_Cylinder:
			collisionShape = m_collisionShape.m_cylinderShape;
			break;
		case CollisionShapeType::CollisionShapeType_Sphere:
			collisionShape = m_collisionShape.m_sphereShape;
			break;
		}

		return collisionShape;
	}
	inline btBoxShape *getCollisionShapeBox()               { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_Box        ? m_collisionShape.m_boxShape        : nullptr; }
	inline btCapsuleShape *getCollisionShapeCapsule()       { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_Capsule    ? m_collisionShape.m_capsuleShape    : nullptr; }
	inline btConeShape *getCollisionShapeCone()             { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_Cone       ? m_collisionShape.m_coneShape       : nullptr; }
	inline btConvexHullShape *getCollisionShapeConvexHull() { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_ConvexHull ? m_collisionShape.m_convexHullShape : nullptr; }
	inline btCylinderShape *getCollisionShapeCylinder()     { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_Cylinder   ? m_collisionShape.m_cylinderShape   : nullptr; }
	inline btSphereShape *getCollisionShapeSphere()         { return m_collisionShapeType == CollisionShapeType::CollisionShapeType_Sphere     ? m_collisionShape.m_sphereShape     : nullptr; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// Consider ignoring LocalTransform change, as Bullet can only accept a transform matrix that does not have scale applied to it. LocalTransform however includes scaling.
		// To avoid scaled transform, only the position is retrieved from the LocalTransform, and the rotation is retrieved by getting a LocalRotation quaternion.
		// This might cause a problem of getting an out-of-date rotation, as it is not certain if the LocalRotation quaternion has been updated.
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransform))
		{
			m_motionState.setPosition(p_subject->getMat4(this, Systems::Changes::Spatial::LocalTransform));
			m_motionState.setRotation(p_subject->getQuaternion(this, Systems::Changes::Spatial::LocalRotation));

			newChanges |= Systems::Changes::Spatial::LocalTransformNoScale;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransformNoScale))
		{
			m_motionState.setWorldTransform(p_subject->getMat4(this, Systems::Changes::Spatial::LocalTransformNoScale));

			newChanges |= Systems::Changes::Spatial::LocalTransformNoScale;
		}
		
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalPosition))
		{
			m_motionState.setPosition(p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition));

			newChanges |= Systems::Changes::Spatial::LocalPosition;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalRotation))
		{
			m_motionState.setRotation(p_subject->getQuaternion(this, Systems::Changes::Spatial::LocalRotation));

			newChanges |= Systems::Changes::Spatial::LocalRotation;
		}

		postChanges(newChanges);
	}

	const glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits)								const override { return m_motionState.getWorldTransform(); }
	const glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits)						const override { return m_motionState.getRotation(); }
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits)								const override { return m_motionState.getPosition(); }

private:
	union
	{
		btBoxShape *m_boxShape;
		btCapsuleShape *m_capsuleShape;
		btConeShape *m_coneShape;
		btConvexHullShape *m_convexHullShape;
		btCylinderShape *m_cylinderShape;
		btSphereShape *m_sphereShape;
	} m_collisionShape;

	CollisionShapeType m_collisionShapeType;

	btRigidBody *m_rigidBody; 
	
	btRigidBody::btRigidBodyConstructionInfo *m_constructionInfo;

	PhysicsMotionState m_motionState;

	bool m_kinematic;
};
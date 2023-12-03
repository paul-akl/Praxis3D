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
		CollisionShapeType_Sphere,
		CollisionShapeType_NumOfTypes
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
		glm::vec3 m_linearVelocity;
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
		if(m_constructionInfo != nullptr)
			delete m_constructionInfo;

	}
	ErrorCode init() final override
	{
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		setActive(true);
	}

	void update(const float p_deltaTime)
	{
		if(m_motionState.getMotionStateDirtyFlagAndReset())
		{
			m_motionState.updateMotionStateTrans();

			postChanges(Systems::Changes::Spatial::LocalTransformNoScale);
		}
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
	inline const std::vector<const char *> &getCollisionTypeText() const { return m_collisionShapeTypeText; }
	inline const btRigidBody *getRigidBody() const { return m_rigidBody; }
	inline const glm::vec3 getCollisionShapeSize() const
	{
		switch(m_collisionShapeType)
		{
			case CollisionShapeType::CollisionShapeType_Box:
				return Math::toGlmVec3(m_collisionShape.m_boxShape->getImplicitShapeDimensions());
				break;
			case CollisionShapeType::CollisionShapeType_Capsule:
				return Math::toGlmVec3(m_collisionShape.m_capsuleShape->getImplicitShapeDimensions());
				break;
			case CollisionShapeType::CollisionShapeType_Cone:
				return Math::toGlmVec3(m_collisionShape.m_coneShape->getImplicitShapeDimensions());
				break;
			case CollisionShapeType::CollisionShapeType_ConvexHull:
				return Math::toGlmVec3(m_collisionShape.m_convexHullShape->getImplicitShapeDimensions());
				break;
			case CollisionShapeType::CollisionShapeType_Cylinder:
				return Math::toGlmVec3(m_collisionShape.m_cylinderShape->getImplicitShapeDimensions());
				break;
			case CollisionShapeType::CollisionShapeType_Sphere:
				return Math::toGlmVec3(m_collisionShape.m_sphereShape->getImplicitShapeDimensions());
				break;
		}

		return glm::vec3(0.5f);
	}

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

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

	std::vector<const char *> m_collisionShapeTypeText { "null", "Box", "Capsule", "Cone", "Convex hull", "Cylinder", "Sphere" };

	btRigidBody *m_rigidBody; 
	
	btRigidBody::btRigidBodyConstructionInfo *m_constructionInfo;

	PhysicsMotionState m_motionState;

	bool m_kinematic;
};
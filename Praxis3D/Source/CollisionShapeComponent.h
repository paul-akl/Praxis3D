#pragma once

#include <bullet3/btBulletDynamicsCommon.h>

#include "InheritanceObjects.h"

class CollisionShapeComponent : public SystemObject, public LoadableGraphicsObject
{
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

	CollisionShapeComponent(SystemScene *p_systemScene, std::string p_name, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::CollisionShapeComponent)
	{
		m_collisionShapeType = CollisionShapeType::CollisionShapeType_Null;
		m_collisionShape.m_boxShape = nullptr;
	}
	~CollisionShapeComponent()
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
			if(p_properties.getPropertyID() == Properties::CollisionShape)
			{
				auto const &typeProperty = p_properties.getPropertyByID(Properties::Type);

				if(typeProperty)
				{
					switch(typeProperty.getID())
					{
					case Properties::Box:
					{
						auto const &sizeProperty = p_properties.getPropertyByID(Properties::Size);
						btVector3 boxHalfExtents(1.0f, 1.0f, 1.0f);

						if(sizeProperty)
							boxHalfExtents = Math::toBtVector3(sizeProperty.getVec3f());
						else
							ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_CallisionShapeComponent, m_name + " - Missing \'" + GetString(Properties::Size) + "\' property");
						
						m_collisionShape.m_boxShape = new btBoxShape(boxHalfExtents);
						m_collisionShapeType = CollisionShapeType::CollisionShapeType_Box;
					}
						break;

					case Properties::Sphere:
					{
						auto const &radiusProperty = p_properties.getPropertyByID(Properties::Radius);
						float radius(1.0f);

						if(radiusProperty)
							radius = radiusProperty.getFloat();
						else
							ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_CallisionShapeComponent, m_name + " - Missing \'" + GetString(Properties::Radius) + "\' property");

						m_collisionShape.m_sphereShape = new btSphereShape(radius);
						m_collisionShapeType = CollisionShapeType::CollisionShapeType_Sphere;
					}
					break;

					default:
						importError = ErrorCode::Failure;
						ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_CallisionShapeComponent, m_name + " - Invalid collision type");
						break;
					}

					importError = ErrorCode::Success;
					ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_CallisionShapeComponent, m_name + " - Collision shape loaded");

				}
				else
				{
					importError = ErrorCode::Failure;
					ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_CallisionShapeComponent, m_name + " - Missing \'" + GetString(Properties::Type) + "\' property");
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

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::None; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	inline const char **getCollisionTypeText() const
	{
		const char *collisionTypeText[] = { "null", "Box", "Capsule", "Cone", "Convex hull", "Cylinder", "Sphere"};
		return collisionTypeText;
	}

	const inline CollisionShapeType getCollisionShapeType() const { return m_collisionShapeType; }
	inline btCollisionShape *getCollisionShape()
	{
		btCollisionShape *collisionShape = nullptr;

		switch(m_collisionShapeType)
		{
		case CollisionShapeComponent::CollisionShapeType_Box:
			collisionShape = m_collisionShape.m_boxShape;
			break;
		case CollisionShapeComponent::CollisionShapeType_Capsule:
			collisionShape = m_collisionShape.m_capsuleShape;
			break;
		case CollisionShapeComponent::CollisionShapeType_Cone:
			collisionShape = m_collisionShape.m_coneShape;
			break;
		case CollisionShapeComponent::CollisionShapeType_ConvexHull:
			collisionShape = m_collisionShape.m_convexHullShape;
			break;
		case CollisionShapeComponent::CollisionShapeType_Cylinder:
			collisionShape = m_collisionShape.m_cylinderShape;
			break;
		case CollisionShapeComponent::CollisionShapeType_Sphere:
			collisionShape = m_collisionShape.m_sphereShape;
			break;
		}

		return collisionShape;
	}
	inline btBoxShape *getCollisionShapeBox()				{ return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_Box ? m_collisionShape.m_boxShape : nullptr; }
	inline btCapsuleShape *getCollisionShapeCapsule()		{ return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_Capsule ? m_collisionShape.m_capsuleShape : nullptr; }
	inline btConeShape *getCollisionShapeCone()				{ return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_Cone ? m_collisionShape.m_coneShape : nullptr; }
	inline btConvexHullShape *getCollisionShapeConvexHull() { return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_ConvexHull ? m_collisionShape.m_convexHullShape : nullptr; }
	inline btCylinderShape *getCollisionShapeCylinder()		{ return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_Cylinder ? m_collisionShape.m_cylinderShape : nullptr; }
	inline btSphereShape *getCollisionShapeSphere()			{ return m_collisionShapeType == CollisionShapeComponent::CollisionShapeType_Sphere ? m_collisionShape.m_sphereShape : nullptr; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

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
};
#pragma once

// TODO: add release and debug libs based on compile configuration
#ifdef _DEBUG
#pragma comment(lib, "bullet/Bullet2FileLoader_Debug.lib")
#pragma comment(lib, "bullet/Bullet3Collision_Debug.lib")
#pragma comment(lib, "bullet/Bullet3Common_Debug.lib")
#pragma comment(lib, "bullet/Bullet3Dynamics_Debug.lib")
#pragma comment(lib, "bullet/Bullet3Geometry_Debug.lib")
#pragma comment(lib, "bullet/Bullet3OpenCL_clew_Debug.lib")
#pragma comment(lib, "bullet/BulletCollision_Debug.lib")
#pragma comment(lib, "bullet/BulletDynamics_Debug.lib")
#pragma comment(lib, "bullet/BulletExampleBrowserLib_Debug.lib")
#pragma comment(lib, "bullet/BulletFileLoader_Debug.lib")
#pragma comment(lib, "bullet/BulletInverseDynamicsUtils_Debug.lib")
#pragma comment(lib, "bullet/BulletInverseDynamics_Debug.lib")
#pragma comment(lib, "bullet/BulletRoboticsGUI_Debug.lib")
#pragma comment(lib, "bullet/BulletRobotics_Debug.lib")
#pragma comment(lib, "bullet/BulletSoftBody_Debug.lib")
#pragma comment(lib, "bullet/BulletWorldImporter_Debug.lib")
#pragma comment(lib, "bullet/BulletXmlWorldImporter_Debug.lib")
#pragma comment(lib, "bullet/BussIK_Debug.lib")
#pragma comment(lib, "bullet/clsocket_Debug.lib")
#pragma comment(lib, "bullet/ConvexDecomposition_Debug.lib")
#pragma comment(lib, "bullet/GIMPACTUtils_Debug.lib")
#pragma comment(lib, "bullet/gtest_Debug.lib")
#pragma comment(lib, "bullet/gwen_Debug.lib")
#pragma comment(lib, "bullet/HACD_Debug.lib")
#pragma comment(lib, "bullet/LinearMath_Debug.lib")
#pragma comment(lib, "bullet/OpenGLWindow_Debug.lib")
#endif

#ifdef _RELEASE
#endif

#include <bullet3/btBulletDynamicsCommon.h>

#include "CollisionEventComponent.h"
#include "ObjectPool.h"
#include "System.h"
#include "PhysicsObject.h"
#include "PhysicsTask.h"

class PhysicsSystem;
struct ComponentsConstructionInfo;

struct PhysicsComponentsConstructionInfo
{
	PhysicsComponentsConstructionInfo()
	{
		m_rigidBodyConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const PhysicsComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<RigidBodyComponent::RigidBodyComponentConstructionInfo>(&m_rigidBodyConstructionInfo, &p_other.m_rigidBodyConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_rigidBodyConstructionInfo != nullptr)
			delete m_rigidBodyConstructionInfo;
	}

	RigidBodyComponent::RigidBodyComponentConstructionInfo *m_rigidBodyConstructionInfo;
};

class PhysicsScene : public SystemScene
{
public:
	PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader);
	~PhysicsScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void exportSetup(PropertySet &p_propertySet);

	void activate()
	{
		s_currentPhysicsScene = this;
	}

	void update(const float p_deltaTime);

	void internalTickCallback(btDynamicsWorld *p_world, btScalar p_timeStep);

	static void internalTickCallbackProxy(btDynamicsWorld *p_world, btScalar p_timeStep)
	{
		s_currentPhysicsScene->internalTickCallback(p_world, p_timeStep);
	}

	ErrorCode preload();

	void loadInBackground();

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const PhysicsComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject*> components;

		if(p_constructionInfo.m_rigidBodyConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_rigidBodyConstructionInfo, p_startLoading));

		// If any physics components were created, add a Collision Event component as well
		if(!components.empty())
			createCollisionEventComponent(p_entityID);

		return components;
	}
	
	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);
	void exportComponents(const EntityID p_entityID, PhysicsComponentsConstructionInfo &p_constructionInfo);

	SystemObject *createComponent(const EntityID &p_entityID, const RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	void createCollisionEventComponent(const EntityID &p_entityID);

	void exportComponent(RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const RigidBodyComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_friction = p_component.getRigidBody()->getFriction();
		p_constructionInfo.m_mass = p_component.getRigidBody()->getMass();
		p_constructionInfo.m_restitution = p_component.getRigidBody()->getRestitution();
		p_constructionInfo.m_kinematic = p_component.getRigidBody()->isKinematicObject();
		p_constructionInfo.m_collisionShapeType = p_component.getCollisionShapeType();
		p_constructionInfo.m_collisionShapeSize = p_component.getCollisionShapeSize();
		p_constructionInfo.m_linearVelocity = Math::toGlmVec3(p_component.getRigidBody()->getLinearVelocity());
	}

	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	void receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
	{
		switch(p_dataType)
		{
			case DataType_SimulationActive:
				m_simulationRunning = static_cast<bool>(p_data);
				break;
		}
	}

	SystemTask *getSystemTask() { return m_physicsTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::Physics; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	const glm::vec3 getGravity() const { return Math::toGlmVec3(m_dynamicsWorld->getGravity()); }

	// Fluch the collision contacts of a rigid body (used after changing the collision shape dimensions)
	void cleanProxyFromPairs(btRigidBody &p_rigidBody)
	{
		m_collisionBroadphase->getOverlappingPairCache()->cleanProxyFromPairs(p_rigidBody.getBroadphaseProxy(), m_collisionDispatcher);
	}

private:
	// Removes an object from a pool, by iterating checking each pool for matched index; returns true if the object was found and removed
	inline bool removeObjectFromPool(PhysicsObject &p_object)
	{
		/*/ Go over each physics object
		for(decltype(m_physicsObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_physicsObjects.getNumAllocated(),
			size = m_physicsObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
		{
			// Check if the physics object is allocated inside the pool container
			if(m_physicsObjects[i].allocated())
			{
				// Increment the number of allocated objects (early bail mechanism)
				numAllocObjecs++;

				// If the object matches with the one we are looking for, remove it from the physics object pool
				if(*m_physicsObjects[i].getObject() == p_object)
				{
					m_physicsObjects.remove(m_physicsObjects[i].getIndex());
					return true;
				}
			}
		}*/

		return false;
	}

	PhysicsTask *m_physicsTask;

	// Collision configuration
	btDefaultCollisionConfiguration *m_collisionConfiguration;
	btSequentialImpulseConstraintSolver *m_collisionSolver;
	btCollisionDispatcher *m_collisionDispatcher;
	btBroadphaseInterface *m_collisionBroadphase;

	// Bullet world
	btDiscreteDynamicsWorld *m_dynamicsWorld;

	// An array of all collision shapes
	btAlignedObjectArray<btCollisionShape*> m_collisionShapes;

	static PhysicsScene *s_currentPhysicsScene;

	bool m_simulationRunning;
};
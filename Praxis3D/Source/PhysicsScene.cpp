#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "PhysicsScene.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

PhysicsScene::PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_physicsTask = nullptr;
	m_collisionConfiguration = nullptr;
	m_collisionSolver = nullptr;
	m_collisionDispatcher = nullptr;
	m_collisionBroadphase = nullptr;
	m_dynamicsWorld = nullptr;
}

PhysicsScene::~PhysicsScene()
{
	//remove the rigidbodies from the dynamics world and delete them
	for(int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject *obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody *body = btRigidBody::upcast(obj);
		if(body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for(int i = 0; i < m_collisionShapes.size(); i++)
	{
		btCollisionShape *shape = m_collisionShapes[i];
		m_collisionShapes[i] = 0;
		delete shape;
	}

	//delete dynamics world
	if(m_dynamicsWorld != nullptr)
		delete m_dynamicsWorld;

	//delete solver
	if(m_collisionSolver != nullptr)
		delete m_collisionSolver;

	//delete broadphase
	if(m_collisionBroadphase != nullptr)
		delete m_collisionBroadphase;

	//delete dispatcher
	if(m_collisionDispatcher != nullptr)
		delete m_collisionDispatcher;

	//delete collision configuration
	if(m_collisionConfiguration != nullptr)
		delete m_collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	m_collisionShapes.clear();
}

ErrorCode PhysicsScene::init()
{
	m_physicsTask = new PhysicsTask(this);

	// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	m_collisionConfiguration = new btDefaultCollisionConfiguration();

	// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	m_collisionDispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	m_collisionBroadphase = new btDbvtBroadphase();

	// the default constraint solver . For parallel processing you can use a different solver (see Extras / BulletMultiThreaded)
	m_collisionSolver = new btSequentialImpulseConstraintSolver();

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_collisionDispatcher, m_collisionBroadphase, m_collisionSolver, m_collisionConfiguration);
	
	return ErrorCode::Success;
}

ErrorCode PhysicsScene::setup(const PropertySet &p_properties)
{	
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:

			break;
		case Properties::Gravity:
			m_dynamicsWorld->setGravity(Math::toBtVector3(p_properties[i].getVec3f()));
			break;
		}
	}

	return ErrorCode::Success;
}

void PhysicsScene::update(const float p_deltaTime)
{
	// Perform the physics simulation for the time step of the last frame
	m_dynamicsWorld->stepSimulation(p_deltaTime);

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Get the rigid body component view and iterate every entity that contains is
	auto rigidBodyView = worldScene->getEntityRegistry().view<RigidBodyComponent>();
	for(auto entity : rigidBodyView)
	{
		auto &component = rigidBodyView.get<RigidBodyComponent>(entity);

		component.update(p_deltaTime);
	}
}

ErrorCode PhysicsScene::preload()
{	
	// Load every physics object. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	/*TaskManagerLocator::get().parallelFor(size_t(0), m_physicsObjects.getPoolSize(), size_t(1), [=](size_t i)
		{
			if(m_physicsObjects[i].allocated())
			{
				m_physicsObjects[i].getObject()->loadToMemory();
			}
		});*/

	return ErrorCode::Success;
}

void PhysicsScene::loadInBackground()
{
}

std::vector<SystemObject*> PhysicsScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_physicsComponents, p_startLoading);
}

SystemObject *PhysicsScene::createComponent(const EntityID &p_entityID, const RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<RigidBodyComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the camera component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.m_collisionShapeType = p_constructionInfo.m_collisionShapeType;
		component.m_kinematic = p_constructionInfo.m_kinematic;
		component.m_objectType = Properties::PropertyID::RigidBodyComponent;
		component.setActive(p_constructionInfo.m_active);
		component.setLoadedToMemory(true);
		component.setLoadedToVideoMemory(true);

		if(component.m_collisionShapeType != RigidBodyComponent::CollisionShapeType::CollisionShapeType_Null)
		{
			switch(component.m_collisionShapeType)
			{
				case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Box:
				{
					btVector3 boxHalfExtents = Math::toBtVector3(p_constructionInfo.m_collisionShapeSize);
					component.m_collisionShape.m_boxShape = new btBoxShape(boxHalfExtents);
				}
				break;

				case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Sphere:
				{
					float radius = p_constructionInfo.m_collisionShapeSize.x;
					component.m_collisionShape.m_sphereShape = new btSphereShape(radius);
				}
				break;
			}

			// Success on the loaded collision shape
			ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_RigidBodyComponent, component.getName() + " - Collision shape loaded");

			// Create the struct that holds all the required information for constructing a rigid body
			component.m_constructionInfo = new btRigidBody::btRigidBodyConstructionInfo(p_constructionInfo.m_mass, &component.m_motionState, component.getCollisionShape());

			component.m_constructionInfo->m_friction = p_constructionInfo.m_friction;
			component.m_constructionInfo->m_restitution = p_constructionInfo.m_restitution;

			// If mass is not zero, rigid body is dynamic; in that case, calculate the local inertia 
			if(component.m_constructionInfo->m_mass != 0.0f)
			{
				// Kinematic objects must have a mass of zero
				if(component.m_kinematic)
				{
					component.m_constructionInfo->m_mass = 0.0f;
					ErrHandlerLoc().get().log(ErrorCode::Kinematic_has_mass, component.getName(), ErrorSource::Source_RigidBodyComponent);
				}
				else
					component.getCollisionShape()->calculateLocalInertia(component.m_constructionInfo->m_mass, component.m_constructionInfo->m_localInertia);
			}

			// Set the body origin in space to the position in Spatial Component, if the Spatial Component is present
			auto *spatialComponent = worldScene->getEntityRegistry().try_get<SpatialComponent>(p_entityID);
			if(spatialComponent != nullptr)
			{
				btTransform groundTransform;
				groundTransform.setIdentity();
				groundTransform.setOrigin(Math::toBtVector3(spatialComponent->getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_position));
				groundTransform.setRotation(Math::toBtQuaternion(spatialComponent->getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_rotationQuat));
				//groundTransform.setFromOpenGLMatrix(&spatialComponent->getSpatialDataChangeManager().getLocalSpaceData().m_transformMat[0][0]);
				component.m_motionState.setWorldTransform(groundTransform);

				component.m_motionState.updateMotionStateTrans();
			}

			// Add the collision shape
			m_collisionShapes.push_back(component.getCollisionShape());

			// Create the rigid body by passing the rigid body construction info
			component.m_rigidBody = new btRigidBody(*component.m_constructionInfo);

			if(component.m_kinematic)
			{
				component.m_rigidBody->setCollisionFlags(component.m_rigidBody->getCollisionFlags() | btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT);
				component.m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
			}

			// Set linear velocity if it is not zero
			if(p_constructionInfo.m_linearVelocity != glm::vec3())
				component.m_rigidBody->setLinearVelocity(Math::toBtVector3(p_constructionInfo.m_linearVelocity));

			// Add the rigid body to the dynamics world, essentially loading it into the physics system
			m_dynamicsWorld->addRigidBody(component.m_rigidBody);

			returnObject = &component;

		}
		else // Remove the component if it didn't have a collision shape
		{
			// Missing the collision shape
			worldScene->removeComponent<RigidBodyComponent>(p_entityID);
			ErrHandlerLoc().get().log(ErrorCode::Collision_missing, component.getName(), ErrorSource::Source_RigidBodyComponent);
		}
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<RigidBodyComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_RigidBodyComponent, component.getName());
	}

	return returnObject;
}

ErrorCode PhysicsScene::destroyObject(SystemObject *p_systemObject)
{	
	// Check if object is valid and belongs to the physics system
	if(p_systemObject != nullptr && p_systemObject->getSystemType() == Systems::Physics)
	{
		// Cast the system object to physics object, as it belongs to the renderer scene
		PhysicsObject *objectToDestroy = static_cast<PhysicsObject *>(p_systemObject);

		// Try to destroy the object; return success if it succeeds
		if(removeObjectFromPool(*objectToDestroy))
			return ErrorCode::Success;
	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

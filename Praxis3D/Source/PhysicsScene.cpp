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

SystemObject *PhysicsScene::createComponent(const EntityID &p_entityID, const std::string &p_entityName, const PropertySet &p_properties)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->createObject(p_properties);

	// Check if property set node is present
	if(p_properties)
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

		switch(p_properties.getPropertyID())
		{
		case Properties::PropertyID::RigidBodyComponent:
			{
				auto &component = worldScene->addComponent<RigidBodyComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::RigidBodyComponent), p_entityID);

				// Try to initialize the camera component
				auto componentInitError = component.init();
				if(componentInitError == ErrorCode::Success)
				{
					// Try to import the component
					auto const &componentImportError = component.importObject(p_properties);

					// Create the rigid body inside the dynamics world, if it was imported successfully
					if(componentImportError == ErrorCode::Success)
					{
						//btCollisionShape *collisionShape = component.getCollisionShape();


						//btVector3 localInertia(0, 0, 0);
						// If mass is not zero, rigid body is dynamic; in that case, calculate local inertia 
						//if(component.m_mass != 0.0f)
						//	collisionShape->calculateLocalInertia(component.m_mass, localInertia);

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
						
						// Set the required information for the rigid body constructor and create the rigid body
						//btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructor(component.m_mass, &component.m_motionState, collisionShape, localInertia);

						// Add the collision shape
						m_collisionShapes.push_back(component.getCollisionShape());

						// Create the rigid body by passing the rigid body construction info
						component.m_rigidBody = new btRigidBody(*component.m_constructionInfo);

						if(component.m_kinematic)
						{
							component.m_rigidBody->setCollisionFlags(component.m_rigidBody->getCollisionFlags() | btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT);
							component.m_rigidBody->setActivationState(DISABLE_DEACTIVATION);						
						}

						// Add the rigid body to the dynamics world, essentially loading it into the physics system
						m_dynamicsWorld->addRigidBody(component.m_rigidBody);

						returnObject = &component;
					}
					else // Remove the component if it failed to import
					{
						worldScene->removeComponent<RigidBodyComponent>(p_entityID);
						ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_RigidBodyComponent, p_entityName);
					}
				}
				else // Remove the component if it failed to initialize
				{
					worldScene->removeComponent<RigidBodyComponent>(p_entityID);
					ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_RigidBodyComponent, p_entityName);
				}
			}
			break;

		}
	}

	return returnObject;
}

SystemObject *PhysicsScene::createObject(const PropertySet &p_properties)
{	
	/*/ Check if property set node is present
	if(p_properties)
	{
		// Check if the Physics property is present
		auto &physicsProperty = p_properties.getPropertySetByID(Properties::Physics);
		if(physicsProperty)
		{
			// Get the object name
			auto &nameProperty = p_properties.getPropertyByID(Properties::Name);

			// Find a place for the new object in the pool
			auto physicsObjectFromPool = m_physicsObjects.newObject();

			// Check if the pool wasn't full
			if(physicsObjectFromPool != nullptr)
			{
				std::string name;

				// If the name property is missing, generate a unique name based on the object's index in the pool
				if(nameProperty)
					name = nameProperty.getString() + " (" + GetString(Properties::PhysicsObject) + ")";
				else
					name = GetString(Properties::PhysicsObject) + Utilities::toString(physicsObjectFromPool->getIndex());

				// Construct the PhysicsObject
				physicsObjectFromPool->construct(this, name);
				auto newGUIObject = physicsObjectFromPool->getObject();

				// Start importing the newly created object in a background thread
				newGUIObject->importObject(physicsProperty);

				return newGUIObject;
			}
			else
			{
				ErrHandlerLoc::get().log(ErrorCode::ObjectPool_full, ErrorSource::Source_GUIObject, "Failed to add PhysicsObject - \'" + nameProperty.getString() + "\'");
			}
		}
	}*/

	// If valid type was not specified, or object creation failed, return a null object instead
	return g_nullSystemBase.getScene()->createObject(p_properties);
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

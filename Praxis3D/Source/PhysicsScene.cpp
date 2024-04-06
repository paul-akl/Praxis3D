#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "PhysicsScene.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

PhysicsScene *PhysicsScene::s_currentPhysicsScene = nullptr;

PhysicsScene::PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Physics)
{
	m_physicsTask = nullptr;
	m_collisionConfiguration = nullptr;
	m_collisionSolver = nullptr;
	m_collisionDispatcher = nullptr;
	m_collisionBroadphase = nullptr;
	m_dynamicsWorld = nullptr;
	m_simulationRunning = true;
	resetErrors();
}

PhysicsScene::~PhysicsScene()
{
	// Remove the rigidbodies from the dynamics world and delete them
	for(int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject *obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody *body = btRigidBody::upcast(obj);
		if(body)
		{
			delete static_cast<EntityID *>(body->getUserPointer());
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	// Delete collision shapes
	for(int i = 0; i < m_collisionShapes.size(); i++)
	{
		btCollisionShape *shape = m_collisionShapes[i];
		m_collisionShapes[i] = nullptr;
		//delete shape;
	}

	// Delete dynamics world
	if(m_dynamicsWorld != nullptr)
		delete m_dynamicsWorld;

	// Delete solver
	if(m_collisionSolver != nullptr)
		delete m_collisionSolver;

	// Delete broadphase
	if(m_collisionBroadphase != nullptr)
		delete m_collisionBroadphase;

	// Delete dispatcher
	if(m_collisionDispatcher != nullptr)
		delete m_collisionDispatcher;

	// Delete collision configuration
	if(m_collisionConfiguration != nullptr)
		delete m_collisionConfiguration;

	// Next line is optional: it will be cleared by the destructor when the array goes out of scope
	m_collisionShapes.clear();
}

ErrorCode PhysicsScene::init()
{
	m_physicsTask = new PhysicsTask(this);

	// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	m_collisionConfiguration = new btDefaultCollisionConfiguration();

	// use the default collision dispatcher . For parallel processing you can use a different dispatcher (see Extras / BulletMultiThreaded)
	m_collisionDispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	m_collisionBroadphase = new btDbvtBroadphase();

	// the default constraint solver . For parallel processing you can use a different solver (see Extras / BulletMultiThreaded)
	m_collisionSolver = new btSequentialImpulseConstraintSolver();

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_collisionDispatcher, m_collisionBroadphase, m_collisionSolver, m_collisionConfiguration);
	
	m_dynamicsWorld->setInternalTickCallback(&PhysicsScene::internalTickCallbackProxy);

	return ErrorCode::Success;
}

ErrorCode PhysicsScene::setup(const PropertySet &p_properties)
{	
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Gravity:
			m_dynamicsWorld->setGravity(Math::toBtVector3(p_properties[i].getVec3f()));
			break;
		}
	}

	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<RigidBodyComponent>(std::max(Config::objectPoolVar().regid_body_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::RigidBodyComponent).getInt()));
	worldScene->reserve<CollisionEventComponent>(std::max(Config::objectPoolVar().regid_body_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::CollisionEventComponent).getInt()));

	return ErrorCode::Success;
}

void PhysicsScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::RigidBodyComponent, (int)worldScene->getPoolSize<RigidBodyComponent>());
	objectPoolSizePropertySet.addProperty(Properties::CollisionEventComponent, (int)worldScene->getPoolSize<CollisionEventComponent>());

	// Add physics world properties
	p_propertySet.addProperty(Properties::Gravity, Math::toGlmVec3(m_dynamicsWorld->getGravity()));
}

void PhysicsScene::update(const float p_deltaTime)
{
	// Reset errors for this frame
	resetErrors();

	// Get double buffering index
	const auto dbIndex = ClockLocator::get().getDoubleBufferingIndexBack();

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Get the collision event component view and iterate every entity while resetting the collision counters
	auto collisionEventsView = worldScene->getEntityRegistry().view<CollisionEventComponent>();
	for(auto entity : collisionEventsView)
	{
		auto &component = collisionEventsView.get<CollisionEventComponent>(entity);

		component.m_numOfDynamicCollisions[dbIndex] = 0;
		component.m_numOfStaticCollisions[dbIndex] = 0;
	}

	if(m_simulationRunning && !(m_sceneLoader->getFirstLoad() && m_sceneLoader->getSceneLoadingStatus()))
	{
		// Perform the physics simulation for the time step of the last frame
		m_dynamicsWorld->stepSimulation(p_deltaTime);
	}

	// Get the rigid body component view and iterate every entity that contains is
	auto rigidBodyView = worldScene->getEntityRegistry().view<RigidBodyComponent>();
	for(auto entity : rigidBodyView)
	{
		auto &component = rigidBodyView.get<RigidBodyComponent>(entity);

		component.update(p_deltaTime);
	}
}

void PhysicsScene::internalTickCallback(btDynamicsWorld *p_world, btScalar p_timeStep)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Get double buffering index
	const auto dbIndex = ClockLocator::get().getDoubleBufferingIndexBack();

	// Go over each manifold
	for(decltype(p_world->getDispatcher()->getNumManifolds()) manifoldIndex = 0, manifoldSize = p_world->getDispatcher()->getNumManifolds(); manifoldIndex < manifoldSize; manifoldIndex++)
	{
		// Get contact manifold
		const btPersistentManifold *contactManifold = p_world->getDispatcher()->getManifoldByIndexInternal(manifoldIndex);

		// Get collision objects that are in contact
		const btCollisionObject *objectA = static_cast<const btCollisionObject *>(contactManifold->getBody0());
		const btCollisionObject *objectB = static_cast<const btCollisionObject *>(contactManifold->getBody1());

		// Get entity IDs of objects that are in contact
		const EntityID entityA = *static_cast<EntityID *>(objectA->getUserPointer());
		const EntityID entityB = *static_cast<EntityID *>(objectB->getUserPointer());

		// Go over each contact
		for(decltype(contactManifold->getNumContacts()) contactIndex = 0, contactSize = contactManifold->getNumContacts(); contactIndex < contactSize; contactIndex++)
		{
			// Get collision event components that will contain the contact information
			auto *collisionEventComponentObjectA = entityRegistry.try_get<CollisionEventComponent>(entityA);
			auto *collisionEventComponentObjectB = entityRegistry.try_get<CollisionEventComponent>(entityB);

			// Get the contact point
			const btManifoldPoint &manifoldPoint = contactManifold->getContactPoint(contactIndex);

			// Check if the contact distance isn't negative and if the lifetime of the contact hasn't passed a set threshold
			if(manifoldPoint.getDistance() < 0.0f && manifoldPoint.m_lifeTime < Config::physicsVar().life_time_threshold)
			{
				// Determine whether the collision is static or dynamic based on whether the applied impulse of the collision is above a set threshold
				if(manifoldPoint.m_appliedImpulse > Config::physicsVar().applied_impulse_threshold)
				{
					// Process the dynamic collision event on the first object
					if(collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex] < NUM_DYNAMIC_COLLISION_EVENTS)
					{
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_firstObjInCollisionPair = true;
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_entityID = entityA;
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_appliedImpulse = manifoldPoint.m_appliedImpulse;
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_position = Math::toGlmVec3(manifoldPoint.getPositionWorldOnA());
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_velocity = Math::toGlmVec3(objectA->getInterpolationLinearVelocity());
						collisionEventComponentObjectA->m_dynamicCollisions[dbIndex][collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]].m_worldTransform = objectA->getWorldTransform();

						collisionEventComponentObjectA->m_numOfDynamicCollisions[dbIndex]++;
					}
					else
					{
						if(!m_maxDynamicCollisionsErrorIssued)
						{
							m_maxDynamicCollisionsErrorIssued = true;

							// Log an error if the dynamic collision event count exceeds the maximum number of supported dynamic events
							ErrHandlerLoc::get().log(ErrorCode::Collision_max_dynamic_events, ErrorSource::Source_Physics);
						}
					}

					// Process the dynamic collision event on the second object
					if(collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex] < NUM_DYNAMIC_COLLISION_EVENTS)
					{
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_firstObjInCollisionPair = false;
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_entityID = entityB;
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_appliedImpulse = manifoldPoint.m_appliedImpulse;
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_position = Math::toGlmVec3(manifoldPoint.getPositionWorldOnB());
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_velocity = Math::toGlmVec3(objectB->getInterpolationLinearVelocity());
						collisionEventComponentObjectB->m_dynamicCollisions[dbIndex][collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]].m_worldTransform = objectB->getWorldTransform();

						collisionEventComponentObjectB->m_numOfDynamicCollisions[dbIndex]++;
					}
					else
					{
						if(!m_maxStaticCollisionsErrorIssued)
						{
							m_maxStaticCollisionsErrorIssued = true;

							// Log an error if the dynamic collision event count exceeds the maximum number of supported dynamic events
							ErrHandlerLoc::get().log(ErrorCode::Collision_max_dynamic_events, ErrorSource::Source_Physics);
						}
					}
				}
				else
				{
					// Process the static collision event on the first object
					if(collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex] < NUM_STATIC_COLLISION_EVENTS)
					{
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_firstObjInCollisionPair = true;
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_entityID = entityA;
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_appliedImpulse = manifoldPoint.m_appliedImpulse;
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_position = Math::toGlmVec3(manifoldPoint.getPositionWorldOnA());
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_velocity = Math::toGlmVec3(objectA->getInterpolationLinearVelocity());
						collisionEventComponentObjectA->m_staticCollisions[dbIndex][collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]].m_worldTransform = objectA->getWorldTransform();

						collisionEventComponentObjectA->m_numOfStaticCollisions[dbIndex]++;
					}
					else
					{
						// Log an error if the static collision event count exceeds the maximum number of supported static events
						ErrHandlerLoc::get().log(ErrorCode::Collision_max_static_events, ErrorSource::Source_Physics);
					}

					// Process the static collision event on the second object
					if(collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex] < NUM_STATIC_COLLISION_EVENTS)
					{
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_firstObjInCollisionPair = false;
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_entityID = entityB;
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_appliedImpulse = manifoldPoint.m_appliedImpulse;
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_position = Math::toGlmVec3(manifoldPoint.getPositionWorldOnB());
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_velocity = Math::toGlmVec3(objectB->getInterpolationLinearVelocity());
						collisionEventComponentObjectB->m_staticCollisions[dbIndex][collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]].m_worldTransform = objectB->getWorldTransform();

						collisionEventComponentObjectB->m_numOfStaticCollisions[dbIndex]++;
					}
					else
					{
						// Log an error if the static collision event count exceeds the maximum number of supported static events
						ErrHandlerLoc::get().log(ErrorCode::Collision_max_static_events, ErrorSource::Source_Physics);
					}
				}
			}
		}
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

std::vector<SystemObject *> PhysicsScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(p_entityID);
	if(rigidBodyComponent != nullptr)
		returnVector.push_back(rigidBodyComponent);

	return returnVector;
}

std::vector<SystemObject*> PhysicsScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_physicsComponents, p_startLoading);
}

void PhysicsScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_physicsComponents);
}

void PhysicsScene::exportComponents(const EntityID p_entityID, PhysicsComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export RigidBodyComponent
	auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(p_entityID);
	if(rigidBodyComponent != nullptr)
	{
		if(p_constructionInfo.m_rigidBodyConstructionInfo == nullptr)
			p_constructionInfo.m_rigidBodyConstructionInfo = new RigidBodyComponent::RigidBodyComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_rigidBodyConstructionInfo, *rigidBodyComponent);
	}
}

SystemObject *PhysicsScene::createComponent(const EntityID &p_entityID, const RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

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

				case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Cylinder:
					{
						btVector3 cylinderHalfExtents = Math::toBtVector3(p_constructionInfo.m_collisionShapeSize);
						component.m_collisionShape.m_cylinderShape = new btCylinderShape(cylinderHalfExtents);
					}
					break;
			}

			// Success on the loaded collision shape
			ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_RigidBodyComponent, component.getName() + " - Collision shape loaded");

			// Create the struct that holds all the required information for constructing a rigid body
			component.m_constructionInfo = new btRigidBody::btRigidBodyConstructionInfo(p_constructionInfo.m_mass, &component.m_motionState, component.getCollisionShape());

			component.m_constructionInfo->m_friction = p_constructionInfo.m_friction;
			component.m_constructionInfo->m_restitution = p_constructionInfo.m_restitution;
			component.m_constructionInfo->m_rollingFriction = p_constructionInfo.m_rollingFriction;
			component.m_constructionInfo->m_spinningFriction = p_constructionInfo.m_spinningFriction;

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
				//groundTransform.setFromOpenGLMatrix(&spatialComponent->getSpatialDataChangeManager().getLocalSpaceData().m_transformMatNoScale[0][0]);
				component.m_motionState.setWorldTransform(groundTransform);

				component.m_motionState.updateMotionStateTrans();
			}

			// Add the collision shape
			m_collisionShapes.push_back(component.getCollisionShape());

			// Create the rigid body by passing the rigid body construction info
			component.m_rigidBody = new btRigidBody(*component.m_constructionInfo);

			// Set the entity ID of the entity that this component belongs to, so it can be retrieved later
			component.m_rigidBody->setUserPointer(new EntityID(p_entityID));

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

void PhysicsScene::createCollisionEventComponent(const EntityID &p_entityID)
{
	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	worldScene->addComponent<CollisionEventComponent>(p_entityID, p_entityID);
}

void PhysicsScene::releaseObject(SystemObject *p_systemObject)
{
	switch(p_systemObject->getObjectType())
	{
		case Properties::PropertyID::RigidBodyComponent:
			{
				auto *component = static_cast<RigidBodyComponent *>(p_systemObject);

				// Remove rigid body from the world
				if(component->m_rigidBody != nullptr)
				{
					//delete component->m_rigidBody->getMotionState();
					delete component->m_rigidBody->getCollisionShape();
					m_dynamicsWorld->removeRigidBody(component->m_rigidBody);
				}
			}
			break;
	}
}

ErrorCode PhysicsScene::destroyObject(SystemObject *p_systemObject)
{
	ErrorCode returnError = ErrorCode::Success;

	// Get the world scene required for deleting components
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	switch(p_systemObject->getObjectType())
	{
		case Properties::PropertyID::RigidBodyComponent:
			{
				// Delete component
				worldScene->removeComponent<RigidBodyComponent>(p_systemObject->getEntityID());
			}
			break;

		default:
			{
				// If this point is reached, no object was found, return an appropriate error
				returnError = ErrorCode::Destroy_obj_not_found;
			}
			break;
	}

	return returnError;
}

void PhysicsScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	if(CheckBitmask(p_changeType, Systems::Changes::Physics::Gravity))
	{
		m_dynamicsWorld->setGravity(Math::toBtVector3(p_subject->getVec3(this, Systems::Changes::Physics::Gravity)));
	}
}

void PhysicsScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
		case DataType::DataType_DeleteComponent:
			{
				// Get the world scene required for getting the entity registry and deleting components
				WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

				// Get the entity registry 
				auto &entityRegistry = worldScene->getEntityRegistry();

				// Get entity and component data
				auto const *componentData = static_cast<EntityAndComponent *>(p_data);

				// Delete the component based on its type
				switch(componentData->m_componentType)
				{
					case ComponentType::ComponentType_RigidBodyComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<RigidBodyComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_Physics);
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;

		case DataType_SimulationActive:
			m_simulationRunning = static_cast<bool>(p_data);
			break;

		default:
			assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
			break;
	}
}

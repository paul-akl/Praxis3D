#include "NullSystemObjects.h"
#include "PhysicsScene.h"
#include "TaskManagerLocator.h"

PhysicsScene::PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_physicsTask = nullptr;
	m_collisionConfiguration = nullptr;
	m_collisionSolver = nullptr;
	m_collisionDispatcher = nullptr;
	m_collisionBroadphase = nullptr;
	m_dynamicsWorld = nullptr;

	// Bullet world
	btDiscreteDynamicsWorld *m_physicsWorld;
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
	m_collisionSolver = new btSequentialImpulseConstraintSolver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_collisionDispatcher, m_collisionBroadphase, m_collisionSolver, m_collisionConfiguration);
	
	return ErrorCode();
}

ErrorCode PhysicsScene::setup(const PropertySet &p_properties)
{	
	// Get default object pool size
	decltype(m_physicsObjects.getPoolSize()) objectPoolSize = Config::objectPoolVar().object_pool_size;

	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:
			objectPoolSize = p_properties[i].getInt();
			break;
		case Properties::Gravity:
			m_dynamicsWorld->setGravity(Math::toBtVector3(p_properties[i].getVec3f()));
			break;
		}
	}

	// Initialize object pools
	m_physicsObjects.init(objectPoolSize);

	return ErrorCode::Success;
}

void PhysicsScene::update(const float p_deltaTime)
{
	for(decltype(m_physicsObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_physicsObjects.getNumAllocated(),
		size = m_physicsObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// Check if the physics object is allocated inside the pool container
		if(m_physicsObjects[i].allocated())
		{
			auto *currentPhysicsObject = m_physicsObjects[i].getObject();

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			// Check if the GUI object is enabled
			if(currentPhysicsObject->isObjectActive())
			{
				// Update the object
				currentPhysicsObject->update(p_deltaTime);
			}
		}
	}
}

ErrorCode PhysicsScene::preload()
{	
	// Load every physics object. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_physicsObjects.getPoolSize(), size_t(1), [=](size_t i)
		{
			if(m_physicsObjects[i].allocated())
			{
				m_physicsObjects[i].getObject()->loadToMemory();
			}
		});

	return ErrorCode::Success;
}

void PhysicsScene::loadInBackground()
{
}

SystemObject *PhysicsScene::createObject(const PropertySet &p_properties)
{	
	// Check if property set node is present
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
	}

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

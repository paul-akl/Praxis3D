
#include "AudioScene.h"
#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

AudioScene::AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_audioTask = nullptr;
}

AudioScene::~AudioScene()
{
}

ErrorCode AudioScene::init()
{
	m_audioTask = new AudioTask(this);

	return ErrorCode::Success;
}

ErrorCode AudioScene::setup(const PropertySet &p_properties)
{
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:

			break;
		case Properties::Gravity:
			//m_dynamicsWorld->setGravity(Math::toBtVector3(p_properties[i].getVec3f()));
			break;
		}
	}

	return ErrorCode::Success;
}

void AudioScene::update(const float p_deltaTime)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Get the rigid body component view and iterate every entity that contains is
	/*auto rigidBodyView = worldScene->getEntityRegistry().view<RigidBodyComponent>();
	for(auto entity : rigidBodyView)
	{
		auto &component = rigidBodyView.get<RigidBodyComponent>(entity);

		component.update(p_deltaTime);
	}*/
}

ErrorCode AudioScene::preload()
{
	return ErrorCode::Success;
}

void AudioScene::loadInBackground()
{
}

std::vector<SystemObject*> AudioScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_audioComponents, p_startLoading);
}

/*SystemObject *AudioScene::createComponent(const EntityID &p_entityID, const RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

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
}*/

ErrorCode AudioScene::destroyObject(SystemObject *p_systemObject)
{
	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

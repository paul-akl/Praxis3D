#pragma once

#include "EntityViewDefinitions.h"
#include "GameObject.h"
#include "MetadataComponent.h"
#include "ObjectMaterialComponent.h"
#include "ObjectPool.h"
#include "ObjectRegister.h"
#include "System.h"
#include "WorldTask.h"

class WorldSystem;
struct ComponentsConstructionInfo;

struct WorldComponentsConstructionInfo
{
	WorldComponentsConstructionInfo()
	{
		m_spatialConstructionInfo = nullptr;
		m_objectMaterialConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const WorldComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<SpatialComponent::SpatialComponentConstructionInfo>(&m_spatialConstructionInfo, &p_other.m_spatialConstructionInfo);
		Utilities::performCopy<ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo>(&m_objectMaterialConstructionInfo, &p_other.m_objectMaterialConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_spatialConstructionInfo != nullptr)
			delete m_spatialConstructionInfo;
		m_spatialConstructionInfo = nullptr;

		if(m_objectMaterialConstructionInfo != nullptr)
			delete m_objectMaterialConstructionInfo;
		m_objectMaterialConstructionInfo = nullptr;
	}

	SpatialComponent::SpatialComponentConstructionInfo *m_spatialConstructionInfo;
	ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo *m_objectMaterialConstructionInfo;
};

class WorldScene : public SystemScene
{
public:
	WorldScene(SystemBase *p_system, SceneLoader *p_sceneLoader);

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void exportSetup(PropertySet &p_propertySet);

	void update(const float p_deltaTime);

	ErrorCode preload() { return ErrorCode::Success; }

	void loadInBackground() { }

	EntityID createEntity(const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	void exportEntity(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const WorldComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);

	ErrorCode destroyObject(SystemObject *p_systemObject);

	SystemObject *createComponent(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	SystemObject *createComponent(const EntityID p_entityID, const ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		ObjectMaterialComponent *objectMaterialComponent = nullptr;

		objectMaterialComponent = &addComponent<ObjectMaterialComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		objectMaterialComponent->setActive(p_constructionInfo.m_active);
		objectMaterialComponent->m_materialType = p_constructionInfo.m_materialType;

		objectMaterialComponent->init();

		return objectMaterialComponent;
	}

	SystemObject *createComponent(const EntityID p_entityID, const SpatialComponent::SpatialComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		SpatialComponent *spatialComponent = nullptr;

		spatialComponent = &addComponent<SpatialComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		spatialComponent->setActive(p_constructionInfo.m_active);
		spatialComponent->m_spatialData.setLocalPosition(p_constructionInfo.m_localPosition);
		spatialComponent->m_spatialData.setLocalScale(p_constructionInfo.m_localScale);

		// If the rotation quaternion is empty, use the Euler angle rotation
		if(p_constructionInfo.m_localRotationQuaternion == glm::quat())
			spatialComponent->m_spatialData.setLocalRotation(p_constructionInfo.m_localRotationEuler);
		else
			spatialComponent->m_spatialData.setLocalRotation(p_constructionInfo.m_localRotationQuaternion);

		// Perform a spatial data update, so that all the transform matrices are calculated
		spatialComponent->m_spatialData.update();
		return spatialComponent;
	}

	void exportComponent(ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo &p_constructionInfo, const ObjectMaterialComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_materialType = p_component.getObjectMaterialType();
	}

	void exportComponent(SpatialComponent::SpatialComponentConstructionInfo &p_constructionInfo, SpatialComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		// Call update to make sure the spatial data is current
		p_component.m_spatialData.update();
		p_component.m_spatialData.calculateLocalRotationEuler();

		p_constructionInfo.m_localPosition = p_component.getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_position;
		p_constructionInfo.m_localRotationEuler = p_component.getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_rotationEuler;
		p_constructionInfo.m_localRotationQuaternion = p_component.getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_rotationQuat;
		p_constructionInfo.m_localScale = p_component.getSpatialDataChangeManager().getLocalSpaceData().m_spatialData.m_scale;
	}

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_worldTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::World; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	// Adds a component with the given parameters and returns a reference to it
	template <class T_Component, class... T_Args>
	T_Component &addComponent(EntityID p_entity, T_Args&&... p_args)
	{
		return m_entityRegistry.emplace_or_replace<T_Component>(p_entity, std::forward<T_Args>(p_args)...);
	}

	// Removes a component from the given entity
	template <class T_Component>
	void removeComponent(EntityID p_entity)
	{
		m_entityRegistry.remove<T_Component>(p_entity);
	}

	// Increases the size of the pool for a given component type
	// When multi-threading is enabled, creating views concurrently on a mostly empty registry can sometimes trigger an access violation error,
	// because a view of a non-existent component creates an empty pool for that component, hence creating views for the first time is not thread-safe.
	// As a solution, reserve must be called for every component type, to create its pool ahead of time
	template <class T_Component>
	void reserve(const size_t p_capacity)
	{
		m_entityRegistry.storage<T_Component>().reserve(p_capacity);
	}

	// Get the internal pool size of a given component
	template <class T_Component>
	size_t getPoolSize()
	{
		return m_entityRegistry.storage<T_Component>().size();
	}

	inline entt::basic_registry<EntityID> &getEntityRegistry() { return m_entityRegistry; }

private:
	struct GameObjectAndParent
	{
		GameObjectAndParent()
		{
			m_gameObject = nullptr;
			m_parent = 0;
		}		
		GameObjectAndParent(GameObject *p_gameObject, decltype(GameObject::m_GameObjectID) p_parent)
		{
			m_gameObject = p_gameObject;
			m_parent = p_parent;
		}

		GameObject *m_gameObject;
		decltype(GameObject::m_GameObjectID) m_parent;
	};
	struct GameObjectAndChildren
	{
		GameObjectAndChildren()
		{
			m_gameObject = nullptr;
		}		
		GameObjectAndChildren(GameObject *p_gameObject)
		{
			m_gameObject = p_gameObject;
		}

		GameObject *m_gameObject;
		std::vector<decltype(GameObject::m_GameObjectID)> m_children;
	};

	inline EntityID addEntity()
	{
		return m_entityRegistry.create();
	}
	inline EntityID addEntity(EntityID p_entityID)
	{
		return m_entityRegistry.create(p_entityID);
	}

	entt::basic_registry<EntityID> m_entityRegistry;

	std::vector<GameObjectAndParent> m_unassignedParents;
	std::vector<GameObjectAndChildren> m_unassignedChildren;

	WorldTask *m_worldTask;

	ObjectRegisterConcurrent<GameObject*> m_objectRegister;
};
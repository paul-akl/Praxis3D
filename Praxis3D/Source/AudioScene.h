#pragma once

#include "AUdioTask.h"
#include "System.h"

struct ComponentsConstructionInfo;

struct AudioComponentsConstructionInfo
{
	AudioComponentsConstructionInfo()
	{
		//m_rigidBodyConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const AudioComponentsConstructionInfo &p_other)
	{
		//Utilities::performCopy<RigidBodyComponent::RigidBodyComponentConstructionInfo>(&m_rigidBodyConstructionInfo, &p_other.m_rigidBodyConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		//if(m_rigidBodyConstructionInfo != nullptr)
		//	delete m_rigidBodyConstructionInfo;
	}

	//RigidBodyComponent::RigidBodyComponentConstructionInfo *m_rigidBodyConstructionInfo;
};

class AudioScene : public SystemScene
{
public:
	AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader);
	~AudioScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload();

	void loadInBackground();

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const AudioComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject*> components;

		//if(p_constructionInfo.m_rigidBodyConstructionInfo != nullptr)
		//	components.push_back(createComponent(p_entityID, *p_constructionInfo.m_rigidBodyConstructionInfo, p_startLoading));

		return components;
	}

	//SystemObject *createComponent(const EntityID &p_entityID, const RigidBodyComponent::RigidBodyComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_audioTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::Audio; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	AudioTask *m_audioTask;
};
#pragma once

#include "GUIObject.h"
#include "GUITask.h"
#include "ObjectPool.h"
#include "System.h"

class GUISystem;
struct ComponentsConstructionInfo;

struct GUIComponentsConstructionInfo
{
	GUIComponentsConstructionInfo()
	{
		m_guiSequenceConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const GUIComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<GUISequenceComponent::GUISequenceComponentConstructionInfo>(&m_guiSequenceConstructionInfo, &p_other.m_guiSequenceConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_guiSequenceConstructionInfo != nullptr)
			delete m_guiSequenceConstructionInfo;
	}

	GUISequenceComponent::GUISequenceComponentConstructionInfo *m_guiSequenceConstructionInfo;
};

struct FileBrowserDialog
{
	FileBrowserDialog()
	{
		m_opened = false;
		m_closed = false;
		m_success = false;
		m_rootPath = ".";
		m_numOfSelectableFiles = 1;
		m_flags = 0;
	}

	// Only reset the values that are assigned after opening the dialog
	// This prepares the dialog to be opened again, without affecting the starting data
	void reset()
	{
		m_opened = false;
		m_closed = false;
		m_success = false;
		m_filePath.clear();
		m_filename.clear();
		m_filePathName.clear();
	}

	std::string m_name,
				m_title,
				m_filter,
				m_rootPath,
				m_definedFilename,
				m_filePath,
				m_filename,
				m_filePathName;

	int m_numOfSelectableFiles,
		m_flags;

	bool m_opened,
		 m_closed,
		 m_success;
};

class GUIScene : public SystemScene
{
public:
	GUIScene(SystemBase* p_system, SceneLoader* p_sceneLoader);
	~GUIScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet& p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload();

	void loadInBackground();

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const GUIComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject *> components;

		if(p_constructionInfo.m_guiSequenceConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_guiSequenceConstructionInfo, p_startLoading));

		return components;
	}

	SystemObject *createComponent(const EntityID p_entityID, const GUISequenceComponent::GUISequenceComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType) { }

	void receiveData(const DataType p_dataType, void *p_data);

	SystemTask *getSystemTask() { return m_GUITask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::GUI; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	GUITask *m_GUITask;
	std::queue<FileBrowserDialog*> m_fileBrowserDialogs;
};
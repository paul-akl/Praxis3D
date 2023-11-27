#pragma once

#include "GUIObject.h"
#include "GUITask.h"
#include "ObjectPool.h"
#include "System.h"

class EditorWindow;
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
	enum FileBrowserDialogFlags : unsigned int
	{
		FileBrowserDialogFlags_None = 0,
		FileBrowserDialogFlags_ConfirmOverwrite = (1 << 0),
		FileBrowserDialogFlags_DontShowHiddenFiles = (1 << 1),
		FileBrowserDialogFlags_DisableCreateDirectoryButton = (1 << 2)
	};

	FileBrowserDialog()
	{
		m_userStringPointer = nullptr;
		m_opened = false;
		m_closed = false;
		m_success = false;
		m_rootPath = ".";
		m_numOfSelectableFiles = 1;
		m_flags = FileBrowserDialogFlags_None;
	}

	// Only reset the values that are assigned after opening the dialog
	// This prepares the dialog to be opened again, without affecting the starting data
	void reset()
	{
		m_userStringPointer = nullptr;
		m_opened = false;
		m_closed = false;
		m_success = false;
		m_filePath.clear();
		m_filename.clear();
		m_filePathName.clear();
	}

	// Reset all values
	void resetAll()
	{
		m_name.clear();
		m_title.clear();
		m_filter.clear();
		m_rootPath.clear();
		m_definedFilename.clear();
		m_filePath.clear();
		m_filename.clear();
		m_filePathName.clear();

		m_userStringPointer = nullptr;

		m_numOfSelectableFiles = 1;
		m_flags = FileBrowserDialogFlags_None;

		m_opened = false;
		m_closed = false;
		m_success = false;
	}

	std::string m_name,
				m_title,
				m_filter,
				m_rootPath,
				m_definedFilename,
				m_filePath,
				m_filename,
				m_filePathName;

	std::string *m_userStringPointer;

	int m_numOfSelectableFiles;
	unsigned int m_flags;

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

	void exportSetup(PropertySet &p_propertySet);

	void activate();

	void deactivate();

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

	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);
	void exportComponents(const EntityID p_entityID, GUIComponentsConstructionInfo &p_constructionInfo);

	SystemObject *createComponent(const EntityID p_entityID, const GUISequenceComponent::GUISequenceComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	void exportComponent(GUISequenceComponent::GUISequenceComponentConstructionInfo &p_constructionInfo, const GUISequenceComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_staticSequence = p_component.isStaticSequence();
	}

	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType) { }

	void receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving);

	SystemTask *getSystemTask() { return m_GUITask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::GUI; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	inline void setGUISequenceEnabled(const bool p_GUISequenceEnabled) { m_GUISequenceEnabled = p_GUISequenceEnabled; }

	GUITask *m_GUITask;
	std::queue<FileBrowserDialog*> m_fileBrowserDialogs;
	EditorWindow *m_editorWindow;
	bool m_GUISequenceEnabled;
};
#pragma once

#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"
#include "Loaders.h"
#include "InheritanceObjects.h"
#include "System.h"

class EditorWindow : public SystemObject
{
	friend class GUIScene;
public:
	EditorWindow(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : 
		SystemObject(p_systemScene, p_name, Properties::PropertyID::EditorWindow, p_entityID)
	{
		m_selectedEntity = NULL_ENTITY_ID;
		m_renderSceneToTexture = true;
		m_GUISequenceEnabled = false;
		m_LUAScriptingEnabled = true;
		m_sceneState = EditorSceneState::EditorSceneState_Pause;

		m_selectedTexture = nullptr;
		m_textureInspectorTabFlags = 0;
	}
	~EditorWindow() { }

	ErrorCode init();

	void loadToMemory()
	{
	}

	void update(const float p_deltaTime);

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties)// && !isLoadedToMemory())
		{
			if(p_properties.getPropertyID() == Properties::Sequence)
			{
				importError = ErrorCode::Success;
				/*			auto const &luaFilenameProperty = p_properties.getPropertyByID(Properties::Filename);

							if(luaFilenameProperty)
							{
								std::string luaFilename = luaFilenameProperty.getString();
								if(!luaFilename.empty())
								{
									luaFilename = Config::filepathVar().script_path + luaFilename;
									if(Filesystem::exists(luaFilename))
									{
										m_luaScript.setScriptFilename(luaFilename);

										importError = ErrorCode::Success;
										ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LuaComponent, m_name + " - Script loaded");
									}
									else
									{
										importError = ErrorCode::File_not_found;
										ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - File \'" + luaFilename + "\' not found");
									}
								}
								else
								{
									importError = ErrorCode::Property_no_filename;
									ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - Property \'" + GetString(Properties::Filename) + "\' is empty");
								}
							}
							else
							{
								importError = ErrorCode::Property_no_filename;
								ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - Missing \'" + GetString(Properties::Filename) + "\' property");
							}*/
			}

			if(importError == ErrorCode::Success)
			{
				//setLoadedToMemory(true);
				//setLoadedToVideoMemory(true);
				setActive(true);
			}
		}

		return importError;
	}

	PropertySet exportObject() final override
	{
		// Create the root Camera property set
		PropertySet propertySet(Properties::Camera);

		return propertySet;
	}

	// System type is GUI
	BitMask getSystemType() final override { return Systems::GUI; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::GUI::All; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		//if(CheckBitmask(p_changeType, Systems::Changes::GUI::Sequence))
		//	m_guiSequence = p_subject->getFunctors(this, Systems::Changes::GUI::Sequence);
	}

	void setup(EditorWindowSettings &p_editorWindowSettings);

private:
	struct ComponentListEntry
	{
		ComponentListEntry(EntityID p_entityID, const std::string &p_name, const std::string &p_combinedEntityIdAndName) : m_entityID(p_entityID), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName) { }

		EntityID m_entityID;
		std::string m_name;
		std::string m_combinedEntityIdAndName;
	};
	struct EntityListEntry
	{
		EntityListEntry(EntityID p_entityID, EntityID p_parentEntityID, const std::string &p_name, const std::string &p_combinedEntityIdAndName) : m_entityID(p_entityID), m_parentEntityID(p_parentEntityID), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName), m_componentFlag(Systems::AllComponentTypes::None) { }
		EntityListEntry(const EntityListEntry &p_entityListEntry) : m_entityID(p_entityListEntry.m_entityID), m_parentEntityID(p_entityListEntry.m_parentEntityID), m_name(p_entityListEntry.m_name), m_combinedEntityIdAndName(p_entityListEntry.m_combinedEntityIdAndName), m_componentFlag(Systems::AllComponentTypes::None) { }
		bool operator==(const EntityListEntry &p_entityListEntry) { return m_entityID == p_entityListEntry.m_entityID; }

		EntityID m_entityID;
		EntityID m_parentEntityID;
		BitMask m_componentFlag;
		std::string m_name;
		std::string m_combinedEntityIdAndName;
	};
	struct EntityHierarchyEntry
	{
		EntityHierarchyEntry() : m_entityID(NULL_ENTITY_ID), m_parent(NULL_ENTITY_ID), m_componentFlag(Systems::AllComponentTypes::None) { }
		EntityHierarchyEntry(EntityID p_entityID, EntityID p_parent, const std::string &p_name, const std::string &p_combinedEntityIdAndName, const BitMask p_componentFlag) : m_entityID(p_entityID), m_parent(p_parent), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName), m_componentFlag(p_componentFlag){ }

		bool operator==(const EntityHierarchyEntry &p_childEntityEntry) { return m_entityID == p_childEntityEntry.m_entityID; }

		bool findParentAndAddChild(const EntityHierarchyEntry &p_childEntityEntry)
		{
			bool parentFound = false;

			if(m_entityID == p_childEntityEntry.m_parent)
			{
				addChild(p_childEntityEntry);
				parentFound = true;
			}
			else
			{
				for(decltype(m_children.size()) size = m_children.size(), i = 0; i < size; i++)
				{
					if(m_children[i].findParentAndAddChild(p_childEntityEntry))
					{
						parentFound = true;
						break;
					}
				}
			}

			return parentFound;
		}
		void addChild(const EntityHierarchyEntry &p_childEntityEntry)
		{
			m_children.push_back(p_childEntityEntry);
		}
		void removeChild(const EntityHierarchyEntry &p_childEntityEntry)
		{
			std::vector<EntityHierarchyEntry>::iterator childPosition = std::find(m_children.begin(), m_children.end(), p_childEntityEntry);
			if(childPosition != m_children.end())
				m_children.erase(childPosition);
		}
		bool containsChildren() const { return !m_children.empty(); }
		void getEntries(std::vector<EntityHierarchyEntry *> &p_entriesList)
		{
			p_entriesList.push_back(this);
			for(decltype(m_children.size()) size = m_children.size(), i = 0; i < size; i++)
				m_children[i].getEntries(p_entriesList);
		}

		EntityID m_entityID;
		EntityID m_parent;
		BitMask m_componentFlag;
		std::string m_name;
		std::string m_combinedEntityIdAndName;
		std::vector<EntityHierarchyEntry> m_children;
	};

	enum ButtonTextureType : unsigned int
	{
		ButtonTextureType_Pause,
		ButtonTextureType_Play,
		ButtonTextureType_Restart,
		ButtonTextureType_GUISequence,
		ButtonTextureType_ScriptingEnable,
		ButtonTextureType_NumOfTypes
	};

	enum EditorSceneState : unsigned int
	{
		EditorSceneState_Play,
		EditorSceneState_Pause
	};

	void drawEntityHierarchyEntry(EntityHierarchyEntry &p_entityEntry);
	//void getComponentsOfEntity(std::vector<ComponentListEntry> &p_componentList, EntityID p_entityID);

	void updateEntityList();
	void updateHierarchyList();
	void updateComponentList();

	inline std::string getTextureFormatString(const TextureFormat p_textureFormat) const
	{
		switch(p_textureFormat)
		{
		case TextureFormat_Red:
			return "Red";
			break;
		case TextureFormat_Green:
			return "Green";
			break;
		case TextureFormat_Blue:
			return "Blue";
			break;
		case TextureFormat_Alpha:
			return "Alpha";
			break;
		case TextureFormat_RGB:
			return "RGB";
			break;
		case TextureFormat_RGBA:
			return "RGBA";
			break;
		}
		return "";
	}
	inline std::string getTextureDataTypeString(const TextureDataType p_textureDataType) const
	{
		switch(p_textureDataType)
		{
		case TextureDataType_Float:
			return "Float";
			break;
		case TextureDataType_Int:
			return "Integer";
			break;
		case TextureDataType_UnsignedByte:
			return "Unsigned byte";
			break;
		}
		return "";
	}
	inline std::string getTextureDataFormat(const TextureDataFormat p_textureDataFormat) const
	{
		switch(p_textureDataFormat)
		{
		case TextureDataFormat_R8:
			return "R8";
			break;
		case TextureDataFormat_R16:
			return "R16";
			break;
		case TextureDataFormat_R16F:
			return "R16F";
			break;
		case TextureDataFormat_R32F:
			return "R32F";
			break;
		case TextureDataFormat_RG8:
			return "RG8";
			break;
		case TextureDataFormat_RG16:
			return "RG16";
			break;
		case TextureDataFormat_RG16F:
			return "RG16F";
			break;
		case TextureDataFormat_RG32F:
			return "RG32F";
			break;
		case TextureDataFormat_RGB8:
			return "RGB8";
			break;
		case TextureDataFormat_RGB16:
			return "RGB16";
			break;
		case TextureDataFormat_RGB16F:
			return "RGB16F";
			break;
		case TextureDataFormat_RGB32F:
			return "RGB32F";
			break;
		case TextureDataFormat_RGBA8:
			return "RGBA8";
			break;
		case TextureDataFormat_RGBA16:
			return "RGBA16";
			break;
		case TextureDataFormat_RGBA16F:
			return "RGBA16F";
			break;
		case TextureDataFormat_RGBA32F:
			return "RGBA32F";
			break;
		case TextureDataFormat_R16I:
			return "R16I";
			break;
		case TextureDataFormat_R32I:
			return "R32I";
			break;
		case TextureDataFormat_R16UI:
			return "R16UI";
			break;
		case TextureDataFormat_R32UI:
			return "R32UI";
			break;
		}
		return "";
	}

	bool show_demo_window;
	bool show_another_window;
	float clear_color;

	// Texture inspector variables
	Texture2D *m_selectedTexture;
	ImGuiTabItemFlags m_textureInspectorTabFlags;
	DoubleBufferedContainer<FunctorSequence> m_textureInspectorSequence;

	bool m_renderSceneToTexture;
	bool m_GUISequenceEnabled;
	bool m_LUAScriptingEnabled;
	EditorSceneState m_sceneState;
	glm::ivec2 m_centerWindowSize;

	// Scene entities
	std::vector<ComponentListEntry> m_componentList;
	std::vector<EntityListEntry> m_entityList;
	std::vector<EntityHierarchyEntry> m_entityHierarchy;
	EntityID m_selectedEntity;

	// Button textures
	std::vector<TextureLoader2D::Texture2DHandle> m_buttonTextures;
};
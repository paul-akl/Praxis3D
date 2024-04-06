#pragma once

#include <TextEditor.h>

#include "ComponentConstructorInfo.h"
#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"
#include "GUIHandlerLocator.h"
#include "LightComponent.h"
#include "Loaders.h"
#include "InheritanceObjects.h"
#include "RigidBodyComponent.h"
#include "System.h"

struct FileBrowserDialog;

class EditorWindow : public SystemObject
{
	friend class GUIScene;
public:
	EditorWindow(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : 
		SystemObject(p_systemScene, p_name, Properties::PropertyID::EditorWindow, p_entityID),
		m_selectedEntity(*this),
		m_imguiStyle(ImGui::GetStyle()),
		m_playPauseButtonSize(Config::GUIVar().editor_play_button_size, Config::GUIVar().editor_play_button_size)
	{
		m_renderSceneToTexture = true;
		m_GUISequenceEnabled = false;
		m_LUAScriptingEnabled = false;
		m_translateGuizmoEnabled = true;
		m_rotateGuizmoEnabled = false;
		m_showNewMapWindow = false;
		m_showImGuiDemoWindow = false;
		m_showImspinnerDemoWindow = false;
		m_fullscreen = false;
		m_enlargedSceneViewport = false;
		m_showingExitDialog = false;
		m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_None;
		m_sceneState = EditorSceneState::EditorSceneState_Pause;
		m_centerWindowSize = glm::ivec2(0);
		m_activeItemID = -1;
		m_mouseCaptured = false;

		m_selectedTexture = nullptr;
		m_textureInspectorTabFlags = 0;

		m_buttonMaterialType = MaterialType::MaterialType_Diffuse;
		m_colorEditFlags = ImGuiColorEditFlags_Float;
		m_browseButtonWidth = 60.0f;
		m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
		m_previouslyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
		m_fileBrowserDialog.m_name = "EditorFileBrowserDialog";

		m_selectedLuaScript = NULL_ENTITY_ID;
		m_selectedModel = nullptr;
		m_selectedProgram = nullptr;
		m_selectedShaderType = -1;

		m_nextEntityIDToSelect = NULL_ENTITY_ID;
		m_nextEntityToSelect = nullptr;
		m_pendingEntityToSelect = false;

		m_newEntityConstructionInfo = nullptr;
		m_openNewEntityPopup = false;
		m_openEntityRightClickOptionsPopup = false;
		m_duplicateParent = false;

		resetActivateAllComponentFlags();

		m_newSceneSettingsTabFlags = 0;

		m_fontSize = ImGui::GetFontSize();
		m_buttonSizedByFont = ImVec2(m_fontSize, m_fontSize);
		m_assetSelectionPopupImageSize = ImVec2(m_fontSize, m_fontSize) * Config::GUIVar().editor_asset_selection_button_size_multiplier;
		m_textureAssetImageSize = ImVec2(Config::GUIVar().editor_asset_texture_button_size_x, Config::GUIVar().editor_asset_texture_button_size_y);

		m_buttonBackgroundEnabled = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
		m_buttonBackgroundDisabled = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		m_mousePositionOnNewEntity = ImVec2(0.0f, 0.0f);
		m_newEntityWindowInitialized = false;
		m_sceneViewportPosition = ImVec2(0.0f, 0.0f);
		m_sceneViewportSize = ImVec2(0.0f, 0.0f);
		m_synchronizeTextureScale = true;
		m_synchronizeTextureFraming = true;
		m_2DTextureScale = true;
		m_2DTextureFraming = false;

		m_numOfLogs = 0;

		for(unsigned int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
			m_physicalMaterialProperties.push_back(GetString(static_cast<ObjectMaterialType>(i)));

		for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
			m_renderingPassesTypeText.push_back(GetString(static_cast<RenderPassType>(i)));

		m_entityRightClickOptionsPopup = "EntityRightClickOptionsPopup";

		m_antialiasingTypeText = { "None", "MSAA", "FXAA" };
		m_ambientOcclusionTypeText = { "None", "SSAO", "HBAO" };
		m_cascadeDistanceTypeText = { "Units", "Divider" };
		m_luaVariableTypeStrings = { "null", "bool", "int", "float", "double", "vec2i", "vec2f", "vec3f", "vec4f", "string", "propertyID" };
		m_shaderTypeStrings = { "Compute", "Fragment", "Geometry", "Vertex", "Tessellation control", "Tessellation evaluation" };
		m_tonemappingMethodText = { "None", "Simple reinhard", "Reinhard with white point", "Filmic tonemapping", "Uncharted 2", "Unreal 3", "ACES", "Lottes", "Uchimura" };
		m_textEditorLanguageTypeText = { "Text", "C++", "C", "Cs", "Python", "Lua", "Json", "SQL", "AngelScript", "GLSL", "HLSL" };
		m_textureWrapModeStrings = { "Clamp to border", "Clamp to edge", "Mirrored clamp to edge", "Mirrored repeat", "Repeat" };
		m_textureWrapModeTypes = { TextureWrapType::TextureWrapType_ClampToBorder, TextureWrapType::TextureWrapType_ClampToEdge, TextureWrapType::TextureWrapType_MirroredClampToEdge, TextureWrapType::TextureWrapType_MirroredRepeat, TextureWrapType::TextureWrapType_Repeat };

		m_currentlyActiveTextEditor = nullptr;

		TextEditor::SetDefaultPalette(TextEditor::PaletteId::Dark);
	}
	~EditorWindow();

	ErrorCode init();

	void loadToMemory()
	{
	}

	void update(const float p_deltaTime);

	void activate();

	void deactivate();

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

	const inline glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_selectedEntity.m_spatialDataManager.getQuaternion(p_observer, p_changedBits);
	}
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits)	const 
	{ 
		if((p_changedBits & Systems::Changes::Type::Spatial) != 0)
		{
			return m_selectedEntity.m_spatialDataManager.getVec3(p_observer, p_changedBits);
		}
		else
		{
			switch(p_changedBits)
			{
				case Systems::Changes::Graphics::Color:
					{
						switch(m_selectedEntity.m_lightType)
						{
							case LightComponent::LightComponentType::LightComponentType_directional:
								return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
								break;
							case LightComponent::LightComponentType::LightComponentType_point:
								return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
								break;
							case LightComponent::LightComponentType::LightComponentType_spot:
								return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
								break;
						}
					}
					break;
				case Systems::Changes::Physics::CollisionShapeSize:
					{
						return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize;
					}
					break;
				case Systems::Changes::Physics::Gravity:
					{
						return m_currentSceneData.m_gravity;
					}
					break;
			}
		}

		return NullObjects::NullVec3f;
	}
	const inline glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
			return m_selectedEntity.m_spatialDataManager.getMat4(p_observer, p_changedBits);
	}

	const bool getBool(const Observer *p_observer, BitMask p_changedBits) const 
	{ 
		switch(p_changedBits)
		{
			case Systems::Changes::Generic::Active:
				{
					switch(p_observer->getObjectType())
					{
						case Properties::PropertyID::SoundComponent:
							return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active;
						case Properties::PropertyID::SoundListenerComponent:
							return m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_active;

						case Properties::PropertyID::CameraComponent:
							return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active;
						case Properties::PropertyID::LightComponent:
						case Properties::PropertyID::DirectionalLight:
						case Properties::PropertyID::PointLight:
						case Properties::PropertyID::SpotLight:
							return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active;
						case Properties::PropertyID::ModelComponent:
							return m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active;
						case Properties::PropertyID::ShaderComponent:
							return m_selectedEntity.m_componentData.m_graphicsComponents.m_shaderConstructionInfo->m_active;

						case Properties::PropertyID::GUISequenceComponent:
							return m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_active;

						case Properties::PropertyID::RigidBodyComponent:
							return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active;

						case Properties::PropertyID::LuaComponent:
							return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_active;

						case Properties::PropertyID::ObjectMaterialComponent:
							return m_selectedEntity.m_componentData.m_worldComponents.m_objectMaterialConstructionInfo->m_active;
						case Properties::PropertyID::SpatialComponent:
							return m_selectedEntity.m_componentData.m_worldComponents.m_spatialConstructionInfo->m_active;
						default:
							return true;
					}
				}
				break;

			case Systems::Changes::Audio::Loop:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop;
				}
				break;

			case Systems::Changes::Audio::Spatialized:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized;
				}
				break;

			case Systems::Changes::Audio::StartPlaying:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying;
				}
				break;

			case Systems::Changes::Physics::Kinematic:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic;
				}
				break;

			case Systems::Changes::GUI::StaticSequence:
				{
					return m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence;
				}
				break;

			case Systems::Changes::Script::PauseInEditor:
				{
					return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_pauseInEditor;
				}
				break;
		}

		return NullObjects::NullBool; 
	}
	const int getInt(const Observer *p_observer, BitMask p_changedBits) const 
	{ 
		switch(p_changedBits)
		{
		case Systems::Changes::Audio::ListenerID:
			{
				if(m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID >= 0)
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID;
			}
			break;

		case Systems::Changes::Graphics::ActiveCameraID:
			{
				return m_currentSceneData.m_activeCameraID;
			}
			break;

		case Systems::Changes::Graphics::CameraID:
			{
				return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_cameraID;
			}
			break;

		case Systems::Changes::Physics::CollisionShapeType:
			{
				if(m_selectedEntity.m_collisionShapeType >= 0 && m_selectedEntity.m_collisionShapeType < RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes)
					return m_selectedEntity.m_collisionShapeType;
			}
			break;
		}

		return NullObjects::NullInt; 
	}
	const unsigned int getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
			case Systems::Changes::Audio::SoundType:
				{
					if(m_selectedEntity.m_soundType >= 0 && m_selectedEntity.m_soundType < SoundComponent::SoundType_NumOfTypes)
						return (unsigned int)m_selectedEntity.m_soundType;
				}
				break;

			case Systems::Changes::Audio::SoundSourceType:
				{
					if(m_selectedEntity.m_soundSourceType >= 0 && m_selectedEntity.m_soundSourceType < SoundComponent::SoundSourceType_NumOfTypes)
						return (unsigned int)m_selectedEntity.m_soundSourceType;
				}
				break;

			case Systems::Changes::Graphics::LightType:
				{
					if(m_selectedEntity.m_lightType >= 0 && m_selectedEntity.m_lightType < LightComponent::LightComponentType::LightComponentType_spot + 1)
						return (unsigned int)m_selectedEntity.m_lightType;
				}
				break;

			case Systems::Changes::Physics::CollisionShapeType:
				{
					if(m_selectedEntity.m_collisionShapeType >= 0 && m_selectedEntity.m_collisionShapeType < RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes)
						return (unsigned int)m_selectedEntity.m_collisionShapeType;
				}
				break;

			case Systems::Changes::World::ObjectMaterialType:
				{
					if(m_selectedEntity.m_objectMaterialType >= 0 && m_selectedEntity.m_objectMaterialType < ObjectMaterialType::NumberOfMaterialTypes)
						return (unsigned int)m_selectedEntity.m_objectMaterialType;
				}
				break;
		}

		return NullObjects::NullUnsignedInt;
	}
	const float getFloat(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
			case Systems::Changes::Audio::Volume:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume;
				}
				break;

			case Systems::Changes::Audio::VolumeAmbient:
				{
					return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Ambient];
				}
				break;

			case Systems::Changes::Audio::VolumeMaster:
				{
					return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Master];
				}
				break;

			case Systems::Changes::Audio::VolumeMusic:
				{
					return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Music];
				}
				break;

			case Systems::Changes::Audio::VolumeSFX:
				{
					return m_currentSceneData.m_volume[AudioBusType::AudioBusType_SFX];
				}
				break;

			case Systems::Changes::Graphics::CutoffAngle:
				{
					if(m_selectedEntity.m_lightType == LightComponent::LightComponentType::LightComponentType_spot)
						return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle;
				}
				break;

			case Systems::Changes::Graphics::FOV:
				{
					return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_fov;
				}
				break;

			case Systems::Changes::Graphics::Intensity:
				{
					switch(m_selectedEntity.m_lightType)
					{
						case LightComponent::LightComponentType::LightComponentType_directional:
						case LightComponent::LightComponentType::LightComponentType_point:
						case LightComponent::LightComponentType::LightComponentType_spot:
							return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity;
							break;
					}
				}
				break;

			case Systems::Changes::Graphics::ZFar:
				{
					return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zFar;
				}
				break;

			case Systems::Changes::Graphics::ZNear:
				{
					return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zNear;
				}
				break;

			case Systems::Changes::Physics::Friction:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction;
				}
				break;

			case Systems::Changes::Physics::RollingFriction:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_rollingFriction;
				}
				break;

			case Systems::Changes::Physics::SpinningFriction:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_spinningFriction;
				}
				break;

			case Systems::Changes::Physics::Mass:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass;
				}
				break;

			case Systems::Changes::Physics::Restitution:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution;
				}
				break;
		}

		return NullObjects::NullFloat;
	}
	const std::string &getString(const Observer *p_observer, BitMask p_changedBits)	const 
	{
		switch(p_changedBits)
		{
			case Systems::Changes::Generic::Name:
				{
					return m_selectedEntity.m_componentData.m_name;
				}
				break;

			case Systems::Changes::Audio::SoundName:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName;
				}
				break;

			case Systems::Changes::Script::Filename:
				{
					return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename;
				}
				break;

			case Systems::Changes::World::PrefabName:
				{
					return m_selectedEntity.m_componentData.m_prefab;
				}
				break;
		}

		return NullObjects::NullString; 
	}
	const inline SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_selectedEntity.m_spatialDataManager.getSpatialData(p_observer, p_changedBits);
	}
	const inline SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_selectedEntity.m_spatialDataManager.getSpatialTransformData(p_observer, p_changedBits);
	}

private:
	enum ButtonTextureType : unsigned int
	{
		ButtonTextureType_Pause,
		ButtonTextureType_Play,
		ButtonTextureType_Restart,
		ButtonTextureType_GUISequence,
		ButtonTextureType_ScriptingEnable,
		ButtonTextureType_DeleteEntry,
		ButtonTextureType_Add,
		ButtonTextureType_Duplicate,
		ButtonTextureType_OpenFile,
		ButtonTextureType_Reload,
		ButtonTextureType_OpenAssetList,
		ButtonTextureType_ArrowUp,
		ButtonTextureType_GuizmoRotate,
		ButtonTextureType_GuizmoTranslate,
		ButtonTextureType_NumOfTypes
	};
	enum EditorSceneState : unsigned int
	{
		EditorSceneState_Play,
		EditorSceneState_Pause
	};
	enum FileBrowserActivated : unsigned int
	{
		FileBrowserActivated_None,
		FileBrowserActivated_LuaScript,
		FileBrowserActivated_LoadScene,
		FileBrowserActivated_SaveScene,
		FileBrowserActivated_SoundFile,
		FileBrowserActivated_ModelFile,
		FileBrowserActivated_TextureFile,
		FileBrowserActivated_AudioBankFile,
		FileBrowserActivated_PrefabFile,
		FileBrowserActivated_SavePrefabFile,
		FileBrowserActivated_ShaderFile
	};
	enum MainMenuButtonType : unsigned int
	{
		MainMenuButtonType_None = 0,
		MainMenuButtonType_Play,
		MainMenuButtonType_Pause,
		MainMenuButtonType_Restart,
		MainMenuButtonType_New,
		MainMenuButtonType_Open,
		MainMenuButtonType_Save,
		MainMenuButtonType_SaveAs,
		MainMenuButtonType_ReloadScene,
		MainMenuButtonType_CloseEditor,
		MainMenuButtonType_Exit,
		MainMenuButtonType_EnlargeSceneViewport,
		MainMenuButtonType_Fullscreen,
		MainMenuButtonType_Undo,
		MainMenuButtonType_Redo,
		MainMenuButtonType_Cut,
		MainMenuButtonType_Copy,
		MainMenuButtonType_Paste,
		MainMenuButtonType_ExportPrefab
	};
	enum KeyType : unsigned int
	{
		KeyType_Ctlr,
		KeyType_Shift,
		KeyType_Alt,
		KeyType_N,
		KeyType_O,
		KeyType_S,
		KeyType_R,
		KeyType_Esc,
		KeyType_F4,
		KeyType_NumOfKeys
	};
	enum TextEditorLanguageType : int
	{
		TextEditorLanguageType_None = 0,
		TextEditorLanguageType_Cpp,
		TextEditorLanguageType_C,
		TextEditorLanguageType_Cs,
		TextEditorLanguageType_Python,
		TextEditorLanguageType_Lua,
		TextEditorLanguageType_JSON,
		TextEditorLanguageType_SQL,
		TextEditorLanguageType_AngelScript,
		TextEditorLanguageType_GLSL,
		TextEditorLanguageType_HLSL
	};

	struct ComponentListEntry
	{
		ComponentListEntry(const EntityID p_entityID, const std::string &p_name, const std::string &p_combinedEntityIdAndName) : m_entityID(p_entityID), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName) { }

		EntityID m_entityID;
		std::string m_name;
		std::string m_combinedEntityIdAndName;
	};
	struct EntityListEntry
	{
		EntityListEntry(const EntityID p_entityID, const EntityID p_parentEntityID, const std::string &p_name, const std::string &p_combinedEntityIdAndName) : m_entityID(p_entityID), m_parentEntityID(p_parentEntityID), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName), m_componentFlag(Systems::AllComponentTypes::None) { }
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
		EntityHierarchyEntry(const EntityID p_entityID, const EntityID p_parent, const std::string &p_name, const std::string &p_combinedEntityIdAndName, const BitMask p_componentFlag) : m_entityID(p_entityID), m_parent(p_parent), m_name(p_name), m_combinedEntityIdAndName(p_combinedEntityIdAndName), m_componentFlag(p_componentFlag){ }
		void init(const EntityID p_entityID, const EntityID p_parent, const std::string &p_name, const std::string &p_combinedEntityIdAndName, const BitMask p_componentFlag)
		{
			m_entityID = p_entityID;
			m_parent = p_parent;
			m_name = p_name;
			m_combinedEntityIdAndName = p_combinedEntityIdAndName;
			m_componentFlag = p_componentFlag;
		}

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
					if(m_children[i]->findParentAndAddChild(p_childEntityEntry))
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
			m_children.push_back(new EntityHierarchyEntry(p_childEntityEntry));
		}
		void addChild(const EntityID p_entityID, const EntityID p_parent, const std::string &p_name, const std::string &p_combinedEntityIdAndName, const BitMask p_componentFlag)
		{
			m_children.push_back(new EntityHierarchyEntry(p_entityID, p_parent, p_name, p_combinedEntityIdAndName, p_componentFlag));
		}
		void removeChild(const EntityHierarchyEntry &p_childEntityEntry)
		{
			std::vector<EntityHierarchyEntry *>::iterator childPosition = std::find_if(m_children.begin(), m_children.end(), [&](EntityHierarchyEntry *e) { return *e == p_childEntityEntry; });// std::find(m_children.begin(), m_children.end(), p_childEntityEntry);
			if(childPosition != m_children.end())
				m_children.erase(childPosition);
		}
		bool containsChildren() const { return !m_children.empty(); }
		void getEntries(std::vector<EntityHierarchyEntry*> &p_entriesList)
		{
			p_entriesList.push_back(this);
			for(decltype(m_children.size()) size = m_children.size(), i = 0; i < size; i++)
				m_children[i]->getEntries(p_entriesList);
		}
		void clear()
		{
			for(auto *child : m_children)
			{
				child->clear();
				delete child;
			}

			m_children.clear();
		}

		EntityID m_entityID;
		EntityID m_parent;
		BitMask m_componentFlag;
		std::string m_name;
		std::string m_combinedEntityIdAndName;
		std::vector<EntityHierarchyEntry*> m_children;
	};
	struct SelectedEntity
	{
		SelectedEntity(const Observer &p_parent) : m_spatialDataManager(p_parent)
		{ 
			setEntity(NULL_ENTITY_ID);
			m_playing = false;
			m_soundType = SoundComponent::SoundType::SoundType_Null;
			m_soundSourceType = 0;
			m_objectMaterialType = ObjectMaterialType::Concrete;
			m_lightType = LightComponent::LightComponentType::LightComponentType_null;
			m_collisionShapeType = RigidBodyComponent::CollisionShapeType::CollisionShapeType_Null;

			m_componentData.m_audioComponents.m_soundConstructionInfo = new SoundComponent::SoundComponentConstructionInfo();
			m_componentData.m_audioComponents.m_soundListenerConstructionInfo = new SoundListenerComponent::SoundListenerComponentConstructionInfo();

			m_componentData.m_graphicsComponents.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo();
			m_componentData.m_graphicsComponents.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo();
			m_componentData.m_graphicsComponents.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo();
			m_componentData.m_graphicsComponents.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo();

			m_componentData.m_guiComponents.m_guiSequenceConstructionInfo = new GUISequenceComponent::GUISequenceComponentConstructionInfo();

			m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo = new RigidBodyComponent::RigidBodyComponentConstructionInfo();

			m_componentData.m_scriptComponents.m_luaConstructionInfo = new LuaComponent::LuaComponentConstructionInfo();

			m_componentData.m_worldComponents.m_objectMaterialConstructionInfo = new ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo();
			m_componentData.m_worldComponents.m_spatialConstructionInfo = new SpatialComponent::SpatialComponentConstructionInfo();

			// Populate the component type text array and also strip the prefix and suffix of ComponentType string
			std::string componentTypePrefix = "ComponentType_";
			std::string componentTypeSuffix = "Component";
			for(unsigned int i = 0; i < ComponentType::ComponentType_NumOfTypes; i++)
			{
				m_componentTypeText[i].first = GetString(static_cast<ComponentType>(i));
				m_componentTypeText[i].first = m_componentTypeText[i].first.substr(componentTypePrefix.size());
				m_componentTypeText[i].first = Utilities::splitStringBeforeDelimiter(componentTypeSuffix, m_componentTypeText[i].first);
				m_componentTypeText[i].first += " component";
			}
		}

		inline operator bool() const { return m_entityID != NULL_ENTITY_ID; }

		void setEntity(const EntityID p_entityID)
		{
			m_entityID = p_entityID;
			m_luaVariablesModified = false;
			m_luaScriptFilenameModified = false;
			m_modelDataModified = false;
			m_prefabNameModified = false;
			m_soundFilenameModified = false;
			m_selectedModelName = nullptr;
			m_selectedTextureName = nullptr;
			m_modelDataPointer = nullptr;
			m_modelDataUpdateAfterLoading = false;
			m_modelDataUpdatedFromFilebrowser = false;

			clearComponentExistFlags();
		}

		void unselect()
		{
			m_entityID = NULL_ENTITY_ID;
			m_soundFilenameModified = false;
			m_luaVariablesModified = false;
			m_luaScriptFilenameModified = false;
		}

		void clearComponentExistFlags()
		{
			for(unsigned int i = 0; i < ComponentType::ComponentType_NumOfTypes; i++)
				m_componentTypeText[i].second = false;
		}

		EntityID m_entityID;

		ComponentsConstructionInfo m_componentData;

		// SoundComponent data
		bool m_playing;
		int m_soundType;
		int m_soundSourceType;

		// SpatialComponent data
		SpatialDataManager m_spatialDataManager;

		// ObjectMaterialComponent data 
		int m_objectMaterialType;

		// LightComponent data
		int m_lightType;

		// ModelComponent data
		std::string *m_selectedModelName;
		std::string *m_selectedTextureName;
		std::vector<ModelData> const *m_modelDataPointer;
		bool m_modelDataUpdateAfterLoading;
		bool m_modelDataUpdatedFromFilebrowser;

		// RigidBodyComponent data
		int m_collisionShapeType;

		// An array of external lua variables
		std::vector<std::pair<std::string, Property>> m_luaVariables;

		// An array containing component type text and flags marking whether a component type exists for the selected entity
		std::pair<std::string, bool> m_componentTypeText[ComponentType::ComponentType_NumOfTypes];

		bool m_luaVariablesModified;
		bool m_luaScriptFilenameModified;
		bool m_modelDataModified;
		bool m_prefabNameModified;
		bool m_soundFilenameModified;
	};
	struct SceneData
	{
		SceneData()
		{
			m_loadInBackground = false;
			m_modified = true;

			for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
				m_volume[i] = 1.0f;

			m_activeCameraID = 0;

			m_aoData.setDefaultValues();

			m_shadowMappingData.setDefaultValues();

			m_renderingPasses.push_back(RenderPassType::RenderPassType_ShadowMapping);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Geometry);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_AmbientOcclusion);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Lighting);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Luminance);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Bloom);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Tonemapping);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Final);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_GUI);

			m_gravity = glm::vec3(0.0f, -9.8f, 0.0f);
		}

		// General
		bool m_loadInBackground;
		bool m_modified;
		std::string m_sceneFilename;

		// Audio scene
		std::vector<std::string> m_audioBanks;
		float m_volume[AudioBusType::AudioBusType_NumOfTypes];

		// Graphics scene
		int m_activeCameraID;
		AmbientOcclusionData m_aoData;
		AtmosphericScatteringData m_atmScatteringData;
		MiscSceneData m_miscSceneData;
		ShadowMappingData m_shadowMappingData;
		RenderingPasses m_renderingPasses;

		// Physics scene
		glm::vec3 m_gravity;
	};
	struct TextEditorData
	{
		TextEditorData()
		{
			m_textEditorEnabled = true;
			m_textEditorTabFlags = 0;
			m_languageType = TextEditorLanguageType::TextEditorLanguageType_None;
			m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
		}

		inline void enable()
		{
			m_textEditorEnabled = true;
			m_textEditorTabFlags = ImGuiTabItemFlags_SetSelected;
		}

		inline void setText(const std::string &p_test)
		{
			m_textEditor.SetText(p_test);
			m_textEditor.ResetTextChanged();
		}

		inline void setLanguage(const TextEditorLanguageType p_languageType)
		{
			m_languageType = p_languageType;
			switch(p_languageType)
			{
				case EditorWindow::TextEditorLanguageType_None:
				default:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
					break;
				case EditorWindow::TextEditorLanguageType_Cpp:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
					break;
				case EditorWindow::TextEditorLanguageType_C:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::C);
					break;
				case EditorWindow::TextEditorLanguageType_Cs:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cs);
					break;
				case EditorWindow::TextEditorLanguageType_Python:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Python);
					break;
				case EditorWindow::TextEditorLanguageType_Lua:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
					break;
				case EditorWindow::TextEditorLanguageType_JSON:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
					break;
				case EditorWindow::TextEditorLanguageType_SQL:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Sql);
					break;
				case EditorWindow::TextEditorLanguageType_AngelScript:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::AngelScript);
					break;
				case EditorWindow::TextEditorLanguageType_GLSL:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
					break;
				case EditorWindow::TextEditorLanguageType_HLSL:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Hlsl);
					break;
			}
		}

		TextEditor m_textEditor;
		bool m_textEditorEnabled;
		std::string m_filename;
		std::string m_filePath;
		ImGuiTabItemFlags m_textEditorTabFlags;
		TextEditorLanguageType m_languageType;
	};

	void drawSceneData(SceneData &p_sceneData, const bool p_sendChanges = false);
	void drawEntityHierarchy(EntityHierarchyEntry *p_rootEntry);
	void drawEntityHierarchyEntry(EntityHierarchyEntry *p_entityEntry);
	inline void drawLeftAlignedLabelText(const char *p_labelText, float p_nextWidgetOffset)
	{
		const auto regionWidth = ImGui::GetContentRegionAvail().x;
		ImGui::AlignTextToFramePadding();
		ImGui::Text(p_labelText);
		ImGui::SameLine();
		ImGui::SetCursorPosX(p_nextWidgetOffset);
		ImGui::SetNextItemWidth(regionWidth - p_nextWidgetOffset);
	}
	inline void drawLeftAlignedLabelText(const char *p_labelText, float p_nextWidgetOffset, float p_nextItemWidth)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(p_labelText);
		ImGui::SameLine();
		ImGui::SetCursorPosX(p_nextWidgetOffset);
		ImGui::SetNextItemWidth(p_nextItemWidth);
	}
	inline bool drawTextSizedButton(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const std::string p_tooltipText = "")
	{
		bool returnBool = false;

		// Draw the open button
		if(ImGui::ImageButton(p_buttonLabel.c_str(),
			(ImTextureID)p_texture.getHandle(),
			m_buttonSizedByFont,
			ImVec2(0, 1),
			ImVec2(1, 0),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
		{
			returnBool = true;
		}

		// Draw the tooltip if the tooltip text is not empty and button is hovered over
		if(!p_tooltipText.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
			ImGui::SetTooltip(p_tooltipText.c_str(), ImGui::GetStyle().HoverDelayShort);

		return returnBool;
	}
	inline bool drawTextSizedButtonInverted(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const std::string p_tooltipText = "")
	{
		bool returnBool = false;

		// Draw the open button
		if(ImGui::ImageButton(p_buttonLabel.c_str(),
			(ImTextureID)p_texture.getHandle(),
			m_buttonSizedByFont,
			ImVec2(1, 0),
			ImVec2(0, 1),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
		{
			returnBool = true;
		}

		// Draw the tooltip if the tooltip text is not empty and button is hovered over
		if(!p_tooltipText.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
			ImGui::SetTooltip(p_tooltipText.c_str(), ImGui::GetStyle().HoverDelayShort);

		return returnBool;
	}
	inline void drawExportPrefabFileBrowser()
	{
		// Only open the file browser if it's not opened already
		if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
		{
			// Set the file browser activation to Save Scene
			m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SavePrefabFile;

			// Define file browser variables
			m_fileBrowserDialog.m_definedFilename = m_selectedEntity.m_componentData.m_name + ".prefab";
			m_fileBrowserDialog.m_filter = ".prefab,.*";
			m_fileBrowserDialog.m_title = "Save prefab";
			m_fileBrowserDialog.m_name = "SavePrefabFileDialog";
			m_fileBrowserDialog.m_rootPath = Config::filepathVar().prefab_path;
			m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_ConfirmOverwrite;

			// Tell the GUI scene to open the file browser
			m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
		}
	}

	// Hides and locks the mouse to the screen while an item (like a float drag) is active
	// Returns the mouse to the original position after the mouse button is released
	void captureMouseWhileItemActive();

	// Calculates the offset for a square text-sized button from the right side of the edge 
	// (p_buttonIndex is the button count from the right side)
	inline float calcTextSizedButtonOffset(const int p_buttonIndex = 0)
	{
		return ImGui::GetContentRegionAvail().x - m_buttonSizedByFont.x - (m_imguiStyle.FramePadding.x * 2.0f) - (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3.0f) * p_buttonIndex;
	}	
	inline float calcTextSizedButtonSize(const unsigned int p_buttonIndex = 0)
	{
		return m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x + (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3) * p_buttonIndex;
	}

	inline int getShaderAssetIndex(const ShaderLoader::ShaderProgram *p_selectedProgram) const
	{
		for(decltype(m_shaderAssets.size()) shaderAssetIndex = 0, shaderAssetSize = m_shaderAssets.size(); shaderAssetIndex < shaderAssetSize; shaderAssetIndex++)
			if(m_shaderAssets[shaderAssetIndex].first->getCombinedFilename() == p_selectedProgram->getCombinedFilename())
				return (int)shaderAssetIndex;
	}
	inline void clearEntityAndComponentPool()
	{
		if(!m_entityAndComponentPool.empty())
		{
			for(decltype(m_entityAndComponentPool.size()) i = 0, size = m_entityAndComponentPool.size(); i < size; i++)
				delete m_entityAndComponentPool[i];

			m_entityAndComponentPool.clear();
		}
	}
	inline void clearConstructionInfoPool()
	{
		if(!m_componentConstructionInfoPool.empty())
		{
			for(decltype(m_componentConstructionInfoPool.size()) i = 0, size = m_componentConstructionInfoPool.size(); i < size; i++)
				delete m_componentConstructionInfoPool[i];

			m_componentConstructionInfoPool.clear();
		}
	}
	inline void resetActivateAllComponentFlags()
	{
		m_lightComponentActivateAllSet = false;
		m_modelComponentActivateAllSet = false;
		m_rigidBodyComponentActivateAllSet = false;
	}

	void exportPrefab(const EntityID p_entityID, std::string p_filename);
	void saveTextFile(TextEditorData &p_textEditorData);
	bool processShortcuts();
	void processMainMenuButton(MainMenuButtonType &p_mainMenuButtonType);
	void updateSceneData(SceneData &p_sceneData);
	void updateEntityList();
	void updateHierarchyList();
	void updateComponentList();
	void updateAssetLists();

	void duplicateEntity(const EntityID p_entityID)
	{
		// Construction info for the new entity
		ComponentsConstructionInfo *newEntityConstructionInfo = new ComponentsConstructionInfo();

		// Get the construction info of the current entity
		WorldScene *worldScene = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
		worldScene->exportEntity(p_entityID, *newEntityConstructionInfo);

		// Set the new entity name by adding a count at the end and checking if an entity of the same name doesn't exist
		{
			int newEntityNameCount = 2;
			std::string baseEntityName = newEntityConstructionInfo->m_name;

			// If the entity name ends with ' 1', replace the existing number
			if(baseEntityName.size() > 2 && baseEntityName[baseEntityName.size() - 1] == '1' && baseEntityName[baseEntityName.size() - 2] == ' ')
			{
				baseEntityName.pop_back();  // Remove '1'
				baseEntityName.pop_back();  // Remove ' '
			}
			std::string newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);

			// Keep increasing the number at the end until there is no other entity with the same name
			while(worldScene->getEntity(newEntityName) != NULL_ENTITY_ID)
			{
				newEntityNameCount++;
				newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);
			}
			newEntityConstructionInfo->m_name = newEntityName;
		}

		// Assign a next available entity ID (start the available ID search from the next ID after the parent)
		{
			EntityID newEntityID = newEntityConstructionInfo->m_parent + 1;
			for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size;)
			{
				if(newEntityID == m_entityList[i].m_entityID)
				{
					newEntityID++;
					i = 0;
				}
				else
					i++;
			}
			newEntityConstructionInfo->m_id = newEntityID;
		}


		m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateEntity, (void *)newEntityConstructionInfo, false);

		// Make the new entity be selected next frame
		m_nextEntityIDToSelect = newEntityConstructionInfo->m_id;
		m_pendingEntityToSelect = true;

		// Add the new entity construction info to the pool (so it will be deleted the next frame)
		m_componentConstructionInfoPool.push_back(newEntityConstructionInfo);
	}
	void deleteEntityAndChildren(const EntityID p_entityID)
	{
		// Get entity children
		std::vector<EntityID> childrenEntityIDs;
		if(p_entityID != 0)
			childrenEntityIDs.push_back(p_entityID);
		if(auto *entityListEntry = getEntityListEntry(p_entityID); entityListEntry != nullptr)
			getEntityChildren(childrenEntityIDs, *entityListEntry);

		// Do not allow the deletion of root entity
		for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
		{
			// Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Entity change with the attached container
			EntityAndComponent *deleteComponentData = new EntityAndComponent(childrenEntityIDs[i], ComponentType::ComponentType_Entity);
			m_entityAndComponentPool.push_back(deleteComponentData);
			m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteEntity, (void *)deleteComponentData, false);
		}
	}
	EntityListEntry *getEntityListEntry(const EntityID p_entityID)
	{
		for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
		{
			if(m_entityList[i].m_entityID == p_entityID)
			{
				return &m_entityList[i];
			}
		}

		return nullptr;
	}
	inline void getEntityChildren(std::vector<EntityID> &p_children, const EntityListEntry &p_entityHierarchyEntry)
	{
		for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
		{
			if(m_entityList[i].m_parentEntityID == p_entityHierarchyEntry.m_entityID)
			{
				p_children.push_back(m_entityList[i].m_entityID);

				// Do not process the root node as it will cause an infinite loop
				if(p_entityHierarchyEntry.m_entityID != 0)
					getEntityChildren(p_children, m_entityList[i]);
			}
		}
	}

	void generateNewMap(PropertySet &p_newSceneProperties, SceneData &p_sceneData);

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
		case TextureFormat_R:
			return "R";
			break;
		case TextureFormat_RG:
			return "RG";
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
			case TextureDataType::TextureDataType_Short:
				return "Short";
				break;
			case TextureDataType::TextureDataType_Float:
				return "Float";
				break;
			case TextureDataType::TextureDataType_Int:
				return "Integer";
				break;
			case TextureDataType::TextureDataType_UnsignedByte:
				return "Unsigned byte";
				break;
		}
		return "";
	}
	inline std::string getTextureDataFormat(const TextureDataFormat p_textureDataFormat) const
	{
		switch(p_textureDataFormat)
		{
			case TextureDataFormat::TextureDataFormat_R8:
				return "R8";
				break;
			case TextureDataFormat::TextureDataFormat_R16:
				return "R16";
				break;
			case TextureDataFormat::TextureDataFormat_R16F:
				return "R16F";
				break;
			case TextureDataFormat::TextureDataFormat_R32F:
				return "R32F";
				break;
			case TextureDataFormat::TextureDataFormat_RG8:
				return "RG8";
				break;
			case TextureDataFormat::TextureDataFormat_RG16:
				return "RG16";
				break;
			case TextureDataFormat::TextureDataFormat_RG16F:
				return "RG16F";
				break;
			case TextureDataFormat::TextureDataFormat_RG32F:
				return "RG32F";
				break;
			case TextureDataFormat::TextureDataFormat_RGB8:
				return "RGB8";
				break;
			case TextureDataFormat::TextureDataFormat_RGB16:
				return "RGB16";
				break;
			case TextureDataFormat::TextureDataFormat_RGB16F:
				return "RGB16F";
				break;
			case TextureDataFormat::TextureDataFormat_RGB32F:
				return "RGB32F";
				break;
			case TextureDataFormat::TextureDataFormat_RGBA8:
				return "RGBA8";
				break;
			case TextureDataFormat::TextureDataFormat_RGBA16:
				return "RGBA16";
				break;
			case TextureDataFormat::TextureDataFormat_RGBA16SN:
				return "RGBA16 SNORM";
				break;
			case TextureDataFormat::TextureDataFormat_RGBA16F:
				return "RGBA16F";
				break;
			case TextureDataFormat::TextureDataFormat_RGBA32F:
				return "RGBA32F";
				break;
			case TextureDataFormat::TextureDataFormat_R16I:
				return "R16I";
				break;
			case TextureDataFormat::TextureDataFormat_R32I:
				return "R32I";
				break;
			case TextureDataFormat::TextureDataFormat_R16UI:
				return "R16UI";
				break;
			case TextureDataFormat::TextureDataFormat_R32UI:
				return "R32UI";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGB:
				return "COMPRESSED RGB";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGBA:
				return "COMPRESSED RGBA";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGB:
				return "DXT1 RGB (BC1)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGBA:
				return "DXT1 RGBA (BC1)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT3_RGBA:
				return "DXT3 RGBA (BC2)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT5_RGBA:
				return "DXT5 RGBA (BC3)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC1_R:
				return "RGTC1 R (BC4)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC2_RG:
				return "RGTC2 RG (BC5)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA:
				return "BPTC RGBA (BC7)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_R:
				return "EAC R11 (ETC2)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_RG:
				return "EAC RG11 (ETC2)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGB:
				return "ETC2 RGB8 (ETC2)";
				break;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGBA:
				return "ETC2 RGBA8 (ETC2)";
				break;
		}
		return "";
	}

	bool m_renderSceneToTexture;
	bool m_GUISequenceEnabled;
	bool m_LUAScriptingEnabled;
	bool m_translateGuizmoEnabled;
	bool m_rotateGuizmoEnabled;
	bool m_showNewMapWindow;
	bool m_showImGuiDemoWindow;
	bool m_showImspinnerDemoWindow;
	bool m_fullscreen;
	bool m_enlargedSceneViewport;
	bool m_showingExitDialog;

	MainMenuButtonType m_activatedMainMenuButton;
	EditorSceneState m_sceneState;
	glm::ivec2 m_centerWindowSize;
	ImGuiID m_activeItemID;

	bool m_mouseCaptured;
	glm::ivec2 m_mousePositionBeforeCapture;

	// GUI settings
	MaterialType m_buttonMaterialType;
	ImGuiColorEditFlags m_colorEditFlags;
	float m_browseButtonWidth;
	FileBrowserActivated m_currentlyOpenedFileBrowser;
	FileBrowserActivated m_previouslyOpenedFileBrowser;
	FileBrowserDialog m_fileBrowserDialog;
	const ImVec2 m_playPauseButtonSize;
	ImVec2 m_assetSelectionPopupImageSize;
	ImVec2 m_textureAssetImageSize;
	ImVec4 m_buttonBackgroundEnabled;
	ImVec4 m_buttonBackgroundDisabled;
	ImVec2 m_mousePositionOnNewEntity;
	bool m_newEntityWindowInitialized;
	ImVec2 m_sceneViewportPosition;
	ImVec2 m_sceneViewportSize;
	bool m_synchronizeTextureScale;
	bool m_synchronizeTextureFraming;
	bool m_2DTextureScale;
	bool m_2DTextureFraming;

	// Console settings
	std::size_t m_numOfLogs;

	// Assets variables
	std::vector<std::pair<const Texture2D *, std::string>> m_textureAssets;
	std::vector<std::pair<Model *, std::string>> m_modelAssets;
	std::vector<std::pair<ShaderLoader::ShaderProgram *, std::string>> m_shaderAssets;
	std::vector<std::pair<EntityID, std::string>> m_luaScriptAssets;
	std::string m_textureAssetLongestName;
	std::string m_modelAssetLongestName;
	std::string m_shaderAssetLongestName;
	std::string m_selectedShaderFilename;
	EntityID m_selectedLuaScript;
	Model *m_selectedModel;
	ShaderLoader::ShaderProgram *m_selectedProgram;
	int m_selectedShaderType;

	// Texture inspector variables
	Texture2D const * m_selectedTexture;
	ImGuiTabItemFlags m_textureInspectorTabFlags;
	DoubleBufferedContainer<FunctorSequence> m_textureInspectorSequence;

	// Scene entities
	std::vector<ComponentListEntry> m_componentList;
	std::vector<EntityListEntry> m_entityList;
	EntityHierarchyEntry m_rootEntityHierarchyEntry;
	SelectedEntity m_selectedEntity;
	SceneData m_currentSceneData;
	EntityID m_nextEntityIDToSelect; 
	ComponentsConstructionInfo *m_nextEntityToSelect;
	bool m_pendingEntityToSelect;

	// Used to hold entity and component data for component creation / deletion until the next frame, after sending the data as a change
	std::vector<EntityAndComponent*> m_entityAndComponentPool;
	std::vector<ComponentsConstructionInfo*> m_componentConstructionInfoPool;
	ComponentsConstructionInfo *m_newEntityConstructionInfo;
	bool m_openNewEntityPopup;
	bool m_openEntityRightClickOptionsPopup;
	bool m_duplicateParent;
	bool m_lightComponentActivateAllSet;
	bool m_modelComponentActivateAllSet;
	bool m_rigidBodyComponentActivateAllSet;

	// New scene settings
	SceneData m_newSceneData;
	ImGuiTabItemFlags m_newSceneSettingsTabFlags;

	// Button textures
	std::vector<TextureLoader2D::Texture2DHandle> m_buttonTextures;

	// ImGui properties
	ImGuiStyle &m_imguiStyle;
	float m_fontSize;

	// Square button size that is the same height as text
	ImVec2 m_buttonSizedByFont;

	// Names for ImGui widgets
	const char *m_entityRightClickOptionsPopup;

	// String arrays and other data used for ImGui Combo inputs
	std::vector<const char *> m_antialiasingTypeText;
	std::vector<const char *> m_ambientOcclusionTypeText;
	std::vector<const char *> m_cascadeDistanceTypeText;
	std::vector<const char *> m_physicalMaterialProperties;
	std::vector<const char *> m_renderingPassesTypeText;
	std::vector<const char *> m_luaVariableTypeStrings;
	std::vector<const char *> m_shaderTypeStrings;
	std::vector<const char *> m_tonemappingMethodText;
	std::vector<const char *> m_textEditorLanguageTypeText;
	std::vector<const char *> m_textureWrapModeStrings;
	std::vector<TextureWrapType> m_textureWrapModeTypes;

	// Text editor
	std::vector<TextEditorData *> m_textEditorFiles;
	TextEditorData *m_currentlyActiveTextEditor;
};
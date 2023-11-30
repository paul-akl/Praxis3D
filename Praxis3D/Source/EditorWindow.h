#pragma once

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
		m_LUAScriptingEnabled = true;
		m_translateGuizmoEnabled = true;
		m_rotateGuizmoEnabled = false;
		m_showNewMapWindow = false;
		m_sceneState = EditorSceneState::EditorSceneState_Pause;
		m_centerWindowSize = glm::ivec2(0);

		m_selectedTexture = nullptr;
		m_textureInspectorTabFlags = 0;

		m_colorEditFlags = ImGuiColorEditFlags_Float;
		m_browseButtonWidth = 60.0f;
		m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
		m_fileBrowserDialog.m_name = "EditorFileBrowserDialog";

		m_luaVariableTypeStrings = { "null", "bool", "int", "float", "double", "vec2i", "vec2f", "vec3f", "vec4f", "string", "propertyID" };

		m_shaderTypeStrings = { "Compute", "Fragment", "Geometry", "Vertex", "Tessellation control", "Tessellation evaluation" };
		m_selectedProgramShader = -1;
		m_selectedShader = -1;

		for(unsigned int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
			m_physicalMaterialProperties.push_back(GetString(static_cast<ObjectMaterialType>(i)));

		for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
			m_renderingPassesTypeText.push_back(GetString(static_cast<RenderPassType>(i)));

		m_newSceneSettingsTabFlags = 0;

		m_fontSize = ImGui::GetFontSize();
		m_buttonSizedByFont = ImVec2(m_fontSize, m_fontSize);
		m_assetSelectionPopupImageSize = ImVec2(m_fontSize, m_fontSize) * Config::GUIVar().editor_asset_selection_button_size_multiplier;
		m_textureAssetImageSize = ImVec2(Config::GUIVar().editor_asset_texture_button_size_x, Config::GUIVar().editor_asset_texture_button_size_y);

		m_buttonBackgroundEnabled = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
		m_buttonBackgroundDisabled = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
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

			case Systems::Changes::Physics::Friction:
				{
					return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction;
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

			case Systems::Changes::Audio::Filename:
				{
					return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundFilename;
				}
				break;

			case Systems::Changes::Script::Filename:
				{
					return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename;
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
	struct SelectedEntity
	{
		SelectedEntity(const Observer &p_parent) : m_spatialDataManager(p_parent)
		{ 
			setEntity(NULL_ENTITY_ID);
			m_playing = false;
			m_soundType = SoundComponent::SoundType::SoundType_Null;
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
			m_soundFilenameModified = false;
			m_selectedModelName = nullptr;
			m_selectedTextureName = nullptr;
			m_modelDataPointer = nullptr;
			m_modelDataUpdateAfterLoading = false;

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

		// RigidBodyComponent data
		int m_collisionShapeType;

		// An array of external lua variables
		std::vector<std::pair<std::string, Property>> m_luaVariables;

		// An array containing component type text and flags marking whether a component type exists for the selected entity
		std::pair<std::string, bool> m_componentTypeText[ComponentType::ComponentType_NumOfTypes];

		bool m_luaVariablesModified;
		bool m_luaScriptFilenameModified;
		bool m_modelDataModified;
		bool m_soundFilenameModified;
	};
	struct SceneData
	{
		SceneData()
		{
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Geometry);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Lighting);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Bloom);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Luminance);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Final);
			m_renderingPasses.push_back(RenderPassType::RenderPassType_GUI);

			for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
				m_volume[i] = 1.0f;

			m_gravity = glm::vec3(0.0f, -9.8f, 0.0f);
			m_loadInBackground = false;
			m_modified = true;
		}

		RenderingPasses m_renderingPasses;
		std::vector<std::string> m_audioBanks;

		float m_volume[AudioBusType::AudioBusType_NumOfTypes];
		glm::vec3 m_gravity;
		bool m_loadInBackground;
		bool m_modified;
	};

	enum ButtonTextureType : unsigned int
	{
		ButtonTextureType_Pause,
		ButtonTextureType_Play,
		ButtonTextureType_Restart,
		ButtonTextureType_GUISequence,
		ButtonTextureType_ScriptingEnable,
		ButtonTextureType_DeleteEntry,
		ButtonTextureType_Add,
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
		FileBrowserActivated_AudioBankFile
	};

	void drawSceneData(SceneData &p_sceneData, const bool p_sendChanges = false);
	void drawEntityHierarchyEntry(EntityHierarchyEntry &p_entityEntry);
	inline void drawLeftAlignedLabelText(const char *p_labelText, float p_nextWidgetOffset)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(p_labelText);
		ImGui::SameLine();
		ImGui::SetCursorPosX(p_nextWidgetOffset);
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - p_nextWidgetOffset);
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

	// Calculates the offset for a square text-sized button from the right side of the edge 
	// (p_buttonIndex is the button count from the right side)
	inline float calcTextSizedButtonOffset(const int p_buttonIndex = 0)
	{
		return ImGui::GetContentRegionAvail().x - m_buttonSizedByFont.x - m_imguiStyle.FramePadding.x - (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3) * p_buttonIndex;
	}	
	inline float calcTextSizedButtonSize(const unsigned int p_buttonIndex = 0)
	{
		return m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x + (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3) * p_buttonIndex;
	}

	void clearEntityAndComponentPool()
	{
		if(!m_entityAndComponentPool.empty())
		{
			for(decltype(m_entityAndComponentPool.size()) i = 0, size = m_entityAndComponentPool.size(); i < size; i++)
				delete m_entityAndComponentPool[i];

			m_entityAndComponentPool.clear();
		}
	}
	void clearConstructionInfoPool()
	{
		if(!m_componentConstructionInfoPool.empty())
		{
			for(decltype(m_componentConstructionInfoPool.size()) i = 0, size = m_componentConstructionInfoPool.size(); i < size; i++)
				delete m_componentConstructionInfoPool[i];

			m_componentConstructionInfoPool.clear();
		}
	}
	void updateSceneData(SceneData &p_sceneData);
	void updateEntityList();
	void updateHierarchyList();
	void updateComponentList();
	void updateAssetLists();

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

	bool m_renderSceneToTexture;
	bool m_GUISequenceEnabled;
	bool m_LUAScriptingEnabled;
	bool m_translateGuizmoEnabled;
	bool m_rotateGuizmoEnabled;
	bool m_showNewMapWindow;
	EditorSceneState m_sceneState;
	glm::ivec2 m_centerWindowSize;
	std::vector<const char *> m_physicalMaterialProperties;

	// GUI settings
	ImGuiColorEditFlags m_colorEditFlags;
	float m_browseButtonWidth;
	FileBrowserActivated m_currentlyOpenedFileBrowser;
	FileBrowserDialog m_fileBrowserDialog;
	const ImVec2 m_playPauseButtonSize;
	ImVec2 m_assetSelectionPopupImageSize;
	ImVec2 m_textureAssetImageSize;
	ImVec4 m_buttonBackgroundEnabled;
	ImVec4 m_buttonBackgroundDisabled;

	// LUA variables editor data
	std::vector<const char *> m_luaVariableTypeStrings;

	// Assets variables
	std::vector<const char *> m_shaderTypeStrings;
	std::vector<std::pair<const Texture2D *, std::string>> m_textureAssets;
	std::vector<std::pair<const Model *, std::string>> m_modelAssets;
	std::vector<std::pair<const ShaderLoader::ShaderProgram *, std::string>> m_shaderAssets;
	std::string m_textureAssetLongestName;
	std::string m_modelAssetLongestName;
	std::string m_shaderAssetLongestName;
	int m_selectedProgramShader;
	int m_selectedShader;

	// Texture inspector variables
	Texture2D const * m_selectedTexture;
	ImGuiTabItemFlags m_textureInspectorTabFlags;
	DoubleBufferedContainer<FunctorSequence> m_textureInspectorSequence;

	// Scene entities
	std::vector<ComponentListEntry> m_componentList;
	std::vector<EntityListEntry> m_entityList;
	std::vector<EntityHierarchyEntry> m_entityHierarchy;
	SelectedEntity m_selectedEntity;
	SceneData m_currentSceneData;

	// Used to hold entity and component data for component creation / deletion until the next frame, after sending the data as a change
	std::vector<EntityAndComponent*> m_entityAndComponentPool;
	std::vector<ComponentsConstructionInfo*> m_componentConstructionInfoPool;

	// New scene settings
	SceneData m_newSceneData;
	ImGuiTabItemFlags m_newSceneSettingsTabFlags;
	std::vector<const char *> m_renderingPassesTypeText;

	// Button textures
	std::vector<TextureLoader2D::Texture2DHandle> m_buttonTextures;

	// ImGui properties
	ImGuiStyle &m_imguiStyle;
	float m_fontSize;

	// Square button size that is the same height as text
	ImVec2 m_buttonSizedByFont;
};
#pragma once

// Data types owned by the EditorWindow component.

#include <string>
#include <utility>
#include <vector>

#include <TextEditor.h>

#include "Systems/GUISystem/Include/GUIScene.hpp"
#include "Systems/WorldSystem/Components/Include/ComponentConstructorInfo.hpp"
#include "Definitions/Include/InheritanceObjects.hpp"
#include "Systems/RendererSystem/Components/Include/LightComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"

class EditorWindow;

namespace EditorWindowTypes
{
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

		void addChild(const EntityHierarchyEntry &p_childEntityEntry)
		{
			m_children.push_back(new EntityHierarchyEntry(p_childEntityEntry));
		}
		void addChild(const EntityID p_entityID, const EntityID p_parent, const std::string &p_name, const std::string &p_combinedEntityIdAndName, const BitMask p_componentFlag)
		{
			m_children.push_back(new EntityHierarchyEntry(p_entityID, p_parent, p_name, p_combinedEntityIdAndName, p_componentFlag));
		}
		bool containsChildren() const { return !m_children.empty(); }
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
			m_renderingPasses.push_back(RenderPassType::RenderPassType_Picking);
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
				case TextEditorLanguageType_None:
				default:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
					break;
				case TextEditorLanguageType_Cpp:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
					break;
				case TextEditorLanguageType_C:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::C);
					break;
				case TextEditorLanguageType_Cs:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cs);
					break;
				case TextEditorLanguageType_Python:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Python);
					break;
				case TextEditorLanguageType_Lua:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
					break;
				case TextEditorLanguageType_JSON:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
					break;
				case TextEditorLanguageType_SQL:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Sql);
					break;
				case TextEditorLanguageType_AngelScript:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::AngelScript);
					break;
				case TextEditorLanguageType_GLSL:
					m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
					break;
				case TextEditorLanguageType_HLSL:
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
}

// Makes every type above a member of EditorWindow. The "using enum" declarations additionally make the enumerators direct members
#define EDITORWINDOW_TYPES_RECORD(memberTypesNamespace) \
	public: \
		using SelectedEntity = memberTypesNamespace::SelectedEntity; \
		using SceneData = memberTypesNamespace::SceneData; \
		using TextEditorData = memberTypesNamespace::TextEditorData; \
		using ComponentListEntry = memberTypesNamespace::ComponentListEntry; \
		using EntityListEntry = memberTypesNamespace::EntityListEntry; \
		using EntityHierarchyEntry = memberTypesNamespace::EntityHierarchyEntry; \
		using ButtonTextureType = memberTypesNamespace::ButtonTextureType; \
		using enum memberTypesNamespace::ButtonTextureType; \
		using EditorSceneState = memberTypesNamespace::EditorSceneState; \
		using enum memberTypesNamespace::EditorSceneState; \
		using FileBrowserActivated = memberTypesNamespace::FileBrowserActivated; \
		using enum memberTypesNamespace::FileBrowserActivated; \
		using MainMenuButtonType = memberTypesNamespace::MainMenuButtonType; \
		using enum memberTypesNamespace::MainMenuButtonType; \
		using TextEditorLanguageType = memberTypesNamespace::TextEditorLanguageType; \
		using enum memberTypesNamespace::TextEditorLanguageType;

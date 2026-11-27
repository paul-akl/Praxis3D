#pragma once

#include "Systems/GUISystem/Objects/Include/EditorWindowTypes.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindowWidgets.hpp"

#include "Definitions/Include/EntityViewDefinitions.hpp"
#include "ServiceLocators/Include/ErrorHandlerLocator.hpp"
#include "Systems/GUISystem/Include/GUIHandler.hpp"
#include "ServiceLocators/Include/GUIHandlerLocator.hpp"
#include "Systems/RendererSystem/Components/Include/LightComponent.hpp"
#include "Loaders/Include/Loaders.hpp"
#include "Definitions/Include/InheritanceObjects.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"
#include "Systems/Base/Include/System.hpp"

class SceneLoader;
class WorldScene;
class RendererScene;
class UniversalScene;
class SystemScene;
class MetadataComponent;
struct FileBrowserDialog;

class EditorWindow : public SystemObject
{
	friend class GUIScene;
	EDITORWINDOW_TYPES_RECORD(EditorWindowTypes)

public:
	EditorWindow(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0);
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
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits) const;
	const inline glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
			return m_selectedEntity.m_spatialDataManager.getMat4(p_observer, p_changedBits);
	}

	const bool getBool(const Observer *p_observer, BitMask p_changedBits) const;
	const int getInt(const Observer *p_observer, BitMask p_changedBits) const;
	const unsigned int getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const;
	const float getFloat(const Observer *p_observer, BitMask p_changedBits) const;
	const std::string &getString(const Observer *p_observer, BitMask p_changedBits) const;
	const inline SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_selectedEntity.m_spatialDataManager.getSpatialData(p_observer, p_changedBits);
	}
	const inline SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_selectedEntity.m_spatialDataManager.getSpatialTransformData(p_observer, p_changedBits);
	}

private:
	//	System access helpers
	//
	WorldScene *getWorldScene() const;
	RendererScene *getRendererScene() const;
	entt::basic_registry<EntityID> &getEntityRegistry() const;

	// Sends a change notification with this window as the subject
	template <class T_Observer>
	void sendChangeTo(T_Observer *p_observer, const BitMask p_changeBits)
	{
		m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, p_observer, p_changeBits);
	}
	// Sends a change notification with an explicit subject (used when a scene is notified about itself)
	template <class T_Subject, class T_Observer>
	void sendChangeTo(T_Subject *p_subject, T_Observer *p_observer, const BitMask p_changeBits)
	{
		m_systemScene->getSceneLoader()->getChangeController()->sendChange(p_subject, p_observer, p_changeBits);
	}
	// Sends data to a system scene / system object, optionally transferring ownership of the data
	void sendDataTo(SystemScene *p_observer, const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false);
	void sendDataTo(SystemObject *p_observer, const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false);

	//	Frame update
	//
	void applyEditorStyle();
	void updatePickedEntitySelection();
	void clearPerFramePools();

	//	Screen regions
	//
	void drawMainMenuBar();
	void drawSecondaryMenuBar(ImGuiViewport *p_mainViewport);
	void drawLeftWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar);
	void drawRightWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar);
	void drawBottomWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar);
	void drawCenterWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar);

	//	Left window tabs
	//
	void drawHierarchyTab();
	void drawEntitiesTab();
	void drawComponentsTab();
	void drawPrefabsTab();
	void drawEntityRightClickOptionsPopup();
	void drawNewEntityPopup();

	//	Right window tabs
	//
	void drawInspectorTab();
	void drawMetadataInspector(const float inputWidgetOffset);
	void drawSpatialInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawCollisionShapeInspector();
	void drawObjectMaterialInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawCameraInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawLightInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawModelInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawModelEntryControls(const int p_modelIndex, const float inputWidgetOffset);
	void drawModelMeshControls(const int p_modelIndex, const int p_meshIndex, const float inputWidgetOffset);
	void drawModelMaterialControls(const int p_modelIndex, const int p_meshIndex, const unsigned int p_materialIndex, const float inputWidgetOffset);
	void drawShaderInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawRigidBodyInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawSoundInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawSoundListenerInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawLuaInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawLuaVariablesTable();
	void drawGuiSequenceInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton);
	void drawAddComponentPopup();

	// Draws the delete button and the collapsing header shared by every component inspector;
	// returns true when the component header is open
	bool drawComponentHeader(const Properties::PropertyID p_labelID, const char *p_deleteButtonID, const char *p_deleteTooltip,
		const ComponentType p_componentType, const Systems::TypeID p_systemType, const float p_headerOffsetAfterDeleteButton);

	void drawSceneSettingsTab();

	//	Bottom window tabs
	//
	void drawTexturesTab();
	void drawModelsTab();
	void drawShadersTab();
	void drawShaderProgramList();
	void drawShaderUniformUpdaterSettings();
	void drawShaderBoundUniforms();
	void drawShaderSettings();
	void drawShaderActions();
	void drawScriptsTab();
	void drawConsoleTab();

	//	Center window tabs
	//
	void drawSceneViewportTab();
	void drawSceneViewportGuizmo();
	void processViewportPicking();
	void drawTextureInspectorTab();
	void drawTextEditorTabs();
	void drawNewSceneSettingsTab();

	//	Scene data
	//
	void drawSceneData(SceneData &p_sceneData, const bool p_sendChanges = false);
	void drawSceneGeneralSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneAudioSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneMiscSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneAmbientOcclusionSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneAtmScatteringSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneShadowMappingSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void drawSceneRenderingPassesSettings(SceneData &p_sceneData, const float p_inputWidgetOffset, const bool p_sendChanges);
	void updateSceneData(SceneData &p_sceneData);

	//	Entity hierarchy
	//
	void drawEntityHierarchy(EntityHierarchyEntry *p_rootEntry);
	void drawEntityHierarchyEntry(EntityHierarchyEntry *p_entityEntry);
	void updateEntityList();
	void updateHierarchyList();
	void updateComponentList();
	void updateAssetLists();
	void duplicateEntity(const EntityID p_entityID);
	void deleteEntityAndChildren(const EntityID p_entityID);
	void exportPrefab(const EntityID p_entityID, std::string p_filename);
	EntityListEntry *getEntityListEntry(const EntityID p_entityID);
	void getEntityChildren(std::vector<EntityID> &p_children, const EntityListEntry &p_entityHierarchyEntry);
	EntityID findNextAvailableEntityID(const EntityID p_startAfterEntityID);
	std::string makeUniqueEntityName(const std::string &p_baseName, const int p_startIndex);

	//	File browser dialog
	//
	void openFileBrowser(const FileBrowserActivated p_fileBrowserType, const char *p_dialogName, const char *p_title, const char *p_filter, const char *p_rootPath,
		const unsigned int p_flags, std::string *p_userString = nullptr, const char *p_definedFilename = nullptr, const bool p_rootPathOnlyIfDifferent = false);
	void drawExportPrefabFileBrowser();
	void processFileBrowserResults();
	void finishFileBrowserDialog(const bool p_wasSuccessful);

	//	Utility
	//
	// Hides and locks the mouse to the screen while an item (like a float drag) is active
	// Returns the mouse to the original position after the mouse button is released
	void captureMouseWhileItemActive();
	inline void drawLeftAlignedLabelText(const char *p_labelText, const float p_nextWidgetOffset)
	{
		EditorWidgets::drawLeftAlignedLabelText(p_labelText, p_nextWidgetOffset);
	}
	inline void drawLeftAlignedLabelText(const char *p_labelText, const float p_nextWidgetOffset, const float p_nextItemWidth)
	{
		EditorWidgets::drawLeftAlignedLabelText(p_labelText, p_nextWidgetOffset, p_nextItemWidth);
	}
	inline bool drawTextSizedButton(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const std::string p_tooltipText = "")
	{
		return EditorWidgets::drawTextSizedButton(p_texture, p_buttonLabel, m_buttonSizedByFont, p_tooltipText);
	}
	inline bool drawTextSizedButtonInverted(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const std::string p_tooltipText = "")
	{
		return EditorWidgets::drawTextSizedButtonInverted(p_texture, p_buttonLabel, m_buttonSizedByFont, p_tooltipText);
	}
	// Calculates the offset for a square text-sized button from the right side of the edge
	// (p_buttonIndex is the button count from the right side)
	inline float calcTextSizedButtonOffset(const int p_buttonIndex = 0)
	{
		return EditorWidgets::calcTextSizedButtonOffset(ImGui::GetContentRegionAvail().x, m_buttonSizedByFont, m_imguiStyle.FramePadding.x, p_buttonIndex);
	}
	inline float calcTextSizedButtonSize(const unsigned int p_buttonIndex = 0)
	{
		return EditorWidgets::calcTextSizedButtonSize(m_buttonSizedByFont, m_imguiStyle.FramePadding.x, p_buttonIndex);
	}

	//	Saving / shortcuts / actions
	//
	void saveTextFile(TextEditorData &p_textEditorData);
	bool processShortcuts();
	void processMainMenuButton(MainMenuButtonType &p_mainMenuButtonType);
	void generateNewMap(PropertySet &p_newSceneProperties, SceneData &p_sceneData);

	//	Pools and flags
	//
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

	//	Texture format strings
	//
	std::string getTextureFormatString(const TextureFormat p_textureFormat) const;
	std::string getTextureDataTypeString(const TextureDataType p_textureDataType) const;
	std::string getTextureDataFormat(const TextureDataFormat p_textureDataFormat) const;

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

	// Pixel of the pending object picking request. Kept as a member (rather than a local in the
	// viewport draw function) because the request travels through the change controller, which only
	// stores the pointer and delivers it after all system tasks for the frame have completed
	glm::ivec2 m_pickingRequestPixel;

	ImGuiID m_activeItemID;

	glm::ivec2 m_mousePositionBeforeCapture;

	// GUI settings
	MaterialType m_buttonMaterialType;
	ImGuiColorEditFlags m_colorEditFlags;
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

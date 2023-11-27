#pragma once

#include "ErrorHandlerLocator.h"
#include "NullSystemObjects.h"
#include "PropertySet.h"
#include "Universal.h"

struct ComponentsConstructionInfo;
struct AudioComponentsConstructionInfo;
struct GraphicsComponentsConstructionInfo;
struct GUIComponentsConstructionInfo;
struct PhysicsComponentsConstructionInfo;
struct ScriptComponentsConstructionInfo;
struct WorldComponentsConstructionInfo;

// Loads and links various objects by requesting them from registered scenes.
// Uses property sets to send data to scenes (about specific objects).
class SceneLoader
{
public:
	SceneLoader();
	~SceneLoader();

	inline void registerSystemScene(SystemScene *p_scene)
	{
		// Make sure passed scene is valid; assign to it's place in scene array
		if(p_scene != nullptr && p_scene->getSystemType() != Systems::Null)
			m_systemScenes[p_scene->getSystemType()] = p_scene;
	}
	inline void registerChangeController(UniversalScene *p_changeCtrl)
	{
		if(p_changeCtrl != nullptr)
			m_changeController = p_changeCtrl;
	}

	inline SystemScene *getSystemScene(Systems::TypeID p_systemType) const { return m_systemScenes[p_systemType]; }
	inline UniversalScene *getChangeController() const { return m_changeController; }

	// Load a scene from properties, pass it along to every system scene and create every entity
	ErrorCode loadFromProperties(const PropertySet &p_sceneProperties);

	// Load a scene from file, pass it along to every system scene and create every entity
	ErrorCode loadFromFile(const std::string &p_filename);

	// Export the entire entity registry and system scene settings to file
	ErrorCode saveToFile(const std::string p_filename = "");

	// Load a single prefab from file
	ErrorCode importPrefab(ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename, const bool p_forceReload = false);

	// Returns all prefabs that have been loaded during scene loading
	const std::map<std::string, ComponentsConstructionInfo> &getPrefabs() { return m_prefabs; }

	// Returns the last loaded scene filename
	const std::string &getSceneFilename() const { return m_filename; }

	// Set the scene filename that is used when loading from file
	inline void setSceneFilename(const std::string &p_filename) { m_filename = p_filename; }

	// Returns the load-in-background flag
	inline bool getLoadInBackground() const { return m_loadInBackground; }

	// Set the load-in-background flag
	inline void setLoadInBackground(const bool p_loadInBackground) { m_loadInBackground = p_loadInBackground; }

private:
	ErrorCode importFromFile(ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename);
	void importFromProperties(ComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties);
	void importFromProperties(AudioComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);
	void importFromProperties(GraphicsComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);
	void importFromProperties(GUIComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);
	void importFromProperties(PhysicsComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);
	void importFromProperties(ScriptComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);
	void importFromProperties(WorldComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name);

	void exportToProperties(const ComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const AudioComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const GraphicsComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const GUIComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const PhysicsComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const ScriptComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);
	void exportToProperties(const WorldComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties);


	// Mutex used to block calls from other threads while import operation is in progress
	SpinWait m_mutex;

	// Contains all the prefabs that have already been imported before. Saves the time of importing them again, upon requesting
	std::map<std::string, ComponentsConstructionInfo> m_prefabs;

	// All of the engine's system scenes
	SystemScene *m_systemScenes[Systems::NumberOfSystems];

	// Change controller, used for storing and distributing messages between subjects and observers
	UniversalScene *m_changeController;

	// Should the imported objects be called to start loading in the background
	bool m_loadInBackground;

	// Holds the last loaded scene filename, used for exporting the scene back to file
	std::string m_filename;
};


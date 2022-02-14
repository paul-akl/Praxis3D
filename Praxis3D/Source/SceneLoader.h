#pragma once

#include "ErrorHandlerLocator.h"
#include "NullSystemObjects.h"
#include "PropertySet.h"
#include "Universal.h"

// Loads and links various objects by requesting them from registered scenes.
// Uses property sets to send data to scenes (about specific objects).
class SceneLoader
{
public:
	SceneLoader()
	{
		m_changeController = nullptr;

		for(int i = 0; i < Systems::NumberOfSystems; i++)
			m_systemScenes[i] = g_nullSystemBase.createScene(this);
	}
	~SceneLoader() { }

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

	// Returns an array of created objects, sorted by the system type
	const inline std::vector<std::pair<const std::string&, SystemObject*>> &getCreatedObjects(Systems::TypeID p_systemType)
	{
		return m_createdObjects[p_systemType];
	}
	inline SystemScene *getSystemScene(Systems::TypeID p_systemType) const { return m_systemScenes[p_systemType]; }
	inline UniversalScene *getChangeController() const { return m_changeController; }

	ErrorCode loadFromFile(const std::string &p_filename);
	ErrorCode saveToFile(const std::string p_filename = "");

private:
	SystemScene *m_systemScenes[Systems::NumberOfSystems];

	std::vector<std::pair<const std::string&, SystemObject*>> m_createdObjects[Systems::NumberOfSystems];

	UniversalScene *m_changeController;

	bool m_loadInBackground;
	std::string m_filename;
};


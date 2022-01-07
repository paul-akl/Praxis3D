#pragma once

#include "BaseScriptObject.h"
#include "ClockLocator.h"
#include "RendererScene.h"
#include "KeyCommand.h"
#include "SceneLoader.h"
#include "WindowLocator.h"

// Provides bare user interface functionality, like button presses to toggle features (for debugging)
class WorldEditScript : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	WorldEditScript(SystemScene *p_systemScene, std::string p_name, SceneLoader *p_sceneLoader)
		: BaseScriptObject(p_systemScene, p_name, Properties::WorldEditScript), m_sceneLoader(p_sceneLoader)
	{
		m_currentObjectLighting = true;
		m_selectedObject = nullptr;
		m_nextObject = true;
		m_updated = true;
		m_movementSpeed = 0.0f;
		m_fasterMovementSpeed = 0.0f;
		m_objectIndex = 0;
	}
	~WorldEditScript()
	{
		// Unbind all the keys
		m_forwardKey.unbindAll();
		m_backwardKey.unbindAll();
		m_upKey.unbindAll();
		m_downKey.unbindAll();
		m_leftKey.unbindAll();
		m_rightKey.unbindAll();
		m_nextKey.unbindAll();
		m_previousKey.unbindAll();
		m_fasterMovementKey.unbindAll();
	}

	virtual ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		// Nothing to load
	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::WorldEditScript);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Speed, m_movementSpeed);
		propertySet.addProperty(Properties::SprintSpeed, m_fasterMovementSpeed);

		// Add root key-binding property set
		auto &keyBinds = propertySet.addPropertySet(Properties::Keybindings);

		// Add individual key-bindings
		keyBinds.addProperty(Properties::ForwardKey,	(int)m_forwardKey.getFirstBinding());
		keyBinds.addProperty(Properties::BackwardKey,	(int)m_backwardKey.getFirstBinding());
		keyBinds.addProperty(Properties::UpKey,			(int)m_upKey.getFirstBinding());
		keyBinds.addProperty(Properties::DownKey,		(int)m_downKey.getFirstBinding());
		keyBinds.addProperty(Properties::LeftKey,		(int)m_leftKey.getFirstBinding());
		keyBinds.addProperty(Properties::RightKey,		(int)m_rightKey.getFirstBinding());
		keyBinds.addProperty(Properties::NextKey,		(int)m_nextKey.getFirstBinding());
		keyBinds.addProperty(Properties::PreviousKey,	(int)m_previousKey.getFirstBinding());
		keyBinds.addProperty(Properties::SprintKey,		(int)m_fasterMovementKey.getFirstBinding());

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		// Check status of next/previous keys. If they have been pressed,
		// mark the selected object to be updated and set the appropriate next/previous object flag
		if(m_nextKey.isActivated())
		{
			m_updated = false;
			m_nextObject = true;
			m_nextKey.deactivate();
		}
		if(m_previousKey.isActivated())
		{
			m_updated = false;
			m_nextObject = false;
			m_previousKey.deactivate();
		}

		if(!m_updated)
		{
			auto &objectList = m_sceneLoader->getCreatedObjects(Systems::Graphics);

			// If previously selected object is valid, send a notification to change back its lighting
			if(m_selectedObject != nullptr)
			{
				if(m_currentObjectLighting)
					m_sceneLoader->getChangeController()->sendChange(this, m_selectedObject, Systems::Changes::Graphics::Lighting);
				m_selectedObject = nullptr;
			}

			// Go over the object list until the whole list has been iterated over
			for(decltype(objectList.size()) i = 0, size = objectList.size(); i < size; i++)
			{
				// Determine if the next or previous object should be selected, 
				// and make sure to clamp the index to a valid range
				if(m_nextObject)
				{
					m_objectIndex++;
					if(m_objectIndex > size - 1)
						m_objectIndex = 0;
				}
				else
				{
					if(m_objectIndex == 0)
						m_objectIndex = size;
					m_objectIndex--;
				}

				// Check if object is valid
				if(objectList[m_objectIndex].second->getObjectType() != Properties::Null)
				{
					// Static cast it to the appropriate type
					m_selectedObject = objectList[m_objectIndex].second;

					// Get the current data from the selected object
					m_sceneLoader->getChangeController()->sendChange(m_selectedObject, this, 
																	 Systems::Changes::Spatial::LocalPosition |
																	 Systems::Changes::Generic::Name |
																	 Systems::Changes::Graphics::Lighting);

					// Set objects lighting to highlight it as the selected one
					m_sceneLoader->getChangeController()->sendChange(this, m_selectedObject, Systems::Changes::Graphics::Lighting);

					break;
				}
			}

			m_updated = true;
		}
		else
		{
			// Set movement speed
			float speed = m_movementSpeed;

			// Increase movement speed if the sprint key is pressed
			if(m_fasterMovementKey.isActivated())
				speed = m_fasterMovementSpeed;

			// Check the status of all the movement keys
			if(m_forwardKey.isActivated())
				m_objectPosition.x += speed * p_deltaTime;
			if(m_backwardKey.isActivated())
				m_objectPosition.x -= speed * p_deltaTime;
			if(m_upKey.isActivated())
				m_objectPosition.y += speed * p_deltaTime;
			if(m_downKey.isActivated())
				m_objectPosition.y -= speed * p_deltaTime;
			if(m_leftKey.isActivated())
				m_objectPosition.z += speed * p_deltaTime;
			if(m_rightKey.isActivated())
				m_objectPosition.z -= speed * p_deltaTime;

			// Check status of modifier key, for key combinations
			if(m_modifierKey.isActivated())
			{
				// If save key is active, spawn a message box asking if the scene should be exported
				if(m_saveKey.isActivated())
				{
					// Deactivate key here manually, to not cause re-triggering for multiple frames
					m_modifierKey.deactivate();
					m_saveKey.deactivate();

					if(WindowLocator::get().spawnYesNoErrorBox("World Editor", "Export the scene to \"" + Config::gameplayVar().default_map + "\"?"))
						m_sceneLoader->saveToFile(Config::gameplayVar().default_map);
				}
			}

			// If an object is selected, update its data
			//if(m_selectedObject != nullptr)
			//	m_sceneLoader->getChangeController()->sendChange(this, m_selectedObject, Systems::Changes::Spacial::Position);
		}
	}

	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::All; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		//if(p_changeType & Systems::Changes::Spacial::Position)
		//	m_objectPosition = p_subject->getVec3(this, Systems::Changes::Spacial::Position);

		if(p_changeType & Systems::Changes::Generic::Name)
			m_objectName = p_subject->getString(this, Systems::Changes::Generic::Name);

		if(p_changeType & Systems::Changes::Graphics::Lighting)
		{
			m_currentObjectLighting = p_subject->getBool(this, Systems::Changes::Graphics::Lighting);
		}

		printf("Selected object: %s\n", m_objectName.c_str());
	}

	const virtual glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		//switch(p_changedBits)
		//{
		//case Systems::Changes::Spacial::Position:
		//	return m_objectPosition;
		//	break;
		//}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Graphics::Lighting:
			// If the currently selected object is requesting the data, we want to highlight it, so return false
			if(p_observer == m_selectedObject)
				return false;
			else
				return true;
			break;
		}

		return ObservedSubject::getBool(p_observer, p_changedBits);
	}

	// Setters
	const inline void setSpeed(float p_speed)		{ m_movementSpeed = p_speed;		}
	const inline void setFasterSpeed(float p_speed) { m_fasterMovementSpeed = p_speed;	}

	// Setters for the key binds:
	inline void setForwardKey(Scancode p_key)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_key);
	}
	inline void setForwardKey(std::string &p_string)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_string);
	}

	inline void setBackwardKey(Scancode p_key)
	{
		m_backwardKey.unbindAll();
		m_backwardKey.bind(p_key);
	}
	inline void setBackwardKey(std::string &p_string)
	{
		m_backwardKey.unbindAll();
		m_backwardKey.bind(p_string);
	}

	inline void setUpKey(Scancode p_key)
	{
		m_upKey.unbindAll();
		m_upKey.bind(p_key);
	}
	inline void setUpKey(std::string &p_string)
	{
		m_upKey.unbindAll();
		m_upKey.bind(p_string);
	}

	inline void setDownKey(Scancode p_key)
	{
		m_downKey.unbindAll();
		m_downKey.bind(p_key);
	}
	inline void setDownKey(std::string &p_string)
	{
		m_downKey.unbindAll();
		m_downKey.bind(p_string);
	}

	inline void setLeftKey(Scancode p_key)
	{
		m_leftKey.unbindAll();
		m_leftKey.bind(p_key);
	}
	inline void setLeftKey(std::string &p_string)
	{
		m_leftKey.unbindAll();
		m_leftKey.bind(p_string);
	}

	inline void setRightKey(Scancode p_key)
	{
		m_rightKey.unbindAll();
		m_rightKey.bind(p_key);
	}
	inline void setRightKey(std::string &p_string)
	{
		m_rightKey.unbindAll();
		m_rightKey.bind(p_string);
	}

	inline void setNextKey(Scancode p_key)
	{
		m_nextKey.unbindAll();
		m_nextKey.bind(p_key);
	}
	inline void setNextKey(std::string &p_string)
	{
		m_nextKey.unbindAll();
		m_nextKey.bind(p_string);
	}

	inline void setPreviousKey(Scancode p_key)
	{
		m_previousKey.unbindAll();
		m_previousKey.bind(p_key);
	}
	inline void setPreviousKey(std::string &p_string)
	{
		m_previousKey.unbindAll();
		m_previousKey.bind(p_string);
	}

	inline void setFasterMovementKey(Scancode p_key)
	{
		m_fasterMovementKey.unbindAll();
		m_fasterMovementKey.bind(p_key);
	}
	inline void setFasterMovementKey(std::string &p_string)
	{
		m_fasterMovementKey.unbindAll();
		m_fasterMovementKey.bind(p_string);
	}

	inline void setCenterKey(Scancode p_key)
	{
		m_centerKey.unbindAll();
		m_centerKey.bind(p_key);
	}
	inline void setCenterKey(std::string &p_string)
	{
		m_centerKey.unbindAll();
		m_centerKey.bind(p_string);
	}

	inline void setSaveKey(Scancode p_key)
	{
		m_saveKey.unbindAll();
		m_saveKey.bind(p_key);
	}
	inline void setSaveKey(std::string &p_string)
	{
		m_saveKey.unbindAll();
		m_saveKey.bind(p_string);
	}

	inline void setModifierKey(Scancode p_key)
	{
		m_modifierKey.unbindAll();
		m_modifierKey.bind(p_key);
	}
	inline void setModifierKey(std::string &p_string)
	{
		m_modifierKey.unbindAll();
		m_modifierKey.bind(p_string);
	}

private:
	KeyCommand  m_forwardKey,
				m_backwardKey,
				m_upKey,
				m_downKey,
				m_leftKey,
				m_rightKey,
				m_nextKey,
				m_previousKey,
				m_fasterMovementKey,
				m_centerKey,
				m_saveKey,
				m_modifierKey;

	SceneLoader *m_sceneLoader;
	SystemObject *m_selectedObject;

	std::string m_objectName;
	glm::vec3 m_objectPosition;

	size_t m_objectIndex;
	float m_movementSpeed;
	float m_fasterMovementSpeed;

	// Used to check if the data has been retrieved from selected object
	bool m_updated;
	// Used to determine if a next or previous object should be selected
	bool m_nextObject;
	// Hold the lighting flag of selected object so we could set it back to how it was
	bool m_currentObjectLighting;
};
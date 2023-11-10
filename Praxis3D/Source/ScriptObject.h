#pragma once

#include "InheritanceObjects.h"
#include "LuaComponent.h"
#include "System.h"

enum ScriptComponentType : std::size_t
{
	ScriptComponentType_LUA = 0,
	ScriptComponentType_NumOfComponents
};

class ScriptObject : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
public:
	ScriptObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::ScriptObject)
	{
		m_luaComponent = nullptr;
		m_componentsFlag = 0;
	}
	~ScriptObject()
	{
		// Iterate over all component types and delete components if they have been created
		for(std::size_t i = 0; i < ScriptComponentType::ScriptComponentType_NumOfComponents; i++)
			removeComponent(static_cast<ScriptComponentType>(i));
	}

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory()
	{
		if(luaComponentPresent())
			m_luaComponent->loadToMemory();

		setActive(true);
		setLoadedToMemory(true);
		setLoadedToVideoMemory(true);
	}

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode returnError = ErrorCode::Success;

		// Check if the property set is valid and the script object hasn't been loaded already
		if(p_properties)
		{
			if(!isLoadedToMemory())
			{
				// Check if there is a property set for lua component and load the lua component if there is
				auto const &luaComponentProperty = p_properties.getPropertySetByID(Properties::Lua);
				if(luaComponentProperty)
				{
					// Create the lua component
					addComponent(new LuaComponent(m_systemScene, m_name + Config::componentVar().lua_component_name));

					// Try to initialize the lua component
					auto componentInitError = m_luaComponent->init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = m_luaComponent->importObject(luaComponentProperty);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							removeComponent(ScriptComponentType::ScriptComponentType_LUA);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_CameraComponent, m_name);
						}
					}
					else // Remove the component if it failed to initialize
					{
						removeComponent(ScriptComponentType::ScriptComponentType_LUA);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_CameraComponent, m_name);
					}
				}
			}
		}
		else
		{
			returnError = ErrorCode::Failure;
		}

		return returnError;
	}

	PropertySet exportObject()
	{
		// If there are components present, export each one; if there are no components, return an empty propertySet
		//if(containsComponents())
		//{
		//	PropertySet exportPropertySet(Properties::Rendering);

		//	//if(cameraComponentPresent())
		//	//	exportPropertySet.addPropertySet(m_cameraComponent->exportObject());
		//	//if(lightComponentPresent())
		//	//	exportPropertySet.addPropertySet(m_lightComponent->exportObject());
		//	//if(modelComponentPresent())
		//	//	exportPropertySet.addPropertySet(m_modelComponent->exportObject());
		//	//if(shaderComponentPresent())
		//	//	exportPropertySet.addPropertySet(m_shaderComponent->exportObject());

		//	return exportPropertySet;
		//}
		//else
			return PropertySet();
	}

	void update(const float p_deltaTime)
	{
		if(!isLoadedToVideoMemory())
		{
			performCheckIsLoadedToVideoMemory();

			if(!isLoadedToVideoMemory())
				return;
		}

		if(luaComponentPresent())
			m_luaComponent->update(p_deltaTime);

		if(hasSpatialDataUpdated())
		{

		}

		if(isUpdateNeeded())
		{


			// Calculate model matrix
			//m_worldSpace.m_transformMatNoScale = Math::createTransformMat(m_worldSpace.m_spatialData.m_position, m_worldSpace.m_spatialData.m_rotationEuler, m_worldSpace.m_spatialData.m_scale);

			// Update components
			//if(modelComponentPresent())
			//	m_modelComponent->update(p_deltaTime);
			//if(shaderComponentPresent())
			//	m_shaderComponent->update(p_deltaTime);
			//if(lightComponentPresent())
			//	m_lightComponent->update(p_deltaTime);

			// Mark as updated
			updatePerformed();
		}
	}

	// Assign a pointer to a const SpatialDataChangeManager, so the object can use it for its spatial data
	// Also assigns the pointer to every component that needs it
	void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData)
	{
		SpatialDataManagerObject::setSpatialDataManagerReference(p_spatialData);

		if(luaComponentPresent())
			m_luaComponent->setSpatialDataManagerReference(*m_spatialData);
	}

	// System type is Graphics
	BitMask getSystemType() { return Systems::Script; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Graphics::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Graphics::All; }

	inline LuaComponent *getLuaComponent() { return m_luaComponent; }

	inline const bool luaComponentPresent()	const { return (m_luaComponent == nullptr) ? false : true; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			postChanges(newChanges);
		}
	}

	void addComponent(LuaComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(ScriptComponentType::ScriptComponentType_LUA);
		m_luaComponent = p_component;

		// Share the ScriptObjects spatial data with the component
		m_luaComponent->setSpatialDataManagerReference(*m_spatialData);

		// Set the flag for the lua component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::ScriptObjectComponents::Lua;
	}
	void removeComponent(const ScriptComponentType p_compType)
	{
		switch(p_compType)
		{
		case ScriptComponentType::ScriptComponentType_LUA:
		{
			if(m_luaComponent != nullptr)
			{
				// Delete the actual component
				delete m_luaComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_luaComponent = nullptr;

				// Remove the bit corresponding to lua component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::ScriptObjectComponents::Lua;
			}
			break;
		}
		}
	}
	
	// Returns true if the graphics object contains any components
	inline const bool containsComponents()
	{
		if(luaComponentPresent())
			return true;

		return false;
	}

private:
	// Components
	LuaComponent *m_luaComponent;

	// Stores a separate flag for each component currently present
	BitMask m_componentsFlag;
};


#pragma once

#include "InheritanceObjects.h"
#include "GUIDataManager.h"
#include "GUISequenceComponent.h"
#include "System.h"

enum GUIComponentType : std::size_t
{
	GUIComponentType_Sequence = 0,
	GUIComponentType_NumOfComponents
};

class GUIObject : public SystemObject, public LoadableGraphicsObject
{
public:
	GUIObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::GUIObject), m_GUIData(*this)
	{
		m_GUISequenceComponent = nullptr;
		m_componentsFlag = 0;
	}
	~GUIObject()
	{
		// Iterate over all component types and delete components if they have been created
		//for(std::size_t i = 0; i < ScriptComponentType::ScriptComponentType_NumOfComponents; i++)
		//	removeComponent(static_cast<ScriptComponentType>(i));
	}

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory()
	{

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
				// Check if there is a property set for sequence and load the GUI sequence component if there is
				//auto const &sequenceComponentProperty = p_properties.getPropertySetByID(Properties::Sequence);
				//if(sequenceComponentProperty)
				//{
				//	// Create the GUI sequence component
				//	addComponent(new GUISequenceComponent(m_systemScene, m_name + Config::componentVar().lua_component_name));

				//	// Try to initialize the lua component
				//	auto componentInitError = m_GUISequenceComponent->init();
				//	if(componentInitError == ErrorCode::Success)
				//	{
				//		// Try to import the component
				//		auto const &componentImportError = m_GUISequenceComponent->importObject(sequenceComponentProperty);

				//		// Remove the component if it failed to import
				//		if(componentImportError != ErrorCode::Success)
				//		{
				//			removeComponent(GUIComponentType::GUIComponentType_Sequence);
				//			ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_GUISequenceComponent, m_name);
				//		}
				//	}
				//	else // Remove the component if it failed to initialize
				//	{
				//		removeComponent(GUIComponentType::GUIComponentType_Sequence);
				//		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_GUISequenceComponent, m_name);
				//	}
				//}
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
		return PropertySet();
	}

	void update(const float p_deltaTime)
	{
		if(GUISequenceComponentPresent())
			m_GUISequenceComponent->update(p_deltaTime);
	}

	// System type is Graphics
	BitMask getSystemType() { return Systems::Script; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Graphics::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Graphics::All; }

	inline GUISequenceComponent *getGUISequenceComponent() { return m_GUISequenceComponent; }

	inline const bool GUISequenceComponentPresent()	const { return (m_GUISequenceComponent == nullptr) ? false : true; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// If the change type is GUI, send the change to the GUI Data Manager
		if(CheckBitmask(p_changeType, Systems::Changes::Type::GUI))
		{
			m_GUIData.changeOccurred(p_subject, p_changeType);

			// If GUI Sequence has been changed, and GUISequenceComponent is present, 
			// notify the component about the change separately, as it has to make a copy of the sequence for later use
			// (the Functors sequence in GUIDataManager only stores a pointer to it and not a copy)
			if(CheckBitmask(p_changeType, Systems::Changes::GUI::Sequence))
				if(GUISequenceComponentPresent())
					m_GUISequenceComponent->changeOccurred(p_subject, Systems::Changes::GUI::Sequence);
		}

		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			postChanges(newChanges);
		}
	}

	void addComponent(GUISequenceComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GUIComponentType::GUIComponentType_Sequence);
		m_GUISequenceComponent = p_component;

		// Share the GUIObjects GUI data with the component
		//p_component->setGUIDataManagerReference(m_GUIData);

		// Set the flag for the GUI sequence component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GUIObjectComponents::Sequence;
	}
	void removeComponent(const GUIComponentType p_compType)
	{
		switch(p_compType)
		{
		case GUIComponentType::GUIComponentType_Sequence:
		{
			if(m_GUISequenceComponent != nullptr)
			{
				// Delete the actual component
				delete m_GUISequenceComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_GUISequenceComponent = nullptr;

				// Remove the bit corresponding to lua component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::GUIObjectComponents::Sequence;
			}
			break;
		}
		}
	}

	// Returns true if the graphics object contains any components
	inline const bool containsComponents()
	{
		if(GUISequenceComponentPresent())
			return true;

		return false;
	}

private:
	// Components
	GUISequenceComponent *m_GUISequenceComponent;

	// GUI data
	GUIDataManager m_GUIData;

	// Stores a separate flag for each component currently present
	BitMask m_componentsFlag;
};


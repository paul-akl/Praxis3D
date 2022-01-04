#pragma once

#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class CameraComponent : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	CameraComponent(SystemScene *p_systemScene, std::string p_name, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::CameraComponent)
	{

	}
	~CameraComponent() { }

	ErrorCode init() final override
	{
		// Mark the object as loaded, because there is nothing to be specifically loaded, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	void update(const float p_deltaTime)
	{

	}

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			// Get the camera properties
			//auto const &cameraProperty = p_properties.getPropertySetByID(Properties::Camera);

			//if(cameraProperty)
			//{
				//importError = ErrorCode::Success;
			//}
			if(p_properties.getPropertyID() == Properties::Camera)
			{
				importError = ErrorCode::Success;
				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_CameraComponent, m_name + " - Camera loaded");
			}

			if(importError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(true);
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

	// System type is Graphics
	BitMask getSystemType() final override { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Graphics::Camera; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

private:

};
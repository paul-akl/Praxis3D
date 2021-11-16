#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"

class CameraComponent : public BaseGraphicsComponent
{
public:
	CameraComponent() { }
	~CameraComponent() { }

	void load(PropertySet &p_properties) final override
	{ 

	}

	PropertySet exportObject() final override
	{ 
		return PropertySet(); 
	}

private:

};
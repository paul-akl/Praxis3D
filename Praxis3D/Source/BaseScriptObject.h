#pragma once

#include "System.h"

class BaseScriptObject : public SystemObject
{
public:
	BaseScriptObject(SystemScene *p_systemScene, std::string p_name, Properties::PropertyID p_objectType)
		: SystemObject(p_systemScene, p_name, p_objectType)
	{

	}
	
	BitMask getSystemType() final { return Systems::Scripting; }

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::None; }

	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::All; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

protected:
};
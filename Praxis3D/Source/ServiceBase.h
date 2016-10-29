#pragma once

#include "Config.h"

class SystemObject;

class ServiceBase
{
public:
	//ServiceBase();
	//~ServiceBase();

	class SystemAccess
	{
	public:
		virtual void *getSystem(Systems::TypeID p_type) = 0;

		virtual void *getScene(Systems::TypeID p_type) = 0;

		virtual void *getSystemObject(Systems::TypeID p_systemType, BitMask p_objectType) = 0;

		// TODO GET SET PROPERTIES

	};

	virtual SystemAccess &getSystemAccess() = 0;

	virtual void registerSystemAccessProvider(SystemAccess *p_systemAccess) = 0;

	virtual void unregisterSystemAccessProvider(SystemAccess *p_systemAccess) = 0;

};


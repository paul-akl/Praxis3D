#include "ObjectDirectory.h"
#include "System.h"

SystemObject::SystemObject() : m_initialized(false), m_active(false), m_updateNeeded(false), m_systemScene(nullptr), m_objectType(Properties::Null), m_objectID(0)
{
	setName("Null Object");
	m_objectID = ObjectDirectory::registerObject(*this);
}

SystemObject::SystemObject(SystemScene * p_systemScene, const std::string & p_name, Properties::PropertyID p_objectType) : m_initialized(false), m_active(false), m_updateNeeded(false), m_systemScene(p_systemScene), m_objectType(p_objectType), m_objectID(0)
{
	setName(p_name);
	m_objectID = ObjectDirectory::registerObject(*this);
}

SystemObject::~SystemObject()
{
	ObjectDirectory::unregisterObject(*this);
}

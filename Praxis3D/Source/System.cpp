#include "ObjectDirectory.h"
#include "System.h"

SystemObject::SystemObject() : m_initialized(false), m_systemScene(nullptr), m_objectType(Properties::Null), m_ID(0)
{
	setName("Null Object");
	m_ID = ObjectDirectory::registerObject(this);
}

SystemObject::SystemObject(SystemScene * p_systemScene, const std::string & p_name, Properties::PropertyID p_objectType) : m_initialized(false), m_systemScene(p_systemScene), m_objectType(p_objectType), m_ID(0)
{
	setName(p_name);
	m_ID = ObjectDirectory::registerObject(this);
}

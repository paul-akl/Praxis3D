#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "ObjectDirectory.h"
#include "System.h"

SystemObject::SystemObject() : Observer(Properties::Null), m_initialized(false), m_active(false), m_updateNeeded(false), m_systemScene(nullptr), m_objectID(0)
{
	setName("Null Object");
	m_entityID = NULL_ENTITY_ID;
	//m_objectID = ObjectDirectory::registerObject(*this);
}

SystemObject::SystemObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType, EntityID p_entityID) : Observer(p_objectType), m_initialized(false), m_active(false), m_updateNeeded(false), m_systemScene(p_systemScene), m_objectID(0), m_entityID(p_entityID)
{
	setName(p_name);
	m_objectID = ObjectDirectory::registerObject(*this);
}

SystemObject::~SystemObject()
{
	ObjectDirectory::unregisterObject(*this);
}

std::vector<SystemObject*> SystemScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return std::vector<SystemObject*>();
}

SystemObject *SystemScene::getNullObject()
{
	return g_nullSystemBase.getScene()->getNullObject();
}

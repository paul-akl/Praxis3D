
#include "Systems/RendererSystem/Components/Include/BaseGraphicsObjects.hpp"
#include "Systems/RendererSystem/Components/Include/EnvironmentMapObjects.hpp"
#include "Systems/RendererSystem/Components/Include/ModelGraphicsObjects.hpp"
#include "Systems/RendererSystem/Components/Include/ShaderGraphicsObjects.hpp"

LoadableGraphicsObjectOLD::LoadableGraphicsObjectOLD(ModelObject *p_modelObject, size_t p_index) :
	m_objectType(LoadableObjectType::LoadableObj_ModelObj), m_index(p_index), m_activateAfterLoading(true), m_baseGraphicsObject(p_modelObject)
{
	m_objectData.m_modelObject = p_modelObject;
	m_name = p_modelObject->getName();
	m_baseSystemObject = p_modelObject;
	m_objectID = p_modelObject->getObjectID();
}

LoadableGraphicsObjectOLD::LoadableGraphicsObjectOLD(EnvironmentMapObject *p_envMapStatic, size_t p_index) :
	m_objectType(LoadableObjectType::LoadableObj_StaticEnvMap), m_index(p_index), m_activateAfterLoading(true), m_baseGraphicsObject(p_envMapStatic)
{
	m_objectData.m_envMapStatic = p_envMapStatic;
	m_name = p_envMapStatic->getName();
	m_baseSystemObject = p_envMapStatic;
	m_objectID = p_envMapStatic->getObjectID();
}

LoadableGraphicsObjectOLD::LoadableGraphicsObjectOLD(ShaderModelGraphicsObject *p_shaderModelObject, size_t p_index) :
	m_objectType(LoadableObjectType::LoadableObj_ShaderObj), m_index(p_index), m_activateAfterLoading(true), m_baseGraphicsObject(p_shaderModelObject)
{
	m_objectData.m_shaderModelObject = p_shaderModelObject;
	m_name = p_shaderModelObject->getName();
	m_baseSystemObject = p_shaderModelObject;
	m_objectID = p_shaderModelObject->getObjectID();
}

void LoadableGraphicsObjectOLD::LoadToMemory()
{
	switch(m_objectType)
	{
	case LoadableObj_ModelObj:
		m_objectData.m_modelObject->loadToMemory();
		break;
	case LoadableObj_ShaderObj:
		m_objectData.m_shaderModelObject->loadToMemory();
		break;
	case LoadableObj_StaticEnvMap:
		m_objectData.m_envMapStatic->loadToMemory();
		break;
	}
}
/*
inline bool LoadableGraphicsObject::isLoadedToMemory() const
{
	bool returnValue = false;

	switch(m_objectType)
	{
	case LoadableObj_ModelObj:
		returnValue = m_objectData.m_modelObject->isLoadedToMemory();
		break;
	case LoadableObj_ShaderObj:
		returnValue = m_objectData.m_shaderModelObject->isLoadedToMemory();
		break;
	case LoadableObj_StaticEnvMap:
		returnValue = m_objectData.m_envMapStatic->isLoadedToMemory();
		break;
	}

	return returnValue;
}*/
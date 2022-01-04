#include "GUIScene.h"

GUIScene::GUIScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_GUITask = new GUITask(this);
}

ErrorCode GUIScene::init()
{

	return ErrorCode::Success;
}

ErrorCode GUIScene::setup(const PropertySet& p_properties)
{
	return ErrorCode();
}

void GUIScene::update(const float p_deltaTime)
{
}

SystemObject* GUIScene::createObject(const PropertySet& p_properties)
{
	return nullptr;
}

ErrorCode GUIScene::destroyObject(SystemObject* p_systemObject)
{
	return ErrorCode();
}

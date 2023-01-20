#include "NullSystemObjects.h"

NullSystemScene NullSystemBase::m_nullSystemScene(nullptr);
NullSystemTask NullSystemScene::m_nullSystemTask(nullptr);
NullSystemObject NullSystemScene::m_nullSystemObject;

SystemScene *NullSystemBase::createScene(SceneLoader *p_sceneLoader)
{
	return &m_nullSystemScene;
}

SystemScene *NullSystemBase::getScene()
{
	return &m_nullSystemScene;
}

NullSystemScene::NullSystemScene(SystemBase *p_system) : SystemScene(p_system, nullptr)
{
	m_nullSystemTask.setSystemScene(this);
}

SystemObject *NullSystemScene::createObject(const PropertySet &p_properties)
{
	return &m_nullSystemObject;
}

SystemObject *NullSystemScene::getNullObject()
{
	return &m_nullSystemObject;
}

SystemTask *NullSystemScene::getSystemTask()
{
	return &m_nullSystemTask;
}

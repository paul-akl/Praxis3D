#pragma once

#include <functional>

#include "BaseGraphicsObjects.h"
#include "Loaders.h"
#include "ModelGraphicsObjects.h"

class RendererScene;

// Obsolete (ModelObjects handle custom shaders)
class ShaderModelGraphicsObject : public ModelObject
{
	friend class RendererScene;
public:
	ShaderModelGraphicsObject(SystemScene *p_systemScene, const std::string &p_name, 
							  ModelLoader::ModelHandle &p_model, ShaderLoader::ShaderProgram *p_shader)
		: ModelObject(p_systemScene, p_name, p_model, p_shader, Properties::ShaderModelObject)
	{
		// Not a post process effect by default
		//m_postProcess = false;

	}

	~ShaderModelGraphicsObject() { }

	void update(const float p_deltaTime)
	{
		if(m_needsUpdate)
		{
			// Update model matrix
			m_baseObjectData.m_modelMat.identity();
			m_baseObjectData.m_modelMat.translate(m_baseObjectData.m_position);
			m_baseObjectData.m_modelMat.rotate(m_baseObjectData.m_rotation);
			m_baseObjectData.m_modelMat.scale(m_baseObjectData.m_scale);

			// Mark as updated
			m_needsUpdate = false;
		}
	}

	virtual ErrorCode loadToVideoMemory()
	{
		ErrorCode error;

		// Load shader, check if it loaded successfully
		//if(!ErrHandlerLoc::get().ifSuccessful(m_shader->loadToVideoMemory(), error))
		//{
			// Log an error on failure
			//ErrHandlerLoc::get().log(error);

			// Replace the shader with a default one
			m_shader = Loaders::shader().load();
		//}

		// Load all other data by calling parent class; log an error on failure
		if(!ErrHandlerLoc::get().ifSuccessful(ModelObject::loadToVideoMemory(), error))
			ErrHandlerLoc::get().log(error);

		return ErrorCode::Success;
	}
	
	const ShaderLoader::ShaderProgram &getShader() const { return *m_shader; }

protected:
	
	ShaderLoader::ShaderProgram *m_shader;
};
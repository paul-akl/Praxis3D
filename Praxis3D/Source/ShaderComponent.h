#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class ShaderComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	ShaderComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::Shaders, p_entityID), m_shaderData(nullptr) { }
	ShaderComponent(SystemScene *p_systemScene, std::string p_name, ShaderLoader::ShaderProgram &p_shader, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::Shaders, p_entityID), m_shaderData(new ShaderData(p_shader)) { }
	~ShaderComponent() 
	{ 
		delete m_shaderData;
	}

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory() { }

	BitMask getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime) { }

	BitMask getDesiredSystemChanges() { return Systems::Changes::None; }

	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	ErrorCode importObject(const PropertySet &p_properties)
	{ 
		ErrorCode importError = ErrorCode::Failure;

		// Check if shaders node is valid and the component hasn't been loaded already
		if(!isLoadedToMemory())
		{
			if(p_properties)
			{
				// Get nodes for different shader types
				auto fragmentShaderNode = p_properties.getPropertyByID(Properties::FragmentShader);
				auto vertexShaderNode = p_properties.getPropertyByID(Properties::VertexShader);

				// Check if any of the shader nodes are present
				if(fragmentShaderNode || vertexShaderNode)
				{
					// Load shader program
					auto shaderProgram = Loaders::shader().load(p_properties);

					// If shader is not default (i.e. at least one of the shader types was loaded)
					if(!shaderProgram->isDefaultProgram())
					{
						// Set the component as not being empty anymore, since at least one of the shaders has been loaded successfully
						//setEmpty(false);

						// Load the shader to memory and assign it to the new shader component
						shaderProgram->loadToMemory();
						m_shaderData = new ShaderData(*shaderProgram);

						ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ShaderComponent, m_name + " - \'" + fragmentShaderNode.getString() + "\' and \'" + vertexShaderNode.getString() + "\' shaders imported.");
					}

					// Set the component as loaded, because the load function was called
					setLoadedToMemory(true);

					importError = ErrorCode::Success;
				}
				else
					ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_ShaderComponent, m_name + " - missing shader properties.");
			}
			else
				ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_ShaderComponent, m_name + " - missing shader properties.");
		}

		return importError;
	}

	PropertySet exportObject()
	{ 
		return PropertySet(); 
	}

	std::vector<LoadableObjectsContainer> getLoadableObjects()
	{
		std::vector<LoadableObjectsContainer> loadableObjects;

		// If shader data is present, add it to the loadable objects
		if(m_shaderData != nullptr)
			loadableObjects.push_back(LoadableObjectsContainer(&m_shaderData->m_shader));

		return loadableObjects;
	}

	void performCheckIsLoadedToVideoMemory()
	{
		if(m_shaderData != nullptr)
		{
			if(m_shaderData->m_shader.isLoadedToVideoMemory())
				return;
			else
				setLoadedToVideoMemory(true);
		}
	}

	const inline ShaderData *getShaderData() const { return m_shaderData; }

private:
	ShaderData *m_shaderData;
};
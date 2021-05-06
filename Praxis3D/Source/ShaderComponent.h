#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"

class ShaderComponent : public BaseGraphicsComponent
{
public:
	ShaderComponent() { }
	~ShaderComponent() 
	{ 
		delete m_shaderData;
	}

	void load(PropertySet &p_properties) final override
	{ 
		// Check if shaders node is valid and the component hasn't been loaded already
		if(p_properties && empty())
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
					setEmpty(false);

					// Load the shader to memory and assign it to the new shader component
					shaderProgram->loadToMemory();
					m_shaderData = new ShaderData(*shaderProgram);
				}
			}
		}

		// Set the component as loaded, because the load function was called
		setLoaded(true);
	}

	PropertySet export() final override
	{ 
		return PropertySet(); 
	}

private:
	ShaderData *m_shaderData;
};
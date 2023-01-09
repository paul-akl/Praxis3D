
#include "Config.h"
#include "ErrorHandlerLocator.h"
#include "ShaderLoader.h"
#include "ShaderUniformUpdater.h"
#include "Utilities.h"

unsigned int ShaderLoader::ShaderProgram::m_defaultProgramHandle = 0;

ShaderLoader::ShaderLoader()
{
	// Assign shader types, so they could be resolved from array type
	//m_shaderTypes[ArrayFragment] = ShaderFragment;
	//m_shaderTypes[ArrayGeometry] = ShaderGeometry;
	//m_shaderTypes[ArrayVertex] = ShaderVertex;
	//m_shaderTypes[ArrayTessControl] = ShaderTessControl;
	//m_shaderTypes[ArrayTessEvaluation] = ShaderTessEvaluation;
}
ShaderLoader::~ShaderLoader()
{
	// Delete shaders
	for(decltype(m_shaderPrograms.size()) i = 0, size = m_shaderPrograms.size(); i < size; i++)
		delete m_shaderPrograms[i];

	m_shaderPrograms.clear();
}

ErrorCode ShaderLoader::init()
{
	// Initialize default shader program
	m_defaultProgram.loadToMemory();
	m_defaultProgram.m_uniformUpdater = new ShaderUniformUpdater(m_defaultProgram);
	m_defaultProgram.m_loadedToVideoMemory = true;

	// Reserve space in the program pool, to speed up push_backs
	m_shaderPrograms.reserve(Config::rendererVar().shader_pool_size);

	ShaderProgram::m_defaultProgramHandle = m_defaultProgram.m_programHandle;

	return ErrorCode::Success;
}

ShaderLoader::ShaderProgram *ShaderLoader::load(const PropertySet &p_properties)
{
	if(p_properties)
	{
		// Create shader filename array
		std::string shaderFilename[ShaderType_NumOfTypes];
		std::string programName;

		int numberOfShaders = 0;

		// Iterate over all passed properties
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Name:
				programName = p_properties[i].getString();
				break;

			case Properties::ComputeShader:
				shaderFilename[ShaderType_Compute] = p_properties[i].getString();
				numberOfShaders++;
				break;

			case Properties::FragmentShader:
				shaderFilename[ShaderType_Fragment] = p_properties[i].getString();
				numberOfShaders++;
				break;

			case Properties::VertexShader:
				shaderFilename[ShaderType_Vertex] = p_properties[i].getString();
				numberOfShaders++;
				break;

			case Properties::GeometryShader:
				shaderFilename[ShaderType_Geometry] = p_properties[i].getString();
				numberOfShaders++;
				break;

			case Properties::TessControlShader:
				shaderFilename[ShaderType_TessControl] = p_properties[i].getString();
				numberOfShaders++;
				break;

			case Properties::TessEvaluationShader:
				shaderFilename[ShaderType_TessEvaluation] = p_properties[i].getString();
				numberOfShaders++;
				break;
			}
		}

		if(numberOfShaders > 0)
		{
			// If program name was not specified, combine all shader names into one program name
			if(programName.empty())
			{
				// For every shader filename, if it's not empty, add it to the program name
				for(unsigned int shaderType = 0; shaderType < ShaderType_NumOfTypes; shaderType++)
					if(!shaderFilename[shaderType].empty())
						programName += shaderFilename[shaderType] + ", ";

				// Remove the last 2 characters from the filename (comma and space)
				if(!programName.empty())
				{
					programName.pop_back();
					programName.pop_back();
				}
			}

			// Generate hash key from program's name
			unsigned int programHashkey = Utilities::getHashKey(programName);

			// Make sure calls from other threads are locked, while current call is in progress
			// This is needed to as the object that is being requested might be currently loading /
			// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
			SpinWait::Lock lock(m_mutex);

			// Iterate over all shader programs and match hash key and name; if match is found, return it
			for(decltype(m_shaderPrograms.size()) i = 0, size = m_shaderPrograms.size(); i < size; i++)
				if(m_shaderPrograms[i]->m_filenameHash == programHashkey)
					if(m_shaderPrograms[i]->m_combinedFilename == programName)
						return m_shaderPrograms[i];

			// Add the new program to the array
			ShaderProgram *newProgram = new ShaderProgram(programName, programHashkey);
			m_shaderPrograms.push_back(newProgram);

			// Iterate over shader types
			for(unsigned int shaderType = 0; shaderType < ShaderType_NumOfTypes; shaderType++)
				// If shader filename is valid
				if(!shaderFilename[shaderType].empty())
					newProgram->addShader(static_cast<ShaderType>(shaderType), shaderFilename[shaderType]);

			// If there is a compute shader source code loaded, mark the shader program as compute only
			if(!newProgram->m_shaderSource[ShaderType::ShaderType_Compute].empty())
				newProgram->m_computeShader = true;

			// Create a uniform updater for the new shader
			newProgram->m_uniformUpdater = new ShaderUniformUpdater(*newProgram);

			return newProgram;
		}
	}

	return &m_defaultProgram;
}

ShaderLoader::ShaderProgram::~ShaderProgram()
{
	delete m_uniformUpdater;
}


#include "Config.h"
#include "ErrorHandlerLocator.h"
#include "ShaderLoader.h"
#include "ShaderUniformUpdater.h"
#include "Utilities.h"

/*
ShaderLoader::ShaderProgram::BaseShader()
{
	m_loaded = false;
	m_fileName = "";
	m_programHandle = 0;
	m_uniformUpdater = nullptr;
}
ShaderLoader::ShaderProgram::BaseShader(const std::string &p_filename) : BaseShader()
{
	m_fileName = p_filename;
}
ShaderLoader::ShaderProgram::~BaseShader()
{
	glDeleteProgram(m_programHandle);

	if(m_uniformUpdater != nullptr)
		delete m_uniformUpdater;
}

ErrorCode ShaderLoader::ShaderProgram::loadToVideoMemory()
{
	// Proceed only if it hasn't been loaded already
	if(!m_loaded)
	{
		m_loaded = true;
		//m_uniformUpdater = new ShaderUniformUpdater(*this);
	}
	return ErrorCode::Success;
}

ErrorCode ShaderLoader::ShaderProgram::loadFromFile(const std::string &p_fileName, std::string &p_source)
{
	p_source = "";

	// Load shader's source code from a file
	std::ifstream sourceStream(Config::PathsVariables().shader_path + p_fileName, std::ios::in);

	// Check if it was loaded successfully
	if(sourceStream.is_open())
	{
		std::string singleLine = "";
		while(std::getline(sourceStream, singleLine))
			p_source += "\n" + singleLine;

		sourceStream.close();
	}
	else
	{
		ErrHandlerLoc::get().log(ErrorCode::Ifstream_failed, ErrorSource::Source_ShaderLoader, p_fileName);
		return ErrorCode::Ifstream_failed;
	}

	return ErrorCode::Success;
}
ErrorCode ShaderLoader::ShaderProgram::compileFromSource(GLuint p_shaderHandle, const std::string &p_shaderSource)
{
	ErrorCode returnError = ErrorCode::Success;

	// Pass shader source code and compile it
	const char *shaderSource = p_shaderSource.c_str();
	glShaderSource(p_shaderHandle, 1, &shaderSource, NULL);
	glCompileShader(p_shaderHandle);

	// Check for shader compilation errors
	GLint shaderCompileResult = 0;
	glGetShaderiv(p_shaderHandle, GL_COMPILE_STATUS, &shaderCompileResult);
	if(!shaderCompileResult)
	{
		// Assign an error
		returnError = ErrorCode::Shader_compile_failed;

		int shaderCompileLogLength;
		glGetShaderiv(p_shaderHandle, GL_INFO_LOG_LENGTH, &shaderCompileLogLength);

		// Get the actual error message
		std::vector<char> shaderCompileErrorMessage(shaderCompileLogLength);
		glGetShaderInfoLog(p_shaderHandle, shaderCompileLogLength, NULL, &shaderCompileErrorMessage[0]);

		// Convert vector of chars to a string
		std::string errorMessageTemp;
		for(int i = 0; shaderCompileErrorMessage[i]; i++)
			errorMessageTemp += shaderCompileErrorMessage[i];

		// Log an error with a shader info log
		ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, errorMessageTemp);
	}

	return returnError;
}
ErrorCode ShaderLoader::ShaderProgram::attachShader(GLuint p_shaderHandle)
{
	// Attach shader to the program handle
	glAttachShader(m_programHandle, p_shaderHandle);

	// Check for errors
	GLenum glError = glGetError();
	if(glError != GL_NO_ERROR)
	{
		// Log an error and pass a filename
		ErrHandlerLoc::get().log(ErrorCode::Shader_attach_failed, ErrorSource::Source_ShaderLoader, 
								 Utilities::toString(glError) + " | Filename - " + getFileName());
		return ErrorCode::Shader_attach_failed;
	}

	return ErrorCode::Success;
}
ErrorCode ShaderLoader::ShaderProgram::linkProgram()
{
	GLint shaderLinkingResult;
	glLinkProgram(m_programHandle);

	// Check for linking errors. If an error has occured, get the error message and throw an exception
	glGetProgramiv(m_programHandle, GL_LINK_STATUS, &shaderLinkingResult);
	if(!shaderLinkingResult)
	{
		int shaderLinkLogLength = 0;
		std::string errorMessageTemp = "Couldn't retrieve the error";
		glGetShaderiv(m_programHandle, GL_INFO_LOG_LENGTH, &shaderLinkLogLength);

		// Sometimes opengl cannot retrieve the error string, so check that just in case
		if(shaderLinkLogLength > 0)
		{
			// Get the actual error message
			std::vector<char> shaderLinkErrorMessage(shaderLinkLogLength);
			glGetShaderInfoLog(m_programHandle, shaderLinkLogLength, NULL, &shaderLinkErrorMessage[0]);

			// Convert vector of chars to a string
			for(int i = 0; shaderLinkErrorMessage[i]; i++)
				errorMessageTemp += shaderLinkErrorMessage[i];
		}

		// Log an error and pass shader info log
		ErrHandlerLoc::get().log(ErrorCode::Shader_link_failed, ErrorSource::Source_ShaderLoader, errorMessageTemp);
		return ErrorCode::Shader_link_failed;
	}

	return ErrorCode::Success;
}

ShaderLoader::VertFragShader::VertFragShader(const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName)
	: m_vertFileName(p_vertexShaderFileName), m_fragFileName(p_fragmentShaderFileName)
{
	// Create the uniform updater
	//m_uniformUpdater = new ShaderUniformUpdater(*this);
}
ShaderLoader::VertFragShader::~VertFragShader()
{

}

ErrorCode ShaderLoader::VertFragShader::loadToVideoMemory()
{
	ErrorCode returnError = ErrorCode::Success;

	// Proceed only if it hasn't been loaded already
	if(!m_loaded)
	{
		m_loaded = true;

		// Shader name is made by combining individual filenames separated by a comma and a space
		m_fileName = m_vertFileName + ", " + m_fragFileName;

		// Create shader program handle
		m_programHandle = glCreateProgram();

		// Create shader handles
		m_vertShaderHandle = glCreateShader(GL_VERTEX_SHADER);
		m_fragShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);

		// Check for errors
		GLenum glError = glGetError();
		if(glError != GL_NO_ERROR)
		{
			returnError = ErrorCode::Shader_creation_failed;
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, Utilities::toString(glError) + " | Filename - " + getFileName());
		}
		else
		{
			std::string shaderSource;

			// Parse vertex shader source code from file and check for errors
			if(ErrHandlerLoc::get().ifSuccessful(loadFromFile(m_vertFileName, shaderSource), returnError))
			{
				// Compile vertex shader and check for errors
				if(ErrHandlerLoc::get().ifSuccessful(compileFromSource(m_vertShaderHandle, shaderSource), returnError))
				{
					// Attach vertex shader to the shader program
					if(ErrHandlerLoc::get().ifSuccessful(attachShader(m_vertShaderHandle), returnError))
					{
						// Parse fragment shader source code from file and check for errors
						if(ErrHandlerLoc::get().ifSuccessful(loadFromFile(m_fragFileName, shaderSource), returnError))
						{
							// Compile fragment shader and check for errors
							if(ErrHandlerLoc::get().ifSuccessful(compileFromSource(m_fragShaderHandle, shaderSource), returnError))
							{
								// Attach fragment shader to the shader program
								if(ErrHandlerLoc::get().ifSuccessful(attachShader(m_fragShaderHandle), returnError))
								{
									returnError = linkProgram();
								}
							}
						}
					}
				}
			}

			// Log an error if it occurred
			if(returnError != ErrorCode::Success)
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader);
			else
			{
				// Generate the uniform updater and check if it was successful
				if(ErrHandlerLoc::get().ifSuccessful(m_uniformUpdater->generateUpdateList(), returnError))
					ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_ShaderLoader,
											 "Vertex (" + m_vertFileName + ") and Fragment (" + m_fragFileName + ") shaders were loaded successfully");
				else
					ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader);
			}
		}
	}

	return returnError;
}
ErrorCode ShaderLoader::VertFragShader::releaseShaderHandles()
{
	// Delete compiled shaders (usually linking them to the shader program)
	glDeleteShader(m_vertShaderHandle);
	glDeleteShader(m_fragShaderHandle);

	return ErrorCode::Success;
}

ShaderLoader::GeomVertFragShader::GeomVertFragShader(const std::string &p_geometryShaderFileName, const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName)
	: VertFragShader(p_vertexShaderFileName, p_fragmentShaderFileName), m_geomFileName(p_geometryShaderFileName)
{

}
ShaderLoader::GeomVertFragShader::~GeomVertFragShader()
{

}

ErrorCode ShaderLoader::GeomVertFragShader::loadToVideoMemory()
{
	ErrorCode returnError = ErrorCode::Success;

	// Proceed only if it hasn't been loaded already
	if(!m_loaded)
	{
		m_loaded = true;

		// Shader name is made by combining individual filenames separated by a comma and a space
		m_fileName = m_geomFileName + ", " + m_vertFileName + ", " + m_fragFileName;

		// Create shader program handle
		m_programHandle = glCreateProgram();

		// Create shader handles
		m_vertShaderHandle = glCreateShader(GL_VERTEX_SHADER);
		m_fragShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
		m_geomShaderHandle = glCreateShader(GL_GEOMETRY_SHADER);

		// Check for errors
		GLenum glError = glGetError();
		if(glError != GL_NO_ERROR)
		{
			returnError = ErrorCode::Shader_creation_failed;
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, glError + " | Filename - " + getFileName());
		}
		else
		{
			std::string shaderSource;

			//
			// GEOMETRY SHADER
			// _______________________________________________________________
			// Parse geometry shader source code from file and check for errors
			if(ErrHandlerLoc::get().ifSuccessful(loadFromFile(m_geomFileName, shaderSource), returnError))
			{
				// Compile geometry shader and check for errors
				if(ErrHandlerLoc::get().ifSuccessful(compileFromSource(m_geomShaderHandle, shaderSource), returnError))
				{
					// Attach geometry shader to the shader program
					if(ErrHandlerLoc::get().ifSuccessful(attachShader(m_geomShaderHandle), returnError))
					{

						//
						// VERTEX SHADER
						// _____________________________________________________________
						// Parse vertex shader source code from file and check for errors
						if(ErrHandlerLoc::get().ifSuccessful(loadFromFile(m_vertFileName, shaderSource), returnError))
						{
							// Compile vertex shader and check for errors
							if(ErrHandlerLoc::get().ifSuccessful(compileFromSource(m_vertShaderHandle, shaderSource), returnError))
							{
								// Attach vertex shader to the shader program
								if(ErrHandlerLoc::get().ifSuccessful(attachShader(m_vertShaderHandle), returnError))
								{

									//
									// FRAMGENT SHADER
									// _______________________________________________________________
									// Parse fragment shader source code from file and check for errors
									if(ErrHandlerLoc::get().ifSuccessful(loadFromFile(m_fragFileName, shaderSource), returnError))
									{
										// Compile fragment shader and check for errors
										if(ErrHandlerLoc::get().ifSuccessful(compileFromSource(m_fragShaderHandle, shaderSource), returnError))
										{
											// Attach fragment shader to the shader program
											if(ErrHandlerLoc::get().ifSuccessful(attachShader(m_fragShaderHandle), returnError))
											{
												returnError = linkProgram();
											}
										}
									}
								}
							}
						}
					}
				}
			}

			// Log an error if it occurred
			if(returnError != ErrorCode::Success)
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader);
			else
			{
				// Create the uniform updater
				//m_uniformUpdater = new ShaderUniformUpdater(*this);

				// Generate the uniform updater and check if it was successful
				if(ErrHandlerLoc::get().ifSuccessful(m_uniformUpdater->generateUpdateList(), returnError))
					ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_ShaderLoader,
											 "Geometry (" + m_geomFileName + "), Vertex (" + m_vertFileName + ") and Fragment (" + m_fragFileName + ") shaders were loaded successfully");
				else
					ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader);
			}
		}
	}
	return returnError;
}
ErrorCode ShaderLoader::GeomVertFragShader::releaseShaderHandles()
{
	// Delete compiled shaders (usually linking them to the shader program)
	glDeleteShader(m_geomShaderHandle);
	glDeleteShader(m_vertShaderHandle);
	glDeleteShader(m_fragShaderHandle);

	return ErrorCode::Success;
}
*/

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

}

ErrorCode ShaderLoader::init()
{
	// Initialize default shader program
	m_defaultProgram.loadToMemory();
	m_defaultProgram.loadToVideoMemory();

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
		std::string shaderFilename[ArrayNumOfTypes];
		std::string programName;

		// Iterate over all passed properties
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Name:
				programName = p_properties[i].getString();
				break;

			case Properties::FragmentShader:
				shaderFilename[ArrayFragment] = p_properties[i].getString();
				break;

			case Properties::VertexShader:
				shaderFilename[ArrayVertex] = p_properties[i].getString();
				break;

			case Properties::GeometryShader:
				shaderFilename[ArrayGeometry] = p_properties[i].getString();
				break;

			case Properties::TessControlShader:
				shaderFilename[ArrayTessControl] = p_properties[i].getString();
				break;

			case Properties::TessEvaluationShader:
				shaderFilename[ArrayTessEvaluation] = p_properties[i].getString();
				break;
			}
		}

		// If program name was not specified, combine all shader names into one program name
		if(programName.empty())
		{
			// For every shader filename, if it's not empty, add it to the program name
			for(unsigned int shaderType = 0; shaderType < ArrayNumOfTypes; shaderType++)
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
			if(m_shaderPrograms[i].m_filenameHash == programHashkey)
				if(m_shaderPrograms[i].m_filename == programName)
					return &m_shaderPrograms[i];

		// Add the new program to the array
		m_shaderPrograms.push_back(ShaderProgram(programName, programHashkey));
		ShaderProgram *newProgram = &m_shaderPrograms[m_shaderPrograms.size() - 1];

		// Iterate over shader types
		for(unsigned int shaderType = 0; shaderType < ArrayNumOfTypes; shaderType++)
			// If shader filename is valid
			if(!shaderFilename[shaderType].empty())
				newProgram->addShader(static_cast<ShaderArrayTypes>(shaderType), shaderFilename[shaderType]);

		return newProgram;
	}

	return &m_defaultProgram;

		/*/ Iterate over shader types
		for(unsigned int shaderType = 0; shaderType < ArrayNumOfTypes; shaderType++)
		{
			// If shader filename is valid
			if(!shaderFilename[shaderType].empty())
			{
				// Calculate a filename hash key
				unsigned int hashkey = Utilities::getHashKey(shaderFilename[shaderType]);

				// Iterate over all shaders of this type; if hash key and filename matches, add it to the new program
				for(decltype(m_shaders[shaderType].size()) i = 0, size = m_shaders[shaderType].size(); i < size; i++)
					if(m_shaders[shaderType][i].m_filenameHash == hashkey)
						if(m_shaders[shaderType][i].m_filename == shaderFilename[shaderType])
							newProgram->addShader(m_shaders[shaderType][i]);
			}
		}

		newProgram->loadToMemory();

		unsigned int fragHashkey = Utilities::getHashKey(fragFilename);

		for(decltype(m_fragmentShaders.size()) i = 0, size = m_fragmentShaders.size(); i < size; i++)
			if(m_fragmentShaders[i].m_filenameHash == fragHashkey)
				if(m_fragmentShaders[i].m_filename == fragFilename)
					return;
	}

	return nullptr;*/
}
/*
ShaderLoader::ShaderProgram *ShaderLoader::load(const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName)
{
	// If any of the filenames are empty, return default shader
	if(p_vertexShaderFileName.empty() || p_fragmentShaderFileName.empty())
		return &m_defaultShader;

	// Shader name is made by combining individual filenames separated by a comma and a space
	std::string shaderName = p_vertexShaderFileName + ", " + p_fragmentShaderFileName;
	
	// Halt concurrency before starting the search in the shader pool
	SpinWait::Lock lock(m_vertFragMutex);

	// Iterate over all shaders, compare names, if they match, return it (so no duplicates are loaded)
	for(decltype(m_vertFragShaders.size()) i = 0, size = m_vertFragShaders.size(); i < size; i++)
		if(m_vertFragShaders[i]->getFileName() == shaderName)
			return m_vertFragShaders[i];

	// Create new vertex / fragment shader
	BaseShader *returnShader = new VertFragShader(p_vertexShaderFileName, p_fragmentShaderFileName);
	
	m_vertFragShaders.push_back(returnShader);

	return returnShader;
}
ShaderLoader::ShaderProgram *ShaderLoader::load(const std::string &p_geometryShaderFileName, const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName)
{
	// If geometry shader filename is empty, try to load a vertex-fragment shader instead
	if(p_geometryShaderFileName.empty())
		return load(p_vertexShaderFileName, p_fragmentShaderFileName);

	// If any of the filenames are empty, return default shader
	if(p_vertexShaderFileName.empty() || p_fragmentShaderFileName.empty())
		return &m_defaultShader;

	// Shader name is made by combining individual filenames separated by a comma and a space
	std::string shaderName = p_geometryShaderFileName + ", " + p_vertexShaderFileName + ", " + p_fragmentShaderFileName;

	// Halt concurrency before starting the search in the shader pool
	SpinWait::Lock lock(m_geomMutex);

	// Iterate over all shaders, compare names, if they match, return it (so no duplicates are loaded)
	for(decltype(m_geomShaders.size()) i = 0, size = m_geomShaders.size(); i < size; i++)
		if(m_geomShaders[i]->getFileName() == shaderName)
			return m_geomShaders[i];

	// Create new geometry / vertex / fragment shader
	BaseShader *returnShader = new GeomVertFragShader(p_geometryShaderFileName, p_vertexShaderFileName, p_fragmentShaderFileName);

	m_geomShaders.push_back(returnShader);

	return returnShader;
}
*/

ErrorCode ShaderLoader::ShaderProgram::loadToVideoMemory()
{
	ErrorCode individualError = ErrorCode::Success;
	ErrorCode returnError = ErrorCode::Success;

	if(!m_loadedToVideoMemory)
	{
		m_loadedToVideoMemory = true;
		bool shadersPresent = false;

		// Create shader program handle
		m_programHandle = glCreateProgram();

		// Iterate over all shaders and load them to video memory
		for(unsigned int i = 0; i < ShaderNumOfTypes; i++)
		{
			// If shader is valid
			if(m_shaders[i] != nullptr)
			{
				// Load to video memory
				individualError = m_shaders[i]->loadToVideoMemory();

				// If it was successful, attach the shader
				if(individualError == ErrorCode::Success)
				{
					attachShader(m_shaders[i]);
					shadersPresent = true;
				}
				else
					returnError = individualError;
			}
		}

		if(shadersPresent)
		{
			GLint shaderLinkingResult;
			glLinkProgram(m_programHandle);

			// Check for linking errors. If an error has occurred, get the error message and throw an exception
			glGetProgramiv(m_programHandle, GL_LINK_STATUS, &shaderLinkingResult);

			if(!shaderLinkingResult)
			{
				GLsizei shaderLinkLogLength = 0;
				std::string errorMessageTemp = "Couldn't retrieve the error";
				glGetShaderiv(m_programHandle, GL_INFO_LOG_LENGTH, &shaderLinkLogLength);

				// Sometimes OpenGL cannot retrieve the error string, so check that just in case
				if(shaderLinkLogLength > 0)
				{
					// Get the actual error message
					std::vector<char> shaderLinkErrorMessage(shaderLinkLogLength);
					glGetShaderInfoLog(m_programHandle, shaderLinkLogLength, NULL, &shaderLinkErrorMessage[0]);

					// Convert vector of chars to a string
					for(int i = 0; shaderLinkErrorMessage[i]; i++)
						errorMessageTemp += shaderLinkErrorMessage[i];
				}

				// Log an error and pass shader info log
				returnError = ErrorCode::Shader_link_failed;
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, errorMessageTemp);
			}

			// Iterate over all shaders and detach them
			for(unsigned int i = 0; i < ShaderNumOfTypes; i++)
			{
				// If shader is valid
				if(m_shaders[i] != nullptr)
				{
					glDetachShader(m_programHandle, m_shaders[i]->m_shaderHandle);
					//delete m_shaders[i];
				}
			}

			// Create the uniform updater
			m_uniformUpdater = new ShaderUniformUpdater(*this);
			m_uniformUpdater->generateUpdateList();
		}
		else
			m_uniformUpdater = new ShaderUniformUpdater(*this);
	}
	return returnError;
}

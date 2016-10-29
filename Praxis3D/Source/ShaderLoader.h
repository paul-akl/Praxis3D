#pragma once

#include <atomic>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ErrorCodes.h"
#include "PropertySet.h"
#include "SpinWait.h"

class ShaderUniformUpdater;

class ShaderLoader
{
	friend class ShaderProgram;
public:
	enum ShaderType : unsigned int
	{
		ShaderNull = 0,
		ShaderFragment = GL_FRAGMENT_SHADER,
		ShaderGeometry = GL_GEOMETRY_SHADER,
		ShaderVertex = GL_VERTEX_SHADER,
		ShaderTessControl = GL_TESS_CONTROL_SHADER,
		ShaderTessEvaluation = GL_TESS_EVALUATION_SHADER,
		ShaderNumOfTypes = 5
	};

	enum ShaderArrayTypes : unsigned int
	{
		ArrayFragment,
		ArrayGeometry,
		ArrayVertex,
		ArrayTessControl,
		ArrayTessEvaluation,
		ArrayNumOfTypes = ShaderNumOfTypes
	};
public:
	class Shader
	{
		friend class ShaderLoader;
	public:
		Shader(const std::string &p_filename, const ShaderType p_shaderType)
		{
			m_filename = p_filename;
			m_shaderType = p_shaderType;
			m_filenameHash = Utilities::getHashKey(p_filename);
			m_shaderHandle = 0;
			m_loadedToMemory = false;
		}

		inline ErrorCode loadToMemory()
		{
			if(!m_loadedToMemory)
			{
				m_loadedToMemory = true;

				// Load shader's source code from a file
				std::ifstream sourceStream(Config::PathsVariables().shader_path + m_filename, std::ios::in);

				// Check if it was loaded successfully
				if(sourceStream.is_open())
				{
					std::string singleLine = "";
					while(std::getline(sourceStream, singleLine))
						m_shaderSource += "\n" + singleLine;

					sourceStream.close();
				}
				else
				{
					ErrHandlerLoc::get().log(ErrorCode::Ifstream_failed, ErrorSource::Source_ShaderLoader, m_filename);
					return ErrorCode::Ifstream_failed;
				}
			}

			return ErrorCode::Success;
		}
		inline ErrorCode loadToVideoMemory()
		{
			// Create a shader handle
			m_shaderHandle = glCreateShader(m_shaderType);

			// Check for errors
			GLenum glError = glGetError();
			if(glError != GL_NO_ERROR)
			{
				ErrHandlerLoc::get().log(Shader_creation_failed, ErrorSource::Source_ShaderLoader, Utilities::toString(glError) + " | Filename - " + m_filename);
				return ErrorCode::Shader_creation_failed;
			}
			else
			{
				// Pass shader source code and compile it
				const char *shaderSource = m_shaderSource.c_str();
				glShaderSource(m_shaderHandle, 1, &shaderSource, NULL);
				glCompileShader(m_shaderHandle);

				// Check for shader compilation errors
				GLint shaderCompileResult = 0;
				glGetShaderiv(m_shaderHandle, GL_COMPILE_STATUS, &shaderCompileResult);

				// Clear the shader source string
				std::string().swap(m_shaderSource);

				// If compilation failed
				if(shaderCompileResult == 0)
				{
					// Assign an error
					int shaderCompileLogLength = 0;
					glGetShaderiv(m_shaderHandle, GL_INFO_LOG_LENGTH, &shaderCompileLogLength);

					// Get the actual error message
					std::vector<char> shaderCompileErrorMessage(shaderCompileLogLength);
					glGetShaderInfoLog(m_shaderHandle, shaderCompileLogLength, NULL, &shaderCompileErrorMessage[0]);

					// Convert vector of chars to a string
					std::string errorMessageTemp;
					for(int i = 0; shaderCompileErrorMessage[i]; i++)
						errorMessageTemp += shaderCompileErrorMessage[i];

					// Log an error with a shader info log
					ErrHandlerLoc::get().log(ErrorCode::Shader_compile_failed, ErrorSource::Source_ShaderLoader, errorMessageTemp);
					return ErrorCode::Shader_compile_failed;
				}
			}

			return ErrorCode::Success;
		}

		// Comparator operators
		const inline bool operator==(const std::string &p_filename) const		{ return (m_filename == p_filename); }
		const inline bool operator==(const unsigned int p_shaderHandle) const	{ return (m_shaderHandle == p_shaderHandle); }
		const inline bool operator==(const Shader &p_shader) const				{ return (m_filenameHash == p_shader.m_filenameHash); }
	
		const inline std::string &getFilename() { return m_filename; }

	private:
		std::string m_filename;
		std::string m_shaderSource;
		unsigned int m_filenameHash;

		ShaderType m_shaderType;
		unsigned int m_shaderHandle;
		std::atomic<bool> m_loadedToMemory;
	};
	class ShaderProgram
	{
		friend class ShaderLoader;
	public:
		inline void addShader(Shader *p_shader)
		{
			if(p_shader != nullptr)
			{
				switch(p_shader->m_shaderType)
				{
				case ShaderLoader::ShaderFragment:
					m_shaders[ArrayFragment] = p_shader;
					break;
				case ShaderLoader::ShaderGeometry:
					m_shaders[ArrayGeometry] = p_shader;
					break;
				case ShaderLoader::ShaderVertex:
					m_shaders[ArrayVertex] = p_shader;
					break;
				case ShaderLoader::ShaderTessControl:
					m_shaders[ArrayTessControl] = p_shader;
					m_tessellated = true;
					break;
				case ShaderLoader::ShaderTessEvaluation:
					m_shaders[ArrayTessEvaluation] = p_shader;
					m_tessellated = true;
					break;
				}
			}
		}
		inline void addShader(ShaderLoader::ShaderArrayTypes p_shaderType, const std::string &p_filename)
		{
			if(!p_filename.empty())
			{
				switch(p_shaderType)
				{
				case ShaderLoader::ArrayFragment:
					m_shaders[ArrayFragment] = new Shader(p_filename, ShaderFragment);
					break;
				case ShaderLoader::ArrayGeometry:
					m_shaders[ArrayGeometry] = new Shader(p_filename, ShaderGeometry);
					break;
				case ShaderLoader::ArrayVertex:
					m_shaders[ArrayVertex] = new Shader(p_filename, ShaderVertex);
					break;
				case ShaderLoader::ArrayTessControl:
					m_shaders[ArrayTessControl] = new Shader(p_filename, ShaderTessControl);
					m_tessellated = true;
					break;
				case ShaderLoader::ArrayTessEvaluation:
					m_shaders[ArrayTessEvaluation] = new Shader(p_filename, ShaderTessEvaluation);
					m_tessellated = true;
					break;
				}
			}
		}

		inline void bind() { glUseProgram(m_programHandle); }

		// Loads shader source code to memory
		inline ErrorCode loadToMemory()
		{
			ErrorCode individualError = ErrorCode::Success;
			ErrorCode returnError = ErrorCode::Success;

			if(!m_loadedToMemory)
			{
				m_loadedToMemory = true;

				// Iterate over all shaders and load them to memory
				for(unsigned int i = 0; i < ShaderNumOfTypes; i++)
				{
					// If shader is valid
					if(m_shaders[i] != nullptr)
					{
						// Load to memory
						individualError = m_shaders[i]->loadToMemory();

						// If it failed, assign a return error
						if(individualError != ErrorCode::Success)
							returnError = individualError;
					}
				}
			}

			return returnError;
		}
		// Compile and attach, shaders, create and link program, populate uniform updaters
		ErrorCode loadToVideoMemory();

		// Returns true if the program contains any tessellation shaders
		const inline bool isTessellated() const { return m_tessellated; }

		// Getters
		const inline std::string &getFilename() const { return m_filename; }
		const inline unsigned int getShaderHandle() const { return m_programHandle; }
		const inline ShaderUniformUpdater &getUniformUpdater() const { return *m_uniformUpdater; }

		const inline bool isDefaultProgram() const { return m_programHandle == m_defaultProgramHandle; }

		// Comparator operators
		const inline bool operator==(const std::string &p_filename) const { return (m_filename == p_filename); }
		const inline bool operator==(const unsigned int p_programHandle) const { return (m_programHandle == p_programHandle); }
		const inline bool operator==(const ShaderProgram &p_shaderProgram) const { return (m_filenameHash == p_shaderProgram.m_filenameHash); }
	
		// Checks if a shader of specific type is present
		const inline bool shaderPresent(const unsigned int p_shaderType)
		{
			// Check if the shader type passed is valid (in bounds) and return true if the shader is not null
			if(p_shaderType >= 0 && p_shaderType < ShaderNumOfTypes)
				return (m_shaders[p_shaderType] != nullptr);

			return false;
		}

		// Returns a filename of a shader of specific type
		const inline std::string getShaderFilename(const unsigned int p_shaderType)
		{
			// Check if the shader type passed is valid (in bounds)
			if(p_shaderType >= 0 && p_shaderType < ShaderNumOfTypes + 1)
				// If the shader is valid, return its filename, otherwise return an empty string
				if(m_shaders[p_shaderType] != nullptr)
					return m_shaders[p_shaderType]->getFilename();

			return std::string();
		}

	private:
		// Constructors aren't public, to allow them to be created only by the ShaderLoader class (makes it act as a factory)
		ShaderProgram(const std::string &p_filename = "", unsigned int p_filenameHashkey = 0)
		{
			m_filename = p_filename;
			m_filenameHash = p_filenameHashkey > 0 ? p_filenameHashkey : Utilities::getHashKey(p_filename);
			m_tessellated = false;
			m_loadedToMemory = false;
			m_loadedToVideoMemory = false;
			m_programHandle = 0;
			m_uniformUpdater = nullptr;

			for(unsigned int i = 0; i < ShaderNumOfTypes; i++)
				m_shaders[i] = nullptr;
		}
		inline ErrorCode attachShader(Shader *p_shader)
		{
			// Attach shader to the program handle
			glAttachShader(m_programHandle, p_shader->m_shaderHandle);

			// Check for errors
			GLenum glError = glGetError();
			if(glError != GL_NO_ERROR)
			{
				// Log an error and pass a filename
				ErrHandlerLoc::get().log(ErrorCode::Shader_attach_failed, ErrorSource::Source_ShaderLoader,
										 Utilities::toString(glError) + " | Filename - " + p_shader->m_filename);
				return ErrorCode::Shader_attach_failed;
			}

			return ErrorCode::Success;
		}

		bool	m_loadedToMemory,
				m_loadedToVideoMemory,
				m_tessellated;

		std::string m_filename;
		unsigned int m_filenameHash;
		unsigned int m_programHandle;
		Shader *m_shaders[ShaderNumOfTypes];

		ShaderUniformUpdater *m_uniformUpdater;
		
		// Holds a default shader handle that the program handle can be tested against
		static unsigned int m_defaultProgramHandle;
	};
	
	ShaderLoader();
	~ShaderLoader();

	// Must be called from graphics thread
	ErrorCode init();

	// Returns a default (empty) shader
	inline ShaderProgram *load() { return &m_defaultProgram; }
	ShaderProgram *load(const PropertySet &p_properties);

	// Returns a default (empty) shader
	//BaseShader *load() { return &m_defaultShader; }
	//BaseShader *load(const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName);
	//BaseShader *load(const std::string &p_geometryShaderFileName, const std::string &p_vertexShaderFileName, const std::string &p_fragmentShaderFileName);

private:
	SpinWait	m_vertFragMutex,
				m_geomMutex;

	//BaseShader m_defaultShader;
	ShaderProgram m_defaultProgram;

	//std::vector<BaseShader*> m_vertFragShaders;
	//std::vector<BaseShader*> m_geomShaders;

	// Mutex used to block calls from other threads while operation is in progress
	SpinWait m_mutex;

	//std::vector<Shader> m_shaders[ArrayNumOfTypes];
	std::vector<ShaderProgram> m_shaderPrograms;

	// Assign shader types, so they could be resolved from array type; order dependent
	/*const static ShaderType m_shaderTypes[ArrayNumOfTypes] =
	{
		ShaderFragment,
		ShaderGeometry,
		ShaderVertex,
		ShaderTessControl,
		ShaderTessEvaluation
	};*/
};
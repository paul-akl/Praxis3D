#pragma once

#include <atomic>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "CommonDefinitions.h"
#include "ErrorCodes.h"
#include "PropertySet.h"
#include "SpinWait.h"

class ShaderUniformUpdater;

class ShaderLoader
{
	friend class ShaderProgram;
public:
	class Shader
	{
		friend class ShaderLoader;
	private:
		enum ShaderType : unsigned int
		{
			ShaderNull = 0,
			ShaderFragment,
			ShaderGeometry,
			ShaderVertex,
			ShaderTessControl,
			ShaderTessEvaluation,
			ShaderNumOfTypes = 5
		};

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
					ErrHandlerLoc::get().log(ErrorCode::Ifstream_failed, ErrorSource::Source_ShaderLoader, "(Filename - \"" + m_filename + "\"): ");
					return ErrorCode::Ifstream_failed;
				}
			}

			return ErrorCode::Success;
		}
		inline ErrorCode loadToVideoMemory()
		{
			// Clear error queue
			glGetError();

			// Create a shader handle
			m_shaderHandle = glCreateShader(m_shaderType);

			// Check for errors
			GLenum glError = glGetError();
			if(glError != GL_NO_ERROR)
			{
				ErrHandlerLoc::get().log(Shader_creation_failed, ErrorSource::Source_ShaderLoader, "(Filename - \"" + m_filename + "\"): " + Utilities::toString(glError));
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
					ErrHandlerLoc::get().log(ErrorCode::Shader_compile_failed, ErrorSource::Source_ShaderLoader, "(Filename - \"" + m_filename + "\"): " + errorMessageTemp);
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
		friend class CommandBuffer;
		friend class ShaderLoader;
		friend class RendererFrontend;

	public:
		inline void addShader(ShaderType p_shaderType, const std::string &p_filename)
		{
			m_shaderFilename[p_shaderType] = p_filename;
		}

		// Loads shader source code to memory
		inline ErrorCode loadToMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// Check if the shader hasn't been already loaded
			if(!m_loadedToMemory)
			{
				m_loadedToMemory = true;
				m_defaultShader = true;

				for(unsigned int i = 0; i < ShaderType_NumOfTypes; i++)
				{
					if(!m_shaderFilename[i].empty())
					{
						// Load shader's source code from a file
						std::ifstream sourceStream(Config::PathsVariables().shader_path + m_shaderFilename[i], std::ios::in);

						// Check if it was loaded successfully
						if(sourceStream.is_open())
						{
							std::string singleLine = "";
							while(std::getline(sourceStream, singleLine))
								m_shaderSource[i] += "\n" + singleLine;

							sourceStream.close();

							m_defaultShader = false;
						}
						else
						{
							ErrHandlerLoc::get().log(ErrorCode::Ifstream_failed, ErrorSource::Source_ShaderLoader, "(Filename - \"" + m_shaderFilename[i] + "\"): ");
							
							// TODO Check if error works without exiting from here
							//return ErrorCode::Ifstream_failed;
							returnError = ErrorCode::Ifstream_failed;
						}
					}
				}
			}

			return returnError;
		}

		// Returns true if the program contains any tessellation shaders
		const inline bool isTessellated() const { return m_tessellated; }

		// Getters
		const inline std::string &getCombinedFilename() const { return m_combinedFilename; }
		const inline unsigned int getShaderHandle() const { return m_programHandle; }
		inline ShaderUniformUpdater &getUniformUpdater() const { return *m_uniformUpdater; }

		const inline bool isDefaultProgram() const { return m_defaultShader; }
		
		// Comparator operators
		const inline bool operator==(const std::string &p_filename) const { return (m_combinedFilename == p_filename); }
		const inline bool operator==(const unsigned int p_programHandle) const { return (m_programHandle == p_programHandle); }
		const inline bool operator==(const ShaderProgram &p_shaderProgram) const { return (m_filenameHash == p_shaderProgram.m_filenameHash); }
	
		// Checks if a shader of specific type is present
		const inline bool shaderPresent(const unsigned int p_shaderType)
		{
			// Check if the shader type passed is valid (in bounds) and return true if the shader is not null
			//if(p_shaderType >= 0 && p_shaderType < ShaderType_NumOfTypes)
			//	return (m_shaders[p_shaderType] != nullptr);

			return false;
		}

		// Returns a filename of a shader of specific type
		const inline std::string getShaderFilename(const unsigned int p_shaderType)
		{
			return m_shaderFilename[p_shaderType];

			/*/ Check if the shader type passed is valid (in bounds)
			if(p_shaderType >= 0 && p_shaderType < ShaderType_NumOfTypes + 1)
				// If the shader is valid, return its filename, otherwise return an empty string
				if(!m_shaderFilename[p_shaderType].empty())
					return m_shaderFilename[p_shaderType];

			return std::string();*/
		}

	private:
		// Constructors aren't public, to allow them to be created only by the ShaderLoader class (makes it act as a factory)
		ShaderProgram(const std::string &p_filename = "", unsigned int p_filenameHashkey = 0)
		{
			m_combinedFilename = p_filename;
			m_filenameHash = p_filenameHashkey > 0 ? p_filenameHashkey : Utilities::getHashKey(p_filename);
			m_tessellated = false;
			m_defaultShader = false;
			m_loadedToMemory = false;
			m_loadedToVideoMemory = false;
			m_programHandle = 0;
			m_uniformUpdater = nullptr;
		}
		~ShaderProgram();

		bool	m_defaultShader, 
				m_loadedToMemory,
				m_loadedToVideoMemory,
				m_tessellated;

		std::string m_combinedFilename;
		std::string m_shaderFilename[ShaderType_NumOfTypes];
		std::string m_shaderSource[ShaderType_NumOfTypes];
		ErrorMessage m_errorMessages[ShaderType_NumOfTypes];

		unsigned int m_filenameHash;
		unsigned int m_programHandle;

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

private:
	SpinWait	m_vertFragMutex,
				m_geomMutex;

	ShaderProgram m_defaultProgram;
	
	// Mutex used to block calls from other threads while operation is in progress
	SpinWait m_mutex;

	std::vector<ShaderProgram*> m_shaderPrograms;
};
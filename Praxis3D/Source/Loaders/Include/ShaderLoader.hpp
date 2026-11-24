#pragma once

#include <atomic>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Definitions/Include/CommonDefinitions.hpp"
#include "ErrorHandler/Include/ErrorCodes.hpp"
#include "Common/Include/PropertySet.hpp"
#include "Multithreading/Include/SpinWait.hpp"

class ShaderUniformUpdater;

class ShaderLoader
{
	friend class ShaderProgram;
public:
	// Holds data on #define variable replacements in the shader source code
	struct ShaderVariableDefinition
	{
		ShaderVariableDefinition(const ShaderType p_shaderType, const std::string &p_variableName, const std::string &p_value)
			: m_shaderType(p_shaderType), m_variableName(p_variableName), m_variableValue(p_value) { }

		bool operator==(const ShaderVariableDefinition &p_variable)
		{
			return	m_shaderType == p_variable.m_shaderType && 
					m_variableName == p_variable.m_variableName;
		}

		ShaderType m_shaderType;
		std::string m_variableName;
		std::string m_variableValue;
	};
	class Shader
	{
		friend class ShaderLoader;
	private:
		enum ShaderType : unsigned int
		{
			ShaderNull = 0,
			ShaderCompute,
			ShaderFragment,
			ShaderGeometry,
			ShaderVertex,
			ShaderTessControl,
			ShaderTessEvaluation,
			ShaderNumOfTypes = 6
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
		friend struct LoadableObjectsContainer;
		friend class CommandBuffer;
		friend class ShaderLoader;
		friend class RendererFrontend;
		friend class RendererScene;
		friend class LightingPass;

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
						// Make sure the shader source string is empty
						m_shaderSource[i].clear();

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

				// Re-set all the previously set variable definitions
				for(const auto &variable : m_variableDefinitions)
				{
					setVariableDefinition(variable);
				}
			}

			return returnError;
		}

		inline ErrorCode reloadToMemory()
		{
			// Reset the loaded flag
			m_loadedToMemory = false;

			// Load shader to memory
			return loadToMemory();
		}

		// Returns true if the program contains any tessellation shaders
		const inline bool isTessellated() const { return m_tessellated; }

		// Getters
		inline std::string &getShaderSource(ShaderType p_shaderType) { return m_shaderSource[p_shaderType]; }
		const inline std::string &getCombinedFilename() const { return m_combinedFilename; }
		const inline unsigned int getShaderHandle() const { return m_programHandle; }
		inline ShaderUniformUpdater &getUniformUpdater() const { return *m_uniformUpdater; }

		const inline bool isDefaultProgram() const		{ return m_defaultShader;				}
		const inline bool isLoadedToVideoMemory() const { return m_loadedToVideoMemory;			}
		const inline bool isNameAutoGenerated() const	{ return m_combinedFilenameGenerated;	}
		
		// Setters
		inline void setShaderFilename(const ShaderType p_shaderType, const std::string &p_filename) { m_shaderFilename[p_shaderType] = p_filename; }
		inline void resetLoadedToVideoMemoryFlag() { m_loadedToVideoMemory = false; }

		// Functions for setting the #define variable values inside the shader code; must either be called before loading the shader to video memory (compiling) or the shader must be reloaded for the new value to take effect
		inline ErrorCode setDefineValue(const ShaderType p_shaderType, const std::string &p_variableName, const std::string &p_value)
		{
			ErrorCode returnError = ErrorCode::Success;

			// Create variable
			ShaderVariableDefinition variable(p_shaderType, p_variableName, p_value);

			// Try to set the variable
			returnError = setVariableDefinition(variable);

			// If the variable was set, save it
			if(returnError == ErrorCode::Success)
				saveVariableDefinition(variable);
			else
				ErrHandlerLoc::get().log(returnError, m_shaderFilename[p_shaderType] + ": " + p_variableName, ErrorSource::Source_ShaderLoader);

			return returnError;
		}
		inline ErrorCode setDefineValue(const std::string &p_variableName, const std::string &p_value)
		{
			ErrorCode returnError = ErrorCode::Success;

			for(unsigned int i = 0; i < ShaderType::ShaderType_NumOfTypes; i++)
			{
				if(!m_shaderSource[i].empty())
				{
					// Try to set the variable
					if(ErrorCode setVariableError = setDefineValue(static_cast<ShaderType>(i), p_variableName, p_value); setVariableError != ErrorCode::Success)
						returnError = setVariableError;
				}
			}

			return returnError;
		}
		
		// Set the #define variable value inside the source code for the given shader type; must either be called before loading the shader to video memory (compiling) or the shader must be reloaded for the new value to take effect
		template <typename VariableType>
		inline ErrorCode setDefineValue(const ShaderType p_shaderType, const std::string &p_variableName, const VariableType p_value) { return setDefineValue(p_shaderType, p_variableName, Utilities::toString(p_value)); }

		// Set the #define variable value inside the source code for all shader types; must either be called before loading the shader to video memory (compiling) or the shader must be reloaded for the new value to take effect
		template <typename VariableType>
		inline ErrorCode setDefineValue(const std::string &p_variableName, const VariableType p_value) { return setDefineValue(p_variableName, Utilities::toString(p_value)); }

		// Comparator operators
		const inline bool operator==(const std::string &p_filename) const { return (m_combinedFilename == p_filename); }
		const inline bool operator==(const unsigned int p_programHandle) const { return (m_programHandle == p_programHandle); }
		const inline bool operator==(const ShaderProgram &p_shaderProgram) const { return (m_filenameHash == p_shaderProgram.m_filenameHash); }
	
		// Checks if a shader of specific type is present
		const inline bool shaderPresent(const unsigned int p_shaderType)
		{
			// Check if the shader type passed is valid (in bounds) and return true if the shader is not null
			if(p_shaderType >= 0 && p_shaderType < ShaderType_NumOfTypes)
				return !m_shaderSource[p_shaderType].empty();

			return false;
		}

		// Returns a filename of a shader of specific type
		const inline std::string &getShaderFilename(const unsigned int p_shaderType) const
		{
			return m_shaderFilename[p_shaderType];
		}

	private:
		// Constructors aren't public, to allow them to be created only by the ShaderLoader class (makes it act as a factory)
		ShaderProgram(const std::string &p_filename = "", unsigned int p_filenameHashkey = 0)
		{
			m_combinedFilename = p_filename;
			m_filenameHash = p_filenameHashkey > 0 ? p_filenameHashkey : Utilities::getHashKey(p_filename);
			m_tessellated = false;
			m_computeShader = false;
			m_defaultShader = false;
			m_loadedToMemory = false;
			m_loadedToVideoMemory = false;
			m_combinedFilenameGenerated = false;
			m_programHandle = 0;
			m_uniformUpdater = nullptr;
		}
		~ShaderProgram();

		void setLoadedToVideoMemory(const bool p_loaded) { m_loadedToVideoMemory = p_loaded; }
		void saveVariableDefinition(const ShaderVariableDefinition &p_variable)
		{
			bool variableSet = false;

			// If the variable definition already exist, update it
			for(auto &variable : m_variableDefinitions)
			{
				if(p_variable == variable)
				{
					variable.m_variableValue = p_variable.m_variableValue;
					variableSet = true;
				}
			}

			// If the variable definition didn't exist, create a new one
			if(!variableSet)
				m_variableDefinitions.push_back(p_variable);
		}
		ErrorCode setVariableDefinition(const ShaderVariableDefinition &p_variable);

		bool	m_defaultShader, 
				m_loadedToMemory,
				m_loadedToVideoMemory,
				m_tessellated,
				m_computeShader;

		// True if the combined filename was generated from individual shader filenames
		// False if the name was specified during loading
		bool m_combinedFilenameGenerated;

		std::string m_combinedFilename;
		std::string m_shaderFilename[ShaderType_NumOfTypes];
		std::string m_shaderSource[ShaderType_NumOfTypes];
		ErrorMessage m_errorMessages[ShaderType_NumOfTypes];

		unsigned int m_filenameHash;
		unsigned int m_programHandle;

		ShaderUniformUpdater *m_uniformUpdater;

		std::vector<ShaderVariableDefinition> m_variableDefinitions;
		
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

	ErrorCode reload(ShaderProgram *p_shaderProgram);

	const inline std::vector<ShaderProgram *> &getObjectPool() const { return m_shaderPrograms; }

private:
	SpinWait	m_vertFragMutex,
				m_geomMutex;

	ShaderProgram m_defaultProgram;
	
	// Mutex used to block calls from other threads while operation is in progress
	SpinWait m_mutex;

	std::vector<ShaderProgram*> m_shaderPrograms;
};
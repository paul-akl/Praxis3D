
#include <Windows.h>

#include "Config.h"
#include "ConfigLoader.h"
#include "ErrorHandler.h"
#include "WindowLocator.h"

// Predefined variables for "AssignErrorType" macro
#define ERR_TYP_PREDEF m_errorTypes
#define ERR_DATA_PREDEF m_errorData
#define ERR_DATA_TYPE_PREDEF m_errorType
#define ERR_HASH_PREDEF m_errHashmap

#define AssignErrorType(ERROR_CODE, ERROR_TYPE) ERR_DATA_PREDEF[ERROR_CODE].ERR_DATA_TYPE_PREDEF = ERROR_TYPE; ERR_HASH_PREDEF[GetString(ERROR_CODE)] = ERROR_CODE

ErrorHandler::ErrorHandler()
{
	m_console = nullptr;
	
	// Add the error codes to the hash map and also assign their error types in the error type array
	AssignErrorType(Undefined, Warning);
	AssignErrorType(Success, Info);
	AssignErrorType(Failure, Warning);
	AssignErrorType(Initialize_success, Info);
	AssignErrorType(Initialize_failure, Warning);
	AssignErrorType(Load_success, Info);
	AssignErrorType(Load_failure, Warning);
	AssignErrorType(Load_to_memory_success, Info);
	AssignErrorType(Load_to_memory_failure, Warning);
	AssignErrorType(File_not_found, Warning);
	AssignErrorType(Filename_empty, Warning);
	AssignErrorType(Audio_invalid_bus_type, Warning);
	AssignErrorType(Audio_no_drivers, Error);
	AssignErrorType(Audio_system_init_failed, Error);
	AssignErrorType(Destroy_obj_not_found, Warning);
	AssignErrorType(Glew_failed, FatalError);
	AssignErrorType(Ifstream_failed, Warning);
	AssignErrorType(Clock_QueryFrequency, FatalError);
	AssignErrorType(Framebuffer_failed, FatalError);
	AssignErrorType(Geometrybuffer_failed, FatalError);
	AssignErrorType(Editor_path_outside_current_dir, Warning);
	AssignErrorType(Font_type_missing_construction, Warning);
	AssignErrorType(GL_context_missing, Error);
	AssignErrorType(Universal_scene_extend_null, Error);
	AssignErrorType(Universal_scene_extend_duplicate, Error);
	AssignErrorType(Window_handle_missing, Error);
	AssignErrorType(Lua_init_func_failed, Warning);
	AssignErrorType(Lua_load_script_failed, Warning);
	AssignErrorType(Lua_update_func_failed, Warning);
	AssignErrorType(AssimpScene_failed, Error);
	AssignErrorType(ObjectPool_full, Warning); 
	AssignErrorType(Collision_invalid, Warning);
	AssignErrorType(Collision_max_dynamic_events, Warning);
	AssignErrorType(Collision_max_static_events, Warning);
	AssignErrorType(Collision_missing, Warning);
	AssignErrorType(Kinematic_has_mass, Warning);
	AssignErrorType(Property_missing_size, Warning);
	AssignErrorType(Property_missing_radius, Warning);
	AssignErrorType(Property_missing_type, Warning);
	AssignErrorType(Property_no_filename, Warning);
	AssignErrorType(Shader_attach_failed, Error);
	AssignErrorType(Shader_compile_failed, Error);
	AssignErrorType(Shader_creation_failed, Error);
	AssignErrorType(Shader_link_failed, Error);
	AssignErrorType(Shader_loading_failed, Error);
	AssignErrorType(Shader_source_empty, Warning);
	AssignErrorType(Shader_variable_not_found, Warning);
	AssignErrorType(GameObjects_missing, Error);
	AssignErrorType(Number_of_meshes_missmatch, Warning);
	AssignErrorType(Texture_not_found, Warning); 
	AssignErrorType(Texture_empty, Warning);
	AssignErrorType(Invalid_num_vid_displays, Warning);
	AssignErrorType(SDL_video_init_failed, FatalError);
	AssignErrorType(SDL_vsync_failed, Warning);
	AssignErrorType(Window_creation_failed, FatalError);
	AssignErrorType(Duplicate_object_id, Warning);
	AssignErrorType(Invalid_object_id, Warning);
	AssignErrorType(Nonexistent_object_id, Warning);
	AssignErrorType(Nonexistent_parent_entity, Warning);

	// Add error sources to the hash map, and offset them by number of error codes, because they share the same hash map
	for(unsigned int i = 0; i < Source_NumberOfErrorSources; i++)
	{
		ErrorSource errorSource = static_cast<ErrorSource>(i);
		m_errHashmap[GetString(errorSource)] = NumberOfErrorCodes + errorSource;
	}

	// Add error types to the hash map, and offset them by number of error codes and error sources, because they share the same hash map
	m_errHashmap[GetString(Info)]		= NumberOfErrorCodes + Source_NumberOfErrorSources + Info;
	m_errHashmap[GetString(Warning)]	= NumberOfErrorCodes + Source_NumberOfErrorSources + Warning;
	m_errHashmap[GetString(Error)]		= NumberOfErrorCodes + Source_NumberOfErrorSources + Error;
	m_errHashmap[GetString(FatalError)]	= NumberOfErrorCodes + Source_NumberOfErrorSources + FatalError;
}
ErrorHandler::~ErrorHandler()
{

}

ErrorCode ErrorHandler::init()
{
	m_console = new CoutConsole;
	
	ConfigFile errorCodes;

	if(errorCodes.import(Config::filepathVar().config_path + Config::configFileVar().error_code_strings_eng, m_errHashmap) != Success)
	{
		log(ErrorType::Error, ErrorSource::Source_General, "Error strings has failed to load");
	}
	else
	{
		NodeIterator rootNode = errorCodes.getRootNode();

		NodeIterator it = rootNode.getNode("Error Codes");
		for(int i = 0; i < ErrorCode::NumberOfErrorCodes; i++)
		{
			m_errorData[i].m_errorString = it.getValue(i).getString();
		}

		it = rootNode.getNode("Error Sources");
		for(int i = ErrorCode::NumberOfErrorCodes, size = ErrorSource::Source_NumberOfErrorSources + ErrorCode::NumberOfErrorCodes; i < size; i++)
			m_errorSources[i - ErrorCode::NumberOfErrorCodes] = it.getValue(i).getString();

		it = rootNode.getNode("Error Types");
		for(int i = ErrorCode::NumberOfErrorCodes + ErrorSource::Source_NumberOfErrorSources,
			size = ErrorSource::Source_NumberOfErrorSources + ErrorCode::NumberOfErrorCodes + ErrorType::NumberOfErrorTypes; i < size; i++)
			m_errorTypeStrings[i - ErrorCode::NumberOfErrorCodes - ErrorSource::Source_NumberOfErrorSources] = it.getValue(i).getString();

		if(Config::engineVar().log_store_logs)
			m_logData.setMaxLogs(Config::engineVar().log_max_num_of_logs);
	}

	return ErrorCode::Success;
}

void ErrorHandler::log(ErrorCode p_errorCode)
{
	log(p_errorCode, ErrorSource::Source_Unknown);
}
void ErrorHandler::log(ErrorCode p_errorCode, ErrorSource p_errorSource)
{
	if(p_errorCode == ErrorCode::CachedError)
	{
		if(m_cachedError.errorPresent())
		{
			log(m_errorData[m_cachedError.m_errorCode].m_errorType, p_errorSource, m_errorData[m_cachedError.m_errorCode].m_errorString + ": " + m_cachedError.m_errorString);
			m_cachedError.clear();
		}
	}
	else
		log(m_errorData[p_errorCode].m_errorType, p_errorSource, m_errorData[p_errorCode].m_errorString);
}
void ErrorHandler::log(ErrorType p_errorType, ErrorSource p_errorSource, std::string p_error)
{
	// If the engine has already been set to shutdown, don't process any errors
	if(Config::m_engineVar.running == true)
	{
		std::string displayMessage;
		switch(p_errorType)
		{
			case ErrorType::Info:
				{
					m_logData.addLogMessage(p_errorType, p_errorSource, p_error);
					m_console->displayMessage("\033[1;32m[" + m_errorTypeStrings[p_errorType] + "] \033[1;36m[" + m_errorSources[p_errorSource] + "]\033[0;37m: " + p_error + ".\033[0m");
					break;
				}

			case ErrorType::Warning:
				{
					m_logData.addLogMessage(p_errorType, p_errorSource, p_error);
					m_console->displayMessage("\033[31m[" + m_errorTypeStrings[p_errorType] + "] \033[1;36m[" + m_errorSources[p_errorSource] + "]\033[0;37m: " + p_error + ".\033[0m");
					break;
				}

			case ErrorType::Error:
				{
					// Remove a 'new line' character if it's present, as it would break the formating
					if(p_error[p_error.size() - 1] == '\n')
						p_error.pop_back();

					// TODO make the error question data driven
					if(!WindowLocator().get().spawnYesNoErrorBox(m_errorTypeStrings[p_errorType] + ": " + m_errorSources[p_errorSource], m_errorSources[p_errorSource] + ": " + p_error + ".\n\nWould you like to continue?"))
						Config::m_engineVar.running = false;

					m_logData.addLogMessage(p_errorType, p_errorSource, p_error);
					m_console->displayMessage("\033[31m[" + m_errorTypeStrings[p_errorType] + "] \033[1;36m[" + m_errorSources[p_errorSource] + "]\033[0;37m: " + p_error + ".\033[0m");
					break;
				}

			case ErrorType::FatalError:
				{
					m_logData.addLogMessage(p_errorType, p_errorSource, p_error);
					WindowLocator().get().spawnErrorBox(m_errorTypeStrings[p_errorType] + ": " + m_errorSources[p_errorSource], m_errorSources[p_errorSource] + ": " + p_error + ".");
					Config::m_engineVar.running = false;

					break;
				}

			default:
				break;
		}
	}
}
void ErrorHandler::log(ErrorCode p_errorCode, ErrorSource p_errorSource, std::string p_error)
{
	if(p_errorCode == ErrorCode::CachedError)
	{
		if(m_cachedError.errorPresent())
		{
			log(m_errorData[m_cachedError.m_errorCode].m_errorType, p_errorSource, m_errorData[m_cachedError.m_errorCode].m_errorString + ": " + p_error + ": " + m_cachedError.m_errorString);
			m_cachedError.clear();
		}
	}
	else
		log(m_errorData[p_errorCode].m_errorType, p_errorSource, m_errorData[p_errorCode].m_errorString + ": " + p_error);
}

void ErrorHandler::log(const ErrorCode p_errorCode, const std::string &p_objectName, const ErrorSource p_errorSource)
{
	log(m_errorData[p_errorCode].m_errorType, p_errorSource, "\033[1;33m\'" + p_objectName + "\'\033[0m: " + m_errorData[p_errorCode].m_errorString);
}

ErrorHandler::CoutConsole::CoutConsole()
{
	m_stdoutHandle = nullptr;
	m_stdinHandle = nullptr;

	m_outModeInit = 0;
	m_inModeInit = 0;

	setupConsole();
}

void ErrorHandler::CoutConsole::setupConsole()
{
	DWORD outMode = 0, inMode = 0;
	m_stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	m_stdinHandle = GetStdHandle(STD_INPUT_HANDLE);

	if(m_stdoutHandle == INVALID_HANDLE_VALUE || m_stdinHandle == INVALID_HANDLE_VALUE)
	{
		exit(GetLastError());
	}

	if(!GetConsoleMode(m_stdoutHandle, &outMode) || !GetConsoleMode(m_stdinHandle, &inMode))
	{
		exit(GetLastError());
	}

	m_outModeInit = outMode;
	m_inModeInit = inMode;

	// Enable ANSI escape codes
	outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	// Set stdin as no echo and unbuffered
	inMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

	if(!SetConsoleMode(m_stdoutHandle, outMode) || !SetConsoleMode(m_stdinHandle, inMode))
	{
		displayMessage("Failed to enable virtual terminal for colored console text output.");
		// ERROR
		//exit(GetLastError());
	}
}

void ErrorHandler::CoutConsole::restoreConsole()
{
	// Reset colors
	printf("\x1b[0m");

	// Reset console mode
	if(!SetConsoleMode(m_stdoutHandle, m_outModeInit) || !SetConsoleMode(m_stdinHandle, m_inModeInit))
	{
		displayMessage("Failed to disable virtual terminal.");
		//exit(GetLastError());
	}
}

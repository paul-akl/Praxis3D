
//#include <Windows.h>

#include "Config.h"
#include "ConfigLoader.h"
#include "ErrorHandler.h"
#include "WindowLocator.h"

// Predefined variables for "AddVariablePredef" macro
#define ERR_TYP_PREDEF m_errorTypes
#define ERR_DATA_PREDEF m_errorData
#define ERR_DATA_TYPE_PREDEF m_errorType
#define ERR_HASH_PREDEF m_errHashmap

#define AssignErrorType(ERROR_CODE, ERROR_TYPE) ERR_DATA_PREDEF[ERROR_CODE].ERR_DATA_TYPE_PREDEF = ERROR_TYPE; ERR_HASH_PREDEF[GetString(ERROR_CODE)] = ERROR_CODE
// AssignErrorType(ERROR_CODE, ERROR_TYPE) ERR_TYP_PREDEF[ERROR_CODE] = ERROR_TYPE; ERR_HASH_PREDEF[GetString(ERROR_CODE)] = ERROR_CODE
//#define AssignErrorSource(ERROR_CODE, ERROR_TYPE) ERR_TYP_PREDEF[ERROR_CODE] = ERROR_TYPE; ERR_HASH_PREDEF[GetString(ERROR_CODE)] = ERROR_CODE

ErrorHandler::ErrorHandler()
{
	m_console = nullptr;
	
	// Add the error codes to the hash map and also assign their error types in the error type array
	AssignErrorType(Undefined, Warning);
	AssignErrorType(Success, Info);
	AssignErrorType(Failure, Warning);
	AssignErrorType(Initialize_success, Info);
	AssignErrorType(Initialize_failure, Info);
	AssignErrorType(File_not_found, Warning);
	AssignErrorType(Filename_empty, Warning);
	AssignErrorType(Destroy_obj_not_found, Warning);
	AssignErrorType(Glew_failed, FatalError);
	AssignErrorType(Ifstream_failed, Warning);
	AssignErrorType(Clock_QueryFrequency, FatalError);
	AssignErrorType(Framebuffer_failed, FatalError);
	AssignErrorType(Geometrybuffer_failed, FatalError);
	AssignErrorType(GL_context_missing, Error);
	AssignErrorType(Window_handle_missing, Error);
	AssignErrorType(AssimpScene_failed, Error);
	AssignErrorType(ObjectPool_full, Warning); 
	AssignErrorType(Collision_invalid, Warning);
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
	AssignErrorType(GameObjects_missing, Error);
	AssignErrorType(Texture_not_found, Warning);
	AssignErrorType(Texture_empty, Warning);
	AssignErrorType(Invalid_num_vid_displays, Warning);
	AssignErrorType(SDL_video_init_failed, FatalError);
	AssignErrorType(SDL_vsync_failed, Warning);
	AssignErrorType(Window_creation_failed, FatalError);
	AssignErrorType(Invalid_object_id, Warning);
	AssignErrorType(Duplicate_object_id, Warning);

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
			m_console->displayMessage(m_errorTypeStrings[p_errorType] + ": " + m_errorSources[p_errorSource] + ": " + p_error + ".");
			break;
		}

		case ErrorType::Warning:
		{
			m_console->displayMessage(m_errorTypeStrings[p_errorType] + ": " + m_errorSources[p_errorSource] + ": " + p_error + ".");
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

			break;
		}

		case ErrorType::FatalError:
		{
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
	log(m_errorData[p_errorCode].m_errorType, p_errorSource, "\'" + p_objectName + "\': " + m_errorData[p_errorCode].m_errorString);
}

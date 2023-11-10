#pragma once

#include <string>

#include "EnumFactory.h"

/*   ___________________________________________________________________________________________________
	|										|					 |										|
	|										|	 ERROR TYPES:	 |										|
	|										|____________________|										|
	|																									|
	| Info		 - not actually an error, conveys a piece of information.								|
	|				User should not be notified unless in debugging.									|
	|---------------------------------------------------------------------------------------------------|
	| Warning	 - low priority error, silent and easily coped with.									|
	|				User should not be notified unless in debugging.									|
	|---------------------------------------------------------------------------------------------------|
	| Error		 - high priority error, can only be coped with by removing some functionality.			|
	|				User should be notified or asked if to continue by an error window.					|
	|---------------------------------------------------------------------------------------------------|
	| FatalError - top priority error, cannot be dealt with, thus the whole engine must be shut down.	|
	|				User should be notified with an error code.											|
	|___________________________________________________________________________________________________|
*/

#define ERROR_TYPES(Code) \
    Code(Info,) \
    Code(Warning,) \
    Code(Error,) \
	Code(FatalError, ) \
	Code(NumberOfErrorTypes, )
DECLARE_ENUM(ErrorType, ERROR_TYPES)

#define ERROR_CODES(Code) \
	/* General errors */ \
    Code(Undefined,) \
    Code(Success,) \
    Code(Failure,) \
    Code(Initialize_success,) \
    Code(Initialize_failure,) \
    Code(File_not_found,) \
    Code(Filename_empty,) \
	/* Audio system errors */\
	Code(Audio_no_drivers, ) \
	Code(Audio_system_init_failed, ) \
	/* General engine errors */\
    Code(Destroy_obj_not_found,) \
    Code(Glew_failed,) \
    Code(Ifstream_failed,) \
	/* Config Loader */\
	/* Clock errors */ \
	Code(Clock_QueryFrequency,) \
	/* Frame-buffer errors */ \
	Code(Framebuffer_failed,) \
	Code(Geometrybuffer_failed,) \
	/* GUI errors */ \
	Code(Editor_path_outside_current_dir,) \
	Code(GL_context_missing,) \
	Code(Universal_scene_extend_null,) \
	Code(Universal_scene_extend_duplicate,) \
	Code(Window_handle_missing,) \
	/* Model loader errors */ \
	Code(AssimpScene_failed,) \
	/* Object pool errors */ \
	Code(ObjectPool_full,) \
	/* Physics system errors */ \
	Code(Collision_invalid,) \
	Code(Collision_missing,) \
	Code(Kinematic_has_mass,) \
	/* Property loader errors */ \
	Code(Property_missing_size,) \
	Code(Property_missing_radius,) \
	Code(Property_missing_type,) \
	Code(Property_no_filename,) \
	/* Shader loader errors */ \
	Code(Shader_attach_failed,) \
	Code(Shader_compile_failed,) \
	Code(Shader_creation_failed,) \
	Code(Shader_link_failed,) \
	Code(Shader_loading_failed,) \
	/* Scene loader errors */ \
	Code(GameObjects_missing,) \
	/* Texture loader errors */ \
	Code(Texture_not_found,) \
	Code(Texture_empty,) \
	/* Window errors */ \
	Code(Invalid_num_vid_displays,) \
	Code(SDL_video_init_failed,) \
	Code(SDL_vsync_failed,) \
	Code(Window_creation_failed,) \
	/* World scene errors */ \
	Code(Invalid_object_id,) \
	Code(Duplicate_object_id,) \
	/* Error management */ \
	Code(NumberOfErrorCodes,) \
	Code(CachedError,)
DECLARE_ENUM(ErrorCode, ERROR_CODES)

#define ERROR_SOURCE(Code) \
    Code(Source_Unknown,) \
    Code(Source_General,) \
    Code(Source_AtmScatteringPass,) \
    Code(Source_AudioScene,) \
    Code(Source_AudioSystem,) \
    Code(Source_AudioTask,) \
    Code(Source_BloomCompositePass,) \
    Code(Source_BloomPass,) \
    Code(Source_BlurPass,) \
    Code(Source_CameraComponent,) \
    Code(Source_CallisionShapeComponent,) \
    Code(Source_Config,) \
    Code(Source_ConfigLoader,) \
    Code(Source_Engine,) \
    Code(Source_FileLoader,) \
    Code(Source_FinalPass,) \
    Code(Source_GameObject,) \
    Code(Source_GeometryBuffer,) \
    Code(Source_GeometryPass,) \
    Code(Source_GraphicsObject,) \
    Code(Source_GUI,) \
    Code(Source_GUIEditor,) \
    Code(Source_GUIObject,) \
    Code(Source_GUIPass,) \
    Code(Source_GUISequenceComponent,) \
    Code(Source_HdrMappingPass,) \
    Code(Source_LensFlareCompositePass,) \
    Code(Source_LensFlarePass,) \
    Code(Source_LightComponent,) \
    Code(Source_LightObject,) \
    Code(Source_LightingPass,) \
    Code(Source_LuaComponent,) \
    Code(Source_LuminancePass,) \
    Code(Source_ModelComponent,) \
    Code(Source_ModelLoader,) \
    Code(Source_ObjectDirectory,) \
    Code(Source_ObjectMaterialComponent,) \
    Code(Source_Physics,) \
    Code(Source_PhysicsObject,) \
    Code(Source_PlayerObject,) \
    Code(Source_PostProcessPass,) \
    Code(Source_PropertyLoader,) \
    Code(Source_ReflectionPass,) \
    Code(Source_Renderer,) \
    Code(Source_RendererScene,) \
    Code(Source_RendererSystem,) \
    Code(Source_RigidBodyComponent,) \
    Code(Source_SceneLoader,) \
    Code(Source_Script,) \
    Code(Source_ScriptObject,) \
    Code(Source_ShaderComponent,) \
    Code(Source_ShaderLoader,) \
    Code(Source_SkyObject,) \
    Code(Source_SkyPass,) \
    Code(Source_SoundComponent,) \
    Code(Source_SoundListenerComponent,) \
    Code(Source_TextureLoader,) \
    Code(Source_UniversalScene,) \
    Code(Source_Window,) \
    Code(Source_World,) \
    Code(Source_WorldScene,) \
    Code(Source_WorldSystem,) \
    Code(Source_NumberOfErrorSources,) 
DECLARE_ENUM(ErrorSource, ERROR_SOURCE)

// Holds an error code and an error message
struct ErrorMessage
{
	ErrorMessage() : m_errorCode(Success), m_errorSource(Source_Unknown) { }
	ErrorMessage(const ErrorCode p_errorCode) : m_errorCode(p_errorCode), m_errorSource(ErrorSource::Source_Unknown) { }
	ErrorMessage(const ErrorCode p_errorCode, const ErrorSource p_errorsource)
		: m_errorCode(p_errorCode), m_errorSource(p_errorsource) { }
	ErrorMessage(const ErrorCode p_errorCode, const ErrorSource p_errorsource, const std::string &p_errorMessage)
		: m_errorCode(p_errorCode), m_errorSource(p_errorsource), m_errorMessage(p_errorMessage) { }

	const inline bool containsError() const { return m_errorCode != Success; }

	ErrorCode m_errorCode;
	ErrorSource m_errorSource;
	std::string m_errorMessage;
};
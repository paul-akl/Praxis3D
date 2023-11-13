#include "AudioSystem.h"

ErrorCode AudioSystem::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_AudioSystem);

	return returnCode;
}

#pragma once

#include "GraphicsDataSets.h"
#include "GUIHandlerLocator.h"
#include "RenderPassBase.h"

class GUIPass : public RenderPass
{
public:
	GUIPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

	~GUIPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "GUI Rendering Pass";

		if(!GUIHandlerLocator::get().isInitialized())
			returnError = ErrorCode::Failure;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_FinalPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_FinalPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		GUIHandlerLocator::get().beginFrame();
		GUIHandlerLocator::get().render();
	}

private:

};
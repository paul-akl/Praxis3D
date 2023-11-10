#pragma once

#include "GraphicsDataSets.h"
#include "GUIHandlerLocator.h"
#include "RenderPassBase.h"

class GUIPass : public RenderPass
{
public:
	GUIPass(RendererFrontend &p_renderer) : RenderPass(p_renderer, RenderPassType::RenderPassType_GUI) { }

	~GUIPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "GUI Rendering Pass";

		if(!GUIHandlerLocator::get().isInitialized())
			returnError = ErrorCode::Failure;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_GUIPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_GUIPass);

		GUIHandlerLocator::get().initRendering();

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// If the GUI frame is not ready yet, wait for it to become ready
		if(!GUIHandlerLocator::get().getFrameReadyFlag().test())
			GUIHandlerLocator::get().getFrameReadyFlag().wait(false);

		// Work through the GUI functor sequence after the GUI frame is ready, so that the GUI isn't called multiple times simultaneously

		// Get GUI functor sequence
		auto GUIFunctorSequence = p_renderPassData.getGUIPassFunctorSequence();

		// Check if the sequence is present
		if(GUIFunctorSequence != nullptr)
		{
			// Get the functors
			const Functors &GUISequence = GUIFunctorSequence->getFunctors();

			// Go over each functor and call the function stored in it
			for(decltype(GUISequence.size()) i = 0, size = GUISequence.size(); i < size; i++)
				GUISequence[i]();

			// Clear the sequence after all the functions have been called
			GUIFunctorSequence->clear();
		}

		// Set the default framebuffer to be drawn to
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);

		// Render the GUI
		GUIHandlerLocator::get().render();
	}

private:

};
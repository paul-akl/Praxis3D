#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "Systems/RendererSystem/Passes/Include/RenderPassBase.hpp"

class ShadowMappingPass : public RenderPass
{
public:
	ShadowMappingPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_ShadowMapping),
		m_csmPassShader(nullptr),
		m_csmPassAlphaDiscardShader(nullptr),
		m_csmDataSetUniformBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw)
	{
		m_numOfCascades = (unsigned int)p_renderer.m_frameData.m_shadowMappingData.m_shadowCascadePlaneDistances.size();
		m_csmResolution = p_renderer.m_frameData.m_shadowMappingData.m_csmResolution;
	}

	~ShadowMappingPass() 
	{
	}

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Shadow Mapping Pass";

		const auto &shadowMappingData = m_renderer.m_frameData.m_shadowMappingData;

		// Initialize the CSM framebuffer
		m_renderer.m_backend.createFramebuffer(FramebufferType::FramebufferType_CSMBuffer, m_renderer.getFrameData());

		// Get the current number of shadow cascades
		m_numOfCascades = (unsigned int)shadowMappingData.m_shadowCascadePlaneDistances.size();

		// Create the Cascaded Shadow Mapping pass shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::Name, std::string("csmPass"));

#if CSM_USE_MULTILAYER_DRAW
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().csm_pass_layered_frag_shader);
			shaderProperties.addProperty(Properties::GeometryShader, Config::rendererVar().csm_pass_layered_geom_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().csm_pass_layered_vert_shader);
#else
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().csm_pass_single_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().csm_pass_single_vert_shader);
#endif

			// Create the shader
			m_csmPassShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_csmPassShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Disable alpha discard in the shader
				if(ErrorCode shaderVariableError = m_csmPassShader->setDefineValue(Config::shaderVar().define_alpha_discard, 0); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_alpha_discard, ErrorSource::Source_ShadowMappingPass);

				// Set the number of shadow cascades in the shader
				if(ErrorCode shaderVariableError = m_csmPassShader->setDefineValue(ShaderType::ShaderType_Geometry, Config::shaderVar().define_numOfCascades, m_numOfCascades); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfCascades, ErrorSource::Source_ShadowMappingPass);

				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_csmPassShader);
			}
			else
				returnError = shaderError;
		}

		// Create the Cascaded Shadow Mapping with Alpha Discard pass shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::Name, std::string("csmPass_AlphaDiscard"));

#if CSM_USE_MULTILAYER_DRAW
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().csm_pass_layered_frag_shader);
			shaderProperties.addProperty(Properties::GeometryShader, Config::rendererVar().csm_pass_layered_geom_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().csm_pass_layered_vert_shader);
#else
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().csm_pass_single_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().csm_pass_single_vert_shader);
#endif

			// Create the shader
			m_csmPassAlphaDiscardShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_csmPassAlphaDiscardShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Enable alpha discard in the shader
				if(ErrorCode shaderVariableError = m_csmPassAlphaDiscardShader->setDefineValue(Config::shaderVar().define_alpha_discard, 1); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_alpha_discard, ErrorSource::Source_ShadowMappingPass);

				// Set the number of shadow cascades in the shader
				if(ErrorCode shaderVariableError = m_csmPassAlphaDiscardShader->setDefineValue(ShaderType::ShaderType_Geometry, Config::shaderVar().define_numOfCascades, m_numOfCascades); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfCascades, ErrorSource::Source_ShadowMappingPass);

				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_csmPassAlphaDiscardShader);
			}
			else
				returnError = shaderError;
		}

		// Initialize CSM dataset
		m_csmDataSet.reserve(m_numOfCascades);
		updateCSMDataSet(shadowMappingData);

		// Set data for CSM buffer
		m_csmDataSetUniformBuffer.m_bindingIndex = UniformBufferBinding::UniformBufferBinding_CSMMatrixBuffer;
		m_csmDataSetUniformBuffer.m_size = sizeof(CascadedShadowMapDataSet) * m_csmDataSet.size();
		m_csmDataSetUniformBuffer.m_data = (void *)&m_csmDataSet[0];

		// Load the CSM buffer
		m_renderer.queueForLoading(m_csmDataSetUniformBuffer);

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_SSAOPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_SSAOPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// Get the current shadow mapping data
		const auto &shadowMappingData = m_renderer.m_frameData.m_shadowMappingData;

		if(p_sceneObjects.m_processDrawing && shadowMappingData.m_shadowMappingEnabled && !shadowMappingData.m_shadowCascadePlaneDistances.empty())
		{
			// If the number of shadow cascades has changed, set the new define inside the shader source and recompile the shader
			// and resize the CSM framebuffer
			if(m_numOfCascades != shadowMappingData.m_shadowCascadePlaneDistances.size())
			{
				m_numOfCascades = (unsigned int)shadowMappingData.m_shadowCascadePlaneDistances.size();

				m_renderer.m_backend.getCSMFramebuffer()->setNumOfCascades(m_numOfCascades);

				if(m_numOfCascades > 0)
				{
					if(ErrorCode shaderVariableError = m_csmPassShader->setDefineValue(ShaderType::ShaderType_Geometry, Config::shaderVar().define_numOfCascades, m_numOfCascades); shaderVariableError == ErrorCode::Success)
					{
						m_csmPassShader->resetLoadedToVideoMemoryFlag();

						// Queue the shader to be loaded to GPU
						m_renderer.queueForLoading(*m_csmPassShader);
						m_renderer.passLoadCommandsToBackend();
					}
					else
						ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfCascades, ErrorSource::Source_ShadowMappingPass);
				}
			}

			// If the number of cascades has changed, update the CSM framebuffer
			if(m_csmResolution != shadowMappingData.m_csmResolution)
			{
				m_csmResolution = shadowMappingData.m_csmResolution;
				m_renderer.m_backend.getCSMFramebuffer()->setBufferSize(m_csmResolution, m_csmResolution);
			}

			// Update the CSM data set
			updateCSMDataSet(shadowMappingData);

			// Send the CSM data set to the GPU
			m_csmDataSetUniformBuffer.m_updateSize = sizeof(CascadedShadowMapDataSet) * m_csmDataSet.size();
			m_csmDataSetUniformBuffer.m_size = sizeof(CascadedShadowMapDataSet) * m_csmDataSet.size();
			m_csmDataSetUniformBuffer.m_data = (void *)&m_csmDataSet[0];
			m_renderer.queueForUpdate(m_csmDataSetUniformBuffer);
			m_renderer.passUpdateCommandsToBackend();

			// Prepare CSM framebuffer for rendering
			m_renderer.m_backend.getCSMFramebuffer()->initFrame();

			// Enable z clipping
			if(shadowMappingData.m_zClipping)
				glEnable(GL_DEPTH_CLAMP);

#if CSM_USE_MULTILAYER_DRAW

			// Iterate over all objects to be rendered with CSM shader
			for(auto entity : p_sceneObjects.m_models)
			{
				ModelComponent &model = p_sceneObjects.m_models.get<ModelComponent>(entity);
				if(model.isObjectActive())
				{
					SpatialComponent &spatialData = p_sceneObjects.m_models.get<SpatialComponent>(entity);
					auto &modelData = model.getModelData();

					// Go over each model
					for(decltype(modelData.size()) modelIndex = 0, modelSize = modelData.size(); modelIndex < modelSize; modelIndex++)
					{
						// Calculate model-view-projection matrix
						const glm::mat4 &modelMatrix = p_sceneObjects.m_models.get<SpatialComponent>(entity).getSpatialDataChangeManager().getWorldTransformWithScale();

						// Go over each mesh
						for(decltype(modelData[modelIndex].m_model.getNumMeshes()) meshIndex = 0, meshSize = modelData[modelIndex].m_model.getNumMeshes(); meshIndex < meshSize; meshIndex++)
						{
							// Only draw active meshes
							if(modelData[modelIndex].m_meshes[meshIndex].m_active)
							{
								if(modelData[modelIndex].m_meshes[meshIndex].m_alphaThreshold > 0.0f)
								{
									// ALPHA DISCARD enabled
									m_renderer.queueForDrawing(
										modelData[modelIndex].m_model[meshIndex], 
										modelData[modelIndex].m_meshes[meshIndex], 
										modelData[modelIndex].m_model.getHandle(), 
										m_csmPassAlphaDiscardShader->getShaderHandle(), 
										m_csmPassAlphaDiscardShader->getUniformUpdater(), 
										DrawCommandTextureBinding::DrawCommandTextureBinding_DiffuseOnly,
										modelData[modelIndex].m_shadowFaceCulling, 
										modelMatrix, 
										modelMatrix);
								}
								else
								{
									// ALPHA DISCARD disabled
									m_renderer.queueForDrawing(
										modelData[modelIndex].m_model[meshIndex], 
										modelData[modelIndex].m_meshes[meshIndex], 
										modelData[modelIndex].m_model.getHandle(), 
										m_csmPassShader->getShaderHandle(), 
										m_csmPassShader->getUniformUpdater(), 
										DrawCommandTextureBinding::DrawCommandTextureBinding_None,
										modelData[modelIndex].m_shadowFaceCulling,
										modelMatrix, 
										modelMatrix);
								}
							}
						}
					}
				}
			}
#else
			for(decltype(m_csmDataSet.size()) i = 0, size = m_csmDataSet.size(); i < size; i++)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_renderer.m_backend.getCSMFramebuffer()->m_depthBuffers, 0, (GLint)i);
				glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame

				// Iterate over all objects to be rendered with CSM shader
				for(auto entity : p_sceneObjects.m_models)
				{
					ModelComponent &model = p_sceneObjects.m_models.get<ModelComponent>(entity);
					if(model.isObjectActive())
					{
						SpatialComponent &spatialData = p_sceneObjects.m_models.get<SpatialComponent>(entity);
						auto &modelData = model.getModelData();

						// Go over each model
						for(decltype(modelData.size()) modelIndex = 0, modelSize = modelData.size(); modelIndex < modelSize; modelIndex++)
						{
							// Calculate model-view-projection matrix
							const glm::mat4 &modelMatrix = m_csmDataSet[i].m_lightSpaceMatrix * p_sceneObjects.m_models.get<SpatialComponent>(entity).getSpatialDataChangeManager().getWorldTransformWithScale();

							// Go over each mesh
							for(decltype(modelData[modelIndex].m_model.getNumMeshes()) meshIndex = 0, meshSize = modelData[modelIndex].m_model.getNumMeshes(); meshIndex < meshSize; meshIndex++)
							{
								// Only draw active meshes
								if(modelData[modelIndex].m_meshes[meshIndex].m_active)
								{
									if(modelData[modelIndex].m_meshes[meshIndex].m_alphaThreshold > 0.0f)
									{
										// ALPHA DISCARD enabled
										m_renderer.queueForDrawing(
											modelData[modelIndex].m_model[meshIndex], 
											modelData[modelIndex].m_meshes[meshIndex], 
											modelData[modelIndex].m_model.getHandle(), 
											m_csmPassAlphaDiscardShader->getShaderHandle(),
											m_csmPassAlphaDiscardShader->getUniformUpdater(),
											DrawCommandTextureBinding::DrawCommandTextureBinding_DiffuseOnly,
											modelData[modelIndex].m_shadowFaceCulling,
											modelMatrix, 
											modelMatrix);
									}
									else
									{
										// ALPHA DISCARD disabled
										m_renderer.queueForDrawing(
											modelData[modelIndex].m_model[meshIndex], 
											modelData[modelIndex].m_meshes[meshIndex], 
											modelData[modelIndex].m_model.getHandle(), 
											m_csmPassShader->getShaderHandle(),
											m_csmPassShader->getUniformUpdater(),
											DrawCommandTextureBinding::DrawCommandTextureBinding_None,
											modelData[modelIndex].m_shadowFaceCulling,
											modelMatrix, 
											modelMatrix);
									}
								}
							}
						}
					}
				}

				// Pass all the draw commands to be executed
				m_renderer.passDrawCommandsToBackend();
			}
#endif

			// Iterate over all objects to be rendered with a custom shader
			//for(auto entity : p_sceneObjects.m_modelsWithShaders)
			//{
			//	ModelComponent &model = p_sceneObjects.m_modelsWithShaders.get<ModelComponent>(entity);
			//	if(model.isObjectActive())
			//	{
			//		SpatialComponent &spatialData = p_sceneObjects.m_modelsWithShaders.get<SpatialComponent>(entity);
			//		ShaderComponent &shader = p_sceneObjects.m_modelsWithShaders.get<ShaderComponent>(entity);
			//		if(shader.isLoadedToVideoMemory())
			//		{
			//			auto &modelData = model.getModelData();

			//			for(decltype(modelData.size()) i = 0, size = modelData.size(); i < size; i++)
			//			{
			//				m_renderer.queueForDrawing(modelData[i],
			//					shader.getShaderData()->m_shader.getShaderHandle(),
			//					shader.getShaderData()->m_shader.getUniformUpdater(),
			//					spatialData.getSpatialDataChangeManager().getWorldTransformWithScale(),
			//					m_renderer.m_viewProjMatrix);
			//			}
			//		}
			//	}
			//}

			// Pass all the draw commands to be executed
			m_renderer.passDrawCommandsToBackend();

			// Disable z clipping if it was turned on before
			if(shadowMappingData.m_zClipping)
				glDisable(GL_DEPTH_CLAMP);
		}
	}

private:
	void calculateSplitPositions(std::vector<float> &p_splits, const float p_numOfSplits, const float p_zNear, const float p_zFar)
	{
		// Practical split scheme:
		//
		// CLi = n*(f/n)^(i/numsplits)
		// CUi = n + (f-n)*(i/numsplits)
		// Ci = CLi*(lambda) + CUi*(1-lambda)
		//
		// lambda scales between logarithmic and uniform
		//

		const float planeSplitWeight = 0.85f;

		p_splits.clear();

		for(int i = 0; i < p_numOfSplits; i++)
		{
			float fIDM = i / (float)p_numOfSplits;
			float fLog = p_zNear * powf(p_zFar / p_zNear, fIDM);
			float fUniform = p_zNear + (p_zFar - p_zNear) * fIDM;
			p_splits.push_back(fLog * planeSplitWeight + fUniform * (1 - planeSplitWeight));
		}

		// make sure border values are accurate
		p_splits.front() = p_zNear;
		p_splits.back() = p_zFar;
	}

	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4 &p_projView)
	{
		std::vector<glm::vec4> returnFrustumCorners;

		// Get the inverse view-projection matrix required for converting position to world space
		const auto inv = glm::inverse(p_projView);

		// Go over each frustum clip plane (X, Y and Z)
		for(unsigned int x = 0; x < 2; ++x)
		{
			for(unsigned int y = 0; y < 2; ++y)
			{
				for(unsigned int z = 0; z < 2; ++z)
				{
					// Get the frustum corner position and convert it to world space
					const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					returnFrustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return returnFrustumCorners;
	}
	glm::mat4 calcLightSpaceMatrix(const ShadowMappingData &p_shadowMappingData, const float p_zNear, const float p_zFar)
	{
		// Calculate the camera projection matrix with the given z near and far clip distances
		const auto cameraProjection = glm::perspectiveFov(
			glm::radians(m_renderer.m_frameData.m_fov), 
			(float)m_renderer.m_frameData.m_screenSize.x, 
			(float)m_renderer.m_frameData.m_screenSize.y, 
			p_zNear, 
			p_zFar);

		// Get the frustum corners of the camera projection
		const auto frustumCorners = getFrustumCornersWorldSpace(cameraProjection * m_renderer.m_frameData.m_viewMatrix);

		// Calculate frustum plane center position
		glm::vec3 center = glm::vec3(0, 0, 0);
		for(const auto &position : frustumCorners)
		{
			center += glm::vec3(position);
		}
		center /= frustumCorners.size();

		// Get the direction light direction
		const glm::vec3 lightDir = glm::normalize(m_renderer.m_frameData.m_directionalLight.m_direction);

		// Calculate the light-view matrix (view matrix from the perspective of the directional light)
		const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		// Calculate the directional light orthogonal projection frustum corners
		for(const auto &position : frustumCorners)
		{
			const auto trf = lightView * position;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Scale the directional light orthogonal projection frustum z plane clip distances
		if(minZ < 0)
		{
			minZ *= p_shadowMappingData.m_csmCascadePlaneZMultiplier;
		}
		else
		{
			minZ /= p_shadowMappingData.m_csmCascadePlaneZMultiplier;
		}
		if(maxZ < 0)
		{
			maxZ /= p_shadowMappingData.m_csmCascadePlaneZMultiplier;
		}
		else
		{
			maxZ *= p_shadowMappingData.m_csmCascadePlaneZMultiplier;
		}

		// Calculate the orthogonal light projection from the given frustum corners
		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		// Calculate the complete light space matrix (light view-projection)
		return lightProjection * lightView;
	}

	glm::mat4 calcLightSpaceMatrix2(const float p_zNear, const float p_zFar)
	{
		glm::vec3 lightDir = glm::normalize(m_renderer.m_frameData.m_directionalLight.m_direction);

		//p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
		glm::mat4 Cam = m_renderer.m_frameData.m_viewProjMatrix;
		glm::mat4 CamInv = glm::inverse(m_renderer.m_frameData.m_viewProjMatrix);

		//const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));
		//p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
		glm::mat4 LightM = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), lightDir, glm::vec3(0.0f, 1.0f, 0.0f));

		float ar = (float)m_renderer.m_frameData.m_screenSize.y / (float)m_renderer.m_frameData.m_screenSize.x;
		float tanHalfHFOV = tanf(glm::radians(m_renderer.m_frameData.m_fov / 2.0f));
		float tanHalfVFOV = tanf(glm::radians((m_renderer.m_frameData.m_fov * ar) / 2.0f));

		//printf("ar %f tanHalfHFOV %f tanHalfVFOV %f\n", ar, tanHalfHFOV, tanHalfVFOV);

		//for(uint i = 0; i < NUM_CASCADES; i++)
		{
			float xn = p_zNear * tanHalfHFOV;
			float xf = p_zFar * tanHalfHFOV;
			float yn = p_zNear * tanHalfVFOV;
			float yf = p_zFar * tanHalfVFOV;

			//printf("xn %f xf %f\n", xn, xf);
			//printf("yn %f yf %f\n", yn, yf);

			glm::vec4 frustumCorners[8] = {
				// near face
				glm::vec4(xn,   yn, p_zNear, 1.0),
				glm::vec4(-xn,  yn, p_zNear, 1.0),
				glm::vec4(xn,  -yn, p_zNear, 1.0),
				glm::vec4(-xn, -yn, p_zNear, 1.0),

				// far face
				glm::vec4(xf,   yf, p_zFar, 1.0),
				glm::vec4(-xf,  yf, p_zFar, 1.0),
				glm::vec4(xf,  -yf, p_zFar, 1.0),
				glm::vec4(-xf, -yf, p_zFar, 1.0)
			};

			glm::vec4 frustumCornersL[8];

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();

			for(unsigned int j = 0; j < 8; j++)
			{
				//printf("Frustum: ");
				glm::vec4 vW = CamInv * frustumCorners[j];
				//vW.Print();
				//printf("Light space: ");
				frustumCornersL[j] = LightM * vW;
				//frustumCornersL[j].Print();
				//printf("\n");

				minX = std::min(minX, frustumCornersL[j].x);
				maxX = std::max(maxX, frustumCornersL[j].x);
				minY = std::min(minY, frustumCornersL[j].y);
				maxY = std::max(maxY, frustumCornersL[j].y);
				minZ = std::min(minZ, frustumCornersL[j].z);
				maxZ = std::max(maxZ, frustumCornersL[j].z);
			}

			//printf("BB: %f %f %f %f %f %f\n", minX, maxX, minY, maxY, minZ, maxZ);

			//m_shadowOrthoProjInfo[i].r = maxX;
			//m_shadowOrthoProjInfo[i].l = minX;
			//m_shadowOrthoProjInfo[i].b = minY;
			//m_shadowOrthoProjInfo[i].t = maxY;
			//m_shadowOrthoProjInfo[i].f = maxZ;
			//m_shadowOrthoProjInfo[i].n = minZ;

			const glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
			const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
			return lightProjection * lightView;
		}
	}

	void updateCSMDataSet(const ShadowMappingData &p_shadowMappingData)
	{
		// Clear the old data
		m_csmDataSet.clear();

		float firstPlaneZFar = 1.0f;

		if(!p_shadowMappingData.m_shadowCascadePlaneDistances.empty())
			firstPlaneZFar = p_shadowMappingData.m_shadowCascadePlaneDistances[0].m_distanceIsDivider ?
				m_renderer.getFrameData().m_zFar / p_shadowMappingData.m_shadowCascadePlaneDistances[0].m_cascadeFarDistance :
				p_shadowMappingData.m_shadowCascadePlaneDistances[0].m_cascadeFarDistance;

		const float initialPoissonSampleScale = 1.0f / (p_shadowMappingData.m_penumbraSize / 1000.0f);

		for(decltype(p_shadowMappingData.m_shadowCascadePlaneDistances.size()) i = 0, size = p_shadowMappingData.m_shadowCascadePlaneDistances.size(); i < size; i++)
		{
			// If the divider flag is set, calculate the cascade z-near distance by diving the camera's z-far distance;
			// Otherwise, set the cascade distance directly
			const float planeZFar = p_shadowMappingData.m_shadowCascadePlaneDistances[i].m_distanceIsDivider ?
				m_renderer.getFrameData().m_zFar / p_shadowMappingData.m_shadowCascadePlaneDistances[i].m_cascadeFarDistance :
				p_shadowMappingData.m_shadowCascadePlaneDistances[i].m_cascadeFarDistance;

			// Get plane z-near, which is the previous plane z-far; for the first entry, use camera z-near
			const float planeZNear = i == 0 ? m_renderer.m_frameData.m_zNear : m_csmDataSet.back().m_cascadePlaneDistance;

			const float poissonSampleScale = initialPoissonSampleScale * (planeZFar / firstPlaneZFar);

			// Add the light space matrix and cascade plane distance to the CSM dataset
			m_csmDataSet.emplace_back(
				calcLightSpaceMatrix(p_shadowMappingData, planeZNear, planeZFar), 
				planeZFar, 
				p_shadowMappingData.m_shadowCascadePlaneDistances[i].m_maxBias, 
				poissonSampleScale, 
				p_shadowMappingData.m_shadowCascadePlaneDistances[i].m_penumbraScale);
		}
	}

	// CSM settings
	unsigned int m_numOfCascades;
	unsigned int m_csmResolution;
	std::vector<std::pair<float, float>> m_csmCascade;

	// Cascaded shadow map pass shaders
	ShaderLoader::ShaderProgram *m_csmPassShader;
	ShaderLoader::ShaderProgram *m_csmPassAlphaDiscardShader;

	// CSM uniform buffer
	RendererFrontend::ShaderBuffer m_csmDataSetUniformBuffer;

	// CSM dataset
	std::vector<CascadedShadowMapDataSet> m_csmDataSet;
};

#include "ErrorHandlerLocator.h"
#include "ShaderUniformUpdater.h"

ErrorCode ShaderUniformUpdater::generateUpdateList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make sure shader handle is up to date
	m_shaderHandle = m_shader.getShaderHandle();

	returnError = generateTextureUpdateList();
	returnError = generatePerFrameList();
	returnError = generatePerModelList();
	returnError = generatePerMeshList();

	m_numUpdatesPerFrame = m_updatesPerFrame.size();
	m_numUpdatesPerModel = m_updatesPerModel.size();
	m_numUpdatesPerMesh = m_updatesPerMesh.size();
	m_numTextureUpdates = m_textureUpdates.size();

	// Check for errors, and cache it if it exists, since we are returning the error to higher layer
	if(returnError != ErrorCode::Success)
		ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, m_shader.getFilename());

	return returnError;
}

ErrorCode ShaderUniformUpdater::generateTextureUpdateList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of uniform classes and populate it
	std::vector<BaseUniform*> uniformList;

	// Framebuffer texture uniforms
	uniformList.push_back(new PositionBufferUniform(m_shaderHandle));
	uniformList.push_back(new DiffuseBufferUniform(m_shaderHandle));
	uniformList.push_back(new NormalBufferUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveBufferUniform(m_shaderHandle));
	uniformList.push_back(new BlurBufferUniform(m_shaderHandle));

	// Skydome texture uniforms
	uniformList.push_back(new SunGlowTextureUniform(m_shaderHandle));
	uniformList.push_back(new SkyMapTextureUniform(m_shaderHandle));

	// Shadow map deph texture uniforms
	uniformList.push_back(new DirShadowMapTextureUniform(m_shaderHandle));

	// Geometry pass textures
	uniformList.push_back(new DiffuseTextureUniform(m_shaderHandle));
	uniformList.push_back(new NormalTextureUniform(m_shaderHandle));
	uniformList.push_back(new SpecularTextureUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveTextureUniform(m_shaderHandle));
	uniformList.push_back(new GlossTextureUniform(m_shaderHandle));
	uniformList.push_back(new HeightTextureUniform(m_shaderHandle));
	
	// Go through each uniform and check if it is valid
	// If it is, add it to the update list, if not, delete it
	for(decltype(uniformList.size()) i = 0, size = uniformList.size(); i < size; i++)
		if(uniformList[i]->isValid())
			m_textureUpdates.push_back(uniformList[i]);
		else
			delete uniformList[i];

	return returnError;
}
ErrorCode ShaderUniformUpdater::generatePerFrameList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of uniform classes and populate it
	std::vector<BaseUniform*> uniformList;

	// View, Projection matrices
	uniformList.push_back(new ViewMatUniform(m_shaderHandle));
	uniformList.push_back(new ProjectionMatUniform(m_shaderHandle));
	uniformList.push_back(new ViewProjectionMatUniform(m_shaderHandle));
	uniformList.push_back(new DirShadowMapMVPUniform(m_shaderHandle));
	uniformList.push_back(new DirShadowMapBiasMVPUniform(m_shaderHandle));

	// Camera uniforms
	uniformList.push_back(new CameraPosVecUniform(m_shaderHandle));
	uniformList.push_back(new CameraTargetVecUniform(m_shaderHandle));

	// Distance based fog uniforms
	uniformList.push_back(new FogDensityUniform(m_shaderHandle));
	uniformList.push_back(new FogColorUniform(m_shaderHandle));

	// Directional light
	uniformList.push_back(new DirLightColorUniform(m_shaderHandle));
	uniformList.push_back(new DirLightDirectionUniform(m_shaderHandle));
	uniformList.push_back(new DirLightIntensityUniform(m_shaderHandle));
	
	// Number of lights
	uniformList.push_back(new NumPointLightsUniform(m_shaderHandle));
	uniformList.push_back(new NumSpotLightsUniform(m_shaderHandle));

	// Screen size uniform
	uniformList.push_back(new ScreenSizeUniform(m_shaderHandle));

	// Misc
	uniformList.push_back(new ElapsedTimeUniform(m_shaderHandle));
	uniformList.push_back(new GammaUniform(m_shaderHandle));
	uniformList.push_back(new ParallaxHeightScaleUniform(m_shaderHandle));

	// Go through each uniform and check if it is valid
	// If it is, add it to the update list, if not, delete it
	for(decltype(uniformList.size()) i = 0, size = uniformList.size(); i < size; i++)
		if(uniformList[i]->isValid())
			m_updatesPerFrame.push_back(uniformList[i]);
		else
			delete uniformList[i];

	return returnError;
}
ErrorCode ShaderUniformUpdater::generatePerModelList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of uniform classes and populate it
	std::vector<BaseUniform*> uniformList;

	// Model matrices
	uniformList.push_back(new ModelMatUniform(m_shaderHandle));
	uniformList.push_back(new ModelViewMatUniform(m_shaderHandle));
	uniformList.push_back(new ModelViewProjectionMatUniform(m_shaderHandle));

	// Billboard uniforms
	uniformList.push_back(new BillboardScaleUniform(m_shaderHandle));
	uniformList.push_back(new DepthTypeUniform(m_shaderHandle));

	// Miscellaneous
	uniformList.push_back(new AlphaCullingUniform(m_shaderHandle));
	uniformList.push_back(new AlphaThresholdUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveThresholdUniform(m_shaderHandle));
	uniformList.push_back(new TextureTilingFactorUniform(m_shaderHandle));

	// Test uniforms, used for debugging, etc
	uniformList.push_back(new TestMatUniform(m_shaderHandle));
	uniformList.push_back(new TestVecUniform(m_shaderHandle));
	uniformList.push_back(new TestFloatUniform(m_shaderHandle));

	// Go through each uniform and check if it is valid
	// If it is, add it to the update list, if not, delete it
	for(decltype(uniformList.size()) i = 0, size = uniformList.size(); i < size; i++)
		if(uniformList[i]->isValid())
			m_updatesPerModel.push_back(uniformList[i]);
		else
			delete uniformList[i];

	return returnError;
}
ErrorCode ShaderUniformUpdater::generatePerMeshList()
{
	return ErrorCode::Success;
}
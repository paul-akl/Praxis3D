
#include "ErrorHandlerLocator.h"
#include "ShaderUniforms.h"
#include "ShaderUniformUpdater.h"

const UniformObjectData ShaderUniformUpdater::m_defaultObjectData;
const UniformFrameData ShaderUniformUpdater::m_defaultFrameData;
const UniformData ShaderUniformUpdater::m_defaultUniformData = UniformData(ShaderUniformUpdater::m_defaultObjectData, ShaderUniformUpdater::m_defaultFrameData);

ErrorCode ShaderUniformUpdater::generateUpdateList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make sure shader handle is up to date
	m_shaderHandle = m_shader.getShaderHandle();

	returnError = generateTextureUpdateList();
	returnError = generatePerFrameList();
	returnError = generatePerModelList();
	returnError = generatePerMeshList();
	returnError = generateUniformBlockList();
	returnError = generateSSBBlockList();

	m_numUpdatesPerFrame = m_updatesPerFrame.size();
	m_numUpdatesPerModel = m_updatesPerModel.size();
	m_numUpdatesPerMesh = m_updatesPerMesh.size();
	m_numTextureUpdates = m_textureUpdates.size();
	m_numUniformBlockUpdates = m_uniformBlockUpdates.size();
	m_numSSBBBlockUpdates = m_SSBBlockUpdates.size();

	// Check for errors, and cache it if it exists, since we are returning the error to higher layer
	if(returnError != ErrorCode::Success)
		ErrHandlerLoc::get().log(returnError, ErrorSource::Source_ShaderLoader, m_shader.getCombinedFilename());

	return returnError;
}

ErrorCode ShaderUniformUpdater::generateTextureUpdateList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of uniform classes and populate it
	std::vector<BaseUniform*> uniformList;

	// Framebuffer texture uniforms
	uniformList.push_back(new PositionMapUniform(m_shaderHandle));
	uniformList.push_back(new DiffuseMapUniform(m_shaderHandle));
	uniformList.push_back(new NormalMapUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveMapUniform(m_shaderHandle));
	uniformList.push_back(new MatPropertiesMapUniform(m_shaderHandle));
	uniformList.push_back(new IntermediateMapUniform(m_shaderHandle));
	uniformList.push_back(new FinalMapUniform(m_shaderHandle));
	uniformList.push_back(new InputMapUniform(m_shaderHandle));
	uniformList.push_back(new OutputMapUniform(m_shaderHandle));

	// Cubemap texture uniforms
	uniformList.push_back(new DynamicEnvironmentMapUniform(m_shaderHandle));
	uniformList.push_back(new StaticEnvironmentMapUniform(m_shaderHandle));
	
	// Skydome texture uniforms
	uniformList.push_back(new SunGlowTextureUniform(m_shaderHandle));
	uniformList.push_back(new SkyMapTextureUniform(m_shaderHandle));

	// Shadow map depth texture uniforms
	uniformList.push_back(new DirShadowMapTextureUniform(m_shaderHandle));

	// Geometry pass textures
	uniformList.push_back(new DiffuseTextureUniform(m_shaderHandle));
	uniformList.push_back(new NormalTextureUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveTextureUniform(m_shaderHandle));
	uniformList.push_back(new CombinedTextureUniform(m_shaderHandle));
		
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
	uniformList.push_back(new DeltaTimeMSUniform(m_shaderHandle));
	uniformList.push_back(new DeltaTimeSUniform(m_shaderHandle));
	uniformList.push_back(new ElapsedTimeUniform(m_shaderHandle));
	uniformList.push_back(new GammaUniform(m_shaderHandle));
	uniformList.push_back(new EyeAdaptionRateUniform(m_shaderHandle));
	uniformList.push_back(new EyeAdaptionIntendedBrightnessUniform(m_shaderHandle));
	uniformList.push_back(new EmissiveMultiplierUniform(m_shaderHandle));
	uniformList.push_back(new LODParallaxMappingUniform(m_shaderHandle));

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
	uniformList.push_back(new HeightScaleUniform(m_shaderHandle));
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
ErrorCode ShaderUniformUpdater::generateUniformBlockList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of uniform classes and populate it
	std::vector<BaseUniformBlock*> uniformBlockList;

	// Light buffers
	uniformBlockList.push_back(new PointLightBufferUniform(m_shaderHandle));
	uniformBlockList.push_back(new SpotLightBufferUniform(m_shaderHandle));

	// Go through each uniform and check if it is valid
	// If it is, add it to the update list, if not, delete it
	for(decltype(uniformBlockList.size()) i = 0, size = uniformBlockList.size(); i < size; i++)
		if(uniformBlockList[i]->isValid())
			m_uniformBlockUpdates.push_back(uniformBlockList[i]);
		else
			delete uniformBlockList[i];

	return returnError;
}
ErrorCode ShaderUniformUpdater::generateSSBBlockList()
{
	ErrorCode returnError = ErrorCode::Success;

	// Make a vector of SSBO classes and populate it
	std::vector<BaseShaderStorageBlock*> SSBBlockList;

	// HDR SSBO
	SSBBlockList.push_back(new HDRShaderStorageBuffer(m_shaderHandle));

	// Go through each uniform and check if it is valid
	// If it is, add it to the update list, if not, delete it
	for(decltype(SSBBlockList.size()) i = 0, size = SSBBlockList.size(); i < size; i++)
		if(SSBBlockList[i]->isValid())
			m_SSBBlockUpdates.push_back(SSBBlockList[i]);
		else
			delete SSBBlockList[i];

	return returnError;
}

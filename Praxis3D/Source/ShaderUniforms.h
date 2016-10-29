#pragma once

#include <GL/glew.h>
#include <string>

#include "Config.h"
#include "Renderer.h"

// A class (derived from BaseUniform) per each uniform in a shader.
// Designed to be adaptive, and update all the uniforms that are being used in a shader.
// This way, one can modify a shader without the need to recompile the whole engine.

class BaseUniform
{
public:
	BaseUniform(std::string p_name, unsigned int p_shaderHandle) : m_name(p_name)
	{
		m_uniformHandle = glGetUniformLocation(p_shaderHandle, p_name.c_str());
	}

	const inline bool isValid() { return (m_uniformHandle != -1); }

	virtual void update(RendererState &p_rendererState) = 0;

protected:
	std::string m_name;
	unsigned int m_uniformHandle;
};

class ModelMatUniform : public BaseUniform
{
public:
	ModelMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelMatrix().m[0]);
	}
};
class ViewMatUniform : public BaseUniform
{
public:
	ViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getViewMatrix().m[0]);
	}
};
class ProjectionMatUniform : public BaseUniform
{
public:
	ProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().projectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getProjectionMatrix().m[0]);
	}
};
class ViewProjectionMatUniform : public BaseUniform
{
public:
	ViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewProjectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getViewProjMatrix().m[0]);
	}
};
class ModelViewMatUniform : public BaseUniform
{
public:
	ModelViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelViewMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewMatrix().m[0]);
	}
};
class ModelViewProjectionMatUniform : public BaseUniform
{
public:
	ModelViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelViewProjectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};

class ScreenSizeUniform : public BaseUniform
{
public:
	ScreenSizeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().screenSizeUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &screenSize = p_rendererState.getScreenSize();
		glUniform2i(m_uniformHandle, screenSize.x, screenSize.y);
	}
};
class ElapsedTimeUniform : public BaseUniform
{
public:
	ElapsedTimeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().elapsedTimeUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1f(m_uniformHandle, p_rendererState.getElapsedTime());
	}
};
class GammaUniform : public BaseUniform
{
public:
	GammaUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().gammaUniform, p_shaderHandle), m_currentGamma(0.0f) { }

	void update(RendererState &p_rendererState)
	{
		// Gamma will rarely be modified, so checking two floats will be faster than updating a uniform
		if(m_currentGamma != Config::graphicsVar().gamma)
		{
			glUniform1f(m_uniformHandle, Config::graphicsVar().gamma);
			m_currentGamma = Config::graphicsVar().gamma;
		}
	}

private:
	float m_currentGamma;
};
class AlphaCullingUniform : public BaseUniform
{
public:
	AlphaCullingUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().alphaCullingUniform, p_shaderHandle), m_currentCullingState(false) { }

	void update(RendererState &p_rendererState)
	{
		float alphaThreshold = p_rendererState.getAlphaThreshold();

		bool state = p_rendererState.getAlphaThreshold() > 0.0 ? true : false;

		if(m_currentCullingState != state)
		{
			if(state)
				glUniform1i(m_uniformHandle, 1);
			else
				glUniform1i(m_uniformHandle, 0);

			m_currentCullingState = state;
		}
	}

private:
	bool m_currentCullingState;
};
class AlphaThresholdUniform : public BaseUniform
{
public:
	AlphaThresholdUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().alphaThresholdUniform, p_shaderHandle), m_currentAlphaThreshold(-1.0) { }

	void update(RendererState &p_rendererState)
	{
		float alphaThreshold = p_rendererState.getAlphaThreshold();

		if(m_currentAlphaThreshold != alphaThreshold)
		{
			glUniform1f(m_uniformHandle, alphaThreshold);

			m_currentAlphaThreshold = alphaThreshold;
		}
	}

private:
	float m_currentAlphaThreshold;
};
class EmissiveThresholdUniform : public BaseUniform
{
public:
	EmissiveThresholdUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveThresholdUniform, p_shaderHandle), m_currentEmissiveThreshold(-1.0) { }

	void update(RendererState &p_rendererState)
	{
		float emissiveThreshold = p_rendererState.getEmissiveThreshold();

		if(m_currentEmissiveThreshold != emissiveThreshold)
		{
			glUniform1f(m_uniformHandle, emissiveThreshold);

			m_currentEmissiveThreshold = emissiveThreshold;
		}
	}

private:
	float m_currentEmissiveThreshold;
};
class ParallaxHeightScaleUniform : public BaseUniform
{
public:
	ParallaxHeightScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().parallaxHeightScale, p_shaderHandle), m_currentHeightScale(-1.0) { }

	void update(RendererState &p_rendererState)
	{
		if(m_currentHeightScale != Config::graphicsVar().parallax_height_scale)
		{
			m_currentHeightScale = Config::graphicsVar().parallax_height_scale;

			glUniform1f(m_uniformHandle, m_currentHeightScale);
		}
	}

private:
	float m_currentHeightScale;
};
class TextureTilingFactorUniform : public BaseUniform
{
public:
	TextureTilingFactorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().textureTilingFactorUniform, p_shaderHandle), m_currentTexTillingFactor(0.0) { }

	void update(RendererState &p_rendererState)
	{
		float texTilingFactor = p_rendererState.getTextureTilingFactor();

		if(m_currentTexTillingFactor != texTilingFactor)
		{
			glUniform1f(m_uniformHandle, texTilingFactor);

			m_currentTexTillingFactor = texTilingFactor;
		}
	}

private:
	float m_currentTexTillingFactor;
};

class DirLightColorUniform : public BaseUniform
{
public:
	DirLightColorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightColor, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &cameraPosVec = p_rendererState.getDirLightColor();
		glUniform3f(m_uniformHandle, cameraPosVec.x, cameraPosVec.y, cameraPosVec.z);
	}
};
class DirLightDirectionUniform : public BaseUniform
{
public:
	DirLightDirectionUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightDirection, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto cameraPosVec = p_rendererState.getDirLightDirection();
		// Normalize the direction here one per frame instead of once per fragment in the shader
		cameraPosVec.normalize();
		glUniform3f(m_uniformHandle, cameraPosVec.x, cameraPosVec.y, cameraPosVec.z);
	}
};
class DirLightIntensityUniform : public BaseUniform
{
public:
	DirLightIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightIntensity, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1f(m_uniformHandle, p_rendererState.getDirLightintensity());
	}
};
class NumPointLightsUniform : public BaseUniform
{
public:
	NumPointLightsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().numPointLightsUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getNumPointLights());
	}
};
class NumSpotLightsUniform : public BaseUniform
{
public:
	NumSpotLightsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().numSpotLightsUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getNumSpotLights());
	}
};
class PointLightViewProjectionMatUniform : public BaseUniform
{
public:
	PointLightViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().pointLightViewProjectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};
class SpotLightViewProjectionMatUniform : public BaseUniform
{
public:
	SpotLightViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().spotLightViewProjectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};
class StencilPassViewProjectionMatUniform : public BaseUniform
{
public:
	StencilPassViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().stencilPassViewProjectionMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};

class DirShadowMapMVPUniform : public BaseUniform
{
public:
	DirShadowMapMVPUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapMVPUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};
class DirShadowMapBiasMVPUniform : public BaseUniform
{
public:
	DirShadowMapBiasMVPUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapBiasMVPUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getModelViewProjMatrix().m[0]);
	}
};

class CameraPosVecUniform : public BaseUniform
{
public:
	CameraPosVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraPosVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &cameraPosVec = p_rendererState.getCameraPosition();
		glUniform3f(m_uniformHandle, cameraPosVec.x, cameraPosVec.y, cameraPosVec.z);
	}
};
class CameraTargetVecUniform : public BaseUniform
{
public:
	CameraTargetVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraTargetVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &cameraPosVec = p_rendererState.getCameraTarget();
		glUniform3f(m_uniformHandle, cameraPosVec.x, cameraPosVec.y, cameraPosVec.z);
	}
};
class CameraUpVecUniform : public BaseUniform // Unused
{
public:
	CameraUpVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraUpVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		//auto &cameraUpVec = p_rendererState.getCameraUpVec();
		//glUniform3f(m_uniformHandle, cameraUpVec.x, cameraUpVec.y, cameraUpVec.z);
	}
};
class CameraRightVecUniform : public BaseUniform // Unused
{
public:
	CameraRightVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraRightVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		//auto &cameraRightVec = p_rendererState.getCameraRightVec();
		//glUniform3f(m_uniformHandle, cameraRightVec.x, cameraRightVec.y, cameraRightVec.z);
	}
};
class CameraAngleUniform : public BaseUniform
{
public:
	CameraAngleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraPosVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &cameraAngle = p_rendererState.getCameraAngle();
		glUniform2f(m_uniformHandle, cameraAngle.x, cameraAngle.y);
	}
};

class PositionBufferUniform : public BaseUniform
{
public:
	PositionBufferUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().positionMapUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getPositionBufferPos());
	}
};
class DiffuseBufferUniform : public BaseUniform
{
public:
	DiffuseBufferUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseMapUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getDiffuseBufferPos());
	}
};
class NormalBufferUniform : public BaseUniform
{
public:
	NormalBufferUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalMapUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getNormalBufferPos());
	}
};
class EmissiveBufferUniform : public BaseUniform
{
public:
	EmissiveBufferUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveMapUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getEmissiveBufferPos());
	}
};
class BlurBufferUniform : public BaseUniform
{
public:
	BlurBufferUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().blurMapUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getBlurBufferPos());
	}
};

class SunGlowTextureUniform : public BaseUniform
{
public:
	SunGlowTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().sunGlowTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getDiffuseTexturePos());
	}
};
class SkyMapTextureUniform : public BaseUniform
{
public:
	SkyMapTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().skyMapTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getDiffuseTexturePos());
	}
};
class DirShadowMapTextureUniform : public BaseUniform
{
public:
	DirShadowMapTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getDiffuseTexturePos());
	}
};
class DiffuseTextureUniform : public BaseUniform
{
public:
	DiffuseTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getDiffuseTexturePos());
	}
};
class NormalTextureUniform : public BaseUniform
{
public:
	NormalTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getNormalTexturePos());
	}
};
class SpecularTextureUniform : public BaseUniform
{
public:
	SpecularTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().specularTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getSpecularTexturePos());
	}
};
class EmissiveTextureUniform : public BaseUniform
{
public:
	EmissiveTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getEmissiveTexturePos());
	}
};
class GlossTextureUniform : public BaseUniform
{
public:
	GlossTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().glossTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getGlossTexturePos());
	}
};
class HeightTextureUniform : public BaseUniform
{
public:
	HeightTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().heightTextureUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, p_rendererState.getHeightTexturePos());
	}
};

class FogDensityUniform : public BaseUniform
{
public:
	FogDensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().fogDensityUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1f(m_uniformHandle, p_rendererState.getFogDensity());
	}
};
class FogColorUniform : public BaseUniform
{
public:
	FogColorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().fogColorUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &fogColor = p_rendererState.getFogColor();
		glUniform3f(m_uniformHandle, fogColor.x, fogColor.y, fogColor.z);
	}
};

class BillboardScaleUniform : public BaseUniform
{
public:
	BillboardScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().billboardScaleUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1f(m_uniformHandle, 1.0f);
	}
};
class DepthTypeUniform : public BaseUniform
{
public:
	DepthTypeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().depthTypeUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1i(m_uniformHandle, 0);
	}
};

class TestMatUniform : public BaseUniform
{
public:
	TestMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testMatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_rendererState.getTestMat().m[0]);
	}
};
class TestVecUniform : public BaseUniform
{
public:
	TestVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testVecUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		auto &testVec = p_rendererState.getTestVec();
		glUniform4f(m_uniformHandle, testVec.x, testVec.y, testVec.z, testVec.w);
	}
};
class TestFloatUniform : public BaseUniform
{
public:
	TestFloatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testFloatUniform, p_shaderHandle) { }

	void update(RendererState &p_rendererState)
	{
		glUniform1f(m_uniformHandle, p_rendererState.getTestFloat());
	}
};

/*modelMatUniform
viewMatUniform
projectionMatUniform
ViewProjectionMatUniform
ModelViewMatUniform
ModelViewProjectionMatUniform
ScreenSizeUniform
NumPointLightsUniform
NumSpotLightsUniform
PointLightViewProjectionMatUniform
SpotLightViewProjectionMatUniform
StencilPassViewProjectionMatUniform
DirLightMatUniform
DirLightProjectionMatUniform*/

/*directionalLightBaseColorUniform
directionalLightBaseAmbientIntensityUniform
directionalLightDirectionUniform
directionalLightDiffuseIntensityUniform
directionalLightMinGradientUniform

pointLightBaseColorUniform
pointLightAmbientIntensityUniform
pointLightPositionUniform
pointLightBaseDiffuseIntensityUniform
pointLightAttenuationConstantUniform
pointLightAttenuationLinearUniform
pointLightAttenuationExponentialUniform

spotLightBaseColorUniform
spotLightPositionUniform
spotLightAmbientIntensityUniform
spotLightBaseDiffuseIntensityUniform
spotLightAttenuationConstantUniform
spotLightAttenuationLinearUniform
spotLightAttenuationExponentialUniform
spotLightDirectionUniform
spotLightCutoffUniform*/

/*DirShadowMapMVPUniform
DirShadowMapBiasMVPUniform

CameraPosVecUniform
CameraUpVecUniform
CameraRightVecUniform

PositionMapUniform
DiffuseMapUniform
NormalMapUniform
EmissiveMapUniform
BlurMapUniform

SunGlowTextureUniform
SkyMapTextureUniform
DirShadowMapTextureUniform
DiffuseTextureUniform
NormalTextureUniform
SpecularTextureUniform
EmissiveTextureUniform

FogDensityUniform
FogColorUniform
BillboardScaleUniform
DepthTypeUniform*/
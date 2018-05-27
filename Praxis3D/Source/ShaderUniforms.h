#pragma once

#include <GL/glew.h>
#include <string>

#include "ClockLocator.h"
#include "CommonDefinitions.h"
#include "Config.h"
#include "GeometryBuffer.h"
#include "UniformData.h"

// A class (derived from BaseUniform) per each uniform in a shader.
// Designed to be adaptive, and update all the uniforms that are being used in a shader.
// This way, one can modify a shader without the need to recompile the whole engine.

class BaseUniform
{
public:
	BaseUniform(const std::string &p_name, const unsigned int p_shaderHandle) : m_name(p_name)
	{
		// Get uniform location (returns -1 in case it is not present in the shader)
		m_uniformHandle = glGetUniformLocation(p_shaderHandle, p_name.c_str());
	}

	// Returns true if the uniform is present in the shader
	const inline bool isValid() const { return (m_uniformHandle != -1); }

	// Updates the uniform
	virtual void update(const UniformData &p_uniformData) = 0;

protected:
	const std::string m_name;
	unsigned int m_uniformHandle;
};
class BaseUniformBlock
{
public:
	BaseUniformBlock(const std::string &p_name, const unsigned int p_shaderHandle)
		: m_name(p_name), m_shaderHandle(p_shaderHandle)
	{
		// Get the uniform location (returns -1 in case it is not present in the shader)
		m_uniformHandle = glGetUniformBlockIndex(m_shaderHandle, p_name.c_str());
	}

	// Returns true if the uniform is present in the shader
	const inline bool isValid() const { return (m_uniformHandle != -1); }
	const inline int getBlockSize() const 
	{ 
		int blockSize = 0;

		// Get the uniform block size
		glGetActiveUniformBlockiv(m_shaderHandle, m_uniformHandle, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

		return blockSize;
	}

	// Updates the uniform block binding index
	virtual void update(const UniformData &p_uniformData) = 0;

protected:
	const inline void updateBlockBinding(const unsigned int p_bindingPoint) const
	{
		// Bind the uniform buffer at the specified binding point
		glUniformBlockBinding(m_shaderHandle, m_uniformHandle, p_bindingPoint);
	}

	const std::string m_name;
	unsigned int m_uniformHandle;
	const unsigned int m_shaderHandle;
}; 
class BaseShaderStorageBlock
{
public:
	BaseShaderStorageBlock(const std::string &p_name, const unsigned int p_shaderHandle)
		: m_name(p_name), m_shaderHandle(p_shaderHandle)
	{
		// Get the SSBO location (returns -1 in case it is not present in the shader)
		m_SSBOHandle = glGetProgramResourceIndex(m_shaderHandle, GL_SHADER_STORAGE_BLOCK, p_name.c_str());
	}

	// Returns true if the uniform is present in the shader
	const inline bool isValid() const { return (m_SSBOHandle != -1); }
	/*const inline int getBlockSize() const
	{
		int blockSize = 0;

		// Get the uniform block size
		glGetActiveUniformBlockiv(m_shaderHandle, m_SSBOHandle, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

		return blockSize;
	}*/

	// Updates the uniform block binding index
	virtual void update(const UniformData &p_uniformData) = 0;

protected:
	const inline void updateBlockBinding(const unsigned int p_bindingPoint) const
	{
		// Bind the uniform buffer at the specified binding point
		glUniformBlockBinding(m_shaderHandle, m_SSBOHandle, p_bindingPoint);
	}

	const std::string m_name;
	unsigned int m_SSBOHandle;
	const unsigned int m_shaderHandle;
};

class AtmScatProjMatUniform : public BaseUniform
{
public:
	AtmScatProjMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().atmScatProjMatUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_TRUE, &p_uniformData.m_frameData.m_atmScatProjMatrix.m[0]);
	}
};
class ModelMatUniform : public BaseUniform
{
public:
	ModelMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_objectData.m_modelMat.m[0]);
	}
};
class ViewMatUniform : public BaseUniform
{
public:
	ViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_viewMatrix.m[0]);
	}
};
class ProjectionMatUniform : public BaseUniform
{
public:
	ProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().projectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_projMatrix.m[0]);
	}
};
class ViewProjectionMatUniform : public BaseUniform
{
public:
	ViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_viewProjMatrix.m[0]);
	}
};
class ModelViewMatUniform : public BaseUniform
{
public:
	ModelViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelViewMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		// TODO
		// Quick hack, for convenience when testing, should not be used, because it's slow
		Math::Mat4f modelViewMat = p_uniformData.m_frameData.m_viewMatrix * p_uniformData.m_objectData.m_modelMat;
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &modelViewMat.m[0]);
	}
};
class ModelViewProjectionMatUniform : public BaseUniform
{
public:
	ModelViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelViewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		// TODO
		// Quick hack, for convenience when testing, should not be used, because it's slow
		Math::Mat4f MVP = p_uniformData.m_frameData.m_projMatrix * p_uniformData.m_frameData.m_viewMatrix * p_uniformData.m_objectData.m_modelMat;
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &MVP.m[0]);
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_objectData.m_modelViewProjMatrix.m[0]);
	}
};
class TransposeViewMatUniform : public BaseUniform
{
public:
	TransposeViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().transposeViewMatUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_transposeViewMatrix.m[0]);
	}
};

class ScreenSizeUniform : public BaseUniform
{
public:
	ScreenSizeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().screenSizeUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform2i(m_uniformHandle, 
					p_uniformData.m_frameData.m_screenSize.x, 
					p_uniformData.m_frameData.m_screenSize.y);
	}
};
class DeltaTimeMSUniform : public BaseUniform
{
public:
	DeltaTimeMSUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().deltaTimeMSUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1f(m_uniformHandle, ClockLocator::get().getDeltaMSF());
	}
};
class DeltaTimeSUniform : public BaseUniform
{
public:
	DeltaTimeSUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().deltaTimeSUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1f(m_uniformHandle, ClockLocator::get().getDeltaSecondsF());
	}
};
class ElapsedTimeUniform : public BaseUniform
{
public:
	ElapsedTimeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().elapsedTimeUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1f(m_uniformHandle, ClockLocator::get().getElapsedSecondsF());
	}
};
class GammaUniform : public BaseUniform
{
public:
	GammaUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().gammaUniform, p_shaderHandle), m_currentGamma(0.0f) { }

	void update(const UniformData &p_uniformData)
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

	void update(const UniformData &p_uniformData)
	{
		bool state = p_uniformData.m_objectData.m_alphaThreshold > 0.0 ? true : false;

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

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentAlphaThreshold != p_uniformData.m_objectData.m_alphaThreshold)
		{
			m_currentAlphaThreshold = p_uniformData.m_objectData.m_alphaThreshold;

			glUniform1f(m_uniformHandle, m_currentAlphaThreshold);
		}
	}

private:
	float m_currentAlphaThreshold;
};
class EmissiveMultiplierUniform : public BaseUniform
{
public:
	EmissiveMultiplierUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveMultiplierUniform, p_shaderHandle), m_currentEmissiveMultiplier(-1.0) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentEmissiveMultiplier != Config::graphicsVar().emissive_multiplier)
		{
			m_currentEmissiveMultiplier = Config::graphicsVar().emissive_multiplier;

			glUniform1f(m_uniformHandle, m_currentEmissiveMultiplier);
		}
	}

private:
	float m_currentEmissiveMultiplier;
};
class EmissiveThresholdUniform : public BaseUniform
{
public:
	EmissiveThresholdUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveThresholdUniform, p_shaderHandle), m_currentEmissiveThreshold(-1.0) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentEmissiveThreshold != p_uniformData.m_objectData.m_emissiveThreshold)
		{
			m_currentEmissiveThreshold = p_uniformData.m_objectData.m_emissiveThreshold;

			glUniform1f(m_uniformHandle, m_currentEmissiveThreshold);
		}
	}

private:
	float m_currentEmissiveThreshold;
};
class HeightScaleUniform : public BaseUniform
{
public:
	HeightScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().heightScaleUniform, p_shaderHandle), m_currentHeightScale(-1.0) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentHeightScale != p_uniformData.m_objectData.m_heightScale)
		{
			m_currentHeightScale = p_uniformData.m_objectData.m_heightScale;

			glUniform1f(m_uniformHandle, m_currentHeightScale);
		}
	}

private:
	float m_currentHeightScale;
};
class TextureTilingFactorUniform : public BaseUniform
{
public:
	TextureTilingFactorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().textureTilingFactorUniform, p_shaderHandle), m_currentTexTillingFactor(0.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentTexTillingFactor != p_uniformData.m_objectData.m_textureTilingFactor)
		{
			m_currentTexTillingFactor = p_uniformData.m_objectData.m_textureTilingFactor;

			glUniform1f(m_uniformHandle, m_currentTexTillingFactor);
		}
	}

private:
	float m_currentTexTillingFactor;
}; 
class EyeAdaptionRateUniform : public BaseUniform
{
public:
	EyeAdaptionRateUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().eyeAdaptionRateUniform, p_shaderHandle), eyeAdaptionRateUniform(-1.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(eyeAdaptionRateUniform != Config::graphicsVar().eye_adaption_rate)
		{
			eyeAdaptionRateUniform = Config::graphicsVar().eye_adaption_rate;

			glUniform1f(m_uniformHandle, eyeAdaptionRateUniform);
		}
	}

private:
	float eyeAdaptionRateUniform;
};
class EyeAdaptionIntendedBrightnessUniform : public BaseUniform
{
public:
	EyeAdaptionIntendedBrightnessUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().eyeAdaptionIntBrightnessUniform, p_shaderHandle), eyeAdaptionIntendedBrightnessUniform(-1.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(eyeAdaptionIntendedBrightnessUniform != Config::graphicsVar().eye_adaption_intended_brightness)
		{
			eyeAdaptionIntendedBrightnessUniform = Config::graphicsVar().eye_adaption_intended_brightness;

			glUniform1f(m_uniformHandle, eyeAdaptionIntendedBrightnessUniform);
		}
	}

private:
	float eyeAdaptionIntendedBrightnessUniform;
};
class LODParallaxMappingUniform : public BaseUniform
{
public:
	LODParallaxMappingUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().LODParallaxUniform, p_shaderHandle), parallaxLOD(-1.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(parallaxLOD != Config::graphicsVar().LOD_prallax_mapping)
		{
			parallaxLOD = Config::graphicsVar().LOD_prallax_mapping;

			glUniform1f(m_uniformHandle, parallaxLOD);
		}
	}

private:
	float parallaxLOD;
};

class DirLightColorUniform : public BaseUniform
{
public:
	DirLightColorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightColor, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		auto &dirLightColor = p_uniformData.m_frameData.m_dirLightColor;
		glUniform3f(m_uniformHandle, dirLightColor.x, dirLightColor.y, dirLightColor.z);
	}
};
class DirLightDirectionUniform : public BaseUniform
{
public:
	DirLightDirectionUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightDirection, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		auto lightDirection = p_uniformData.m_frameData.m_dirLightDirection;
		glUniform3f(m_uniformHandle, lightDirection.x, lightDirection.y, lightDirection.z);
	}
};
class DirLightIntensityUniform : public BaseUniform
{
public:
	DirLightIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightIntensity, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1f(m_uniformHandle, p_uniformData.m_frameData.m_dirLightIntensity);
	}
};
class NumPointLightsUniform : public BaseUniform
{
public:
	NumPointLightsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().numPointLightsUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, p_uniformData.m_frameData.m_numPointLights);
	}
};
class NumSpotLightsUniform : public BaseUniform
{
public:
	NumSpotLightsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().numSpotLightsUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, p_uniformData.m_frameData.m_numSpotLights);
	}
};
/* Unused */ class PointLightViewProjectionMatUniform : public BaseUniform
{
public:
	PointLightViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().pointLightViewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getModelViewProjMatrix().m[0]);
	}
};
/* Unused */ class SpotLightViewProjectionMatUniform : public BaseUniform
{
public:
	SpotLightViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().spotLightViewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getModelViewProjMatrix().m[0]);
	}
};
/* Unused */ class StencilPassViewProjectionMatUniform : public BaseUniform
{
public:
	StencilPassViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().stencilPassViewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getModelViewProjMatrix().m[0]);
	}
};

/* Unused */ class DirShadowMapMVPUniform : public BaseUniform
{
public:
	DirShadowMapMVPUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapMVPUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getModelViewProjMatrix().m[0]);
	}
};
/* Unused */ class DirShadowMapBiasMVPUniform : public BaseUniform
{
public:
	DirShadowMapBiasMVPUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapBiasMVPUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getModelViewProjMatrix().m[0]);
	}
};

class CameraPosVecUniform : public BaseUniform
{
public:
	CameraPosVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraPosVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform3f(m_uniformHandle, 
					p_uniformData.m_frameData.m_cameraPosition.x, 
					p_uniformData.m_frameData.m_cameraPosition.y,
					p_uniformData.m_frameData.m_cameraPosition.z);
	}
};
class CameraTargetVecUniform : public BaseUniform
{
public:
	CameraTargetVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraTargetVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform3f(m_uniformHandle,
			p_uniformData.m_frameData.m_cameraTarget.x,
			p_uniformData.m_frameData.m_cameraTarget.y,
			p_uniformData.m_frameData.m_cameraTarget.z);
	}
};
/* Unused */ class CameraUpVecUniform : public BaseUniform // Unused
{
public:
	CameraUpVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraUpVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//auto &cameraUpVec = p_uniformData.getCameraUpVec();
		//glUniform3f(m_uniformHandle, cameraUpVec.x, cameraUpVec.y, cameraUpVec.z);
	}
};
/* Unused */ class CameraRightVecUniform : public BaseUniform // Unused
{
public:
	CameraRightVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraRightVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//auto &cameraRightVec = p_uniformData.getCameraRightVec();
		//glUniform3f(m_uniformHandle, cameraRightVec.x, cameraRightVec.y, cameraRightVec.z);
	}
};
/* Unused */ class CameraAngleUniform : public BaseUniform
{
public:
	CameraAngleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().cameraPosVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//auto &cameraAngle = p_uniformData.getCameraAngle();
		//glUniform2f(m_uniformHandle, cameraAngle.x, cameraAngle.y);
	}
};

class PositionMapUniform : public BaseUniform
{
public:
	PositionMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().positionMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferPosition);
	}
};
class DiffuseMapUniform : public BaseUniform
{
public:
	DiffuseMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferDiffuse);
	}
};
class NormalMapUniform : public BaseUniform
{
public:
	NormalMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferNormal);
	}
};
class EmissiveMapUniform : public BaseUniform
{
public:
	EmissiveMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferEmissive);
	}
};
class MatPropertiesMapUniform : public BaseUniform
{
public:
	MatPropertiesMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().matPropertiesMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferMatProperties);
	}
};
class IntermediateMapUniform : public BaseUniform
{
public:
	IntermediateMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().intermediateMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferIntermediate);
	}
};
class FinalMapUniform : public BaseUniform
{
public:
	FinalMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().finalMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferFinal);
	}
};
class InputMapUniform : public BaseUniform
{
public:
	InputMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().inputColorMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GBufferInputTexture);
	}
};
class OutputMapUniform : public BaseUniform
{
public:
	OutputMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().outputColorMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GeometryBuffer::GBufferTextureType::GbufferOutputTexture);
	}
};

/* Unused */ class SunGlowTextureUniform : public BaseUniform
{
public:
	SunGlowTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().sunGlowTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getDiffuseTexturePos());
	}
};
/* Unused */ class SkyMapTextureUniform : public BaseUniform
{
public:
	SkyMapTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().skyMapTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getDiffuseTexturePos());
	}
};
/* Unused */ class DirShadowMapTextureUniform : public BaseUniform
{
public:
	DirShadowMapTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirShadowMapTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getDiffuseTexturePos());
	}
};
class DiffuseTextureUniform : public BaseUniform
{
public:
	DiffuseTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getDiffuseTexturePos());
		glUniform1i(m_uniformHandle, MaterialType_Diffuse);
	}
};
class NormalTextureUniform : public BaseUniform
{
public:
	NormalTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getNormalTexturePos());
		glUniform1i(m_uniformHandle, MaterialType_Normal);
	}
};
class EmissiveTextureUniform : public BaseUniform
{
public:
	EmissiveTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, MaterialType_Emissive);
	}
};
class CombinedTextureUniform : public BaseUniform
{
public:
	CombinedTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().combinedTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getCombinedTexturePos());
		glUniform1i(m_uniformHandle, MaterialType_Combined);
	}
};

class AtmIrradianceTextureUniform : public BaseUniform
{
public:
	AtmIrradianceTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().atmIrradianceTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, AtmScatteringTextureType::AtmScatteringTextureType_Irradiance);
	}
};
class AtmScatteringTextureUniform : public BaseUniform
{
public:
	AtmScatteringTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().atmScatteringTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, AtmScatteringTextureType::AtmScatteringTextureType_Scattering);
	}
};
class AtmSingleMieTextureUniform : public BaseUniform
{
public:
	AtmSingleMieTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().atmSingleMieScatTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, AtmScatteringTextureType::AtmScatteringTextureType_SingleMie);
	}
};
class AtmTransmittanceTextureUniform : public BaseUniform
{
public:
	AtmTransmittanceTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().atmTransmittanceTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, AtmScatteringTextureType::AtmScatteringTextureType_Transmittance);
	}
};

class LensFlareDirtTextureUniform : public BaseUniform
{
public:
	LensFlareDirtTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().lensFlareDirtTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, LensFlareTextureType::LensFlareTextureType_LenseDirt);
	}
};
class LensFlareGhostGradientTextureUniform : public BaseUniform
{
public:
	LensFlareGhostGradientTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().lensFlareGhostGradientTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, LensFlareTextureType::LensFlareTextureType_GhostGradient);
	}
};
class LensFlareStarburstTextureUniform : public BaseUniform
{
public:
	LensFlareStarburstTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().lensFlareStarburstTextureUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, LensFlareTextureType::LensFlareTextureType_Starburst);
	}
};

class DynamicEnvironmentMapUniform : public BaseUniform
{
public:
	DynamicEnvironmentMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dynamicEnvMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getDynamicEnvMapPos());
	}
};
class StaticEnvironmentMapUniform : public BaseUniform
{
public:
	StaticEnvironmentMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().staticEnvMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
//		glUniform1i(m_uniformHandle, p_uniformData.m_objectData.);
	}
};

// Unused (old shading model)
/* Unused */ class SpecularTextureUniform : public BaseUniform
{
public:
	SpecularTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().specularTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getSpecularTexturePos());
	}
};
/* Unused */ class GlossTextureUniform : public BaseUniform
{
public:
	GlossTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().glossTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getGlossTexturePos());
	}
};
/* Unused */ class HeightTextureUniform : public BaseUniform
{
public:
	HeightTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().heightTextureUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, p_uniformData.getHeightTexturePos());
	}
};

/* Unused */ class FogDensityUniform : public BaseUniform
{
public:
	FogDensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().fogDensityUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1f(m_uniformHandle, p_uniformData.getFogDensity());
	}
};
/* Unused */ class FogColorUniform : public BaseUniform
{
public:
	FogColorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().fogColorUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//auto &fogColor = p_uniformData.getFogColor();
		//glUniform3f(m_uniformHandle, fogColor.x, fogColor.y, fogColor.z);
	}
};

/* Unused */ class BillboardScaleUniform : public BaseUniform
{
public:
	BillboardScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().billboardScaleUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1f(m_uniformHandle, 1.0f);
	}
};
/* Unused */ class DepthTypeUniform : public BaseUniform
{
public:
	DepthTypeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().depthTypeUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1i(m_uniformHandle, 0);
	}
};

/* Unused */ class TestMatUniform : public BaseUniform
{
public:
	TestMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.getTestMat().m[0]);
	}
};
/* Unused */ class TestVecUniform : public BaseUniform
{
public:
	TestVecUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testVecUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//auto &testVec = p_uniformData.getTestVec();
		//glUniform4f(m_uniformHandle, testVec.x, testVec.y, testVec.z, testVec.w);
	}
};
/* Unused */ class TestFloatUniform : public BaseUniform
{
public:
	TestFloatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().testFloatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		//glUniform1f(m_uniformHandle, p_uniformData.getTestFloat());
	}
};

class PointLightBufferUniform : public BaseUniformBlock
{
public:
	PointLightBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().pointLightBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding_PointLights);
	}
};
class SpotLightBufferUniform : public BaseUniformBlock
{
public:
	SpotLightBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().spotLightBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding_SpotLights);
	}
};
class AtmScatParametersUniform : public BaseUniformBlock
{
public:
	AtmScatParametersUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().atmScatParamBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding_AtmScatParam);
	}
};
class LensFlareParametersUniform : public BaseUniformBlock
{
public:
	LensFlareParametersUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().lensFlareParametersBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding_LensFlareParam);
	}
};

class HDRShaderStorageBuffer : public BaseShaderStorageBlock
{
public:
	HDRShaderStorageBuffer(unsigned int p_shaderHandle) : BaseShaderStorageBlock(Config::shaderVar().HDRSSBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(SSBOBinding_HDR);
	}
};
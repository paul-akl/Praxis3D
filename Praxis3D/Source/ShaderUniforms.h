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

	// Get uniform name
	const inline std::string &getName() const { return m_name; }

	// Get uniform handle
	const inline unsigned int getHandle() const { return m_uniformHandle; }

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
		: m_name(p_name), m_shaderHandle(p_shaderHandle), m_uniformHandle(-1)
	{
		// Get the uniform location (returns -1 in case it is not present in the shader)
		m_uniformHandle = glGetUniformBlockIndex(m_shaderHandle, p_name.c_str());
	}

	// Get uniform name
	const inline std::string &getName() const { return m_name; }

	// Get uniform handle
	const inline unsigned int getHandle() const { return m_uniformHandle; }

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

	// Get SSBO name
	const inline std::string &getName() const { return m_name; }

	// Get SSBO handle
	const inline unsigned int getHandle() const { return m_SSBOHandle; }

	// Returns true if the uniform is present in the shader
	const inline bool isValid() const { return (m_SSBOHandle != -1); }

	const inline int getBlockSize() const
	{
		int blockSize = 0;

		// Get the uniform block size
		glGetActiveUniformBlockiv(m_shaderHandle, m_SSBOHandle, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

		return blockSize;
	}

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
		glUniformMatrix4fv(m_uniformHandle, 1, GL_TRUE, &p_uniformData.m_frameData.m_atmScatProjMatrix[0][0]);
	}
};
class ModelMatUniform : public BaseUniform
{
public:
	ModelMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().modelMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_objectData.m_modelMat[0][0]);
	}
};
class ViewMatUniform : public BaseUniform
{
public:
	ViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_viewMatrix[0][0]);
	}
};
class ProjectionMatUniform : public BaseUniform
{
public:
	ProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().projectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_projMatrix[0][0]);
	}
};
class ViewProjectionMatUniform : public BaseUniform
{
public:
	ViewProjectionMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().viewProjectionMatUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_viewProjMatrix[0][0]);
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
		glm::mat4 modelViewMat = p_uniformData.m_frameData.m_viewMatrix * p_uniformData.m_objectData.m_modelMat;
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &modelViewMat[0][0]);
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
		glm::mat4 MVP = p_uniformData.m_frameData.m_projMatrix * p_uniformData.m_frameData.m_viewMatrix * p_uniformData.m_objectData.m_modelMat;
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &MVP[0][0]);
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
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_transposeViewMatrix[0][0]);
	}
};
class TransposeInverseViewMatUniform : public BaseUniform
{
public:
	TransposeInverseViewMatUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().transposeInverseViewMatUniform, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniformMatrix4fv(m_uniformHandle, 1, GL_FALSE, &p_uniformData.m_frameData.m_transposeInverseViewMatrix[0][0]);
	}
};

class ScreenSizeUniform : public BaseUniform
{
public:
	ScreenSizeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().screenSizeUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_screenSize != p_uniformData.m_frameData.m_screenSize)
		{
			m_screenSize = p_uniformData.m_frameData.m_screenSize;

			glUniform2i(m_uniformHandle, m_screenSize.x, m_screenSize.y);
		}
	}

private:
	glm::ivec2 m_screenSize;
};
class InverseScreenSizeUniform : public BaseUniform
{
public:
	InverseScreenSizeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().inverseScreenSizeUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_inverseScreenSize != p_uniformData.m_frameData.m_inverseScreenSize)
		{
			m_inverseScreenSize = p_uniformData.m_frameData.m_inverseScreenSize;

			glUniform2f(m_uniformHandle, m_inverseScreenSize.x, m_inverseScreenSize.y);
		}
	}

private:
	glm::vec2 m_inverseScreenSize;
};
class ScreenNumOfPixelsUniform : public BaseUniform
{
public:
	ScreenNumOfPixelsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().screenNumOfPixelsUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_screenSize != p_uniformData.m_frameData.m_screenSize)
		{
			m_screenSize = p_uniformData.m_frameData.m_screenSize;

			glUniform1ui(m_uniformHandle, (unsigned int)(m_screenSize.x * m_screenSize.y));
		}
	}

private:
	glm::ivec2 m_screenSize;
};
class ProjPlaneRangeUniform : public BaseUniform
{
public:
	ProjPlaneRangeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().projPlaneRange, p_shaderHandle), m_zFar(0.0f), m_zNear(0.0f) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_zFar != p_uniformData.m_frameData.m_zFar || m_zNear != p_uniformData.m_frameData.m_zNear)
		{
			m_zFar = p_uniformData.m_frameData.m_zFar;
			m_zNear = p_uniformData.m_frameData.m_zNear;

			glUniform2f(m_uniformHandle, m_zFar, m_zNear);
		}
	}

private:
	float m_zFar;
	float m_zNear;
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
		if(m_currentEmissiveMultiplier != p_uniformData.m_objectData.m_emissiveIntensity)
		{
			m_currentEmissiveMultiplier = p_uniformData.m_objectData.m_emissiveIntensity;

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
		if(m_currentEmissiveThreshold != Config::graphicsVar().emissive_threshold)
		{
			m_currentEmissiveThreshold = Config::graphicsVar().emissive_threshold;

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
class StochasticSamplingScaleUniform : public BaseUniform
{
public:
	StochasticSamplingScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().stochasticSamplingScaleUniform, p_shaderHandle), m_currentStochasticSamplingScale(0.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_currentStochasticSamplingScale != p_uniformData.m_objectData.m_stochasticSamplingScale)
		{
			m_currentStochasticSamplingScale = p_uniformData.m_objectData.m_stochasticSamplingScale;

			glUniform1f(m_uniformHandle, m_currentStochasticSamplingScale);
		}
	}

private:
	float m_currentStochasticSamplingScale;
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
	LODParallaxMappingUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().LODParallaxUniform, p_shaderHandle), m_parallaxLOD(-1.0f) { }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_parallaxLOD != Config::graphicsVar().LOD_parallax_mapping)
		{
			m_parallaxLOD = Config::graphicsVar().LOD_parallax_mapping;

			glUniform1f(m_uniformHandle, m_parallaxLOD);
		}
	}

private:
	float m_parallaxLOD;
};
class ParallaxMappingNumOfStepsUniform : public BaseUniform
{
public:
	ParallaxMappingNumOfStepsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().parallaxMappingNumOfStepsLayersUniform, p_shaderHandle), m_minNumOfSteps(1.0f), m_maxNumOfSteps(1.0f){ }

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_minNumOfSteps != Config::rendererVar().parallax_mapping_min_steps || m_maxNumOfSteps != Config::rendererVar().parallax_mapping_max_steps)
		{
			m_minNumOfSteps = Config::rendererVar().parallax_mapping_min_steps;
			m_maxNumOfSteps = Config::rendererVar().parallax_mapping_max_steps;

			glUniform2f(m_uniformHandle, m_minNumOfSteps, m_maxNumOfSteps);
		}
	}

private:
	float m_minNumOfSteps;
	float m_maxNumOfSteps;
};
class TexelSizeUniform : public BaseUniform
{
public:
	TexelSizeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().texelSize, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_texelSize != p_uniformData.m_frameData.m_texelSize)
		{
			m_texelSize = p_uniformData.m_frameData.m_texelSize;

			glUniform2f(m_uniformHandle, m_texelSize.x, m_texelSize.y);
		}
	}
private:
	glm::vec2 m_texelSize;
};
class NumOFTexelsUniform : public BaseUniform
{
public:
	NumOFTexelsUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().numOfTexels, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_texelSize != p_uniformData.m_frameData.m_texelSize)
		{
			m_texelSize = p_uniformData.m_frameData.m_texelSize;

			glUniform1ui(m_uniformHandle, (unsigned int)(m_texelSize.x * m_texelSize.y));
		}
	}
private:
	glm::vec2 m_texelSize;
};
class MipLevelUniform : public BaseUniform
{
public:
	MipLevelUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().mipLevel, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, p_uniformData.m_frameData.m_mipLevel);
	}
};

class BloomTresholdUniform : public BaseUniform
{
public:
	BloomTresholdUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().bloomTreshold, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_bloomTreshold != p_uniformData.m_frameData.m_bloomTreshold)
		{
			m_bloomTreshold = p_uniformData.m_frameData.m_bloomTreshold;

			glUniform4f(m_uniformHandle, m_bloomTreshold.x, m_bloomTreshold.y, m_bloomTreshold.z, m_bloomTreshold.w);
		}
	}

private:
	glm::vec4 m_bloomTreshold;
};
class BloomDirtIntensityUniform : public BaseUniform
{
public:
	BloomDirtIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().bloomDirtIntensity, p_shaderHandle), m_bloomDirtIntensity(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_bloomDirtIntensity != Config::graphicsVar().bloom_dirt_intensity)
		{
			m_bloomDirtIntensity = Config::graphicsVar().bloom_dirt_intensity;

			glUniform1f(m_uniformHandle, m_bloomDirtIntensity);
		}
	}

private:
	float m_bloomDirtIntensity;
};
class BloomIntensityUniform : public BaseUniform
{
public:
	BloomIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().bloomIntensity, p_shaderHandle), m_bloomIntensity(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		// Check if the same value is not already assigned (a small optimization)
		if(m_bloomIntensity != Config::graphicsVar().bloom_intensity)
		{
			m_bloomIntensity = Config::graphicsVar().bloom_intensity;

			glUniform1f(m_uniformHandle, m_bloomIntensity);
		}
	}

private:
	float m_bloomIntensity;
};

class AmbientLightIntensityUniform : public BaseUniform
{
public:
	AmbientLightIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().ambientLightIntensity, p_shaderHandle), m_intensityDirectional(-1.0f), m_intensityPoint(-1.0f), m_intensitySpot(-1.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_intensityDirectional != p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensityDirectional ||
			m_intensityPoint != p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensityPoint ||
			m_intensitySpot != p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensitySpot)
		{
			m_intensityDirectional = p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensityDirectional;
			m_intensityPoint = p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensityPoint;
			m_intensitySpot = p_uniformData.m_frameData.m_miscSceneData.m_ambientIntensitySpot;

			glUniform3f(m_uniformHandle, m_intensityDirectional, m_intensityPoint, m_intensitySpot);
		}
	}

private:
	float m_intensityDirectional;
	float m_intensityPoint;
	float m_intensitySpot;
};
class DirLightColorUniform : public BaseUniform
{
public:
	DirLightColorUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightColor, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		auto &dirLightColor = p_uniformData.m_frameData.m_directionalLight.m_color;
		glUniform3f(m_uniformHandle, dirLightColor.x, dirLightColor.y, dirLightColor.z);
	}
};
class DirLightDirectionUniform : public BaseUniform
{
public:
	DirLightDirectionUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightDirection, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		auto lightDirection = p_uniformData.m_frameData.m_directionalLight.m_direction;
		glUniform3f(m_uniformHandle, lightDirection.x, lightDirection.y, lightDirection.z);
	}
};
class DirLightIntensityUniform : public BaseUniform
{
public:
	DirLightIntensityUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().dirLightIntensity, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1f(m_uniformHandle, p_uniformData.m_frameData.m_directionalLight.m_intensity);
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

// Gbuffer textures
class PositionMapUniform : public BaseUniform
{
public:
	PositionMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().positionMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferPosition);
	}
};
class DiffuseMapUniform : public BaseUniform
{
public:
	DiffuseMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferDiffuse);
	}
};
class NormalMapUniform : public BaseUniform
{
public:
	NormalMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferNormal);
	}
};
class EmissiveMapUniform : public BaseUniform
{
public:
	EmissiveMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferEmissive);
	}
};
class MatPropertiesMapUniform : public BaseUniform
{
public:
	MatPropertiesMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().matPropertiesMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferMatProperties);
	}
};
class IntermediateMapUniform : public BaseUniform
{
public:
	IntermediateMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().intermediateMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferIntermediate);
	}
};
class FinalMapUniform : public BaseUniform
{
public:
	FinalMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().finalMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferFinal);
	}
};
class DepthMapUniform : public BaseUniform
{
public:
	DepthMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().depthMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GbufferDepth);
	}
};
class InputMapUniform : public BaseUniform
{
public:
	InputMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().inputColorMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GBufferInputTexture);
	}
};
class OutputMapUniform : public BaseUniform
{
public:
	OutputMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().outputColorMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, GBufferTextureType::GbufferOutputTexture);
	}
};

// CSM textures
class CSMDepthMapUniform : public BaseUniform
{
public:
	CSMDepthMapUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().csmDepthMapUniform, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, CSMBufferTextureType::CSMBufferTextureType_CSMDepthMap);
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
	DiffuseTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().diffuseTextureUniform, p_shaderHandle), m_uniformSet(false) { }

	void update(const UniformData &p_uniformData)
	{
		if(!m_uniformSet)
		{
			m_uniformSet = true;
			glUniform1i(m_uniformHandle, MaterialType_Diffuse);
		}
	}

private:
	bool m_uniformSet;
};
class NormalTextureUniform : public BaseUniform
{
public:
	NormalTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().normalTextureUniform, p_shaderHandle), m_uniformSet(false) { }

	void update(const UniformData &p_uniformData)
	{
		if(!m_uniformSet)
		{
			m_uniformSet = true;
			glUniform1i(m_uniformHandle, MaterialType_Normal);
		}
	}

private:
	bool m_uniformSet;
};
class EmissiveTextureUniform : public BaseUniform
{
public:
	EmissiveTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().emissiveTextureUniform, p_shaderHandle), m_uniformSet(false) { }

	void update(const UniformData &p_uniformData)
	{
		if(!m_uniformSet)
		{
			m_uniformSet = true;
			glUniform1i(m_uniformHandle, MaterialType_Emissive);
		}
	}

private:
	bool m_uniformSet;
};
class CombinedTextureUniform : public BaseUniform
{
public:
	CombinedTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().combinedTextureUniform, p_shaderHandle), m_uniformSet(false) { }

	void update(const UniformData &p_uniformData)
	{
		if(!m_uniformSet)
		{
			m_uniformSet = true;
			glUniform1i(m_uniformHandle, MaterialType_Combined);
		}
	}

private:
	bool m_uniformSet;
};

class NoiseTextureUniform : public BaseUniform
{
public:
	NoiseTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().noiseTexture, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, MaterialType::MaterialType_Noise);
	}
};

class HBAOBlurNumOfSamples : public BaseUniform
{
public:
	HBAOBlurNumOfSamples(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().hbaoBlurNumOfSamples, p_shaderHandle), m_numOfSamples(0)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_numOfSamples != p_uniformData.m_frameData.m_aoData.m_aoBlurNumOfSamples)
		{
			m_numOfSamples = p_uniformData.m_frameData.m_aoData.m_aoBlurNumOfSamples;

			glUniform1i(m_uniformHandle, m_numOfSamples);
		}
	}

private:
	int m_numOfSamples;
}; 
class HBAOBlurSharpness : public BaseUniform
{
public:
	HBAOBlurSharpness(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().hbaoBlurSharpness, p_shaderHandle), m_blurSharpness(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_blurSharpness != p_uniformData.m_frameData.m_aoData.m_aoBlurSharpness)
		{
			m_blurSharpness = p_uniformData.m_frameData.m_aoData.m_aoBlurSharpness;

			glUniform1f(m_uniformHandle, m_blurSharpness);
		}
	}

private:
	float m_blurSharpness;
};
class HBAOBlurHorizontalInvResDirection : public BaseUniform
{
public:
	HBAOBlurHorizontalInvResDirection(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().hbaoBlurHorizontalInvResDirection, p_shaderHandle), m_screenHeight(0)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_screenHeight != p_uniformData.m_frameData.m_screenSize.y)
		{
			m_screenHeight = p_uniformData.m_frameData.m_screenSize.y;

			glUniform2f(m_uniformHandle, 0.0f, 1.0f / (float)m_screenHeight);
		}
	}

private:
	int m_screenHeight;
};
class HBAOBlurVerticalInvResDirection : public BaseUniform
{
public:
	HBAOBlurVerticalInvResDirection(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().hbaoBlurVerticalInvResDirection, p_shaderHandle), m_screenWidth(0)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_screenWidth != p_uniformData.m_frameData.m_screenSize.x)
		{
			m_screenWidth = p_uniformData.m_frameData.m_screenSize.x;

			glUniform2f(m_uniformHandle, 1.0f / (float)m_screenWidth, 0.0f);
		}
	}

private:
	int m_screenWidth;
};

class CSMBiasScaleUniform : public BaseUniform
{
public:
	CSMBiasScaleUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().csmBiasScaleUniform, p_shaderHandle), m_csmBiasScale(0.0f) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_csmBiasScale != p_uniformData.m_frameData.m_shadowMappingData.m_csmBiasScale)
		{
			m_csmBiasScale = p_uniformData.m_frameData.m_shadowMappingData.m_csmBiasScale;

			glUniform1f(m_uniformHandle, m_csmBiasScale);
		}
	}

private:
	float m_csmBiasScale;
};
class CSMPenumbraScaleRangeUniform : public BaseUniform
{
public:
	CSMPenumbraScaleRangeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().csmPenumbraScaleRange, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		if(m_penumbraScaleRange != p_uniformData.m_frameData.m_shadowMappingData.m_penumbraScaleRange)
		{
			m_penumbraScaleRange = p_uniformData.m_frameData.m_shadowMappingData.m_penumbraScaleRange;

			glUniform2f(m_uniformHandle, m_penumbraScaleRange.x, m_penumbraScaleRange.y);
		}
	}

private:
	glm::vec2 m_penumbraScaleRange;
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

class AverageLuminanceTextureUniform : public BaseUniform
{
public:
	AverageLuminanceTextureUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().averageLuminanceTexture, p_shaderHandle)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		glUniform1i(m_uniformHandle, LuminanceTextureType::LensFlareTextureType_AverageLuminance);
	}
};
class InverseLogLuminanceRangeUniform : public BaseUniform
{
public:
	InverseLogLuminanceRangeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().inverseLogLuminanceRange, p_shaderHandle), m_minLuminanceRange(0.0f), m_maxLuminanceRange(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_minLuminanceRange != Config::graphicsVar().luminance_range_min || m_maxLuminanceRange != Config::graphicsVar().luminance_range_max)
		{
			m_minLuminanceRange = Config::graphicsVar().luminance_range_min;
			m_maxLuminanceRange = Config::graphicsVar().luminance_range_max;

			float minLogLuminance = glm::log2(m_minLuminanceRange);
			float maxLogLuminance = glm::log2(m_maxLuminanceRange);
			float inverseLogRangeLuminance = 1.0f / (maxLogLuminance - minLogLuminance);

			glUniform1f(m_uniformHandle, inverseLogRangeLuminance);
		}
	}
private:
	float	m_minLuminanceRange,
			m_maxLuminanceRange;
};
class LogLuminanceRangeUniform : public BaseUniform
{
public:
	LogLuminanceRangeUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().logLuminanceRange, p_shaderHandle), m_minLuminanceRange(0.0f), m_maxLuminanceRange(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_minLuminanceRange != Config::graphicsVar().luminance_range_min || m_maxLuminanceRange != Config::graphicsVar().luminance_range_max)
		{
			m_minLuminanceRange = Config::graphicsVar().luminance_range_min;
			m_maxLuminanceRange = Config::graphicsVar().luminance_range_max;

			float minLogLuminance = glm::log2(m_minLuminanceRange);
			float maxLogLuminance = glm::log2(m_maxLuminanceRange);

			glUniform1f(m_uniformHandle, maxLogLuminance - minLogLuminance);
		}
	}
private:
	float	m_minLuminanceRange,
			m_maxLuminanceRange;
};
class LuminanceMultiplierUniform : public BaseUniform
{
public:
	LuminanceMultiplierUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().luminanceMultiplier, p_shaderHandle), m_luminanceMultiplier(-1.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_luminanceMultiplier != Config::graphicsVar().luminance_multiplier)
		{
			m_luminanceMultiplier = Config::graphicsVar().luminance_multiplier;

			glUniform1f(m_uniformHandle, m_luminanceMultiplier);
		}
	}
private:
	float m_luminanceMultiplier;
};
class MinLogLuminanceUniform : public BaseUniform
{
public:
	MinLogLuminanceUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().minLogLuminance, p_shaderHandle), m_minLuminanceRange(0.0f)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_minLuminanceRange != Config::graphicsVar().luminance_range_min)
		{
			m_minLuminanceRange = Config::graphicsVar().luminance_range_min;

			float minLogLuminance = glm::log2(m_minLuminanceRange);

			glUniform1f(m_uniformHandle, minLogLuminance);
		}
	}
private:
	float m_minLuminanceRange;
};
class TonemapMethodUniform : public BaseUniform
{
public:
	TonemapMethodUniform(unsigned int p_shaderHandle) : BaseUniform(Config::shaderVar().tonemapMethod, p_shaderHandle), m_tonemapMethod(-1)
	{
	}

	void update(const UniformData &p_uniformData)
	{
		if(m_tonemapMethod != Config::graphicsVar().tonemap_method)
		{
			m_tonemapMethod = Config::graphicsVar().tonemap_method;

			glUniform1i(m_uniformHandle, m_tonemapMethod);
		}
	}

private:
	int m_tonemapMethod;
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

class AODataSetBufferUniform : public BaseUniformBlock
{
public:
	AODataSetBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().AODataSetBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_AODataSet);
	}
};
class AtmScatParametersUniform : public BaseUniformBlock
{
public:
	AtmScatParametersUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().atmScatParamBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_AtmScatParam);
	}
};
class CSMMatrixBufferUniform : public BaseUniformBlock
{
public:
	CSMMatrixBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().CSMDataSetBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_CSMMatrixBuffer);
	}
};
class LensFlareParametersUniform : public BaseUniformBlock
{
public:
	LensFlareParametersUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().lensFlareParametersBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_LensFlareParam);
	}
};
class MaterialDataBufferUniform : public BaseUniformBlock
{
public:
	MaterialDataBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().materialDataBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_MaterialDataBuffer);
	}
};
class PointLightBufferUniform : public BaseUniformBlock
{
public:
	PointLightBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().pointLightBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_PointLights);
	}
};
class SpotLightBufferUniform : public BaseUniformBlock
{
public:
	SpotLightBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().spotLightBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_SpotLights);
	}
};
class SSAOSampleBufferUniform : public BaseUniformBlock
{
public:
	SSAOSampleBufferUniform(unsigned int p_shaderHandle) : BaseUniformBlock(Config::shaderVar().SSAOSampleBuffer, p_shaderHandle) { }

	void update(const UniformData &p_uniformData)
	{
		updateBlockBinding(UniformBufferBinding::UniformBufferBinding_SSAOSampleBuffer);
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
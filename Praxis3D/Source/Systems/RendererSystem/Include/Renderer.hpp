#pragma once

#include <bitset>

#include "Systems/RendererSystem/Components/Include/CameraGraphicsObject.hpp"
#include "ServiceLocators/Include/ClockLocator.hpp"
#include "ErrorHandler/Include/ErrorCodes.hpp"
#include "Systems/RendererSystem/Include/GeometryBuffer.hpp"
#include "Systems/RendererSystem/Include/GraphicsDataSets.hpp"
#include "Math/Include/Math.hpp"
#include "Loaders/Include/ModelLoader.hpp"
#include "Loaders/Include/ShaderLoader.hpp"
#include "Loaders/Include/TextureLoader.hpp"

struct SceneObjects;

class Renderer
{
	friend class RendererState;
public:
	Renderer() : m_currentObjectData(nullptr), m_currentCamera(nullptr) { }
	virtual ~Renderer() { }

	virtual ErrorCode init() { return ErrorCode::Success; }

	virtual void beginRenderCycle(const float p_deltaTime) { }
	virtual void endRenderCycle(const float p_deltaTime) { }
	virtual void renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime);

protected:
	// Note: caution with modifying. Correlates with enum in Model class, for convenience
	enum TextureTypes : unsigned int
	{
		DiffuseTexture	= MaterialType_Diffuse,
		NormalTexture	= MaterialType_Normal,
		EmissiveTexture = MaterialType_Emissive,
		CombinedTexture = MaterialType_Combined,
		NumTextureTypes,
	};

	enum CubemapTypes : unsigned int
	{
		StaticEnvMap = GBufferTextureType::GBufferNumTextures,
		DynamicEnvMap
	};

	// Recalculates the projection matrix
	void updateProjectionMatrix()
	{
		m_projMatrix = glm::perspectiveFov(Config::graphicsVar().fov, (float)m_screenSize.x, (float)m_screenSize.y, Config::graphicsVar().z_near, Config::graphicsVar().z_far);
	}

	const CameraObject *m_currentCamera;
	const GraphicsData *m_currentObjectData;

	glm::ivec2 m_screenSize;
	glm::mat4 m_projMatrix,
				m_viewProjMatrix,
				m_modelViewMatrix,
				m_modelViewProjMatrix;
};

class RendererState
{
public:
	RendererState(Renderer *p_renderer) : m_renderer(p_renderer), m_emptyVec(1.0f) { }
	virtual ~RendererState() { }

	const inline glm::mat4 &getModelMatrix()			const { return m_renderer->m_currentObjectData->m_modelMat;				 }
	const inline glm::mat4 &getViewMatrix()				const { return m_renderer->m_currentCamera->m_baseObjectData.m_modelMat; }
	const inline glm::mat4 &getProjectionMatrix()		const { return m_renderer->m_projMatrix;								 }
	const inline glm::mat4 &getViewProjMatrix()			const { return m_renderer->m_viewProjMatrix;							 }
	const inline glm::mat4 &getModelViewMatrix()		const { return m_renderer->m_modelViewMatrix;							 }
	const inline glm::mat4 &getModelViewProjMatrix()	const { return m_renderer->m_modelViewProjMatrix;						 }

	const inline glm::ivec2 getScreenSize()				const { return m_renderer->m_screenSize;								}
	const inline float getElapsedTime()					const { return ClockLocator::get().getElapsedSecondsF();				}
	const inline float getAlphaThreshold()				const { return m_renderer->m_currentObjectData->m_alphaThreshold;		}
	const inline float getEmissiveThreshold()			const { return m_renderer->m_currentObjectData->m_emissiveThreshold;	}
	const inline float getHeightScale()					const { return m_renderer->m_currentObjectData->m_heightScale;			}
	const inline float getTextureTilingFactor()			const { return m_renderer->m_currentObjectData->m_textureTilingFactor;	}

	const virtual glm::vec3 &getDirLightColor()			const { return m_emptyVec; }
	const virtual glm::vec3 &getDirLightDirection()		const { return m_emptyVec; }
	const virtual float getDirLightintensity()			const { return 1.0f; }
	const virtual unsigned int getNumPointLights()		const { return 0; }
	const virtual unsigned int getNumSpotLights()		const { return 0; }

	const inline glm::vec3 &getCameraPosition()			const { return m_renderer->m_currentCamera->m_baseObjectData.m_position; }
	const inline glm::vec2 &getCameraAngle()			const { return m_renderer->m_currentCamera->m_cameraAngle;				 }
	const inline glm::vec3 &getCameraTarget()			const { return m_renderer->m_currentCamera->m_baseObjectData.m_rotation; }
	const inline glm::vec3 &getCameraRightVec()			const { return m_emptyVec; } // Unused
	const inline glm::vec3 &getCameraUpVec()			const { return m_emptyVec; } // Unused

	const virtual glm::vec3 getFogColor()				const { return m_emptyVec; }
	const virtual float getFogDensity()					const { return 0.0f; }

	const unsigned int getBlurMapPosition()				const { return GBufferTextureType::GBufferIntermediate;	 }
	const unsigned int getDiffuseMapPosition()			const { return GBufferTextureType::GBufferDiffuse;		 }
	const unsigned int getEmissiveMapPosition()			const { return GBufferTextureType::GBufferEmissive;		 }
	const unsigned int getMatPropertiesMapPosition()	const { return GBufferTextureType::GBufferMatProperties; }
	const unsigned int getNormalMapPosition()			const { return GBufferTextureType::GBufferNormal;		 }
	const unsigned int getPositionMapPosition()			const { return GBufferTextureType::GBufferPosition;		 }
	const unsigned int getFinalMapPosition()			const { return GBufferTextureType::GBufferFinal;		 }

	const inline unsigned int getDiffuseTexturePos()	const { return Renderer::TextureTypes::DiffuseTexture;		}
	const inline unsigned int getEmissiveTexturePos()	const { return Renderer::TextureTypes::EmissiveTexture;		}
	const inline unsigned int getNormalTexturePos()		const { return Renderer::TextureTypes::NormalTexture;		}
	const inline unsigned int getCombinedTexturePos()	const { return Renderer::TextureTypes::CombinedTexture;		}

	const inline unsigned int getDynamicEnvMapPos()	const { return Renderer::CubemapTypes::DynamicEnvMap;	}
	const inline unsigned int getStaticEnvMapPos()	const { return Renderer::CubemapTypes::StaticEnvMap;	}

	const virtual glm::mat4 getTestMat()	const { return m_emptyMatrix;	}
	const virtual glm::vec4 getTestVec()	const { return glm::vec4();		}
	const virtual float getTestFloat()		const { return 0.0f;			}

private:
	Renderer *m_renderer;

	glm::vec3 m_emptyVec;
	glm::mat4 m_emptyMatrix;
};
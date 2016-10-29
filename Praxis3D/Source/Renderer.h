#pragma once

#include <bitset>

#include "CameraGraphicsObject.h"
#include "ClockLocator.h"
#include "ErrorCodes.h"
#include "GeometryBuffer.h"
#include "GraphicsDataSets.h"
#include "Math.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

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
		StaticEnvMap = GeometryBuffer::GBufferTextureType::GBufferNumTextures,
		DynamicEnvMap
	};

	// Recalculates the projection matrix
	void updateProjectionMatrix()
	{
		m_projMatrix.perspective(Config::graphicsVar().fov, m_screenSize.x, m_screenSize.y, Config::graphicsVar().z_near, Config::graphicsVar().z_far);
	}

	const CameraObject *m_currentCamera;
	const GraphicsData *m_currentObjectData;

	Math::Vec2i m_screenSize;
	Math::Mat4f m_projMatrix,
				m_viewProjMatrix,
				m_modelViewMatrix,
				m_modelViewProjMatrix;
};

class RendererState
{
public:
	RendererState(Renderer *p_renderer) : m_renderer(p_renderer), m_emptyVec(1.0f) { }
	virtual ~RendererState() { }

	const inline Math::Mat4f &getModelMatrix()			const { return m_renderer->m_currentObjectData->m_modelMat;				 }
	const inline Math::Mat4f &getViewMatrix()			const { return m_renderer->m_currentCamera->m_baseObjectData.m_modelMat; }
	const inline Math::Mat4f &getProjectionMatrix()		const { return m_renderer->m_projMatrix;								 }
	const inline Math::Mat4f &getViewProjMatrix()		const { return m_renderer->m_viewProjMatrix;							 }
	const inline Math::Mat4f &getModelViewMatrix()		const { return m_renderer->m_modelViewMatrix;							 }
	const inline Math::Mat4f &getModelViewProjMatrix()	const { return m_renderer->m_modelViewProjMatrix;						 }

	const inline Math::Vec2i getScreenSize()			const { return m_renderer->m_screenSize;								}
	const inline float getElapsedTime()					const { return ClockLocator::get().getElapsedSecondsF();				}
	const inline float getAlphaThreshold()				const { return m_renderer->m_currentObjectData->m_alphaThreshold;		}
	const inline float getEmissiveThreshold()			const { return m_renderer->m_currentObjectData->m_emissiveThreshold;	}
	const inline float getHeightScale()					const { return m_renderer->m_currentObjectData->m_heightScale;			}
	const inline float getTextureTilingFactor()			const { return m_renderer->m_currentObjectData->m_textureTilingFactor;	}

	const virtual Math::Vec3f &getDirLightColor()		const { return m_emptyVec; }
	const virtual Math::Vec3f &getDirLightDirection()	const { return m_emptyVec; }
	const virtual float getDirLightintensity()			const { return 1.0f; }
	const virtual unsigned int getNumPointLights()		const { return 0; }
	const virtual unsigned int getNumSpotLights()		const { return 0; }

	const inline Math::Vec3f &getCameraPosition()		const { return m_renderer->m_currentCamera->m_baseObjectData.m_position; }
	const inline Math::Vec2f &getCameraAngle()			const { return m_renderer->m_currentCamera->m_cameraAngle;				 }
	const inline Math::Vec3f &getCameraTarget()			const { return m_renderer->m_currentCamera->m_baseObjectData.m_rotation; }
	const inline Math::Vec3f &getCameraRightVec()		const { return m_emptyVec; } // Unused
	const inline Math::Vec3f &getCameraUpVec()			const { return m_emptyVec; } // Unused

	const virtual Math::Vec3f getFogColor()				const { return m_emptyVec; }
	const virtual float getFogDensity()					const { return 0.0f; }

	const unsigned int getBlurMapPosition()				const { return GeometryBuffer::GBufferTextureType::GBufferBlur;			 }
	const unsigned int getDiffuseMapPosition()			const { return GeometryBuffer::GBufferTextureType::GBufferDiffuse;		 }
	const unsigned int getEmissiveMapPosition()			const { return GeometryBuffer::GBufferTextureType::GBufferEmissive;		 }
	const unsigned int getMatPropertiesMapPosition()	const { return GeometryBuffer::GBufferTextureType::GBufferMatProperties; }
	const unsigned int getNormalMapPosition()			const { return GeometryBuffer::GBufferTextureType::GBufferNormal;		 }
	const unsigned int getPositionMapPosition()			const { return GeometryBuffer::GBufferTextureType::GBufferPosition;		 }
	const unsigned int getFinalMapPosition()			const { return GeometryBuffer::GBufferTextureType::GBufferFinal;		 }

	const inline unsigned int getDiffuseTexturePos()	const { return Renderer::TextureTypes::DiffuseTexture;		}
	const inline unsigned int getEmissiveTexturePos()	const { return Renderer::TextureTypes::EmissiveTexture;		}
	const inline unsigned int getNormalTexturePos()		const { return Renderer::TextureTypes::NormalTexture;		}
	const inline unsigned int getCombinedTexturePos()	const { return Renderer::TextureTypes::CombinedTexture;		}

	const inline unsigned int getDynamicEnvMapPos()	const { return Renderer::CubemapTypes::DynamicEnvMap;	}
	const inline unsigned int getStaticEnvMapPos()	const { return Renderer::CubemapTypes::StaticEnvMap;	}

	const virtual Math::Mat4f getTestMat()	const { return m_emptyMatrix;	}
	const virtual Math::Vec4f getTestVec()	const { return Math::Vec4f();	}
	const virtual float getTestFloat()		const { return 0.0f;			}

private:
	Renderer *m_renderer;

	Math::Vec3f m_emptyVec;
	Math::Mat4f m_emptyMatrix;
};
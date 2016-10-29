#pragma once

#include "Config.h"
#include "Loaders.h"
#include "GeometryBuffer.h"
#include "ModelGraphicsObjects.h"
#include "Renderer.h"

class DeferredRenderer : public Renderer
{
	friend class DeferredRendererState;
public: 
	DeferredRenderer();
	virtual ~DeferredRenderer();

	virtual ErrorCode init();

	virtual void beginRenderCycle(const float p_deltaTime);
	virtual void endRenderCycle(const float p_deltaTime);

	// Renders a complete frame
	virtual void renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime);

protected:
	// Uniform buffer handles for light buffers
	enum LightUniformBuffers : unsigned int
	{
		PointLightBindingPoint = 0,
		SpotLightBindingPoint
	};

	// An empty buffer used to render a single triangle that covers the whole screen
	// (triangles vertices are calculated inside the shader, hence empty buffer)
	class SingleTriangle
	{
	public:
		SingleTriangle() : m_vao(0), m_vbo(0) { }
		~SingleTriangle()
		{
			glDeleteBuffers(1, &m_vbo);
			glDeleteBuffers(1, &m_vao);
		}

		// Creates an empty buffer, assigns a VAO
		inline void load()
		{
			glGenBuffers(1, &m_vbo);
			glGenVertexArrays(1, &m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		// Binds VAO and VBO
		inline void bind() const
		{
			glBindVertexArray(m_vao);
			glBindVertexBuffer(0, m_vbo, 0, 0);
		}
		// Issues a draw call
		inline void render() const { glDrawArrays(GL_TRIANGLES, 0, 3); }

	private:
		GLuint	m_vao,
				m_vbo;
	};

	// Renders meshes and populates the geometry buffers
	virtual void geometryPass(const SceneObjects &p_sceneObjects, const float p_deltaTime);

	// Calculates lighting in a screen-space pass
	virtual void lightingPass(const SceneObjects &p_sceneObjects, const float p_deltaTime);

	// Renders the objects that are unaffected by lighting
	virtual void postLightingPass(const SceneObjects &p_sceneObjects, const float p_deltaTime);

	// Copies the final buffer to the screen by blitting it
	virtual void finalPass();

	// Updates frame-dependent variables (like view, projection matrices)
	virtual void update();

	void drawModelObject(const RenderableObjectData *p_renderableObject, const ShaderLoader::ShaderProgram *p_shader);
	void drawTessellatedObject(const RenderableObjectData *p_renderableObject, const ShaderLoader::ShaderProgram *p_shader);

	// Shaders
	ShaderLoader::ShaderProgram	*m_shaderGeometry,
								*m_shaderLightPass;

	// Framebuffers
	GeometryBuffer *m_gbuffer;
	
	// Light buffer handles (on VRAM)
	unsigned int m_pointLightBufferHandle,
				 m_spotLightBufferHandle;

	// Current number of lights in the light buffers
	size_t	m_numPointLights,
			m_numSpotLights;

	// Currently bound object handles
	unsigned int m_boundTextureHandles[Model::NumOfModelMaterials],
				 m_boundShaderHandle;
	
	DirectionalLightDataSet m_directionalLight;
	DeferredRendererState *m_rendererState;
	static SingleTriangle m_fullscreenTriangle;
};

class DeferredRendererState : public RendererState
{
	friend class DeferredRenderer;
public:
	~DeferredRendererState() { }

	const virtual Math::Vec3f getFogColor()				const { return Math::Vec3f(Config::graphicsVar().fog_color_x, Config::graphicsVar().fog_color_y, Config::graphicsVar().fog_color_z); }
	const virtual float getFogDensity()					const { return Config::graphicsVar().fog_density; }

	const virtual Math::Vec3f &getDirLightColor()		const { return m_deferredRenderer->m_directionalLight.m_color;		}
	const virtual Math::Vec3f &getDirLightDirection()	const { return m_deferredRenderer->m_directionalLight.m_direction;	}
	const virtual float getDirLightintensity()			const { return m_deferredRenderer->m_directionalLight.m_intensity;	}
	const virtual unsigned int getNumPointLights()		const { return (unsigned int)m_deferredRenderer->m_numPointLights;	}
	const virtual unsigned int getNumSpotLights()		const { return (unsigned int)m_deferredRenderer->m_numSpotLights;	}
	
	const virtual unsigned int getBlurBufferPos()		const { return GeometryBuffer::GBufferTextureType::GBufferBlur;		}
	const virtual unsigned int getDiffuseBufferPos()	const { return GeometryBuffer::GBufferTextureType::GBufferDiffuse;	}
	const virtual unsigned int getEmissiveBufferPos()	const { return GeometryBuffer::GBufferTextureType::GBufferEmissive; }
	const virtual unsigned int getNormalBufferPos()		const { return GeometryBuffer::GBufferTextureType::GBufferNormal;	}
	const virtual unsigned int getPositionBufferPos()	const { return GeometryBuffer::GBufferTextureType::GBufferPosition; }
	
protected:
	DeferredRendererState(DeferredRenderer *p_renderer) : RendererState(p_renderer), m_deferredRenderer(p_renderer) { }

	DeferredRenderer *m_deferredRenderer;
};
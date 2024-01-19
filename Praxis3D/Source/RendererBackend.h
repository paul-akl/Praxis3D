#pragma once
#pragma warning (disable : 4996)

#include <stdint.h>

#include "Config.h"
#include "CSMFramebuffer.h"
#include "GeometryBuffer.h"
#include "Loaders.h"
#include "ShaderUniformUpdater.h"
#include "UniformData.h"

class RendererBackend
{
public:
	// An empty buffer used to render a single triangle that covers the whole screen
	// (the vertices are calculated inside the shader, hence an empty buffer)
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

	// Draw command holds all the necessary data to draw a piece of geometry (a mesh)
	struct DrawCommand
	{
		DrawCommand(ShaderUniformUpdater &p_uniformUpdater, 
					const UniformObjectData &p_uniformObjectData, 
					const unsigned int p_shaderHandle,
					const unsigned int p_modelHandle,
					const unsigned int p_numIndices,
					const unsigned int p_baseVertex,
					const unsigned int p_baseIndex,
					const unsigned int p_matDiffuse,
					const unsigned int p_matNormal,
					const unsigned int p_matEmissive,
					const unsigned int p_matCombined,
					const int p_matWrapMode) :

			m_uniformUpdater(&p_uniformUpdater), 
			m_uniformObjectData(p_uniformObjectData),
			m_shaderHandle(p_shaderHandle),
			m_modelHandle(p_modelHandle),
			m_numIndices(p_numIndices),
			m_baseVertex(p_baseVertex),
			m_baseIndex(p_baseIndex),
			m_matDiffuse(p_matDiffuse),
			m_matNormal(p_matNormal),
			m_matEmissive(p_matEmissive),
			m_matCombined(p_matCombined),
			m_matWrapMode(p_matWrapMode) { }

		ShaderUniformUpdater *m_uniformUpdater;
		UniformObjectData m_uniformObjectData;

		unsigned int m_shaderHandle;
		unsigned int m_modelHandle;

		unsigned int m_numIndices;
		unsigned int m_baseVertex;
		unsigned int m_baseIndex;

		int m_matWrapMode;

		unsigned int m_matDiffuse;
		unsigned int m_matNormal;
		unsigned int m_matEmissive;
		unsigned int m_matCombined;
	};

	// Screen-space draw command is used to render a full-screen triangle, intended for
	// post-processing effects and off-screen rendering
	struct ScreenSpaceDrawCommand
	{
		ScreenSpaceDrawCommand(const ShaderUniformUpdater &p_uniformUpdater,
							   const UniformObjectData &p_uniformObjectData,
							   const unsigned int p_shaderHandle) :
			m_uniformUpdater(&p_uniformUpdater),
			m_uniformObjectData(p_uniformObjectData),
			m_shaderHandle(p_shaderHandle) { }

		const ShaderUniformUpdater *m_uniformUpdater;
		const UniformObjectData m_uniformObjectData;

		const unsigned int m_shaderHandle;
	};

	// A command to execute a compute shader
	struct ComputeDispatchCommand
	{
		ComputeDispatchCommand(const unsigned int p_numOfGroupsX, 
							   const unsigned int p_numOfGroupsY, 
							   const unsigned int p_numOfGroupsZ, 
							   const MemoryBarrierType p_memoryBarrier, 
							   ShaderUniformUpdater &p_uniformUpdater,
							   const UniformObjectData &p_uniformObjectData,
							   const unsigned int p_shaderHandle) :
			m_numOfGroups{ p_numOfGroupsX, p_numOfGroupsY, p_numOfGroupsZ },
			m_memoryBarrier(p_memoryBarrier),
			m_uniformUpdater(p_uniformUpdater),
			m_uniformObjectData(p_uniformObjectData),
			m_shaderHandle(p_shaderHandle) { }

		unsigned int m_numOfGroups[3];
		MemoryBarrierType m_memoryBarrier;

		ShaderUniformUpdater &m_uniformUpdater;
		UniformObjectData m_uniformObjectData;

		unsigned int m_shaderHandle;
	};

	// Used for binding textures and framebuffers for reading and writing
	struct BindCommand
	{
		BindCommand(BindCommandType p_type,
					const unsigned int p_bufferHandle,
					const unsigned int p_bindPosition,
					const bool p_bindForReading) :
			m_type(p_type),
			m_bufferHandle(p_bufferHandle),
			m_bindPosition(p_bindPosition),
			m_bindForReading(p_bindForReading) { }

		const unsigned int m_bufferHandle;
		const unsigned int m_bindPosition;	// used as glActiveTexture parameter
		const bool m_bindForReading;		// true = bind for reading; false = bind for writing

		const BindCommandType m_type;
	};

	// Used to upload data to a buffer that's on GPU, in a specified manner
	struct BufferUpdateCommand
	{
		BufferUpdateCommand(const unsigned int p_bufferHandle,
			const int64_t p_offset,
			const int64_t p_size,
			const void *p_data = NULL,
			const BufferUpdateType p_updateType = BufferUpdate_Data,
			const BufferType p_bufferType = BufferType_Uniform,
			const BufferUsageHint p_bufferUsageHint = BufferUsageHint_DynamicDraw) :
			m_bufferHandle(p_bufferHandle),
			m_offset(p_offset),
			m_size(p_size),
			m_data(p_data),
			m_updateType(p_updateType),
			m_bufferType(p_bufferType),
			m_bufferUsageHint(p_bufferUsageHint) { }
		
		const BufferUpdateType m_updateType;
		const BufferType m_bufferType;
		const BufferUsageHint m_bufferUsageHint;

		const unsigned int m_bufferHandle;
		const void *m_data;
		const int64_t	m_offset,
						m_size;
	};

	// Used for loading various objects (i.e. textures, models, shader, etc) to GPU
	struct LoadCommand
	{
		LoadCommand(unsigned int &p_handle,
					const BufferType p_bufferType,
					const BufferBindTarget p_bufferBindTarget,
					const BufferUsageHint p_bufferUsage,
					const unsigned int p_bindingIndex,
					const int64_t p_size,
					const void *p_data) :
			m_handle(p_handle),
			m_objectType(LoadObject_Buffer),
			m_objectData(p_bufferType, p_bufferBindTarget, p_bufferUsage, p_bindingIndex, p_size, p_data) { }

		LoadCommand(const std::string &p_name, 
					unsigned int &p_handle,
					unsigned int (&p_buffers)[ModelBuffer_NumAllTypes],
					const int (&p_numElements)[ModelBuffer_NumAllTypes],
					const int64_t(&p_size)[ModelBuffer_NumAllTypes],
					const void **m_data) :
			m_handle(p_handle),
			m_objectType(LoadObject_Model),
			m_objectData(p_name, p_buffers, p_numElements, p_size, m_data) { }

		LoadCommand(const std::string(&p_names)[ShaderType_NumOfTypes],
					unsigned int &p_handle,
					ShaderUniformUpdater &p_uniformUpdater,
					std::string(&p_source)[ShaderType_NumOfTypes],
					ErrorMessage(&p_errorMessages)[ShaderType_NumOfTypes]) :
			m_handle(p_handle),
			m_objectType(LoadObject_Shader),
			m_objectData(p_names, p_uniformUpdater, p_source, p_errorMessages) { }

		LoadCommand(const std::string &p_name, 
					unsigned int &p_handle,
					const TextureFormat p_texFormat,
					const TextureDataFormat p_texDataFormat,
					const TextureDataType p_texDataType,
					const TextureFilterType p_texMagFilter,
					const TextureFilterType p_texMinFilter,
					const bool p_enableMipmap,
					const int p_mipmapLevel,
					const unsigned int p_textureWidth,
					const unsigned int p_textureHeight,
					const void *p_data) :
			m_handle(p_handle),
			m_objectType(LoadObject_Texture2D),
			m_objectData(p_name, p_texFormat, p_texDataFormat, p_texDataType, p_texMagFilter, p_texMinFilter, p_enableMipmap, p_mipmapLevel, p_textureWidth, p_textureHeight, p_data) { }

		LoadCommand(unsigned int &p_handle,
					const TextureFormat p_texFormat,
					const int p_mipmapLevel,
					const unsigned int p_textureWidth,
					const unsigned int p_textureHeight,
					const void **p_data) :
			m_handle(p_handle),
			m_objectType(LoadObject_TextureCube),
			m_objectData(p_texFormat, p_mipmapLevel, p_textureWidth, p_textureHeight, p_data) { }


		struct BufferLoadData
		{
			BufferLoadData(const BufferType p_bufferType,
						   const BufferBindTarget p_bufferBindTarget,
						   const BufferUsageHint p_bufferUsage,
						   const unsigned int p_bindingIndex,
						   const int64_t p_size,
						   const void *p_data) :
				m_bufferType(p_bufferType),
				m_bufferBindTarget(p_bufferBindTarget),
				m_bufferUsage(p_bufferUsage),
				m_bindingIndex(p_bindingIndex),
				m_size(p_size),
				m_data(p_data) { }

			const BufferType m_bufferType;
			const BufferBindTarget m_bufferBindTarget;
			const BufferUsageHint m_bufferUsage;
			const unsigned int m_bindingIndex;
			const int64_t m_size;
			const void *m_data;
		};
		struct ModelLoadData
		{
			ModelLoadData(const std::string &p_name,
						  unsigned int(&p_buffers)[ModelBuffer_NumAllTypes],
						  const int(&p_numElements)[ModelBuffer_NumAllTypes],
						  const int64_t(&p_size)[ModelBuffer_NumAllTypes],
						  const void **m_data) :
				m_name(p_name),
				m_data(m_data),
				m_buffers(p_buffers)
			{
				std::copy(std::begin(p_numElements), std::end(p_numElements), std::begin(m_numElements));
				std::copy(std::begin(p_size), std::end(p_size), std::begin(m_size));
			}

			const std::string &m_name;
			unsigned int (&m_buffers)[ModelBuffer_NumAllTypes];
			int m_numElements[ModelBuffer_NumAllTypes];
			int64_t m_size[ModelBuffer_NumAllTypes];
			const void **m_data;
		};
		struct ShaderLoadData
		{
			ShaderLoadData(const std::string(&p_names)[ShaderType_NumOfTypes],
						   ShaderUniformUpdater &p_uniformUpdater,
						   std::string(&p_source)[ShaderType_NumOfTypes],
						   ErrorMessage(&p_errorMessages)[ShaderType_NumOfTypes]):
				m_names(p_names),
				m_uniformUpdater(p_uniformUpdater),
				m_errorMessages(p_errorMessages),
				m_source(p_source) { }

			ShaderUniformUpdater &m_uniformUpdater;
			std::string(&m_source)[ShaderType_NumOfTypes];
			ErrorMessage (&m_errorMessages)[ShaderType_NumOfTypes];
			const std::string (&m_names)[ShaderType_NumOfTypes];
		};
		struct Texture2DLoadData
		{
			Texture2DLoadData(const std::string &p_name,
							  const TextureFormat p_texFormat,
							  const TextureDataFormat p_texDataFormat,
							  const TextureDataType p_texDataType,
							  const TextureFilterType p_texMagFilter,
							  const TextureFilterType p_texMinFilter,
							  const bool p_enableMipmap,
							  const int p_mipmapLevel,
							  const unsigned int p_textureWidth,
							  const unsigned int p_textureHeight,
							  const void *p_data) :
				m_name(p_name),
				m_texFormat(p_texFormat),
				m_texDataFormat(p_texDataFormat),
				m_texDataType(p_texDataType),
				m_magnificationFilter(p_texMagFilter),
				m_minificationFilter(p_texMinFilter),
				m_enableMipmap(p_enableMipmap),
				m_mipmapLevel(p_mipmapLevel),
				m_textureWidth(p_textureWidth),
				m_textureHeight(p_textureHeight),
				m_data(p_data) { }

			const std::string &m_name;
			const TextureFormat m_texFormat;
			const TextureDataFormat m_texDataFormat;
			const TextureDataType m_texDataType;
			const TextureFilterType m_magnificationFilter;
			const TextureFilterType m_minificationFilter;
			const bool m_enableMipmap;
			const int m_mipmapLevel;
			const unsigned int m_textureWidth;
			const unsigned int m_textureHeight;
			const void *m_data;
		};
		struct CubemapLoadData
		{
			CubemapLoadData(const TextureFormat p_texFormat,
							const int p_mipmapLevel,
							const unsigned int p_textureWidth,
							const unsigned int p_textureHeight,
							const void **p_data) :
				m_texFormat(p_texFormat),
				m_mipmapLevel(p_mipmapLevel),
				m_textureWidth(p_textureWidth),
				m_textureHeight(p_textureHeight),
				m_data(p_data) { }

			const TextureFormat m_texFormat;
			const unsigned int m_textureWidth;
			const unsigned int m_textureHeight;
			const int m_mipmapLevel;
			const void **m_data;
		};

		union ObjectData
		{
			ObjectData(const BufferType p_bufferType,
					   const BufferBindTarget p_bufferBindTarget,
					   const BufferUsageHint p_bufferUsage,
					   const unsigned int p_bindingIndex,
					   const int64_t p_size,
					   const void *p_data) :
				m_bufferData(p_bufferType, p_bufferBindTarget, p_bufferUsage, p_bindingIndex, p_size, p_data) { }

			ObjectData(const std::string &p_name, 
					   unsigned int(&p_buffers)[ModelBuffer_NumAllTypes],
					   const int(&p_numElements)[ModelBuffer_NumAllTypes],
					   const int64_t(&p_size)[ModelBuffer_NumAllTypes],
					   const void **m_data) :
				m_modelData(p_name, p_buffers, p_numElements, p_size, m_data) { }

			ObjectData(const std::string(&p_names)[ShaderType_NumOfTypes],
					   ShaderUniformUpdater &p_uniformUpdater,
					   std::string(&p_source)[ShaderType_NumOfTypes],
					   ErrorMessage(&p_errorMessages)[ShaderType_NumOfTypes]) :
				m_shaderData(p_names, p_uniformUpdater, p_source, p_errorMessages) { }

			ObjectData(const std::string &p_name, 
					   const TextureFormat p_texFormat,
					   const TextureDataFormat p_texDataFormat,
					   const TextureDataType p_texDataType,
					   const TextureFilterType p_texMagFilter,
					   const TextureFilterType p_texMinFilter,
					   const bool p_enableMipmap,
					   const int p_mipmapLevel,
					   const unsigned int p_textureWidth,
					   const unsigned int p_textureHeight,
					   const void *p_data) :
				m_tex2DData(p_name, p_texFormat, p_texDataFormat, p_texDataType, p_texMagFilter, p_texMinFilter, p_enableMipmap, p_mipmapLevel, p_textureWidth, p_textureHeight, p_data) { }

			ObjectData(const TextureFormat p_texFormat,
					   const int p_mipmapLevel,
					   const unsigned int p_textureWidth,
					   const unsigned int p_textureHeight,
					   const void **p_data) :
				m_cubeMapData(p_texFormat, p_mipmapLevel, p_textureWidth, p_textureHeight, p_data) { }


			BufferLoadData m_bufferData;
			ModelLoadData m_modelData;
			ShaderLoadData m_shaderData;
			Texture2DLoadData m_tex2DData;
			CubemapLoadData m_cubeMapData;
		};

		const LoadObjectType m_objectType;
		ObjectData m_objectData;
		unsigned int &m_handle;
	};

	// First element is a sort key, second element is an object to draw
	typedef std::vector<std::pair<uint64_t, DrawCommand>> DrawCommands;
	typedef std::vector<std::pair<uint64_t, ScreenSpaceDrawCommand>> ScreenSpaceDrawCommands;

	typedef std::vector<LoadCommand> LoadCommands;
	typedef std::vector<std::pair<UnloadObjectType, unsigned int>> UnloadCommands;
	typedef std::vector<BufferUpdateCommand> BufferUpdateCommands;
	typedef std::vector<ComputeDispatchCommand> ComputeDispatchCommands;

	RendererBackend();
	~RendererBackend();

	ErrorCode init(const UniformFrameData &p_frameData);

	ErrorCode createFramebuffer(const FramebufferType p_frambufferType, const UniformFrameData &p_frameData);

	void setScreenSize(const UniformFrameData &p_frameData)
	{
		// Set gbuffer textures size
		m_gbuffer->setBufferSize((unsigned int)p_frameData.m_screenSize.x, (unsigned int)p_frameData.m_screenSize.y);

		// Set the viewport
		glViewport(0, 0, p_frameData.m_screenSize.x, p_frameData.m_screenSize.y);
	}
	
	void processUpdate(const BufferUpdateCommands &p_updateCommands, const UniformFrameData &p_frameData);
	void processLoading(LoadCommands &p_loadCommands, const UniformFrameData &p_frameData);
	void processUnloading(UnloadCommands &p_unloadCommands);
	void processDrawing(const DrawCommands &p_drawCommands, const UniformFrameData &p_frameData);
	void processDrawing(const ScreenSpaceDrawCommands &p_screenSpaceDrawCommands, const UniformFrameData &p_frameData);
	void processDrawing(const ComputeDispatchCommands &p_computeDispatchCommands, const UniformFrameData &p_frameData);

	inline void bindTextureForReadering(const unsigned int p_bindingLocation, const TextureLoader2D::Texture2DHandle &p_texture) const
	{
		glActiveTexture(GL_TEXTURE0 + p_bindingLocation);
		glBindTexture(GL_TEXTURE_2D, p_texture.getHandle());
	}

	inline CSMFramebuffer *getCSMFramebuffer() { return m_csmBuffer; }
	inline GeometryBuffer *getGeometryBuffer() { return m_gbuffer; }

	inline unsigned int getFramebufferTextureHandle(GBufferTextureType p_bufferType) const { return m_gbuffer->getBufferTextureHandle(p_bufferType); }

protected:
	// Currently bound and last updated objects
	struct CurrentState
	{
		CurrentState()
		{
			reset();
		}

		inline void reset()
		{
			m_boundUniformBuffer = 0;
			m_boundShader = 0;
			m_boundVAO = 0;

			m_lastTexUpdate = 0;
			m_lastFrameUpdate = 0;
			m_lastModelUpdate = 0;
			m_lastMeshUpdate = 0;
		}

		unsigned int m_boundUniformBuffer;
		unsigned int m_boundShader;
		unsigned int m_boundVAO;

		unsigned int m_lastTexUpdate;
		unsigned int m_lastFrameUpdate;
		unsigned int m_lastModelUpdate;
		unsigned int m_lastMeshUpdate;
	};
	
	// Handle binding functions
	inline void bindShader(const unsigned int p_shaderHandle)
	{
		// Check if the shader is not already bound
		if(p_shaderHandle != m_rendererState.m_boundShader)
		{
			// Bind the shader
			glUseProgram(p_shaderHandle);
			m_rendererState.m_boundShader = p_shaderHandle;
		}
	}
	inline void bindVAO(const unsigned int p_VAO)
	{
		// Check if the VAO is not already bound
		if(p_VAO != m_rendererState.m_boundVAO)
		{
			// Bind the VAO
			glBindVertexArray(p_VAO);
			m_rendererState.m_boundVAO = p_VAO;
		}
	}
	inline void resetVAO() 
	{
		// Set the currently bound VAO back to 0, so the VAO will need to be bound again
		m_rendererState.m_boundVAO = 0; 
	}
	inline void bindUniformBuffer(const unsigned int p_bufferHandle)
	{
		// Check if the uniform buffer is not already bound
		if(p_bufferHandle != m_rendererState.m_boundUniformBuffer)
		{
			// Bind the uniform buffer
			glBindBuffer(GL_UNIFORM_BUFFER, p_bufferHandle);
			m_rendererState.m_boundUniformBuffer = p_bufferHandle;
		}
	}

	// Uniform update functions
	inline void textureUniformUpdate(const unsigned int p_shaderHandle, const ShaderUniformUpdater *p_uniformUpdater, const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
	{
		// Check if the texture uniforms haven't been updated already
		//if(m_rendererState.m_lastTexUpdate != p_shaderHandle)
		//{
			// Declare uniform data
			UniformData uniformData(p_objectData, p_frameData);

			// Update texture uniforms
			p_uniformUpdater->updateTextureUniforms(uniformData);
			m_rendererState.m_lastTexUpdate = p_shaderHandle;
		//}
	}
	inline void frameUniformUpdate(const unsigned int p_shaderHandle, const ShaderUniformUpdater *p_uniformUpdater, const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
	{
		// Check if the frame uniforms haven't been updated already
		//if(m_rendererState.m_lastFrameUpdate != p_shaderHandle)
		//{
			bindShader(p_shaderHandle);

			// Declare uniform data
			UniformData uniformData(p_objectData, p_frameData);

			// Update frame uniforms
			p_uniformUpdater->updateFrame(uniformData);
			m_rendererState.m_lastFrameUpdate = p_shaderHandle;
		//}
	}
	inline void modelUniformUpdate(const unsigned int p_shaderHandle, const ShaderUniformUpdater *p_uniformUpdater, const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
	{
		// Check if the model uniforms haven't been updated already
		//if(m_rendererState.m_lastModelUpdate != p_shaderHandle)
		//{
			// Declare uniform data
			UniformData uniformData(p_objectData, p_frameData);
			
			// Update model uniforms
			p_uniformUpdater->updateModel(uniformData);
			m_rendererState.m_lastModelUpdate = p_shaderHandle;
		//}
	}
	inline void meshUniformUpdate(const unsigned int p_shaderHandle, const ShaderUniformUpdater *p_uniformUpdater, const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
	{
		// Check if the mesh uniforms haven't been updated already
		//if(m_rendererState.m_lastMeshUpdate != p_shaderHandle)
		//{
			// Declare uniform data
			UniformData uniformData(p_objectData, p_frameData);

			// Update mesh uniforms
			p_uniformUpdater->updateMesh(uniformData);
			m_rendererState.m_lastMeshUpdate = p_shaderHandle;
		//}
	}
	
	inline void processCommand(const DrawCommand &p_command, const UniformFrameData &p_frameData)
	{
		// Get uniform data
		UniformData uniformData(p_command.m_uniformObjectData, p_frameData);
		
		// Bind the shader
		bindShader(p_command.m_shaderHandle);

		// Update shader uniforms
		frameUniformUpdate(	 p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);
		modelUniformUpdate(	 p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);
		meshUniformUpdate(	 p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);

		//p_command.m_uniformUpdater.updateBlockBindingPoints();

		// Bind VAO
		bindVAO(p_command.m_modelHandle);

		// Draw the geometry
		glDrawElementsBaseVertex(GL_TRIANGLES,
								 p_command.m_numIndices,
								 GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * p_command.m_baseIndex),
								 p_command.m_baseVertex);
	}
	inline void processCommand(const ScreenSpaceDrawCommand &p_command, const UniformFrameData &p_frameData)
	{
		// Get uniform data
		UniformData uniformData(p_command.m_uniformObjectData, p_frameData);
		
		// Bind the shader
		bindShader(p_command.m_shaderHandle);

		// Update shader uniforms
		frameUniformUpdate(p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);
		textureUniformUpdate(p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);
		//modelUniformUpdate(p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);
		//meshUniformUpdate(p_command.m_shaderHandle, p_command.m_uniformUpdater, p_command.m_uniformObjectData, p_frameData);

		m_fullscreenTriangle.bind();
		m_fullscreenTriangle.render();
	}
	inline void processCommand(const BufferUpdateCommand &p_command, const UniformFrameData &p_frameData)
	{
		// Bind the buffer
		bindUniformBuffer(p_command.m_bufferHandle);
		
		// Update the buffer based on the specified way
		switch(p_command.m_updateType)
		{
		case BufferUpdate_Data:
			glBufferData(p_command.m_bufferType,
						 p_command.m_size,
						 p_command.m_data,
						 p_command.m_bufferUsageHint);
			break;

		case BufferUpdate_SubData:
			glBufferSubData(p_command.m_bufferType,
							p_command.m_offset,
							p_command.m_size,
							p_command.m_data);
			break;
		}
	}
	inline void processCommand(LoadCommand &p_command, const UniformFrameData &p_frameData)
	{
		switch(p_command.m_objectType)
		{
			case LoadObject_Buffer:
				{
					// Create the buffer
					glGenBuffers(1, &p_command.m_handle);

					// Bind the buffer
					glBindBuffer(p_command.m_objectData.m_bufferData.m_bufferType, p_command.m_handle);

					// Fill the buffer
					glBufferData(p_command.m_objectData.m_bufferData.m_bufferType,
						p_command.m_objectData.m_bufferData.m_size,
						p_command.m_objectData.m_bufferData.m_data,
						p_command.m_objectData.m_bufferData.m_bufferUsage);

					// Bind the buffer to the binding index so it can be accessed in a shader
					glBindBufferBase(p_command.m_objectData.m_bufferData.m_bufferBindTarget,
						p_command.m_objectData.m_bufferData.m_bindingIndex,
						p_command.m_handle);
				}
				break;

			case LoadObject_Shader:
				{
					unsigned int shaderHandles[ShaderType_NumOfTypes] = { 0 };
					unsigned int shaderTypes[ShaderType_NumOfTypes] = {
						GL_COMPUTE_SHADER,
						GL_FRAGMENT_SHADER,
						GL_GEOMETRY_SHADER,
						GL_VERTEX_SHADER,
						GL_TESS_CONTROL_SHADER,
						GL_TESS_EVALUATION_SHADER };

					// Clear error queue (TODO: remove in the future)
					glGetError();

					// Create shader program handle
					if(p_command.m_handle == 0)
						p_command.m_handle = glCreateProgram();

					// Check for errors
					GLenum glError = glGetError();
					if(glError != GL_NO_ERROR)
					{
						// Log an error with every shader info log
						for(unsigned int i = 0; i < ShaderType_NumOfTypes; i++)
						{
							p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorCode = ErrorCode::Shader_creation_failed;
							p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorSource = ErrorSource::Source_ShaderLoader;
						}

						std::string combinedNames;

						// For every shader filename, if it's not empty, add it to the combined filename
						for(unsigned int i = 0; i < ShaderType_NumOfTypes; i++)
							if(!p_command.m_objectData.m_shaderData.m_names[i].empty())
								combinedNames += p_command.m_objectData.m_shaderData.m_names[i] + ", ";

						// Remove the last 2 characters from the filename (comma and space)
						if(!combinedNames.empty())
						{
							combinedNames.pop_back();
							combinedNames.pop_back();
						}

						// Log an error with the error handler
						ErrHandlerLoc::get().log(ErrorCode::Shader_creation_failed,
							ErrorSource::Source_ShaderLoader,
							"\"" + combinedNames + "\":\n" + Utilities::toString(glError));
					}
					else
					{
						unsigned int numOfShaders = ShaderType::ShaderType_NumOfTypes;

						// If a compute shader exists, then load ONLY the compute shader
						if(!p_command.m_objectData.m_shaderData.m_source[ShaderType::ShaderType_Compute].empty())
							numOfShaders = 1;

						// Create individual shaders
						for(unsigned int i = 0; i < numOfShaders; i++)
						{
							if(!p_command.m_objectData.m_shaderData.m_source[i].empty())
							{
								// Create a shader handle
								shaderHandles[i] = glCreateShader(shaderTypes[i]);

								// Check for errors
								glError = glGetError();
								if(glError != GL_NO_ERROR)
								{
									// Log an error with a shader info log
									p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorCode = ErrorCode::Shader_creation_failed;
									p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorSource = ErrorSource::Source_ShaderLoader;

									// Log an error with the error handler
									ErrHandlerLoc::get().log(ErrorCode::Shader_creation_failed,
										ErrorSource::Source_ShaderLoader,
										"\"" + p_command.m_objectData.m_shaderData.m_names[i] + "\":\n" + Utilities::toString(glError));
								}
								else
								{
									// Pass shader source code and compile it
									const char *shaderSource = p_command.m_objectData.m_shaderData.m_source[i].c_str();
									glShaderSource(shaderHandles[i], 1, &shaderSource, NULL);
									glCompileShader(shaderHandles[i]);

									// Check for shader compilation errors
									GLint shaderCompileResult = 0;
									glGetShaderiv(shaderHandles[i], GL_COMPILE_STATUS, &shaderCompileResult);

									// Check for errors
									glError = glGetError();

									// If compilation failed
									if(shaderCompileResult == 0)
									{
										// Assign an error
										int shaderCompileLogLength = 0;
										glGetShaderiv(shaderHandles[i], GL_INFO_LOG_LENGTH, &shaderCompileLogLength);

										// Get the actual error message
										std::vector<char> shaderCompileErrorMessage(shaderCompileLogLength);
										glGetShaderInfoLog(shaderHandles[i], shaderCompileLogLength, NULL, &shaderCompileErrorMessage[0]);

										// Convert vector of chars to a string
										std::string errorMessageTemp;
										for(int j = 0; shaderCompileErrorMessage[j]; j++)
											errorMessageTemp += shaderCompileErrorMessage[j];

										// Log an error with a shader info log
										p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorCode = ErrorCode::Shader_compile_failed;
										p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorSource = ErrorSource::Source_ShaderLoader;
										p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorMessage = errorMessageTemp;

										// Log an error with the error handler
										ErrHandlerLoc::get().log(ErrorCode::Shader_compile_failed,
											ErrorSource::Source_ShaderLoader,
											"\"" + p_command.m_objectData.m_shaderData.m_names[i] + "\":\n" + errorMessageTemp);

										// Reset the shader handle
										shaderHandles[i] = 0;
									}
									else
									{
										// Attach shader to the program handle
										glAttachShader(p_command.m_handle, shaderHandles[i]);

										// Check for errors
										glError = glGetError();

										if(glError != GL_NO_ERROR)
										{
											// Reset the shader handle
											shaderHandles[i] = 0;

											// Log an error with a shader info log
											p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorCode = ErrorCode::Shader_attach_failed;
											p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorSource = ErrorSource::Source_ShaderLoader;
											p_command.m_objectData.m_shaderData.m_errorMessages[i].m_errorMessage = Utilities::toString(glError);

											// Log an error with the error handler
											ErrHandlerLoc::get().log(ErrorCode::Shader_compile_failed,
												ErrorSource::Source_ShaderLoader,
												"\"" + p_command.m_objectData.m_shaderData.m_names[i] + "\":\n" + Utilities::toString(glError));
										}
									}
								}
							}
						}

						GLint shaderLinkingResult;
						glLinkProgram(p_command.m_handle);

						// Check for linking errors. If an error has occurred, get the error message and throw an exception
						glGetProgramiv(p_command.m_handle, GL_LINK_STATUS, &shaderLinkingResult);

						// If shader loading was successful
						if(shaderLinkingResult)
						{
							glUseProgram(p_command.m_handle);

							// Generate uniform update list, after the shader has been compiled
							p_command.m_objectData.m_shaderData.m_uniformUpdater.generateUpdateList();

							// Update some of the uniforms (that do not change frame to frame)
							p_command.m_objectData.m_shaderData.m_uniformUpdater.updateTextureUniforms();
							p_command.m_objectData.m_shaderData.m_uniformUpdater.updateBlockBindingPoints();
							p_command.m_objectData.m_shaderData.m_uniformUpdater.updateSSBBindingPoints();
						}
						// If shader loading failed
						else
						{
							// Reset shader handle
							p_command.m_handle = 0;

							GLsizei shaderLinkLogLength = 0;
							std::string errorMessageTemp;
							glGetShaderiv(p_command.m_handle, GL_INFO_LOG_LENGTH, &shaderLinkLogLength);

							// Sometimes OpenGL cannot retrieve the error string, so check that just in case
							if(shaderLinkLogLength > 0)
							{
								// Get the actual error message
								std::vector<char> shaderLinkErrorMessage(shaderLinkLogLength);
								glGetShaderInfoLog(p_command.m_handle, shaderLinkLogLength, NULL, &shaderLinkErrorMessage[0]);

								// Convert vector of chars to a string
								for(int i = 0; shaderLinkErrorMessage[i]; i++)
									errorMessageTemp += shaderLinkErrorMessage[i];
							}
							else
								errorMessageTemp = "Couldn't retrieve the error";

							// Log an error with a shader info log
							p_command.m_objectData.m_shaderData.m_errorMessages[0].m_errorCode = ErrorCode::Shader_link_failed;
							p_command.m_objectData.m_shaderData.m_errorMessages[0].m_errorSource = ErrorSource::Source_ShaderLoader;
							p_command.m_objectData.m_shaderData.m_errorMessages[0].m_errorMessage = errorMessageTemp;

							// Log an error with the error handler
							ErrHandlerLoc::get().log(ErrorCode::Shader_link_failed, ErrorSource::Source_ShaderLoader, errorMessageTemp);
						}

						// Iterate over all shaders, detach and delete them
						for(unsigned int i = 0; i < ShaderType_NumOfTypes; i++)
						{
							// If shader is valid
							if(shaderHandles[i] != 0)
							{
								glDetachShader(p_command.m_handle, shaderHandles[i]);
								glDeleteShader(shaderHandles[i]);
							}
						}
					}
				}
				break;

			case LoadObject_Texture2D:
				{
					// Generate, bind and upload the texture
					glGenTextures(1, &p_command.m_handle);
					glBindTexture(GL_TEXTURE_2D, p_command.m_handle);
					glTexImage2D(GL_TEXTURE_2D,
						p_command.m_objectData.m_tex2DData.m_mipmapLevel,
						p_command.m_objectData.m_tex2DData.m_texDataFormat,
						p_command.m_objectData.m_tex2DData.m_textureWidth,
						p_command.m_objectData.m_tex2DData.m_textureHeight,
						0,
						p_command.m_objectData.m_tex2DData.m_texFormat,
						p_command.m_objectData.m_tex2DData.m_texDataType,
						p_command.m_objectData.m_tex2DData.m_data);

					// Generate mipmaps if they are enabled
					if(p_command.m_objectData.m_tex2DData.m_enableMipmap)
						glGenerateMipmap(GL_TEXTURE_2D);

					// Texture filtering mode, when image is magnified
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p_command.m_objectData.m_tex2DData.m_magnificationFilter);

					// Texture filtering mode, when image is minimized
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p_command.m_objectData.m_tex2DData.m_minificationFilter);

					// Texture anisotropic filtering
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Config::textureVar().gl_texture_anisotropy);
				}
				break;

			case LoadObject_TextureCube:
				{
					// Generate, bind and upload the texture
					glGenTextures(1, &p_command.m_handle);
					glBindTexture(GL_TEXTURE_CUBE_MAP, p_command.m_handle);

					for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
							p_command.m_objectData.m_cubeMapData.m_mipmapLevel,
							p_command.m_objectData.m_cubeMapData.m_texFormat,
							p_command.m_objectData.m_cubeMapData.m_textureWidth,
							p_command.m_objectData.m_cubeMapData.m_textureHeight,
							0,
							p_command.m_objectData.m_cubeMapData.m_texFormat,
							GL_UNSIGNED_BYTE,
							p_command.m_objectData.m_cubeMapData.m_data[face]);

						// Release memory
						//FreeImage_Unload(m_bitmap[face]);
						//m_pixelData[face] = nullptr;
						//m_bitmap[face] = nullptr;

					}

					glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

					// Generate  mipmaps if they are enabled
					if(Config::textureVar().generate_mipmaps)
						glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

					// Texture filtering mode, when image is minimized
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, Config::textureVar().gl_texture_minification);
					// Texture filtering mode, when image is magnified
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, Config::textureVar().gl_texture_magnification);
					// Texture anisotropic filtering
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, Config::textureVar().gl_texture_anisotropy);
				}
				break;

			case LoadObject_Model:
				{
					// Create and bind the Vertex Array Object
					glGenVertexArrays(1, &p_command.m_handle);
					glBindVertexArray(p_command.m_handle);

					// Create the m_buffers
					glGenBuffers(sizeof(p_command.m_objectData.m_modelData.m_buffers) / sizeof(p_command.m_objectData.m_modelData.m_buffers[0]), p_command.m_objectData.m_modelData.m_buffers);

					// Upload indices
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_command.m_objectData.m_modelData.m_buffers[ModelBuffer_Index]);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						p_command.m_objectData.m_modelData.m_size[ModelBuffer_Index],
						p_command.m_objectData.m_modelData.m_data[ModelBuffer_Index],
						GL_STATIC_DRAW);

					// Loop over all the buffer types except index buffer 
					// (since index buffer does not share the same properties as other buffer types)
					for(unsigned int i = 0; i < ModelBuffer_NumTypesWithoutIndex; i++)
					{
						// Bind a buffer and upload data to it
						glBindBuffer(GL_ARRAY_BUFFER, p_command.m_objectData.m_modelData.m_buffers[i]);
						glBufferData(GL_ARRAY_BUFFER,
							p_command.m_objectData.m_modelData.m_size[i],
							p_command.m_objectData.m_modelData.m_data[i],
							GL_STATIC_DRAW);

						// Enable and bind the buffer to a vertex attribute array (so it can be accessed in the shader)
						glEnableVertexAttribArray(i);
						glVertexAttribPointer(i, p_command.m_objectData.m_modelData.m_numElements[i], GL_FLOAT, GL_FALSE, 0, 0);
					}
				}
				break;

			default:
				break;
		}
	}
	inline void processCommand(const UnloadObjectType p_objectType, const int p_count, unsigned int *p_handles)
	{
		switch(p_objectType)
		{
			case UnloadObjectType_VAO:
				glDeleteVertexArrays(p_count, p_handles);
				break;
			case UnloadObjectType_Buffer:
				glDeleteBuffers(p_count, p_handles);
				break;
			case UnloadObjectType_Shader:
				break;
			case UnloadObjectType_Texture:
				glDeleteTextures(p_count, p_handles);
				break;
		}
	}

	CurrentState m_rendererState;

	static SingleTriangle m_fullscreenTriangle;

	GeometryBuffer *m_gbuffer;
	CSMFramebuffer *m_csmBuffer;
};
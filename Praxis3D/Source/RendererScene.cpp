
#include "ComponentConstructorInfo.h"
#include "WorldScene.h"
#include "RendererScene.h"
#include "RendererSystem.h"
#include "SceneLoader.h"
#include "SpatialComponent.h"
#include "WorldScene.h"

RendererScene::RendererScene(RendererSystem *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Renderer)
{
	m_renderTask = new RenderTask(this, p_system->getRenderer());

	m_sceneAOData.setDefaultValues();

	m_renderToTexture = false;
	m_renderToTextureResolution = glm::ivec2(Config::graphicsVar().current_resolution_x, Config::graphicsVar().current_resolution_y);
}

RendererScene::~RendererScene()
{
	//delete m_camera;
}

ErrorCode RendererScene::init()
{
	ErrorCode returnError = ErrorCode::Success;
			
	// Create a default camera, in case it is not created upon loading a scene
	//m_camera = new CameraObject(this, "Default Camera");

	// Create a default directional light, in case it is not created upon loading a scene
	//m_directionalLight = new DirectionalLightObject(this, "Default Directional Light", DirectionalLightDataSet());
	
	// Create a default static environment map, so it can be used while a real one hasn't been loaded yet
	//m_sceneObjects.m_staticSkybox = new EnvironmentMapObject(this, "default", Loaders::textureCubemap().load());

	return returnError;
}

ErrorCode RendererScene::setup(const PropertySet &p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Get model and shader components pool sizes
	int modelComponentPoolSize = std::max(Config::objectPoolVar().model_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::ModelComponent).getInt());
	int shaderComponentPoolSize = std::max(Config::objectPoolVar().shader_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::ShaderComponent).getInt());

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<CameraComponent>(std::max(Config::objectPoolVar().camera_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::CameraComponent).getInt()));
	worldScene->reserve<LightComponent>(std::max(Config::objectPoolVar().light_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::LightComponent).getInt()));
	worldScene->reserve<ModelComponent>(modelComponentPoolSize);
	worldScene->reserve<ShaderComponent>(shaderComponentPoolSize);
	worldScene->reserve<GraphicsLoadToMemoryComponent>(modelComponentPoolSize + shaderComponentPoolSize);
	worldScene->reserve<GraphicsLoadToVideoMemoryComponent>(modelComponentPoolSize + shaderComponentPoolSize);

	// Load graphics property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
			case Properties::CameraID:
				m_sceneObjects.m_activeCameraID = p_properties[i].getInt();
				break;
			case Properties::StochasticSamplingSeamFix:
				m_miscSceneData.m_stochasticSamplingSeamFix = p_properties[i].getBool();
				break;
			case Properties::ZFar:
				m_sceneObjects.m_zFar = p_properties[i].getFloat();
				break;
			case Properties::ZNear:
				m_sceneObjects.m_zNear = p_properties[i].getFloat();
				break;
		}
	}

	// Load ambient intensity
	if(auto &ambientProperty = p_properties.getPropertySetByID(Properties::AmbientIntensity); ambientProperty)
	{
		for(decltype(ambientProperty.getNumProperties()) i = 0, size = ambientProperty.getNumProperties(); i < size; i++)
		{
			switch(ambientProperty[i].getPropertyID())
			{
				case Properties::DirectionalLight:
					m_miscSceneData.m_ambientIntensityDirectional = ambientProperty[i].getFloat();
					break;
				case Properties::PointLight:
					m_miscSceneData.m_ambientIntensityPoint = ambientProperty[i].getFloat();
					break;
				case Properties::SpotLight:
					m_miscSceneData.m_ambientIntensitySpot = ambientProperty[i].getFloat();
					break;
			}
		}
	}

	// Load ambient occlusion data
	if(auto &aoProperty = p_properties.getPropertySetByID(Properties::AmbientOcclusion); aoProperty)
	{
		for(decltype(aoProperty.getNumProperties()) i = 0, size = aoProperty.getNumProperties(); i < size; i++)
		{
			switch(aoProperty[i].getPropertyID())
			{
				case Properties::Type:
					m_sceneAOData.m_aoType = AmbientOcclusionData::propertyIDToAmbientOcclusionType(aoProperty[i].getID());
					break;
				case Properties::Bias:
					m_sceneAOData.m_aoBias = aoProperty[i].getFloat();
					break;
				case Properties::Directions:
					m_sceneAOData.m_aoNumOfDirections = aoProperty[i].getInt();
					break;
				case Properties::Radius:
					m_sceneAOData.m_aoRadius = aoProperty[i].getFloat();
					break;
				case Properties::Intensity:
					m_sceneAOData.m_aoIntensity = aoProperty[i].getFloat();
					break;
				case Properties::Samples:
					m_sceneAOData.m_aoNumOfSamples = aoProperty[i].getInt();
					break;
				case Properties::Steps:
					m_sceneAOData.m_aoNumOfSteps = aoProperty[i].getInt();
					break;
				case Properties::BlurSamples:
					m_sceneAOData.m_aoBlurNumOfSamples = aoProperty[i].getInt();
					break;
				case Properties::BlurSharpness:
					m_sceneAOData.m_aoBlurSharpness = aoProperty[i].getFloat();
					break;
			}
		}
	}

	// Load atmospheric light scattering
	if(auto &atmScatteringProperty = p_properties.getPropertySetByID(Properties::AtmosphericScattering); atmScatteringProperty)
	{
		if(auto &atmosphereProperty = atmScatteringProperty.getPropertySetByID(Properties::Atmosphere); atmosphereProperty)
		{
			for(decltype(atmosphereProperty.getNumProperties()) i = 0, size = atmosphereProperty.getNumProperties(); i < size; i++)
			{
				switch(atmosphereProperty[i].getPropertyID())
				{
					case Properties::Bottom:
						m_sceneAtmScatteringData.m_atmosphereBottomRadius = atmosphereProperty[i].getFloat();
						break;
					case Properties::Top:
						m_sceneAtmScatteringData.m_atmosphereTopRadius = atmosphereProperty[i].getFloat();
						break;
				}
			}

			if(auto &rayleighProperty = atmosphereProperty.getPropertySetByID(Properties::Rayleigh); rayleighProperty)
			{
				if(auto &scatteringProperty = rayleighProperty.getPropertyByID(Properties::Scattering); scatteringProperty)
					m_sceneAtmScatteringData.m_rayleighScattering = scatteringProperty.getVec3f();

				if(auto &densityProperty = rayleighProperty.getPropertySetByID(Properties::Density); densityProperty)
				{
					for(unsigned int i = 0; i < 2; i++)
						if(densityProperty.getNumPropertySets() > i)
							loadAtmosphericDensityLayer(densityProperty.getPropertySet(i), m_sceneAtmScatteringData.m_rayleighDensity[i]);
				}
			}

			if(auto &mieProperty = atmosphereProperty.getPropertySetByID(Properties::Mie); mieProperty)
			{
				if(auto &scatteringProperty = mieProperty.getPropertyByID(Properties::Scattering); scatteringProperty)
					m_sceneAtmScatteringData.m_mieScattering = scatteringProperty.getVec3f();

				if(auto &extinctionProperty = mieProperty.getPropertyByID(Properties::Extinction); extinctionProperty)
					m_sceneAtmScatteringData.m_mieExtinction = extinctionProperty.getVec3f();

				if(auto &densityProperty = mieProperty.getPropertySetByID(Properties::Density); densityProperty)
				{
					for(unsigned int i = 0; i < 2; i++)
						if(densityProperty.getNumPropertySets() > i)
							loadAtmosphericDensityLayer(densityProperty.getPropertySet(i), m_sceneAtmScatteringData.m_mieDensity[i]);
				}
			}

			if(auto &absorptionProperty = atmosphereProperty.getPropertySetByID(Properties::Absorption); absorptionProperty)
			{
				if(auto &extinctionProperty = absorptionProperty.getPropertyByID(Properties::Extinction); extinctionProperty)
					m_sceneAtmScatteringData.m_absorptionExtinction = extinctionProperty.getVec3f();

				if(auto &densityProperty = absorptionProperty.getPropertySetByID(Properties::Density); densityProperty)
				{
					for(unsigned int i = 0; i < 2; i++)
						if(densityProperty.getNumPropertySets() > i)
							loadAtmosphericDensityLayer(densityProperty.getPropertySet(i), m_sceneAtmScatteringData.m_absorptionDensity[i]);
				}
			}
		}
		if(auto &groundProperty = atmScatteringProperty.getPropertySetByID(Properties::Ground); groundProperty)
		{
			for(decltype(groundProperty.getNumProperties()) i = 0, size = groundProperty.getNumProperties(); i < size; i++)
			{
				switch(groundProperty[i].getPropertyID())
				{
					case Properties::Color:
						m_sceneAtmScatteringData.m_planetGroundColor = groundProperty[i].getVec3f();
						break;
					case Properties::LocalPosition:
						m_sceneAtmScatteringData.m_planetCenterPosition = groundProperty[i].getVec3f();
						break;
				}
			}
		}
		if(auto &sunProperty = atmScatteringProperty.getPropertySetByID(Properties::Sun); sunProperty)
		{
			for(decltype(sunProperty.getNumProperties()) i = 0, size = sunProperty.getNumProperties(); i < size; i++)
			{
				switch(sunProperty[i].getPropertyID())
				{
					case Properties::Irradiance:
						m_sceneAtmScatteringData.m_sunIrradiance = sunProperty[i].getVec3f();
						break;
					case Properties::Size:
						m_sceneAtmScatteringData.m_sunSize = sunProperty[i].getFloat();
						break;
				}
			}
		}
	}

	// Load the rendering passes
	auto &renderPassesProperty = p_properties.getPropertySetByID(Properties::RenderPasses);
	if(renderPassesProperty)
	{
		// Iterate over the property array
		for(decltype(renderPassesProperty.getNumPropertySets()) objIndex = 0, objSize = renderPassesProperty.getNumPropertySets(); objIndex < objSize; objIndex++)
		{
			auto &typeProperty = renderPassesProperty.getPropertySetUnsafe(objIndex).getPropertyByID(Properties::Type);
			if(typeProperty)
			{
				switch(typeProperty.getID())
				{
					case Properties::AmbientOcclusionRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_AmbientOcclusion);
						break;
					case Properties::AtmScatteringRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
						break;
					case Properties::BloomRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_Bloom);
						break;
					case Properties::FinalRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_Final);
						break;
					case Properties::GeometryRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_Geometry);
						break;
					case Properties::GUIRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_GUI);
						break;
					case Properties::LightingRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_Lighting);
						break;
					case Properties::LuminanceRenderPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_Luminance);
						break;
					case Properties::ShadowMappingPass:
						m_renderingPasses.push_back(RenderPassType::RenderPassType_ShadowMapping);
						break;
				}
			}
		}
	}

	// Load shadow mapping data
	if(auto &shadowMappingProperty = p_properties.getPropertySetByID(Properties::ShadowMapping); shadowMappingProperty)
	{
		for(decltype(shadowMappingProperty.getNumProperties()) i = 0, size = shadowMappingProperty.getNumProperties(); i < size; i++)
		{
			switch(shadowMappingProperty[i].getPropertyID())
			{
				// Set CSM bias scale
				case Properties::BiasScale:
					m_shadowMappingData.m_csmBiasScale = shadowMappingProperty[i].getFloat();
					break;

				// Set CSM penumbra size (shadow edge softness)
				case Properties::PenumbraSize:
					m_shadowMappingData.m_penumbraSize = shadowMappingProperty[i].getFloat();
					break;

				// Set CSM penumbra size scale range
				case Properties::PenumbraScaleRange:
					m_shadowMappingData.m_penumbraScaleRange = shadowMappingProperty[i].getVec2f();
					break;

				// Set CSM resolution property
				case Properties::Resolution:
					m_shadowMappingData.m_csmResolution = (unsigned int)shadowMappingProperty[i].getInt();
					break;

				// Set CSM cascade plane z clip distance multiplier
				case Properties::ZClipping:
					m_shadowMappingData.m_zClipping = shadowMappingProperty[i].getBool();
					break;

				// Set CSM cascade plane z clip distance multiplier
				case Properties::ZPlaneMultiplier:
					m_shadowMappingData.m_csmCascadePlaneZMultiplier = shadowMappingProperty[i].getFloat();
					break;
			}
		}

		// Find PCF property set
		if(auto &pcfProperty = shadowMappingProperty.getPropertySetByID(Properties::PCF); pcfProperty)
		{
			// Find the Samples property
			if(auto &samplesProperty = pcfProperty.getPropertyByID(Properties::Samples); samplesProperty)
			{
				// Get the number of samples and make sure it is not zero or lower
				int samples = samplesProperty.getInt();
				if(samples > 0)
					m_shadowMappingData.m_numOfPCFSamples = (unsigned int)samples;
			}
		}

		// Find Cascades property set
		if(auto &cascadesProperty = shadowMappingProperty.getPropertySetByID(Properties::Cascades); cascadesProperty)
		{
			// Go over each array entry of Cascades property set
			for(decltype(cascadesProperty.getNumPropertySets()) i = 0, size = cascadesProperty.getNumPropertySets(); i < size; i++)
			{
				ShadowCascadeData cascadeData;

				// If the Distance property is present, add the shadow cascade distance and set the divider flag to FALSE
				// Otherwise, if the Divider property is present, add the divider value and set the divider flag to TRUE
				if(auto &distanceProperty = cascadesProperty.getPropertySetUnsafe(i).getPropertyByID(Properties::Distance); distanceProperty)
				{
					cascadeData.m_cascadeFarDistance = distanceProperty.getFloat();
					cascadeData.m_distanceIsDivider = false;
				}
				else
					if(auto &dividerProperty = cascadesProperty.getPropertySetUnsafe(i).getPropertyByID(Properties::Divider); dividerProperty)
					{
						cascadeData.m_cascadeFarDistance = dividerProperty.getFloat();
						cascadeData.m_distanceIsDivider = true;
					}

				// Find and add the max bias value
				if(auto &biasMaxProperty = cascadesProperty.getPropertySetUnsafe(i).getPropertyByID(Properties::BiasMax); biasMaxProperty)
					cascadeData.m_maxBias = biasMaxProperty.getFloat();

				// Find and add the penumbra scale value
				if(auto &penumbraScaleProperty = cascadesProperty.getPropertySetUnsafe(i).getPropertyByID(Properties::PenumbraScale); penumbraScaleProperty)
					cascadeData.m_penumbraScale = penumbraScaleProperty.getFloat();

				m_shadowMappingData.m_shadowCascadePlaneDistances.push_back(cascadeData);
			}
		}
	}

	return ErrorCode::Success;
}

void RendererScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add graphics data
	{
		p_propertySet.addProperty(Properties::CameraID, m_sceneObjects.m_activeCameraID);
		p_propertySet.addProperty(Properties::StochasticSamplingSeamFix, m_miscSceneData.m_stochasticSamplingSeamFix);
	}

	// Add ambient intensity
	{
		auto &ambientPropertySet = p_propertySet.addPropertySet(Properties::AmbientIntensity);
		ambientPropertySet.addProperty(Properties::DirectionalLight, m_miscSceneData.m_ambientIntensityDirectional);
		ambientPropertySet.addProperty(Properties::PointLight, m_miscSceneData.m_ambientIntensityPoint);
		ambientPropertySet.addProperty(Properties::SpotLight, m_miscSceneData.m_ambientIntensitySpot);
	}

	// Add ambient occlusion data
	{
		auto &aoPropertySet = p_propertySet.addPropertySet(Properties::AmbientOcclusion);
		aoPropertySet.addProperty(Properties::Type, AmbientOcclusionData::ambientOcclusionTypeToPropertyID(m_sceneAOData.m_aoType));
		aoPropertySet.addProperty(Properties::Bias, m_sceneAOData.m_aoBias);
		aoPropertySet.addProperty(Properties::Radius, m_sceneAOData.m_aoRadius);
		aoPropertySet.addProperty(Properties::Intensity, m_sceneAOData.m_aoIntensity);
		aoPropertySet.addProperty(Properties::Directions, m_sceneAOData.m_aoNumOfDirections);
		aoPropertySet.addProperty(Properties::Samples, m_sceneAOData.m_aoNumOfSamples);
		aoPropertySet.addProperty(Properties::Steps, m_sceneAOData.m_aoNumOfSteps);
		aoPropertySet.addProperty(Properties::BlurSamples, m_sceneAOData.m_aoBlurNumOfSamples);
		aoPropertySet.addProperty(Properties::BlurSharpness, m_sceneAOData.m_aoBlurSharpness);
	}

	// Add atmospheric light scattering data
	{
		auto &atmScatteringProperty = p_propertySet.addPropertySet(Properties::AtmosphericScattering);
		{
			auto &atmosphereProperty = atmScatteringProperty.addPropertySet(Properties::Atmosphere);
			atmosphereProperty.addProperty(Properties::Bottom, m_sceneAtmScatteringData.m_atmosphereBottomRadius);
			atmosphereProperty.addProperty(Properties::Top, m_sceneAtmScatteringData.m_atmosphereTopRadius);

			{
				auto &rayleighProperty = atmosphereProperty.addPropertySet(Properties::Rayleigh);
				atmosphereProperty.addProperty(Properties::Scattering, m_sceneAtmScatteringData.m_rayleighScattering);

				auto &densityProperty = rayleighProperty.addPropertySet(Properties::Density);
				for(unsigned int i = 0; i < 2; i++)
				{
					auto &layerProperty = densityProperty.addPropertySet(Properties::ArrayEntry);

					layerProperty.addProperty(Properties::ConstantTerm, m_sceneAtmScatteringData.m_rayleighDensity[i].m_constantTerm);
					layerProperty.addProperty(Properties::LinearTerm, m_sceneAtmScatteringData.m_rayleighDensity[i].m_linearTerm);
					layerProperty.addProperty(Properties::ExpScale, m_sceneAtmScatteringData.m_rayleighDensity[i].m_expScale);
					layerProperty.addProperty(Properties::ExpTerm, m_sceneAtmScatteringData.m_rayleighDensity[i].m_expTerm);
					layerProperty.addProperty(Properties::Width, m_sceneAtmScatteringData.m_rayleighDensity[i].m_width);
				}
			}

			{
				auto &mieProperty = atmosphereProperty.addPropertySet(Properties::Mie);
				mieProperty.addProperty(Properties::Extinction, m_sceneAtmScatteringData.m_mieExtinction);
				mieProperty.addProperty(Properties::Scattering, m_sceneAtmScatteringData.m_mieScattering);

				auto &densityProperty = mieProperty.addPropertySet(Properties::Density);
				for(unsigned int i = 0; i < 2; i++)
				{
					auto &layerProperty = densityProperty.addPropertySet(Properties::ArrayEntry);

					layerProperty.addProperty(Properties::ConstantTerm, m_sceneAtmScatteringData.m_mieDensity[i].m_constantTerm);
					layerProperty.addProperty(Properties::LinearTerm, m_sceneAtmScatteringData.m_mieDensity[i].m_linearTerm);
					layerProperty.addProperty(Properties::ExpScale, m_sceneAtmScatteringData.m_mieDensity[i].m_expScale);
					layerProperty.addProperty(Properties::ExpTerm, m_sceneAtmScatteringData.m_mieDensity[i].m_expTerm);
					layerProperty.addProperty(Properties::Width, m_sceneAtmScatteringData.m_mieDensity[i].m_width);
				}
			}

			{
				auto &absorptionProperty = atmosphereProperty.addPropertySet(Properties::Absorption);
				absorptionProperty.addProperty(Properties::Extinction, m_sceneAtmScatteringData.m_absorptionExtinction);

				auto &densityProperty = absorptionProperty.addPropertySet(Properties::Density);
				for(unsigned int i = 0; i < 2; i++)
				{
					auto &layerProperty = densityProperty.addPropertySet(Properties::ArrayEntry);

					layerProperty.addProperty(Properties::ConstantTerm, m_sceneAtmScatteringData.m_absorptionDensity[i].m_constantTerm);
					layerProperty.addProperty(Properties::LinearTerm, m_sceneAtmScatteringData.m_absorptionDensity[i].m_linearTerm);
					layerProperty.addProperty(Properties::ExpScale, m_sceneAtmScatteringData.m_absorptionDensity[i].m_expScale);
					layerProperty.addProperty(Properties::ExpTerm, m_sceneAtmScatteringData.m_absorptionDensity[i].m_expTerm);
					layerProperty.addProperty(Properties::Width, m_sceneAtmScatteringData.m_absorptionDensity[i].m_width);
				}
			}
		}
		{
			auto &groundProperty = atmScatteringProperty.addPropertySet(Properties::Ground);
			groundProperty.addProperty(Properties::Color, m_sceneAtmScatteringData.m_planetGroundColor);
			groundProperty.addProperty(Properties::LocalPosition, m_sceneAtmScatteringData.m_planetCenterPosition);
		}
		{
			auto &sunProperty = atmScatteringProperty.addPropertySet(Properties::Sun);
			sunProperty.addProperty(Properties::Irradiance, m_sceneAtmScatteringData.m_sunIrradiance);
			sunProperty.addProperty(Properties::Size, m_sceneAtmScatteringData.m_sunSize);
		}
	}

	// Add object pool sizes
	{
		auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
		objectPoolSizePropertySet.addProperty(Properties::CameraComponent, (int)worldScene->getPoolSize<CameraComponent>());
		objectPoolSizePropertySet.addProperty(Properties::LightComponent, (int)worldScene->getPoolSize<LightComponent>());
		objectPoolSizePropertySet.addProperty(Properties::ModelComponent, (int)worldScene->getPoolSize<ModelComponent>());
		objectPoolSizePropertySet.addProperty(Properties::ShaderComponent, (int)worldScene->getPoolSize<ShaderComponent>());
	}

	// Add rendering passes
	exportRenderingPasses(p_propertySet, m_renderingPasses);

	// Add shadow mapping data
	{
		auto &shadowMappingPropertySet = p_propertySet.addPropertySet(Properties::ShadowMapping);

		shadowMappingPropertySet.addProperty(Properties::BiasScale, m_shadowMappingData.m_csmBiasScale);
		shadowMappingPropertySet.addProperty(Properties::PenumbraSize, m_shadowMappingData.m_penumbraSize);
		shadowMappingPropertySet.addProperty(Properties::PenumbraScaleRange, m_shadowMappingData.m_penumbraScaleRange);
		shadowMappingPropertySet.addProperty(Properties::Resolution, (int)m_shadowMappingData.m_csmResolution);
		shadowMappingPropertySet.addProperty(Properties::ZClipping, m_shadowMappingData.m_zClipping);
		shadowMappingPropertySet.addProperty(Properties::ZPlaneMultiplier, m_shadowMappingData.m_csmCascadePlaneZMultiplier);

		auto &pcfPropertySet = shadowMappingPropertySet.addPropertySet(Properties::PCF);
		pcfPropertySet.addProperty(Properties::Samples, (int)m_shadowMappingData.m_numOfPCFSamples);

		// Go over each CSM cascade plane
		auto &cascadesPropertySet = shadowMappingPropertySet.addPropertySet(Properties::Cascades);
		for(decltype(m_shadowMappingData.m_shadowCascadePlaneDistances.size()) i = 0, size = m_shadowMappingData.m_shadowCascadePlaneDistances.size(); i < size; i++)
		{
			// Set the export cascade type property to either Divider or Distance, based on the divider flag
			Properties::PropertyID cascadeTypeProperty = m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_distanceIsDivider ? Properties::Divider : Properties::Distance;

			// Add the cascade plane array entry
			auto &cascadeArrayEntryProperty = cascadesPropertySet.addPropertySet(Properties::ArrayEntry);

			// Add distance or divider property
			cascadeArrayEntryProperty.addProperty(cascadeTypeProperty, m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_cascadeFarDistance);

			// Add max bias property
			cascadeArrayEntryProperty.addProperty(Properties::BiasMax, m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_maxBias);

			// Add penumbra scale property
			cascadeArrayEntryProperty.addProperty(Properties::PenumbraScale, m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_penumbraScale);
		}
	}
}

void RendererScene::activate()
{
	auto rendererSystem = static_cast<RendererSystem *>(m_system);

	// Set whether render the scene to texture or the whole screen
	rendererSystem->getRenderer().setRenderFinalToTexture(m_renderToTexture);

	// Set render-to-texture resolution, if enabled
	if(m_renderToTexture)
	{
		rendererSystem->getRenderer().setRenderToTextureResolution(m_renderToTextureResolution);
	}

	// Set the AO data
	rendererSystem->getRenderer().setAmbientOcclusionData(m_sceneAOData);

	// Set the atmospheric scattering data
	rendererSystem->getRenderer().setAtmScatteringData(m_sceneAtmScatteringData);

	// Set the misc scene data
	rendererSystem->getRenderer().setMiscSceneData(m_miscSceneData);

	// Set the shadow mapping data
	rendererSystem->getRenderer().setShadowMappingData(m_shadowMappingData);

	// Set rendering passes
	rendererSystem->setRenderingPasses(m_renderingPasses);

	// Update the internal shadow mapping enabled flag, as it is set based on the rendering passes that are given to the renderer system
	m_shadowMappingData.m_shadowMappingEnabled = rendererSystem->getRenderer().getShadowMappingData().m_shadowMappingEnabled;
}

void RendererScene::deactivate()
{
	auto rendererSystem = static_cast<RendererSystem *>(m_system);

	// Save the current AO data
	m_sceneAOData = rendererSystem->getRenderer().getFrameData().m_aoData;
}

ErrorCode RendererScene::preload()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	std::vector<SystemObject *> componentsToLoad;

	auto modelAndLoadView = entityRegistry.view<ModelComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : modelAndLoadView)
		componentsToLoad.push_back(&modelAndLoadView.get<ModelComponent>(entity));

	auto shaderAndLoadView = entityRegistry.view<ShaderComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : shaderAndLoadView)
		componentsToLoad.push_back(&shaderAndLoadView.get<ShaderComponent>(entity));
		
	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), componentsToLoad.size(), size_t(1), [=](size_t i)
		{
			componentsToLoad[i]->loadToMemory();
		});

	return ErrorCode::Success;
}

void RendererScene::loadInBackground()
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	auto modelView = entityRegistry.view<ModelComponent>();
	for(auto entity : modelView)
	{
		auto &component = modelView.get<ModelComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadToMemory, &component));
	}

	auto shaderView = entityRegistry.view<ShaderComponent>();
	for(auto entity : shaderView)
	{
		auto &component = shaderView.get<ShaderComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ShaderComponent::loadToMemory, &component));
	}
}

void RendererScene::update(const float p_deltaTime)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	bool loadingStatus = false;

	//	 _______________________________
	//	|							    |
	//	|	 CHECK FOR LOAD PENDING		|
	//	|_______________________________|
	//
	// Go over each not-loading components and check whether they have anything that needs to be loaded
	auto modelNoLoadView = entityRegistry.view<ModelComponent, SpatialComponent>(entt::exclude<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>);
	for(auto entity : modelNoLoadView)
	{
		// Get the component
		auto &component = modelNoLoadView.get<ModelComponent>(entity);

		// Check if there is anything that needs to be loaded
		if(component.getLoadPending())
		{
			component.importModels();
			if(!component.getModelsNeedsLoading())
			{
				component.importTextures();
				if(!component.getTexturesNeedsLoading())
				{
					// Reset load pending flag
					component.resetLoadPending();
					component.setLoadedToMemory(true);
					component.setLoadedToVideoMemory(true);
				}
				else
				{
					// Add load to memory component, to mark the entity as loading
					worldScene->addComponent<GraphicsLoadToMemoryComponent>(entity, entity);

					// Set the loading status flag to true
					loadingStatus = true;

					// Reset load pending flag
					component.resetLoadPending();
					component.setLoadedToMemory(false);
					component.setLoadedToVideoMemory(false);

					// Start loading the component to memory in the background
					TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadTexturesToMemory, &component));
				}
			}
			else
			{
				// Add load to memory component, to mark the entity as loading
				worldScene->addComponent<GraphicsLoadToMemoryComponent>(entity, entity);

				// Set the loading status flag to true
				loadingStatus = true;

				// Reset load pending flag
				component.resetLoadPending();
				component.setLoadedToMemory(false);
				component.setLoadedToVideoMemory(false);

				// Start loading the component to memory in the background
				TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadModelsToMemory, &component));
			}
		}
	}

	//	 _______________________________
	//	|							    |
	//	| CURRENTLY LOADING COMPONENTS	|
	//	|		TO VIDEO MEMORY			|
	//	|_______________________________|
	//
	// Remove the load-to-video-memory component for entities that have already been loaded during the previous frame
	auto loadToVideoMemoryView = entityRegistry.view<GraphicsLoadToVideoMemoryComponent>(entt::exclude<GraphicsLoadToMemoryComponent>);
	for(auto entity : loadToVideoMemoryView)
	{
		auto &loadToVideoMemoryComponent = loadToVideoMemoryView.get<GraphicsLoadToVideoMemoryComponent>(entity);
		if(loadToVideoMemoryComponent.m_loaded)
		{
			// Set model component as loaded to video memory, if it is present
			auto modelComponent = entityRegistry.try_get<ModelComponent>(entity);
			if(modelComponent != nullptr)
			{
				modelComponent->setLoadedToVideoMemory(true);
				//modelComponent->unloadFromMemory();
			}

			// Set shader component as loaded to video memory, if it is present
			auto shaderComponent = entityRegistry.try_get<ShaderComponent>(entity);
			if(shaderComponent != nullptr)
			{
				shaderComponent->setLoadedToVideoMemory(true);
				//shaderComponent->unloadFromMemory();
			}

			// Remove load-to-video-memory component to mark the entity as loaded to GPU
			worldScene->removeComponent<GraphicsLoadToVideoMemoryComponent>(entity);
		}
		else
		{
			// Set the loading status flag to true
			loadingStatus = true;
		}
	}

	//	 _______________________________
	//	|							    |
	//	| CURRENTLY LOADING COMPONENTS	|
	//	|		   TO MEMORY			|
	//	|_______________________________|
	//
	// Check the entities that are being loaded to memory
	// If they have already been loaded, remove the load-to-memory component and add the load-to-video-memory component, so they can be loaded to GPU in the renderer
	auto modelAndLoadView = entityRegistry.view<ModelComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : modelAndLoadView)
	{
		auto &modelComponent = modelAndLoadView.get<ModelComponent>(entity);
		auto shaderComponent = entityRegistry.try_get<ShaderComponent>(entity);

		// In addition to model component, the entity might contain a shader component too
		// In that case, process both model and shader components
		if(shaderComponent == nullptr)
		{
			// Perform a check that marks an object if it is loaded to memory
			if(!modelComponent.isLoadedToMemory())
				modelComponent.performCheckIsLoadedToMemory();

			// If the object has loaded to memory already, add to load queue
			if(modelComponent.isLoadedToMemory())
			{
				// Remove the load-to-memory component, signifying that it has already been loaded to memory
				worldScene->removeComponent<GraphicsLoadToMemoryComponent>(entity);

				// Make the component active, so it is processed in the renderer
				modelComponent.setActive(modelComponent.m_setActiveAfterLoading);

				// Do not add the load-to-video-memory component if the object is already loaded to GPU
				modelComponent.performCheckIsLoadedToVideoMemory();
				if(!modelComponent.isLoadedToVideoMemory())
				{
					auto &loadToVideoMemoryComponent = worldScene->addComponent<GraphicsLoadToVideoMemoryComponent>(entity, entity);

					// Get all loadable objects from the model component
					auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

					// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
					for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
					{
						if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
						{
							loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromModel[i]);
						}
					}

					// Set the loading status flag to true
					loadingStatus = true;
				}
			}
			else
			{
				// Set the loading status flag to true
				loadingStatus = true;
			}
		}
		else
		{
			// Perform a check on model component that marks an object if it is loaded to memory
			if(!modelComponent.isLoadedToMemory())
				modelComponent.performCheckIsLoadedToMemory();

			// If the model component has loaded to memory already, add to load queue
			if(shaderComponent->isLoadedToMemory())
			{
				// Perform a check on shader component that marks an object if it is loaded to memory
				if(!shaderComponent->isLoadedToMemory())
					shaderComponent->performCheckIsLoadedToMemory();

				// If the shader component has loaded to memory already, add to load queue
				if(shaderComponent->isLoadedToMemory())
				{
					// Remove the load-to-memory component, signifying that it has already been loaded to memory
					worldScene->removeComponent<GraphicsLoadToMemoryComponent>(entity);

					// Make the components active, so it is processed in the renderer
					modelComponent.setActive(modelComponent.m_setActiveAfterLoading);
					shaderComponent->setActive(shaderComponent->m_setActiveAfterLoading);

					// Do not add the load-to-video-memory component if the object is already loaded to GPU
					modelComponent.performCheckIsLoadedToVideoMemory();
					shaderComponent->performCheckIsLoadedToVideoMemory();
					if(!modelComponent.isLoadedToVideoMemory() || !shaderComponent->isLoadedToVideoMemory())
					{
						auto &loadToVideoMemoryComponent = worldScene->addComponent<GraphicsLoadToVideoMemoryComponent>(entity, entity);

						if(!modelComponent.isLoadedToVideoMemory())
						{
							// Get all loadable objects from the model component
							auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

								// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
								for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
									if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
										loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromModel[i]);
						}

						if(!shaderComponent->isLoadedToVideoMemory())
						{
							// Get all loadable objects from the shader component
							auto loadableObjectsFromShader = shaderComponent->getLoadableObjects();

							// Iterate over all loadable objects from the shader component, and if any of them are not loaded to video memory already, add them to the to-load list
							for(decltype(loadableObjectsFromShader.size()) size = loadableObjectsFromShader.size(), i = 0; i < size; i++)
								if(!loadableObjectsFromShader[i].isLoadedToVideoMemory())
									loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromShader[i]);
						}
					}
				}
			}
			else
			{
				// Set the loading status flag to true
				loadingStatus = true;
			}
		}
	}

	m_loadingStatus = loadingStatus;
	m_sceneObjects.m_processDrawing = true;

	// If the render-to-texture is not set (i.e. no scene editor), do not process drawing while there are objects being loaded
	// Perform only during the first initial scene loading
	if(m_sceneLoader->getFirstLoad() && !m_renderTask->m_renderer.getRenderFinalToTexture())
	{
		m_sceneObjects.m_processDrawing = !m_loadingStatus;
	}

	//	 ___________________________
	//	|							|
	//	|	  COMPONENT UPDATES		|
	//	|___________________________|
	//

	//	 ___________________________
	//	|							|
	//	|	  MODEL COMPONENTS		|
	//	|___________________________|
	//
	m_sceneObjects.m_models = entityRegistry.view<ModelComponent, SpatialComponent>(entt::exclude<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>);
	m_sceneObjects.m_modelsWithShaders = entityRegistry.view<ModelComponent, ShaderComponent, SpatialComponent>(entt::exclude<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>);
	m_sceneObjects.m_objectsToLoadToVideoMemory = entityRegistry.view<GraphicsLoadToVideoMemoryComponent>(entt::exclude<GraphicsLoadToMemoryComponent>);

	//	 ___________________________
	//	|							|
	//	|	  LIGHT COMPONENTS		|
	//	|___________________________|
	//
	m_sceneObjects.m_lights = entityRegistry.view<LightComponent, SpatialComponent>();

	//	 ___________________________
	//	|							|
	//	|	  CAMERA COMPONENTS		|
	//	|___________________________|
	//
	EntityID activeCameraEntityID = NULL_ENTITY_ID;
	auto cameraView = entityRegistry.view<CameraComponent, SpatialComponent>();
	for(auto entity : cameraView)
	{
		activeCameraEntityID = entity;

		if(cameraView.get<CameraComponent>(entity).m_cameraID == m_sceneObjects.m_activeCameraID)
			break;
	}

	if(activeCameraEntityID != NULL_ENTITY_ID)
	{
		m_sceneObjects.m_activeCameraEntityID = activeCameraEntityID;

		auto &cameraComponent = cameraView.get<CameraComponent>(m_sceneObjects.m_activeCameraEntityID);
		auto &spatialComponent = cameraView.get<SpatialComponent>(m_sceneObjects.m_activeCameraEntityID);

		m_sceneObjects.m_fov = cameraComponent.m_fov;
		m_sceneObjects.m_zFar = cameraComponent.m_zFar;
		m_sceneObjects.m_zNear = cameraComponent.m_zNear;
		m_sceneObjects.m_cameraViewMatrix = spatialComponent.getSpatialDataChangeManager().getWorldTransform();
	}
}

std::vector<SystemObject *> RendererScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *cameraComponent = entityRegistry.try_get<CameraComponent>(p_entityID);
	if(cameraComponent != nullptr)
		returnVector.push_back(cameraComponent);

	auto *lightComponent = entityRegistry.try_get<LightComponent>(p_entityID);
	if(lightComponent != nullptr)
		returnVector.push_back(lightComponent);

	auto *modelComponent = entityRegistry.try_get<ModelComponent>(p_entityID);
	if(modelComponent != nullptr)
		returnVector.push_back(modelComponent);

	auto *shaderComponent = entityRegistry.try_get<ShaderComponent>(p_entityID);
	if(shaderComponent != nullptr)
		returnVector.push_back(shaderComponent);

	return returnVector;
}

std::vector<SystemObject*> RendererScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_graphicsComponents, p_startLoading);
}

void RendererScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_graphicsComponents);
}

void RendererScene::exportComponents(const EntityID p_entityID, GraphicsComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export CameraComponent
	auto *cameraComponent = entityRegistry.try_get<CameraComponent>(p_entityID);
	if(cameraComponent != nullptr)
	{
		if(p_constructionInfo.m_cameraConstructionInfo == nullptr)
			p_constructionInfo.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_cameraConstructionInfo, *cameraComponent);
	}

	// Export LightComponent
	auto *lightComponent = entityRegistry.try_get<LightComponent>(p_entityID);
	if(lightComponent != nullptr)
	{
		if(p_constructionInfo.m_lightConstructionInfo == nullptr)
			p_constructionInfo.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_lightConstructionInfo, *lightComponent);
	}

	// Export ModelComponent
	auto *modelComponent = entityRegistry.try_get<ModelComponent>(p_entityID);
	if(modelComponent != nullptr)
	{
		if(p_constructionInfo.m_modelConstructionInfo == nullptr)
			p_constructionInfo.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_modelConstructionInfo, *modelComponent);
	}

	// Export ShaderComponent
	auto *shaderComponent = entityRegistry.try_get<ShaderComponent>(p_entityID);
	if(shaderComponent != nullptr)
	{
		if(p_constructionInfo.m_shaderConstructionInfo == nullptr)
			p_constructionInfo.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_shaderConstructionInfo, *shaderComponent);
	}
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const CameraComponent::CameraComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<CameraComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the camera component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.setActive(p_constructionInfo.m_active);
		component.setLoadedToMemory(true);
		component.setLoadedToVideoMemory(true);

		component.m_fov = p_constructionInfo.m_fov;
		component.m_cameraID = p_constructionInfo.m_cameraID;
		component.m_zFar = p_constructionInfo.m_zFar;
		component.m_zNear = p_constructionInfo.m_zNear;

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<CameraComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_CameraComponent, p_constructionInfo.m_name);
	}

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const LightComponent::LightComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Proceed only if the light type is not null
	if(p_constructionInfo.m_lightComponentType != LightComponent::LightComponentType::LightComponentType_null)
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<LightComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the light component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_lightComponentType = p_constructionInfo.m_lightComponentType;
			component.setActive(p_constructionInfo.m_active);

			// Load values based on the type of light
			switch(component.m_lightComponentType)
			{
			case LightComponent::LightComponentType::LightComponentType_directional:
				component.m_objectType = Properties::PropertyID::DirectionalLight;

				component.m_lightComponent.m_directional.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_directional.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorCode::Load_success, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
				break;

			case LightComponent::LightComponentType::LightComponentType_point:
				component.m_objectType = Properties::PropertyID::PointLight;

				component.m_lightComponent.m_point.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_point.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorCode::Load_success, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
				break;

			case LightComponent::LightComponentType::LightComponentType_spot:
				component.m_objectType = Properties::PropertyID::SpotLight;

				component.m_lightComponent.m_spot.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_spot.m_cutoffAngle = glm::radians(p_constructionInfo.m_cutoffAngle);
				component.m_lightComponent.m_spot.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorCode::Load_success, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
				break;

			case LightComponent::LightComponentType::LightComponentType_null:
			default:
				ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
				worldScene->removeComponent<LightComponent>(p_entityID);
				break;
			}

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<CameraComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
		}
	}

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const ModelComponent::ModelComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Make sure there are models present
	//if(!p_constructionInfo.m_modelsProperties.m_models.empty())
	//{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<ModelComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the model component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_setActiveAfterLoading = p_constructionInfo.m_active;
			component.setActive(false);

			component.m_modelsProperties = new ModelComponent::ModelsProperties(p_constructionInfo.m_modelsProperties);

			// Add the load-to-memory component signifying that it is currently being loaded to memory
			//m_componentsLoadingToMemory.emplace_back(component);
			worldScene->addComponent<GraphicsLoadToMemoryComponent>(p_entityID, p_entityID);

			// Start loading the component to memory in the background if the flag is set to do so
			if(p_startLoading)
				TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadToMemory, &component));

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<ModelComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ModelComponent, p_constructionInfo.m_name);
		}
	//}
	//else
	//	ErrHandlerLoc().get().log(ErrorCode::Initialize_failure, ErrorSource::Source_ModelComponent, p_constructionInfo.m_name);

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const ShaderComponent::ShaderComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Check if any of the shader nodes are present
	if(!p_constructionInfo.m_vetexShaderFilename.empty() && !p_constructionInfo.m_fragmentShaderFilename.empty())
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<ShaderComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the shader component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_setActiveAfterLoading = p_constructionInfo.m_active;
			component.setActive(false);

			// Create a property-set used to load the shaders
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::VertexShader, p_constructionInfo.m_vetexShaderFilename);
			shaderProperties.addProperty(Properties::FragmentShader, p_constructionInfo.m_fragmentShaderFilename);
			if(!p_constructionInfo.m_geometryShaderFilename.empty())
				shaderProperties.addProperty(Properties::GeometryShader, p_constructionInfo.m_geometryShaderFilename);

			// Load shader program
			auto shaderProgram = Loaders::shader().load(shaderProperties);

			// If shader is not default (i.e. at least one of the shader types was loaded)
			if(!shaderProgram->isDefaultProgram())
			{
				// Load the shader to memory and assign it to the new shader component
				shaderProgram->loadToMemory();
				component.m_shaderData = new ShaderData(*shaderProgram);

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ShaderComponent, p_constructionInfo.m_name + " - \'" + p_constructionInfo.m_fragmentShaderFilename + "\' and \'" + p_constructionInfo.m_vetexShaderFilename + "\' shaders imported.");
			}

			// Set the component as loaded, because the load function was called
			//component.setLoadedToMemory(true);
			// 
			// Add the load-to-memory component signifying that it is currently being loaded to memory
			worldScene->addComponent<GraphicsLoadToMemoryComponent>(p_entityID, p_entityID);

			// Start loading the component to memory in the background if the flag is set to do so
			if(p_startLoading)
				TaskManagerLocator::get().startBackgroundThread(std::bind(&ShaderComponent::loadToMemory, &component));

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<ShaderComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ShaderComponent, p_constructionInfo.m_name);
		}
	}
	else
		ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, p_constructionInfo.m_name, ErrorSource::Source_ShaderComponent);

	return returnObject;
}

void RendererScene::releaseObject(SystemObject *p_systemObject)
{
	switch(p_systemObject->getObjectType())
	{
		case Properties::PropertyID::CameraComponent:
			{
				auto *component = static_cast<CameraComponent *>(p_systemObject);

				// Nothing to release
			}
			break;

		case Properties::PropertyID::LightComponent:
			{
				auto *component = static_cast<LightComponent *>(p_systemObject);

				// Nothing to release
			}
			break;

		case Properties::PropertyID::ModelComponent:
			{
				auto *component = static_cast<ModelComponent *>(p_systemObject);

				// Nothing to release
			}
			break;

		case Properties::PropertyID::ShaderComponent:
			{
				auto *component = static_cast<ShaderComponent *>(p_systemObject);

				// Nothing to release
			}
			break;
	}
}

ErrorCode RendererScene::destroyObject(SystemObject *p_systemObject)
{
	ErrorCode returnError = ErrorCode::Success;

	// Get the world scene required for deleting components
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	switch(p_systemObject->getObjectType())
	{
		case Properties::PropertyID::CameraComponent:
			{
				// Delete component
				worldScene->removeComponent<CameraComponent>(p_systemObject->getEntityID());
			}
			break;

		case Properties::PropertyID::LightComponent:
		case Properties::PropertyID::DirectionalLight:
		case Properties::PropertyID::PointLight:
		case Properties::PropertyID::SpotLight:
			{
				// Delete component
				worldScene->removeComponent<LightComponent>(p_systemObject->getEntityID());
			}
			break;

		case Properties::PropertyID::ModelComponent:
			{
				// Delete component
				worldScene->removeComponent<ModelComponent>(p_systemObject->getEntityID());
			}
			break;

		case Properties::PropertyID::ShaderComponent:
			{
				// Delete component
				worldScene->removeComponent<ShaderComponent>(p_systemObject->getEntityID());
			}
			break;

		default:
			{
				// If this point is reached, no object was found, return an appropriate error
				returnError = ErrorCode::Destroy_obj_not_found;
			}
			break;
	}

	return returnError;
}

void RendererScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	if(CheckBitmask(p_changeType, Systems::Changes::Graphics::ActiveCameraID))
	{
		m_sceneObjects.m_activeCameraID = p_subject->getInt(this, Systems::Changes::Graphics::ActiveCameraID);
	}
}

void RendererScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
		case DataType::DataType_AtmScatteringData:
			{
				auto *atmScatteringData = static_cast<AtmosphericScatteringData *>(p_data);

				// Set the new atmospheric light scattering data
				m_sceneAtmScatteringData = *atmScatteringData;

				// Send the new data to the renderer
				auto rendererSystem = static_cast<RendererSystem *>(m_system);
				rendererSystem->getRenderer().setAtmScatteringData(m_sceneAtmScatteringData);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete atmScatteringData;
			}
			break;

		case DataType::DataType_AmbientOcclusionData:
			{
				auto *aoData = static_cast<AmbientOcclusionData *>(p_data);

				// Set the new AO data
				m_sceneAOData = *aoData;

				// Send the new AO data to the renderer
				auto rendererSystem = static_cast<RendererSystem *>(m_system);
				rendererSystem->getRenderer().setAmbientOcclusionData(m_sceneAOData);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete aoData;
			}
			break;

		case DataType::DataType_CreateComponent:
			{
				auto *componentInfo = static_cast<ComponentsConstructionInfo *>(p_data);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete componentInfo;
			}
			break;

		case DataType::DataType_DeleteComponent:
			{
				// Get the world scene required for getting the entity registry and deleting components
				WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

				// Get the entity registry 
				auto &entityRegistry = worldScene->getEntityRegistry();

				// Get entity and component data
				auto const *componentData = static_cast<EntityAndComponent *>(p_data);

				// Delete the component based on its type
				switch(componentData->m_componentType)
				{
					case ComponentType::ComponentType_CameraComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<CameraComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_RendererScene);
						}
						break;

					case ComponentType::ComponentType_LightComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<LightComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_RendererScene);
						}
						break;

					case ComponentType::ComponentType_ModelComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<ModelComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_RendererScene);
						}
						break;

					case ComponentType::ComponentType_ShaderComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<ShaderComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_RendererScene);
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;

		case DataType::DataType_GUIPassFunctors:
			{
				auto *functors = static_cast<FunctorSequence *>(p_data);
				m_renderTask->m_renderer.setGUIPassFunctorSequence(functors);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete functors;
			}
			break;

		case DataType::DataType_MiscSceneData:
			{
				auto *miscSceneData = static_cast<MiscSceneData *>(p_data);

				// Set the new misc scene data
				m_miscSceneData = *miscSceneData;

				// Send the new misc scene data to the renderer
				auto rendererSystem = static_cast<RendererSystem *>(m_system);
				rendererSystem->getRenderer().setMiscSceneData(m_miscSceneData);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete miscSceneData;
			}
			break;

		case DataType::DataType_RenderToTexture:
			{
				m_renderToTexture = static_cast<bool>(p_data);
				m_renderTask->m_renderer.setRenderFinalToTexture(m_renderToTexture);
			}
			break;

		case DataType::DataType_RenderToTextureResolution:
			{
				auto renderToTextureResolution = static_cast<glm::ivec2 *>(p_data);
				m_renderToTextureResolution = *renderToTextureResolution;
				m_renderTask->m_renderer.setRenderToTextureResolution(m_renderToTextureResolution);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete renderToTextureResolution;
			}
			break;

		case DataType::DataType_ShadowMappingData:
			{
				auto *shadowMappingData = static_cast<ShadowMappingData *>(p_data);

				// Set the new shadow mapping data
				m_shadowMappingData = *shadowMappingData;

				// Send the new shadow mapping data to the renderer
				auto rendererSystem = static_cast<RendererSystem *>(m_system);
				rendererSystem->getRenderer().setShadowMappingData(m_shadowMappingData);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete shadowMappingData;
			}
			break;

		case DataType::DataType_LoadShader:
			{
				ShaderLoader::ShaderProgram *shaderProgram = static_cast<ShaderLoader::ShaderProgram *>(p_data);

				if(shaderProgram != nullptr)
				{
					shaderProgram->setLoadedToVideoMemory(false);
					m_sceneObjects.m_loadToVideoMemory.emplace_back(shaderProgram);
				}
			}
			break;

		case DataType::DataType_LoadTexture2D:
			{
				TextureLoader2D::Texture2DHandle *textureHandle = static_cast<TextureLoader2D::Texture2DHandle *>(p_data);
				if(textureHandle->isLoadedToMemory())
					m_sceneObjects.m_loadToVideoMemory.emplace_back(*textureHandle);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete textureHandle;
			}
			break;

		case DataType::DataType_UnloadTexture2D:
			{
				unsigned int *textureHandle = static_cast<unsigned int *>(p_data);

				m_sceneObjects.m_unloadFromVideoMemory.emplace_back(UnloadObjectType::UnloadObjectType_Texture, *textureHandle);
				
				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete textureHandle;
			}
			break;

		case DataType::DataType_LoadTexture3D:
			{
				TextureLoaderCubemap::TextureCubemapHandle *textureHandle = static_cast<TextureLoaderCubemap::TextureCubemapHandle *>(p_data);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete textureHandle;
			}
			break;

		case DataType::DataType_UnloadModel:
			{
				unsigned int *modelHandle = static_cast<unsigned int *>(p_data);

				m_sceneObjects.m_unloadFromVideoMemory.emplace_back(UnloadObjectType::UnloadObjectType_VAO, *modelHandle);

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete modelHandle;
			}
			break;

		default:
			assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
			break;
	}
}

const unsigned int RendererScene::getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const
{
	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::RenderToTextureBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(static_cast<GBufferTextureType>(Config::rendererVar().render_to_texture_buffer));

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::PositionBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferPosition);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::DiffuseBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferDiffuse);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::NormalBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferNormal);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::EmissiveBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferEmissive);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::MatPropertiesBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferMatProperties);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::IntermediateBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferIntermediate);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::FinalBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferFinal);

	return NullObjects::NullUnsignedInt;
}

const glm::mat4 &RendererScene::getViewMatrix() const
{
	return static_cast<RendererSystem *>(m_system)->getRenderer().getFrameData().m_viewMatrix;
}

const glm::mat4 &RendererScene::getProjectionMatrix() const
{
	return static_cast<RendererSystem *>(m_system)->getRenderer().getFrameData().m_projMatrix;
}

MaterialData RendererScene::loadMaterialData(PropertySet &p_materialProperty, Model::MaterialArrays &p_materialArraysFromModel, MaterialType p_materialType, std::size_t p_meshIndex)
{
	// Declare the material data that is to be returned and a flag showing whether the material data was loaded successfully
	MaterialData newMaterialData;
	bool materialWasLoaded = false;

	/*/ Try to load the material from the filename retrieved from properties
	if (p_materialProperty)
	{
		// Get texture filename property, check if it is valid
		auto filenameProperty = p_materialProperty.getPropertyByID(Properties::Filename);
		if (filenameProperty.isVariableTypeString())
		{
			// Get texture filename string, check if it is valid
			auto filename = filenameProperty.getString();
			if (!filename.empty())
			{
				// Get the texture and load it to memory
				auto materialHandle = Loaders::texture2D().load(filename, p_materialType, false);
				auto materialLoadError = materialHandle.loadToMemory();

				// Check if the texture was loaded successfully
				if (materialLoadError == ErrorCode::Success)
				{
					newMaterialData.m_texture = materialHandle;
					materialWasLoaded = true;
				}
				else
				{
					ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
				}
			}
		}
	}

	// Try to load the material from the filename retrieved from the model
	if (!materialWasLoaded)
	{
		// Check if there are enough materials, and if the material isn't default
		if (p_materialArraysFromModel.m_numMaterials > p_meshIndex
			&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isEmpty()
			&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isDefaultMaterial())
		{
			// Get the texture and load it to memory
			auto materialHandle = Loaders::texture2D().load(p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].m_filename, p_materialType, false);
			auto materialLoadError = materialHandle.loadToMemory();

			// Check if the texture was loaded successfully
			if (materialLoadError == ErrorCode::Success)
			{
				newMaterialData.m_texture = materialHandle;
				materialWasLoaded = true;
			}
			else
			{
				ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
			}
		}
	}

	// All attempts to load the material were unsuccessful, so load a default material
	if (!materialWasLoaded)
	{
		newMaterialData.m_texture = Loaders::texture2D().getDefaultTexture(p_materialType);
	}
	*/
	// Return the newly loaded material data
	return newMaterialData;
}

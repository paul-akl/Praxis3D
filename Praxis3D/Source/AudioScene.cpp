#include <glm/gtc/type_ptr.hpp>

#include "AudioScene.h"
#include "AudioSystem.h"
#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

AudioScene::AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Audio)
{
	m_audioTask = nullptr;
	m_coreSystem = nullptr;
	m_studioSystem = nullptr;

	m_volume[AudioBusType::AudioBusType_Ambient] = Config::audioVar().volume_ambient;
	m_volume[AudioBusType::AudioBusType_Master] = Config::audioVar().volume_master;
	m_volume[AudioBusType::AudioBusType_Music] = Config::audioVar().volume_music;
	m_volume[AudioBusType::AudioBusType_SFX] = Config::audioVar().volume_sfx;

	for(unsigned int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
		m_impactEvents[i] = nullptr;

	// Get audio system
	m_audioSystem = static_cast<AudioSystem *>(p_system);
}

AudioScene::~AudioScene()
{
	deactivate();
}

ErrorCode AudioScene::init()
{
	ErrorCode returnError = ErrorCode::Success;

	// Create audio task, required for task scheduler to call update of this scene
	m_audioTask = new AudioTask(this);

	// Get the handles to FMOD studio and core systems
	m_studioSystem = m_audioSystem->getStudioSystem();
	m_coreSystem = m_audioSystem->getCoreSystem();

	// Assign audio channel groups to sound types
	m_soundTypeChannelGroups[SoundComponent::SoundType::SoundType_Null] = m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Master);
	m_soundTypeChannelGroups[SoundComponent::SoundType::SoundType_Music] = m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Music);
	m_soundTypeChannelGroups[SoundComponent::SoundType::SoundType_Ambient] = m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Ambient);
	m_soundTypeChannelGroups[SoundComponent::SoundType::SoundType_SoundEffect] = m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_SFX);

	return returnError;
}

ErrorCode AudioScene::setup(const PropertySet &p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<SoundComponent>(std::max(Config::objectPoolVar().sound_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::SoundComponent).getInt()));
	worldScene->reserve<SoundListenerComponent>(std::max(Config::objectPoolVar().sound_listener_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::SoundListenerComponent).getInt()));

	// Process banks
	auto const &banksProperty = p_properties.getPropertySetByID(Properties::Banks);
	if(banksProperty)
	{
		// Iterate over all audio banks
		for(decltype(banksProperty.getNumPropertySets()) objIndex = 0, objSize = banksProperty.getNumPropertySets(); objIndex < objSize; objIndex++)
		{
			// Get audio bank filename property
			auto const &filenameProperty = banksProperty.getPropertySetUnsafe(objIndex).getPropertyByID(Properties::Filename);
			if(filenameProperty)
			{
				// Get audio bank filename
				auto filename = filenameProperty.getString();

				// Try to load the audio bank
				FMOD::Studio::Bank *soundBank = m_audioSystem->loadBankFile(Config::filepathVar().sound_path + filename, FMOD_STUDIO_LOAD_BANK_NORMAL);

				// If the audio bank was loaded, add it to the banks-and-filenames array
				if(soundBank != nullptr)
					m_bankFilenames.push_back(std::make_pair(filename, soundBank));
			}
		}
	}

	// Process volume
	auto const &volumeProperty = p_properties.getPropertySetByID(Properties::Volume);
	if(volumeProperty)
	{
		for(decltype(volumeProperty.getNumProperties()) i = 0, size = volumeProperty.getNumProperties(); i < size; i++)
		{
			switch(volumeProperty[i].getPropertyID())
			{
				case Properties::Ambient:
					m_volume[AudioBusType::AudioBusType_Ambient] = volumeProperty[i].getFloat();
					Config::m_audioVar.volume_ambient = m_volume[AudioBusType::AudioBusType_Ambient];
					break;
				case Properties::Master:
					m_volume[AudioBusType::AudioBusType_Master] = volumeProperty[i].getFloat();
					Config::m_audioVar.volume_master = m_volume[AudioBusType::AudioBusType_Master];
					break;
				case Properties::Music:
					m_volume[AudioBusType::AudioBusType_Music] = volumeProperty[i].getFloat();
					Config::m_audioVar.volume_music = m_volume[AudioBusType::AudioBusType_Music];
					break;
				case Properties::SoundEffect:
					m_volume[AudioBusType::AudioBusType_SFX] = volumeProperty[i].getFloat();
					Config::m_audioVar.volume_sfx = m_volume[AudioBusType::AudioBusType_SFX];
					break;
			}
		}
	}

	// Find an impact sound event for each object material type
	for(unsigned int materialTypeIndex = 0; materialTypeIndex < ObjectMaterialType::NumberOfMaterialTypes; materialTypeIndex++)
	{
		// Convert object material type to text string
		const std::string materialTypeString = GetString(static_cast<ObjectMaterialType>(materialTypeIndex));

		// Try to find a matching audio event
		auto soundEvent = m_audioSystem->getEvent(materialTypeString);

		// If the impact event was found, add it to the impact events array entry for this object material type
		if(soundEvent != nullptr)
		{
			ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_AudioScene, "Impact sound found: \"" + materialTypeString + "\"");
			m_impactEvents[materialTypeIndex] = soundEvent;
		}
	}

	return ErrorCode::Success;
}

void AudioScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::SoundComponent, (int)worldScene->getPoolSize<SoundComponent>());
	objectPoolSizePropertySet.addProperty(Properties::SoundListenerComponent, (int)worldScene->getPoolSize<SoundListenerComponent>());

	// Add sound banks
	auto &banksPropertySet = p_propertySet.addPropertySet(Properties::Banks);
	for(const auto &filenameAndBank : m_bankFilenames)
	{
		banksPropertySet.addPropertySet(Properties::ArrayEntry).addProperty(Properties::Filename, filenameAndBank.first);
	}

	// Add volume settings
	auto &volumePropertySet = p_propertySet.addPropertySet(Properties::Volume);
	volumePropertySet.addProperty(Properties::Ambient, m_volume[AudioBusType::AudioBusType_Ambient]);
	volumePropertySet.addProperty(Properties::Master, m_volume[AudioBusType::AudioBusType_Master]);
	volumePropertySet.addProperty(Properties::Music, m_volume[AudioBusType::AudioBusType_Music]);
	volumePropertySet.addProperty(Properties::SoundEffect, m_volume[AudioBusType::AudioBusType_SFX]);

}

void AudioScene::activate()
{
	// Set volume of all buses
	m_audioSystem->getBus(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
	m_audioSystem->getBus(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
	m_audioSystem->getBus(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
	m_audioSystem->getBus(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);

	// Set volume of all channel groups
	m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
	m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
	m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
	m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);
}

void AudioScene::deactivate()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	// Stop all audio from Sound Components (FMOD CORE)
	auto soundComponentView = entityRegistry.view<SoundComponent>();
	for(auto entity : soundComponentView)
	{
		auto &component = soundComponentView.get<SoundComponent>(entity);

		if(component.isObjectActive())
		{
			if(component.m_playing)
			{
				component.m_channel->stop();
				component.m_playing = false;
			}
		}
	}

	// Stop all audio going through master bus (FMOD STUDIO)
	m_audioSystem->getBus(AudioBusType::AudioBusType_Master)->stopAllEvents(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
}

void AudioScene::update(const float p_deltaTime)
{
	//FMOD::Studio::EventDescription *musicDescription;
	//m_studioSystem->getEvent("event:/MainMenuMusic", &musicDescription);

	//int instanceCount = 0;
	//musicDescription->getInstanceCount(&instanceCount);

	//if(instanceCount == 0)
	//{
	//	FMOD::Studio::EventInstance *eventInstance;

	//	musicDescription->createInstance(&eventInstance);

	//	eventInstance->start();
	//	eventInstance->release();
	//}

	// Get double buffering FRONT index
	auto frontIndex = ClockLocator::get().getDoubleBufferingIndexFront();

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	//	 ___________________________
	//	|							|
	//	|	SOUND LISTENER UPDATE	|
	//	|___________________________|
	//
	// Find the first active sound listener and get its spatial component
	auto listenerSpatialComponentView = entityRegistry.view<SoundListenerComponent, SpatialComponent>();
	for(auto entity : listenerSpatialComponentView)
	{
		auto &listenerComponent = listenerSpatialComponentView.get<SoundListenerComponent>(entity);

		// If an active sound listener was found, set the 3D listener attributes
		if(listenerComponent.isObjectActive())
		{
			// Get the spatial component
			auto &listenerSpatialComponent = listenerSpatialComponentView.get<SpatialComponent>(entity);

			// Get the world matrix of the camera
			auto &worldTransform = listenerSpatialComponent.getSpatialDataChangeManager().getWorldTransform();
			auto const worldTranslate = glm::mat3(worldTransform);

			// Calculate all 3D listener attributes
			FMOD_3D_ATTRIBUTES spatialAttributes;
			spatialAttributes.position = Math::toFmodVector(worldTransform[3]);
			spatialAttributes.velocity = Math::toFmodVector(listenerSpatialComponent.getSpatialDataChangeManager().getVelocity());
			spatialAttributes.forward = Math::toFmodVector(glm::vec3(0.0f, 0.0f, -1.0f) * worldTranslate);
			spatialAttributes.up = Math::toFmodVector(glm::vec3(0.0f, 1.0f, 0.0f) * worldTranslate);

			// Update the listener
			m_studioSystem->setListenerAttributes(listenerComponent.m_listenerID, &spatialAttributes);

			break;
		}
	}

	//	 ___________________________
	//	|							|
	//	|  SOUND COMPONENTS UPDATE	|
	//	|___________________________|
	//
	auto soundComponentView = entityRegistry.view<SoundComponent>();
	for(auto entity : soundComponentView)
	{
		auto &component = soundComponentView.get<SoundComponent>(entity);

		if(component.isObjectActive())
		{
			if(component.m_changePending)
			{
				if(component.m_reloadSound)
				{
					createSound(component);
				}
				else
				{
					if(component.m_volumeChanged)
						component.m_channel->setVolume(component.m_volume);

					if(component.m_loopChanged)
						if(component.m_loop)
							component.m_channel->setMode(FMOD_LOOP_NORMAL);
						else
							component.m_channel->setMode(FMOD_LOOP_OFF);

					if(component.m_spatializedChanged)
						if(component.m_spatialized)
							component.m_channel->setMode(FMOD_3D);
						else
							component.m_channel->setMode(FMOD_2D);
				}

				component.resetChanges();
			}

			if(!component.m_playing)
			{
				if(component.m_startPlaying && component.m_sound != nullptr)
				{
					component.m_playing = true;

					AudioSystem::fmodErrorLog(m_coreSystem->playSound(component.m_sound, m_soundTypeChannelGroups[component.m_soundType], true, &component.m_channel), component.m_soundFilename);
					component.m_channel->setVolume(component.m_volume);

					if(component.m_spatialized)
					{
						auto spatialComponent = entityRegistry.try_get<SpatialComponent>(entity);
						if(spatialComponent != nullptr)
						{
							FMOD_VECTOR velocity = Math::toFmodVector(spatialComponent->getSpatialDataChangeManager().getVelocity());
							FMOD_VECTOR position = Math::toFmodVector(spatialComponent->getSpatialDataChangeManager().getWorldTransform()[3]);

							component.m_channel->set3DAttributes(&position, &velocity);
						}
					}

					component.m_channel->setPaused(false);
				}
			}

			if(component.m_spatialized)
			{
				auto spatialComponent = entityRegistry.try_get<SpatialComponent>(entity);
				if(spatialComponent != nullptr)
				{
					FMOD_VECTOR velocity = Math::toFmodVector(spatialComponent->getSpatialDataChangeManager().getVelocity());
					FMOD_VECTOR position = Math::toFmodVector(spatialComponent->getSpatialDataChangeManager().getWorldTransform()[3]);

					component.m_channel->set3DAttributes(&position, &velocity);
				}
			}
		}
		else
		{
			if(component.m_playing)
			{
				component.m_channel->stop();
				component.m_playing = false;
			}
		}
	}	

	//	 ___________________________
	//	|							|
	//	|  COLLISION EVENTS UPDATE	|
	//	|___________________________|
	//
	auto collisionEventMaterialView = worldScene->getEntityRegistry().view<CollisionEventComponent, ObjectMaterialComponent>();
	for(auto entity : collisionEventMaterialView)
	{
		auto &collisionComponent = collisionEventMaterialView.get<CollisionEventComponent>(entity);

		if(collisionComponent.m_numOfDynamicCollisions[frontIndex] > 0)
		{
			auto &materialComponent = collisionEventMaterialView.get<ObjectMaterialComponent>(entity);

			for(size_t i = 0, size = collisionComponent.m_numOfDynamicCollisions[frontIndex]; i < size; i++)
			{
				//if(collisionComponent.m_dynamicCollisions[frontIndex][i].m_firstObjInCollisionPair)
				{
					// Get the transform matrix
					glm::mat4 transformMatrix;
					collisionComponent.m_dynamicCollisions[frontIndex][i].m_worldTransform.getOpenGLMatrix(glm::value_ptr(transformMatrix));
					const glm::mat3 translateMatrix = glm::mat3(transformMatrix);

					// Get 3D attributes
					FMOD_3D_ATTRIBUTES spatialAttributes;
					spatialAttributes.position = Math::toFmodVector(transformMatrix[3]);
					spatialAttributes.velocity = Math::toFmodVector(collisionComponent.m_dynamicCollisions[frontIndex][i].m_velocity);
					spatialAttributes.forward = Math::toFmodVector(glm::vec3(0.0f, 0.0f, -1.0f) * translateMatrix);
					spatialAttributes.up = Math::toFmodVector(glm::vec3(0.0f, 1.0f, 0.0f) * translateMatrix);
					const float volume = glm::min(collisionComponent.m_dynamicCollisions[frontIndex][i].m_appliedImpulse / Config::audioVar().impact_impulse_volume_divider, Config::audioVar().impact_max_volume_threshold);

					// Create an event (sound) instance
					FMOD::Studio::EventInstance *eventInstance;
					m_impactEvents[materialComponent.getObjectMaterialType()]->createInstance(&eventInstance);

					// Set sound parameters and play the sound
					eventInstance->setParameterByName("Impulse", collisionComponent.m_dynamicCollisions[frontIndex][i].m_appliedImpulse / Config::audioVar().impact_impulse_param_divider);
					eventInstance->setVolume(volume);
					eventInstance->set3DAttributes(&spatialAttributes);
					eventInstance->start();
					eventInstance->setPaused(false);
					eventInstance->release();
				}
			}
		}
	}

	//	 ___________________________
	//	|							|
	//	|	   VOLUME CHANGES		|
	//	|___________________________|
	//
	// Ambient volume
	if(m_volume[AudioBusType::AudioBusType_Ambient] != Config::audioVar().volume_ambient)
	{
		m_volume[AudioBusType::AudioBusType_Ambient] = Config::audioVar().volume_ambient;
		m_audioSystem->getBus(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
	}
	// Master volume
	if(m_volume[AudioBusType::AudioBusType_Master] != Config::audioVar().volume_master)
	{
		m_volume[AudioBusType::AudioBusType_Master] = Config::audioVar().volume_master;
		m_audioSystem->getBus(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
	}
	// Music volume
	if(m_volume[AudioBusType::AudioBusType_Music] != Config::audioVar().volume_music)
	{
		m_volume[AudioBusType::AudioBusType_Music] = Config::audioVar().volume_music;
		m_audioSystem->getBus(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
	}
	// SFX volume
	if(m_volume[AudioBusType::AudioBusType_SFX] != Config::audioVar().volume_sfx)
	{
		m_volume[AudioBusType::AudioBusType_SFX] = Config::audioVar().volume_sfx;
		m_audioSystem->getBus(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);
	}

	// Update the sound system
	m_studioSystem->update();
}

ErrorCode AudioScene::preload()
{
	return ErrorCode::Success;
}

void AudioScene::loadInBackground()
{
}

std::vector<SystemObject *> AudioScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *soundComponent = entityRegistry.try_get<SoundComponent>(p_entityID);
	if(soundComponent != nullptr)
		returnVector.push_back(soundComponent);

	auto *soundListenerComponent = entityRegistry.try_get<SoundListenerComponent>(p_entityID);
	if(soundListenerComponent != nullptr)
		returnVector.push_back(soundListenerComponent);

	return returnVector;
}

std::vector<SystemObject*> AudioScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_audioComponents, p_startLoading);
}

void AudioScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_audioComponents);
}

void AudioScene::exportComponents(const EntityID p_entityID, AudioComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export SoundComponent
	auto *soundComponent = entityRegistry.try_get<SoundComponent>(p_entityID);
	if(soundComponent != nullptr)
	{
		if(p_constructionInfo.m_soundConstructionInfo == nullptr)
			p_constructionInfo.m_soundConstructionInfo = new SoundComponent::SoundComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_soundConstructionInfo, *soundComponent);
	}

	// Export SoundListenerComponent
	auto *soundListenerComponent = entityRegistry.try_get<SoundListenerComponent>(p_entityID);
	if(soundListenerComponent != nullptr)
	{
		if(p_constructionInfo.m_soundListenerConstructionInfo == nullptr)
			p_constructionInfo.m_soundListenerConstructionInfo = new SoundListenerComponent::SoundListenerComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_soundListenerConstructionInfo, *soundListenerComponent);
	}
}

SystemObject *AudioScene::createComponent(const EntityID &p_entityID, const SoundComponent::SoundComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<SoundComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the camera component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.m_soundType = p_constructionInfo.m_soundType;
		component.m_soundFilename = p_constructionInfo.m_soundFilename;
		component.m_loop = p_constructionInfo.m_loop;
		component.m_spatialized = p_constructionInfo.m_spatialized;
		component.m_startPlaying = p_constructionInfo.m_startPlaying;
		component.m_volume = p_constructionInfo.m_volume;
		component.m_objectType = Properties::PropertyID::SoundComponent;
		component.setActive(p_constructionInfo.m_active);

		createSound(component);

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<SoundComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_SoundComponent, component.getName());
	}

	return returnObject;
}

SystemObject *AudioScene::createComponent(const EntityID &p_entityID, const SoundListenerComponent::SoundListenerComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<SoundListenerComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the camera component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.setActive(p_constructionInfo.m_active);
		component.m_listenerID = p_constructionInfo.m_listenerID;

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<SoundListenerComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_SoundListenerComponent, component.getName());
	}

	return returnObject;
}

ErrorCode AudioScene::destroyObject(SystemObject *p_systemObject)
{
	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

void AudioScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	if(CheckBitmask(p_changeType, Systems::Changes::Audio::VolumeAmbient))
	{
		m_volume[AudioBusType::AudioBusType_Ambient] = p_subject->getFloat(this, Systems::Changes::Audio::VolumeAmbient);

		Config::m_audioVar.volume_ambient = m_volume[AudioBusType::AudioBusType_Ambient];
		m_audioSystem->getBus(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Ambient)->setVolume(m_volume[AudioBusType::AudioBusType_Ambient]);
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Audio::VolumeMaster))
	{
		m_volume[AudioBusType::AudioBusType_Master] = p_subject->getFloat(this, Systems::Changes::Audio::VolumeMaster);

		Config::m_audioVar.volume_master = m_volume[AudioBusType::AudioBusType_Master];
		m_audioSystem->getBus(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Master)->setVolume(m_volume[AudioBusType::AudioBusType_Master]);
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Audio::VolumeMusic))
	{
		m_volume[AudioBusType::AudioBusType_Music] = p_subject->getFloat(this, Systems::Changes::Audio::VolumeMusic);

		Config::m_audioVar.volume_music = m_volume[AudioBusType::AudioBusType_Music];
		m_audioSystem->getBus(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_Music)->setVolume(m_volume[AudioBusType::AudioBusType_Music]);
	}

	if(CheckBitmask(p_changeType, Systems::Changes::Audio::VolumeSFX))
	{
		m_volume[AudioBusType::AudioBusType_SFX] = p_subject->getFloat(this, Systems::Changes::Audio::VolumeSFX);

		Config::m_audioVar.volume_sfx = m_volume[AudioBusType::AudioBusType_SFX];
		m_audioSystem->getBus(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);
		m_audioSystem->getChannelGroup(AudioBusType::AudioBusType_SFX)->setVolume(m_volume[AudioBusType::AudioBusType_SFX]);
	}
}

void AudioScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
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
					case ComponentType::ComponentType_SoundComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<SoundComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// If component is active and sound is playing, stop the sound
								if(component->isObjectActive())
								{
									if(component->m_playing)
									{
										component->m_channel->stop();
										component->m_playing = false;
									}
								}

								// If sound exists, release its memory
								if(component->m_sound != nullptr)
									component->m_sound->release();

								// Delete component
								worldScene->removeComponent<SoundComponent>(componentData->m_entityID);
							}
						}
						break;

					case ComponentType::ComponentType_SoundListenerComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<SoundListenerComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								worldScene->removeComponent<SoundListenerComponent>(componentData->m_entityID);
							}
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;
	}
}

void AudioScene::createSound(SoundComponent &p_soundComponent)
{
	if(p_soundComponent.m_sound != nullptr)
	{
		if(p_soundComponent.m_playing)
		{
			p_soundComponent.m_channel->stop();
			p_soundComponent.m_playing = false;
		}

		p_soundComponent.m_sound->release(); 
		p_soundComponent.m_sound = nullptr;
	}

	FMOD_MODE mode = 0;

	if(p_soundComponent.m_loop)
		mode |= FMOD_LOOP_NORMAL;
	else
		mode |= FMOD_LOOP_OFF;

	if(p_soundComponent.m_spatialized)
		mode |= FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF;
	else
		mode |= FMOD_2D;


	switch(p_soundComponent.m_soundType)
	{
		case SoundComponent::SoundType::SoundType_Music:
			{
				m_coreSystem->createStream((Config::filepathVar().sound_path + p_soundComponent.m_soundFilename).c_str(), mode, p_soundComponent.m_soundExInfo, &p_soundComponent.m_sound);
			}
			break;

		case SoundComponent::SoundType::SoundType_Ambient:
			{
				m_coreSystem->createStream((Config::filepathVar().sound_path + p_soundComponent.m_soundFilename).c_str(), mode, p_soundComponent.m_soundExInfo, &p_soundComponent.m_sound);
			}
			break;

		case SoundComponent::SoundType::SoundType_SoundEffect:
			{
				m_coreSystem->createSound((Config::filepathVar().sound_path + p_soundComponent.m_soundFilename).c_str(), mode, p_soundComponent.m_soundExInfo, &p_soundComponent.m_sound);
			}
			break;

		case SoundComponent::SoundType::SoundType_Null:
		default:
			break;
	}
}

void AudioScene::playSound(SoundComponent &p_soundComponent)
{
}

void AudioScene::loadParameterGUIDs()
{
	int numOfBanks = 0;
	m_studioSystem->getBankCount(&numOfBanks);

	if(numOfBanks > 0)
	{
		FMOD::Studio::Bank **banks = new FMOD::Studio::Bank * [numOfBanks];

		m_studioSystem->getBankList(banks, numOfBanks, &numOfBanks);

		for(int bankIndex = 0; bankIndex < numOfBanks; bankIndex++)
		{
			int numOfEvents = 0;

			banks[bankIndex]->getEventCount(&numOfEvents);

			if(numOfEvents > 0)
			{
				FMOD::Studio::EventDescription **events = new FMOD::Studio::EventDescription * [numOfEvents];

				banks[bankIndex]->getEventList(events, numOfEvents, &numOfEvents);

				for(int eventIndex = 0; eventIndex < numOfEvents; eventIndex++)
				{
					int numOfParameters = 0;

					events[eventIndex]->getParameterDescriptionCount(&numOfParameters);

					char path[512];

					events[eventIndex]->getPath(path, 512, nullptr);

					//std::cout << path << std::endl;

					//std::cout << Utilities::splitStringAfterDelimiter(Config::audioVar().pathDelimiter, std::string(path)) << std::endl;

					if(numOfParameters > 0)
					{
						for(int parameterIndex = 0; parameterIndex < numOfParameters; parameterIndex++)
						{
							FMOD_STUDIO_PARAMETER_DESCRIPTION parameter;
							events[eventIndex]->getParameterDescriptionByIndex(parameterIndex, &parameter);

							//std::cout << parameter.name << std::endl;
						}
					}
				}

				delete events;
			}
		}

		delete banks;
	}
}


#include "AudioScene.h"
#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

AudioScene::AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_audioTask = nullptr;
	m_fmodSystem = nullptr;
	m_masterChannelGroup = nullptr;
	m_musicChannelGroup = nullptr;
	m_ambientChannelGroup = nullptr;
	m_numSoundDrivers = 0;
}

AudioScene::~AudioScene()
{
	if(m_fmodSystem != nullptr)
	{
		m_fmodSystem->close();
		m_fmodSystem->release();
	}
}

ErrorCode AudioScene::init()
{
	ErrorCode returnError = ErrorCode::Success;

	m_audioTask = new AudioTask(this);

	// Create the fmod sound system
	auto fmodError = FMOD::System_Create(&m_fmodSystem);

	// Check if the sound system was created successfully
	if(fmodError != FMOD_OK)
	{
		ErrHandlerLoc::get().log(ErrorCode::Audio_system_init_failed, ErrorSource::Source_AudioScene);
		returnError = ErrorCode::Audio_system_init_failed;
	}
	else
	{
		// Get the number of sound drivers
		m_fmodSystem->getNumDrivers(&m_numSoundDrivers);

		// Do not continue if there are no sound drivers
		if(m_numSoundDrivers == 0)
		{
			ErrHandlerLoc::get().log(ErrorCode::Audio_no_drivers, ErrorSource::Source_AudioScene);
			returnError = ErrorCode::Audio_no_drivers;
		}
		else
		{
			// Initialize our Instance with 36 Channels
			m_fmodSystem->init(Config::audioVar().num_audio_channels, FMOD_INIT_NORMAL, NULL);

			m_fmodSystem->createChannelGroup("music", &m_musicChannelGroup);
		}
	}

	return returnError;
}

ErrorCode AudioScene::setup(const PropertySet &p_properties)
{
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:

			break;
		}
	}

	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Reserve every component type that belongs to this scene
	worldScene->reserve<SoundComponent>(Config::objectPoolVar().sound_component_default_pool_size);

	return ErrorCode::Success;
}

void AudioScene::activate()
{
}

void AudioScene::deactivate()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

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
}

void AudioScene::update(const float p_deltaTime)
{
	m_fmodSystem->update();

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	auto soundComponentView = entityRegistry.view<SoundComponent>();
	for(auto entity : soundComponentView)
	{
		auto &component = soundComponentView.get<SoundComponent>(entity);

		if(component.isObjectActive())
		{
			if(!component.m_playing)
			{
				if(component.m_startPlaying)
				{
					component.m_playing = true;
					m_fmodSystem->playSound(component.m_sound, 0, true, &component.m_channel);
					component.m_channel->setVolume(component.m_volume);
					component.m_channel->setPaused(false);
				}
			}
		}
	}
}

ErrorCode AudioScene::preload()
{
	return ErrorCode::Success;
}

void AudioScene::loadInBackground()
{
}

std::vector<SystemObject*> AudioScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_audioComponents, p_startLoading);
}

SystemObject *AudioScene::createComponent(const EntityID &p_entityID, const SoundComponent::SoundComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

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

		FMOD_MODE mode = 0;

		if(component.m_loop)
			mode |= FMOD_LOOP_NORMAL;
		else
			mode |= FMOD_LOOP_OFF;

		if(component.m_spatialized)
			mode |= FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF;
		else
			mode |= FMOD_2D;


		switch(component.m_soundType)
		{
			case SoundComponent::SoundType::SoundType_Music:
			{
				m_fmodSystem->createStream((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
			}
			break;

			case SoundComponent::SoundType::SoundType_Ambient:
			{
				m_fmodSystem->createStream((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
			}
			break;

			case SoundComponent::SoundType::SoundType_SoundEffect:
			{
				m_fmodSystem->createSound((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
			}
			break;
		}

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<SoundComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_SoundComponent, component.getName());
	}

	return returnObject;
}

ErrorCode AudioScene::destroyObject(SystemObject *p_systemObject)
{
	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

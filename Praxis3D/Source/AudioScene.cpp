
#include "AudioScene.h"
#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

AudioScene::AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Audio)
{
	m_audioTask = nullptr;
	m_coreSystem = nullptr;
	m_studioSystem = nullptr;
	m_masterChannelGroup = nullptr;
	m_ambientChannelGroup = nullptr;
	m_sfxChannelGroup = nullptr;
	m_musicChannelGroup = nullptr;
	m_impactBank = nullptr;
	m_defaultImpactSoundsLoaded = false;
	m_numSoundDrivers = 0;

	m_lastPlayedImpactSound = 0;
	m_distribution = std::uniform_real_distribution<float>(0.5f, 1.5f);

	m_dTime = 0.0f;
}

AudioScene::~AudioScene()
{
	if(m_studioSystem != nullptr)
	{
		if(m_coreSystem != nullptr)
			m_coreSystem->release();

		m_studioSystem->release();
	}
}

ErrorCode AudioScene::init()
{
	ErrorCode returnError = ErrorCode::Success;

	m_audioTask = new AudioTask(this);

	// Create the FMOD studio system
	auto fmodError = FMOD::Studio::System::create(&m_studioSystem);

	// Check if the sound system was created successfully
	if(fmodError != FMOD_OK)
	{
		ErrHandlerLoc::get().log(ErrorCode::Audio_system_init_failed, ErrorSource::Source_AudioScene);
		returnError = ErrorCode::Audio_system_init_failed;
	}
	else
	{
		// Get the FMOD core system
		m_studioSystem->getCoreSystem(&m_coreSystem);

		// Get the number of sound drivers
		m_coreSystem->getNumDrivers(&m_numSoundDrivers);

		// Do not continue if there are no sound drivers
		if(m_numSoundDrivers == 0)
		{
			ErrHandlerLoc::get().log(ErrorCode::Audio_no_drivers, ErrorSource::Source_AudioScene);
			returnError = ErrorCode::Audio_no_drivers;
		}
		else
		{
			// Initialize our Instance with 36 Channels
			m_studioSystem->initialize(Config::audioVar().num_audio_channels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL | FMOD_INIT_3D_RIGHTHANDED, NULL);

			m_coreSystem->createChannelGroup("music", &m_musicChannelGroup);

			//fmodErrorLog(m_coreSystem->loadPlugin((Config::filepathVar().sound_path + "fmod_distance_filter.dll").c_str(), 0, 0));

			//fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + "Master.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_testBank1));
			//fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + "bmw_1m_s3.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_testBank1));
			//fmodErrorLog(m_testBank1->loadSampleData());
			//fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + "Master.strings.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_testBank1));
			//fmodErrorLog(m_testBank1->loadSampleData());
			//fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + "SFX.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_testBank1));

			auto loadError = m_testBank1->loadSampleData();

			if(loadError != FMOD_OK)
			{
				std::cout << "LOAD FAILED" << std::endl;
			}

			//m_testBank1->

			int eventCount;

			m_testBank1->getEventCount(&eventCount);

			std::cout << "Event count: " << eventCount << std::endl;

			FMOD::Studio::EventDescription *eventDescription[50];
			//FMOD::Studio::EventInstance *eventInstance;

			m_testBank1->getEventList(eventDescription, 50, &eventCount);

			FMOD::Studio::EventDescription *description1;
			FMOD::Studio::EventInstance *instance1;

			auto error1 = m_studioSystem->getEvent("event:/Interactables/Wooden Collision", &description1);

			if(error1 == FMOD_OK)
			{
				std::cout << "EVENT FOUND" << std::endl;
				//description1->
				description1->loadSampleData();
				description1->createInstance(&m_eventInstance1);
				m_eventInstance1->start();
			}
			else
			{
				std::cout << "EVENT NOT FOUND" << std::endl;
			}

			for(int i = 0; i < eventCount; i++)
			{
				char path[100];
				int retrieved;

				eventDescription[i]->getPath(path, 100, &retrieved);

				//eventDescription[i]->

				std::cout << path << std::endl;

				//eventDescription[i]->loadSampleData();

				int descrCount;
				eventDescription[i]->getParameterDescriptionCount(&descrCount);

				FMOD_STUDIO_USER_PROPERTY paramDescr;
				auto userPropError = eventDescription[i]->getUserProperty("Music_prop", &paramDescr);

				if(userPropError == FMOD_OK)
				{
					std::cout << paramDescr.name << std::endl;

					switch(paramDescr.type)
					{
					case FMOD_STUDIO_USER_PROPERTY_TYPE_INTEGER:
						std::cout << paramDescr.intvalue << std::endl;
						break;
					case FMOD_STUDIO_USER_PROPERTY_TYPE_BOOLEAN:
						std::cout << paramDescr.boolvalue << std::endl;
						break;
					case FMOD_STUDIO_USER_PROPERTY_TYPE_FLOAT:
						std::cout << paramDescr.floatvalue << std::endl;
						break;
					case FMOD_STUDIO_USER_PROPERTY_TYPE_STRING:
						std::cout << paramDescr.stringvalue << std::endl;
						break;
					}
				}

				//for(int i = 0; i < descrCount; i++)
				//{
				//	FMOD_STUDIO_PARAMETER_DESCRIPTION *paramDescr = nullptr;
				//	eventDescription[i]->getParameterDescriptionByIndex(i, paramDescr);

				//	std::cout << paramDescr->name << std::endl;
				//}

				//auto instanceError = eventDescription[i]->createInstance(&m_eventInstance1);
				//if(instanceError != FMOD_OK)
				//{
				//	std::cout << "CREATE INSTANCE FAILED" << std::endl;
				//}
				//else
				{
					//auto startError = m_eventInstance1->start();
					//if(startError != FMOD_OK)
					//{
					//	std::cout << "START FAILED" << std::endl;
					//}
				}

				//std::cout << i << ": Path: " << path << std::endl;
			}
		}
	}

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

	FMOD::Studio::Bank *defaultSoundBank = nullptr;

	// Load default sound bank
	if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + Config::audioVar().default_sound_bank).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &defaultSoundBank), Config::audioVar().default_sound_bank))
		if(fmodErrorLog(defaultSoundBank->loadSampleData(), Config::audioVar().default_sound_bank))
			addImpactSoundBank(defaultSoundBank);

	// Load default string bank
	if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + Config::audioVar().default_sound_bank_string).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &defaultSoundBank), Config::audioVar().default_sound_bank_string))
		fmodErrorLog(defaultSoundBank->loadSampleData(), Config::audioVar().default_sound_bank_string);

	auto const &banksProperty = p_properties.getPropertySetByID(Properties::Banks);
	if(banksProperty)
	{
		// Iterate over all game objects
		for(decltype(banksProperty.getNumPropertySets()) objIndex = 0, objSize = banksProperty.getNumPropertySets(); objIndex < objSize; objIndex++)
		{
			auto const &filenameProperty = banksProperty.getPropertySetUnsafe(objIndex).getPropertyByID(Properties::Filename);
			if(filenameProperty)
			{
				auto filename = filenameProperty.getString();

				FMOD::Studio::Bank *soundBank = nullptr;

				if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + filename).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &soundBank), filename))
				{
					if(fmodErrorLog(soundBank->loadSampleData(), filename))
						addImpactSoundBank(soundBank);

					m_bankFilenames.push_back(std::make_pair(filename, soundBank));
				}
			}
		}
	}

	// Load default impact sound bank for basic object impact sounds
	//if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + Config::audioVar().default_impact_sound_bank).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_impactBank), Config::audioVar().default_impact_sound_bank))
	//{
	//	m_defaultImpactSoundsLoaded = true;

	//	// Load default impact sound bank to memory
	//	if(fmodErrorLog(m_impactBank->loadSampleData(), Config::audioVar().default_impact_sound_bank))
	//	{
	//		addImpactSoundBank(m_impactBank);
	//		for(int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
	//		{
	//			std::string eventName = GetString(static_cast<ObjectMaterialType>(i));
	//			std::string fullEventName = "event:/" + eventName;
	//			fmodErrorLog(m_studioSystem->getEvent(fullEventName.c_str(), &m_impactEvents[i]), eventName);
	//		}
	//	}

	//	m_studioSystem->getEvent("event:/Metal", &m_eventDescr1);
	//	m_eventDescr1->createInstance(&m_eventInstance1);
	//}

	FMOD_MODE mode = FMOD_LOOP_OFF | FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF;

	for(unsigned int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
	{
		m_collisionSounds[i].m_filename = "Footsteps_MetalV1_Jump_Land_02.wav";

		m_coreSystem->createSound((Config::filepathVar().sound_path + m_collisionSounds[i].m_filename).c_str(), mode, nullptr, &m_collisionSounds[i].m_sound);
	}

	m_coreSystem->createChannelGroup("sfx", &m_sfxChannelGroup);

	m_coreSystem->createDSPByType(FMOD_DSP_TYPE::FMOD_DSP_TYPE_PITCHSHIFT, &m_pitch);
	m_sfxChannelGroup->addDSP(0, m_pitch);

	m_coreSystem->createSound((Config::filepathVar().sound_path + "metal_solid_impact_soft1.wav").c_str(), FMOD_LOOP_OFF | FMOD_2D, nullptr, &m_metalImpactSound[0]);
	m_coreSystem->createSound((Config::filepathVar().sound_path + "metal_solid_impact_soft2.wav").c_str(), FMOD_LOOP_OFF | FMOD_2D, nullptr, &m_metalImpactSound[1]);
	m_coreSystem->createSound((Config::filepathVar().sound_path + "metal_solid_impact_soft3.wav").c_str(), FMOD_LOOP_OFF | FMOD_2D, nullptr, &m_metalImpactSound[2]);

	m_coreSystem->playSound(m_metalImpactSound[0], m_sfxChannelGroup, true, &m_metalImpactChannel[0]);
	//m_coreSystem->playSound(m_metalImpactSound[1], m_sfxChannelGroup, false, &m_metalImpactChannel[1]);
	//m_coreSystem->playSound(m_metalImpactSound[2], m_sfxChannelGroup, false, &m_metalImpactChannel[2]);

	std::cout << "EVENT NAMES:" << std::endl;
	for(auto &it : m_impactSounds)
	{
		// Do stuff
		std::cout << it.first << std::endl;
	}

	loadParameterGUIDs();

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
}

void AudioScene::activate()
{
}

void AudioScene::deactivate()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

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
	m_dTime += p_deltaTime;

	FMOD_STUDIO_PLAYBACK_STATE playbackState;
	m_eventInstance1->getPlaybackState(&playbackState);
	if(playbackState != FMOD_STUDIO_PLAYBACK_STATE::FMOD_STUDIO_PLAYBACK_PLAYING)
	{
		//m_eventInstance1->setParameterByName("Speed", 4.0f);
		//m_eventInstance1->setParameterByName("Box", 1.0f);
		//m_eventInstance1->start();
	}
	
	if(m_dTime > 1.0f)
	{
		m_dTime -= 1.0f;

		FMOD::Studio::EventInstance *eventInstance;

		m_eventDescr1->createInstance(&eventInstance);

		eventInstance->setParameterByName("Speed", 4.0f);
		eventInstance->setParameterByName("Barrel", 1.0f);
		//eventInstance->start();
		//eventInstance->release();

	}

	bool metalImpactPlaying = false;
	m_metalImpactChannel[0]->isPlaying(&metalImpactPlaying);
	if(!metalImpactPlaying)
	{
		std::uniform_real_distribution<float> dis(0.95f, 1.05f);
		std::uniform_int_distribution<int> dis2(0, 2);

		float randomPitch = dis(m_randomEngine);
		int randomSound = dis2(m_randomEngine);

		while(randomSound == m_lastPlayedImpactSound)
		{
			randomSound = dis2(m_randomEngine);
		}

		//std::cout << "random sound: " << randomSound << std::endl;

		m_lastPlayedImpactSound = randomSound;

		m_pitch->setParameterFloat(0, randomPitch);
		//m_coreSystem->playSound(m_metalImpactSound[randomSound], m_sfxChannelGroup, false, &m_metalImpactChannel[0]);
	}

	// Get double buffering FRONT index
	auto frontIndex = ClockLocator::get().getDoubleBufferingIndexFront();

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

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

			// Calculate all 3D listener attributes
			FMOD_VECTOR position = Math::toFmodVector(worldTransform[3]);
			FMOD_VECTOR velocity = Math::toFmodVector(listenerSpatialComponent.getSpatialDataChangeManager().getVelocity());
			FMOD_VECTOR forwardDirection = Math::toFmodVector(glm::vec3(0.0f, 0.0f, -1.0f) * glm::mat3(worldTransform));
			FMOD_VECTOR upDirection = Math::toFmodVector(glm::vec3(0.0f, 1.0f, 0.0f) * glm::mat3(worldTransform));

			// Update the listener
			m_coreSystem->set3DListenerAttributes(listenerComponent.m_listenerID, &position, &velocity, &forwardDirection, &upDirection);

			break;
		}
	}

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
					m_coreSystem->playSound(component.m_sound, 0, true, &component.m_channel);
					component.m_channel->setVolume(component.m_volume);
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
	
	auto collisionEventMaterialView = worldScene->getEntityRegistry().view<CollisionEventComponent, ObjectMaterialComponent>();
	for(auto entity : collisionEventMaterialView)
	{
		auto &collisionComponent = collisionEventMaterialView.get<CollisionEventComponent>(entity);

		if(collisionComponent.m_numOfDynamicCollisions[frontIndex] > 0)
		{
			auto &materialComponent = collisionEventMaterialView.get<ObjectMaterialComponent>(entity);

			for(size_t i = 0, size = collisionComponent.m_numOfDynamicCollisions[frontIndex]; i < size; i++)
			{
				if(collisionComponent.m_dynamicCollisions[frontIndex][i].m_firstObjInCollisionPair)
				{
					FMOD_VECTOR position = Math::toFmodVector(collisionComponent.m_dynamicCollisions[frontIndex][i].m_position);
					FMOD_VECTOR velocity = Math::toFmodVector(collisionComponent.m_dynamicCollisions[frontIndex][i].m_velocity);

					float volume = glm::min(collisionComponent.m_dynamicCollisions[frontIndex][i].m_appliedImpulse / 200.0f, 1.0f);

					FMOD::Channel *channel;
					m_coreSystem->playSound(m_collisionSounds[materialComponent.getObjectMaterialType()].m_sound, 0, true, &channel);
					channel->setVolume(volume);
					channel->set3DAttributes(&position, &velocity);
					channel->setPaused(false);

					//m_eventInstance1->set3DAttributes()
					//m_eventInstance1->start();
				}
			}
		}
	}

	// Update the sound system
	m_studioSystem->update();
	//m_coreSystem->update();
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
				m_coreSystem->createStream((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
			}
			break;

			case SoundComponent::SoundType::SoundType_Ambient:
			{
				m_coreSystem->createStream((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
			}
			break;

			case SoundComponent::SoundType::SoundType_SoundEffect:
			{
				m_coreSystem->createSound((Config::filepathVar().sound_path + component.m_soundFilename).c_str(), mode, component.m_soundExInfo, &component.m_sound);
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

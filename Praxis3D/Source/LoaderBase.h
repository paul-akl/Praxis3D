#pragma once

#include <atomic>
#include <list>
#include <string>
#include <tbb\atomic.h>
#include <queue>

#include "ErrorCodes.h"
#include "TaskManagerLocator.h"

template <class TDerived, class TObject>
class LoaderBase
{
	friend class UniqueObject;
public:
	class UniqueObject
	{
		friend class LoaderBase;
	public:
		UniqueObject(LoaderBase *p_loaderBase, size_t p_uniqueIDs, std::string p_filename)
			: m_loaderBase(p_loaderBase), m_uniqueID(p_uniqueIDs), m_filename(p_filename)
		{
			m_loadingToMemoryError = ErrorCode::Failure;
			m_queuedLoadToVideoMemory = false;
			m_loadedToMemory = false;
			m_loadedToVideoMemory = false;
			m_beingLoaded = false;
			m_refCounter = 0;
		}
		virtual ~UniqueObject()
		{
			//unload(*this);
		}

		// Increments reference counter
		inline void incRefCounter() noexcept { m_refCounter++; }

		// Decrements reference counter, if it goes to 0, object is put into unload queue
		inline void decRefCounter() noexcept
		{
			assert(m_refCounter > 0 && "m_refCounter of UniqueObject of LoaderBase was set to below zero (constructor-destructor were not paired)");

			m_refCounter--;
			if(m_refCounter == 0)
			{
				m_loaderBase->queueUnload(*this);
			}
		}

		// Setters
		inline void setQueuedLoadToVideoMemory(const bool p_queued) { m_queuedLoadToVideoMemory = p_queued; }
		inline void setLoadedToMemory(const bool p_loaded)			{ m_loadedToMemory = p_loaded;			}
		inline void setLoadedToVideoMemory(const bool p_loaded)		{ m_loadedToVideoMemory = p_loaded;		}
		inline void setUniqueID(const unsigned int p_uniqueID)		{ m_uniqueID = p_uniqueID;				}

		// Getters
		inline const bool isQueuedLoadToVideoMemory() const { return m_queuedLoadToVideoMemory;	}
		inline const bool isLoadedToMemory() const			{ return m_loadedToMemory;			}
		inline const bool isLoadedToVideoMemory() const		{ return m_loadedToVideoMemory;		}
		inline const unsigned int getUniqueID()	const		{ return (unsigned int)m_uniqueID;	}
		inline const std::string &getFilename() const		{ return m_filename;				}
		inline size_t getReferenceCounter() const			{ return m_refCounter;				}

		// Equality operator; compares filenames
		inline bool operator==(std::string p_string) { return m_filename == p_string; }

		//inline ErrorCode unload() { return static_cast<TObject *>(this)->unloadMemory(); }
	protected:
		inline const bool isBeingLoaded() { return m_beingLoaded; }

		std::atomic_bool	m_beingLoaded,
							m_queuedLoadToVideoMemory,
							m_loadedToMemory,
							m_loadedToVideoMemory;

		ErrorCode m_loadingToMemoryError;
		std::string m_filename;
		SpinWait m_mutex;
		size_t m_uniqueID;
		int m_refCounter;

	private:
		LoaderBase *m_loaderBase;
	};

	LoaderBase() : m_queueIsEmpty(true) 
	{ 
		m_objectPool.reserve(SETTING_LOADER_RESERVE_SIZE);
	}
	~LoaderBase()
	{
		// Swap the queue with an empty one, effectively clearing it
		std::queue<UniqueObject*> emptyQueue;
		std::swap(m_objectUnloadQueue, emptyQueue);

		// Clear the object pool
		for(decltype(m_objectPool.size()) i = 0, size = m_objectPool.size(); i < size; i++)
			if(m_objectPool[i] != nullptr)
				delete m_objectPool[i];

		m_objectPool.clear();
	}

	// Unloads the oldest object in the queue (if there are any)
	// Only process one object, and should be called once per frame, to level out performance degradation
	inline void processReleaseQueue(SceneLoader &p_sceneLoader)
	{
		// First check if the queue isn't empty
		if(!m_queueIsEmpty)
		{
			for(decltype(Config::engineVar().loaders_num_of_unload_per_frame) i = 0, max = Config::engineVar().loaders_num_of_unload_per_frame; i < max;)
			{
				auto *object = m_objectUnloadQueue.front();

				// Check if the reference counter is less than 0. If it's not, that means that
				// something is still using the object, therefore it got on the unload queue by mistake,
				// so just remove it without deleting it
				if(object->m_refCounter < 1)
				{
					// Unload the object from RAM and VRAM
					unload(*static_cast<TObject *>(object), p_sceneLoader);
					m_objectRemoveQueue.push_back(object);
					i++;
				}

				// Remove object from queue
				m_objectUnloadQueue.pop();

				if(m_objectUnloadQueue.empty())
				{
					m_queueIsEmpty = true;
					return;
				}
			}
		}

		if(!m_objectRemoveQueue.empty())
		{
			for(decltype(m_objectRemoveQueue.size()) i = 0, size = m_objectRemoveQueue.size(); i < size; i++)
				removeObject(*m_objectRemoveQueue[i]);

			m_objectRemoveQueue.clear();
		}
	}

	// Preload all the objects to memory, in parallel; returns after the loading has been completed
	void preloadAllToMemory()
	{
		TaskManagerLocator::get().parallelFor(0, m_objectPool.size(), 1, [=](decltype(m_objectPool.size()) i)
		{
			// Increment the reference counter before loading the object, as a safety precaution
			// so it doesn't get deleted while it's loading
			m_objectPool[i]->incRefCounter();
			m_objectPool[i]->loadToMemory();
			m_objectPool[i]->decRefCounter();
		});
	}

	// Retrieve a const reference to an internal object pool
	const std::vector<TObject *> &getObjectPool() const { return m_objectPool; }

protected:
	virtual void unload(TObject &p_object, SceneLoader &p_sceneLoader) { }

	// Queue an object to be removed from memory
	inline void queueUnload(UniqueObject &p_object)
	{
		m_objectUnloadQueue.push(&p_object);
		m_queueIsEmpty = false;
	}

	// Swap the object with the last element of vector and pop_back
	inline void removeObject(UniqueObject &p_object)
	{
		auto uniqueID = p_object.getUniqueID();
		if(!(uniqueID < 0) && uniqueID < m_objectPool.size())
		{
			if(uniqueID != (m_objectPool.size() - 1))
			{
				std::swap(m_objectPool[uniqueID], m_objectPool.back());
				m_objectPool[uniqueID]->setUniqueID(uniqueID);
			}

			delete m_objectPool[m_objectPool.size() - 1];
			m_objectPool.pop_back();
		}
	}
	
	// Pool of template objects; accessible by derived classes
	std::vector<TObject*> m_objectPool;

	// Mutex used to block calls from other threads while operation is in progress
	SpinWait m_mutex;

private:
	// Most of the time queue will be empty, 
	// so checking a bool instead of .size() will be a bit faster
	bool m_queueIsEmpty;
	
	std::queue<UniqueObject *> m_objectUnloadQueue;
	std::vector<UniqueObject *> m_objectRemoveQueue;
};
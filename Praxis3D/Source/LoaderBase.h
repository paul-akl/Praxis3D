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
			m_loadedToMemory = false;
			m_loadedToVideoMemory = false;
			m_beingLoaded = false;
			m_refCounter = 0;
		}
		virtual ~UniqueObject()
		{
			unload();
		}

		// Increments reference counter
		inline void incRefCounter() { m_refCounter++; }

		// Decrements reference counter, if it goes to 0, object is put into unload queue
		inline void decRefCounter()
		{
			m_refCounter--;
			if(m_refCounter == 0)
			{
				// TODO: queue texture unload upon reference counter reaching zero
				//m_loaderBase->queueUnload(*this);
			}
		}

		// Setters
		inline void setLoadedToMemory(bool p_loaded)		{ m_loadedToMemory = p_loaded;		}
		inline void setLoadedToVideoMemory(bool p_loaded)	{ m_loadedToVideoMemory = p_loaded; }
		inline void setUniqueID(unsigned int p_uniqueID)	{ m_uniqueID = p_uniqueID;			}

		// Getters
		inline const bool isLoadedToMemory() const		{ return m_loadedToMemory;		}
		inline const bool isLoadedToVideoMemory() const { return m_loadedToVideoMemory; }
		inline const unsigned int getUniqueID()	const	{ return m_uniqueID;			}
		inline const std::string &getFilename() const	{ return m_filename;			}

		// Equality operator; compares filenames
		inline bool operator==(std::string p_string) { return m_filename == p_string; }

		inline ErrorCode unload()		 { return static_cast<TObject*>(this)->unloadMemory();		}

	protected:
		inline const bool isBeingLoaded() { return m_beingLoaded; }

		std::atomic_bool	m_beingLoaded,
							m_loadedToMemory,
							m_loadedToVideoMemory;

		ErrorCode m_loadingToMemoryError;
		std::string m_filename;
		SpinWait m_mutex;
		size_t	m_uniqueID,
				m_refCounter;

	private:
		LoaderBase *m_loaderBase;
	};

	LoaderBase() : m_queueIsEmpty(false) { }
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
	// Only process one object, and should be called once per frame, to level out performance
	inline void processReleaseQueue()
	{
		// First check if the queue isn't empty
		if(!m_queueIsEmpty)
		{
			// Check if the reference counter is less than 0. If it's not, that means that
			// something is still using the object, therefore it got on the unload queue by mistake,
			// so just remove it without deleting it
			if(m_objectUnloadQueue.front()->m_refCounter < 1)
			{
				// Unload the object from RAM and VRAM
				m_objectUnloadQueue.front()->unloadMemory();
				m_objectUnloadQueue.front()->unloadVideoMemory();
				removeObject(m_objectUnloadQueue.front());
				//delete m_objectUnloadQueue.front();
			}

			// Remove object from queue
			m_objectUnloadQueue.pop();

			if(m_objectUnloadQueue.size() < 1)
				m_queueIsEmpty = true;
		}
	}

	// Preload all the objects to memory, in parallel; returns after the loading has been completed
	void preloadAllToMemory()
	{
		//static_cast<TDerived*>(this)->preloadAllToMemory();

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

	// Returns false if there are any objects in the LoadToVideoMemory queue
	//const inline bool isLoadToVideoMemoryQueueEmpty() const { return m_objectLoadToVideoMemoryQueue.empty(); }

protected:
	// Queue and object to be removed from memory
	inline void queueUnload(UniqueObject &p_object)
	{
		m_objectUnloadQueue.push(&p_object);
		m_queueIsEmpty = false;
	}

	// Put an object into a queue for loading it into video memory
	//inline void queueLoadToVideoMemory(TObject &p_object)
	//{
	//	m_objectLoadToVideoMemoryQueue.push(&p_object);
	//}

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
	
	std::queue<UniqueObject*> m_objectUnloadQueue;
	//std::queue<TObject*> m_objectLoadToVideoMemoryQueue;
};
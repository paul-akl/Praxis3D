#pragma once

#include <string>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <vector>

#include "NullSystemObjects.h"
#include "System.h"

// A directory of all system objects, used to quickly get the name or the object itself.
// Every newly created system object registers itself with the directory.
class ObjectDirectory
{
public:

	// Initialize the service locator
	inline static ErrorCode init() 
	{
		// Set the object pool size; not thread-safe
		m_systemObjectPool.reserve(Config::engineVar().object_directory_init_pool_size);

		// Reserve the first ID for null object
		registerObject(m_nullObject);

		return ErrorCode::Success; 
	}

	// Register a system object, so it gets added to the directory. Returns a unique object ID, that can be used to retrieve the object later.
	inline static std::size_t registerObject(const SystemObject &p_systemObject)
	{
		// Index at which the object will be stored
		ObjectListIndexType objectIndex = 0;

		// Try to get an index of an empty element in the object list and check if the element is empty (null)
		if(m_emptyObjectListElements.try_pop(objectIndex) && m_systemObjectPool[objectIndex] == nullptr)
		{
			// Use the index that was acquired from the empty element list
			m_systemObjectPool[objectIndex] = &p_systemObject;
		}
		else
		{
			// There was no existing empty slot in the object list, so create a new slot
			// Add the object to the list, get an iterator and calculate the index at which the object was inserted
			objectIndex = (m_systemObjectPool.push_back(&p_systemObject) - m_systemObjectPool.begin());
		}

		// Cast the object index (int) to size_t (unsigned int)
		return static_cast<std::size_t>(objectIndex);
	}
	
	// Unregister object by the object ID
	inline static void unregisterObject(const std::size_t p_objectID)
	{
		// Check if the object ID is within bounds
		if(p_objectID < m_systemObjectPool.size())
		{
			// Check whether the slot is not empty already
			if(m_systemObjectPool[p_objectID] != nullptr)
			{
				// Make the slot empty
				m_systemObjectPool[p_objectID] = nullptr;

				// Add the slot to the empty element pool, so it can be used for a new object
				m_emptyObjectListElements.push(p_objectID);
			}
		}
	}

	// Unregister object by the object name
	inline static void unregisterObject(const std::string &p_objectName)
	{
		unregisterObject(getSystemObjectID(p_objectName));
	}

	// Unregister object by the system object 
	inline static void unregisterObject(const SystemObject &p_systemObject)
	{
		unregisterObject(p_systemObject.getObjectID());
	}

	// Returns the name of an object found by its ID. If the ID is incorrect or the object is not found, returns a null name
	const inline static std::string &getSystemObjectName(const std::size_t p_objectID)
	{
		return getSystemObject(p_objectID)->getName();
	}

	// Returns object ID found by its name.
	// This is slow, as it traverses through all the objects and compares names.
	// Note: to find the exact object, its name has to be unique.
	inline static std::size_t getSystemObjectID(const std::string &p_objectName)
	{
		// Check if the name isn't empty
		if(p_objectName.size() > 0)
		{
			// Go through all the objects
			for(decltype(m_systemObjectPool.size()) i = 0, size = m_systemObjectPool.size(); i < size; i++)
			{
				// Check if the object is valid and its name matches
				if(m_systemObjectPool[i] != nullptr && m_systemObjectPool[i]->getName() == p_objectName)
					return i;
			}
		}

		return 0;
	}

private:
	const inline static SystemObject *getSystemObject(const std::size_t p_objectID)
	{	
		// Default the return object to null system object
		const SystemObject *returnObject = &m_nullObject;

		// Check if the object ID is within bounds and assign the return object if it is
		if(p_objectID < m_systemObjectPool.size())
			returnObject = m_systemObjectPool[p_objectID];

		// Return the object; "nullptr" if it wasn't found
		return returnObject;
	}

	// System object directory index variable type
	typedef tbb::concurrent_vector<const SystemObject *>::size_type ObjectListIndexType;
	
	// System object directory
	static tbb::concurrent_vector<const SystemObject*> m_systemObjectPool;

	// A list of all empty slots in the system object directory
	static tbb::concurrent_queue<ObjectListIndexType> m_emptyObjectListElements;
	
	// Null system object that gets returned if no object is found
	static NullSystemObject m_nullObject;
};
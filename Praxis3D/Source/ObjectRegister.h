#pragma once

constexpr auto OBJECT_REGISTER_FREED_IDS_RESERVE_DIVIDER = 10;
constexpr auto OBJECT_REGISTER_RESIZE_ADDED_OVERHEAD_MULTIPLIER = 1.5;

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <vector>

#include "CommonDefinitions.h"

// An object directory, used for getting unique IDs to all registered objects.
// Also contains a list of registered object pointers, thus an object can be retrieved based on its ID.
// Implementation is NOT thread-safe.
template <class T_Object>
class ObjectRegister
{
public:
	ObjectRegister() { }
	~ObjectRegister() { }

	// Reserve internal space for given amount of objects to speed up the object registering
	void reserveSize(const std::size_t p_size)
	{
		m_objectPool.reserve(p_size);
		m_emptyObjectIDPool.reserve(p_size / OBJECT_REGISTER_FREED_IDS_RESERVE_DIVIDER);
	}

	// Returns a unique ID
	inline EntityID registerObject(T_Object p_object)
	{
		EntityID objectID = 0;
		
		// Check if there is an index of an unregistered element in the object pool
		if(!m_emptyObjectIDPool.empty())
		{
			// Get an index of an unregistered object
			objectID = m_emptyObjectIDPool.back();
			m_emptyObjectIDPool.pop_back();

			// If the retrieved object ID is empty, use it for the new given object
			// If it is not empty, recursively call registerObject to get a new object ID
			if(m_objectPool[objectID] == nullptr)
			{
				// Insert the given object into an empty slot
				m_objectPool[objectID] = p_object;
			}
			else
			{
				// Get a new object ID
				objectID = registerObject(p_object);
			}
		}
		else
		{
			// There was no existing unregistered ID in the object pool, so create a new ID
			// Add the object to the pool, get an iterator and calculate the index at which the object was inserted
			m_objectPool.push_back(p_object);
			objectID = m_objectPool.size() - 1;
		}

		return objectID;
	}

	// Tries to register an object with the given ID
	// Returns the ID object gets registered at. If ID is different than the given one, it was taken already
	inline EntityID registerObject(T_Object p_object, EntityID p_id)
	{
		EntityID objectID = 0;

		// Check if ID is within object pool bounds
		if(p_id >= m_objectPool.size())
		{
			// Get the current size of the pool and calculate the new size
			decltype(m_objectPool.size()) oldSize = m_objectPool.size();
			decltype(m_objectPool.size()) newSize = (p_id * OBJECT_REGISTER_RESIZE_ADDED_OVERHEAD_MULTIPLIER) + 1;

			// Resize the object pool to fit in the given ID
			// Add some overhead to the size, as there's a high chance that there will be more objects to be registered
			m_objectPool.resize(newSize, nullptr);

			// Add the newly created ID slots to the empty object ID pool
			for(decltype(m_objectPool.size()) i = oldSize - 1; i < newSize; i++)
			{
				// Add each new element to the empty object ID pool, excluding the given ID of the given object to be registered
				if(i != p_id)
					m_emptyObjectIDPool.push_back(i);
			}

			// Recursively call to register the given object, as the size should now fit the given ID
			objectID = registerObject(p_object, p_id);
		}
		else
		{
			// ID is within object pool
			// Check if ID slot is empty
			if(m_objectPool[p_id] == nullptr)
			{
				// Insert the given object into an empty slot
				m_objectPool[p_id] = p_object;
				objectID = p_id;
			}
			else
			{
				// Get a new ID for the object, as the given one is taken
				objectID = registerObject(p_object);
			}
		}

		return objectID;
	}

	// Removes object from directory, freeing up its ID
	inline void unregisterObject(const EntityID p_ID)
	{
		// Check if the object ID is within bounds
		if(p_ID < m_objectPool.size())
		{
			// Check if the object slot at the given ID isn't empty already
			if(m_objectPool[p_ID] != nullptr)
			{
				// Remove object from pool by setting its pointer to null
				m_objectPool[p_ID] = nullptr;

				// Add the index of removed object to empty object ID pool
				m_emptyObjectIDPool.push_back(p_ID);
			}
		}
	}

	// Returns object based on its ID. Returns 'nullptr' if object wasn't found
	inline T_Object getObject(const EntityID p_ID) const
	{		
		// Default the return object to nullptr
		T_Object *returnObject;
		
		// Check if the object ID is within bounds and assign the return object if it is
		if(p_ID < m_objectPool.size())
			returnObject = m_objectPool[p_ID];
		
		// Return the object; 'nullptr' if it wasn't found
		return returnObject;
	}

	// Good to call after registering multiple objects with specified IDs, as that might create
	// many empty ID slots, which might get filled, but not removed from internal register
	inline void sortRegisteredObjectIDs()
	{
		// If the empty object ID pool has elements
		if(!m_emptyObjectIDPool.empty())
		{
			// Declare a temporary pool as a copy of empty object ID pool
			decltype(m_emptyObjectIDPool) tempPool(m_emptyObjectIDPool);

			// Clear the original pool of all elements
			m_emptyObjectIDPool.clear();

			// Loop over each element in the temporary pool
			for(decltype(tempPool.size()) i = 0, size = tempPool.size(); i < size; i++)
			{
				// If the element of the contained ID is empty, add it back to the empty object ID pool
				if(m_objectPool[tempPool[i]] == nullptr)
					m_emptyObjectIDPool.push_back(tempPool[i]);
			}
		}
	}

private:
	// Finds and removes the element of empty object ID pool that contains the given ID
	// Slow, as it compares each element and uses std::vector erase
	void removeIDfromEmptyObjectPool(EntityID p_id)
	{
		// Iterate over each element in empty object ID pool
		for(decltype(m_emptyObjectIDPool.size()) i = 0, size = m_emptyObjectIDPool.size(); i < size; i++)
		{
			// If the ID of the current element matches
			if(m_emptyObjectIDPool[i] == p_id)
			{
				// Erase the matched element and return from the function
				m_emptyObjectIDPool.erase(m_emptyObjectIDPool.begin() + i);
				return;
			}
		}
	}

	// Object pool index variable type
	using ObjectPoolIndexType = typename std::vector<T_Object>::size_type;

	// All registered objects
	std::vector<T_Object> m_objectPool;

	// Indexes (IDs) of unregistered objects in the object pool
	std::vector<ObjectPoolIndexType> m_emptyObjectIDPool;
};

// An object directory, used for getting unique IDs to all registered objects.
// Also contains a list of registered object pointers, thus an object can be retrieved based on its ID.
// Implementation is thread-safe.
template <class T_Object>
class ObjectRegisterConcurrent 
{
public:
	ObjectRegisterConcurrent() { }
	~ObjectRegisterConcurrent() { }

	// Reserve internal space for given amount of objects to speed up the object registering
	void reserveSize(const std::size_t p_size)
	{
		m_objectPool.reserve(p_size);
	}

	// Returns a unique ID
	inline EntityID registerObject(T_Object p_object)
	{
		ObjectPoolIndexType objectID = 0;
		
		// Try to get an index of an unregistered element in the object pool
		if(m_emptyObjectIDPool.try_pop(objectID))
		{
			// If the retrieved object ID is empty, use it for the new given object
			// If it is not empty, recursively call registerObject to get a new object ID
			if(m_objectPool[objectID] == nullptr)
			{
				// Insert the given object into an empty slot
				m_objectPool[objectID] = p_object;
			}
			else
			{
				// Get a new object ID
				objectID = registerObject(p_object);
			}
		}
		else
		{
			// There was no existing unregistered ID in the object pool, so create a new ID
			// Add the object to the pool, get an iterator and calculate the index at which the object was inserted
			objectID = (m_objectPool.push_back(p_object) - m_objectPool.begin());
		}
		
		// Cast the object index (int) to size_t (unsigned int)
		return static_cast<EntityID>(objectID);
	}
	
	// Tries to register an object with the given ID
	// Returns the ID object gets registered at. If ID is different than the given one, it was taken already
	inline EntityID registerObject(T_Object p_object, EntityID p_id)
	{
		EntityID objectID = 0;

		// Check if ID is within object pool bounds
		if(p_id >= m_objectPool.size())
		{
			// Get the current size of the pool and calculate the new size
			decltype(m_objectPool.size()) oldSize = m_objectPool.size();
			decltype(m_objectPool.size()) newSize = static_cast<decltype(m_objectPool.size())>((p_id * OBJECT_REGISTER_RESIZE_ADDED_OVERHEAD_MULTIPLIER) + 1);

			// Grow the object pool to fit in the given ID and get the starting index at which it was grown from
			// Add some overhead to the size, as there's a high chance that there will be more objects to be registered
			ObjectPoolIndexType poolResizeStartIndex = m_objectPool.grow_to_at_least(newSize, nullptr) - m_objectPool.begin();

			// Iterate over each element that was added by growing the object pool
			for(decltype(poolResizeStartIndex) i = poolResizeStartIndex; i < newSize; i++)
			{
				// Add each new element to the empty object ID pool, excluding the given ID of the given object to be registered
				if(i != p_id)
					m_emptyObjectIDPool.push(i);
			}

			// Recursively call to register the given object, as the size should now fit the given ID
			objectID = registerObject(p_object, p_id);
		}
		else
		{
			// ID is within object pool
			// Check if ID slot is empty
			if(m_objectPool[p_id] == nullptr)
			{
				// Insert the given object into an empty slot
				m_objectPool[p_id] = p_object;
				objectID = p_id;
			}
			else
			{
				// Get a new ID for the object, as the given one is taken
				objectID = registerObject(p_object);
			}
		}

		return objectID;
	}

	// Removes object from directory, freeing up its ID
	inline void unregisterObject(const EntityID p_ID)
	{
		// Check if the object ID is within bounds
		if(p_ID < m_objectPool.size())
		{
			// Check if the object slot at the given ID isn't empty already
			if(m_objectPool[p_ID] != nullptr)
			{
				// Remove object from pool by setting its pointer to null
				m_objectPool[p_ID] = nullptr;

				// Add the index of removed object to empty object ID pool
				m_emptyObjectIDPool.push(p_ID);
			}
		}
	}

	// Returns object based on its ID. Returns 'nullptr' if object wasn't found
	inline T_Object *getObject(const EntityID p_ID)
	{	
		// Default the return object to nullptr
		T_Object *returnObject = nullptr;

		// Check if the object ID is within bounds and assign the return object if it is
		if(p_ID < m_objectPool.size())
			returnObject = &m_objectPool[p_ID];

		// Return the object; 'nullptr' if it wasn't found
		return returnObject;
	}
	
	// Good to call after registering multiple objects with specified IDs, as that might create
	// many empty ID slots, which might get filled, but not removed from internal register
	inline void sortRegisteredObjectIDs()
	{
		// Declare a temporary pool as a copy of empty object ID pool
		decltype(m_emptyObjectIDPool) tempPool(m_emptyObjectIDPool);
		
		// Clear the original pool of all elements
		m_emptyObjectIDPool.clear();

		// Temporary ID, used for getting an ID from the tempPool by passing it as an argument
		ObjectPoolIndexType tempID = 0;
		
		// Try to get an element from the temporary pool
		while(tempPool.try_pop(tempID))
		{
			// If the element of the contained ID is empty, add it back to the empty object ID pool
			if(m_objectPool[tempID] == nullptr)
				m_emptyObjectIDPool.push(tempID);
		}
	}

private:
	// Object pool index variable type
	using ObjectPoolIndexType = typename tbb::concurrent_vector<T_Object>::size_type;
	
	// All registered objects
	tbb::concurrent_vector<T_Object> m_objectPool;

	// Indexes (IDs) of unregistered objects in the object pool
	tbb::concurrent_queue<ObjectPoolIndexType> m_emptyObjectIDPool;
};
#include "ErrorCodes.h"

// Generic object pool (used to optimize data locality). Simplistic design intended
// for game objects (i.e. entities that need fast random access, add/remove, etc)
// Maintains a static array of objects in contiguous memory, to reduce cache misses.
template <class T_Object>
class ObjectPool
{
public:
	// Object wrapper container, used to store book-keeping values next to the object
	// Safe to pass it to other systems, since all objectPool-specific data is hidden away
	class Object
	{
		friend class ObjectPool;
	public:
		const inline bool allocated() const { return m_allocated; }

		const inline size_t getIndex() const {return m_index; }

		// Returns the raw object stored in the wrapper
		inline T_Object *getObject() { return m_object; }

		// Assignment operator; accepts raw object; convenient for providing raw object
		inline Object &operator=(const T_Object &p_object) { setObject(p_object); }

		// Setter for the raw object
		inline void setObject(T_Object &p_object)
		{
			new (m_object) T_Object(p_object);
			m_allocated = true;
		}

		// Calls the constructor of template object (using placement new); passes the given arguments
		template<class... T_Args>
		inline void construct(T_Args&&... p_args)
		{
			// Allocate the template object
			::new (static_cast<void*>(m_object)) T_Object(std::forward<T_Args>(p_args)...);
			m_allocated = true;
		}

	private:
		Object(size_t p_index) : m_allocated(false), m_object(nullptr), m_nextAvailable(nullptr), m_index(p_index) { }
		~Object() { }

		// Assignment operator
		inline Object &operator=(const Object &p_object)
		{
			m_allocated = p_object.m_allocated;
			//m_index = p_object.m_index;
			m_object = &p_object.m_object;
			m_nextAvailable = p_object.m_nextAvailable;
		}
		inline Object *getNext() const { return m_nextAvailable; }
		inline void setNext(Object *p_nextAvailable) { m_nextAvailable = p_nextAvailable; }
		inline void setRawObject(T_Object *p_rawObject) { m_object = p_rawObject; }

		size_t m_index;
		bool m_allocated;
		T_Object *m_object;
		Object *m_nextAvailable;
	};

	// Initializes the object pool on construction
	ObjectPool() : m_poolSize(1), m_numAllocatedObjects(0)
	{
		m_lastAddedObject = nullptr;
		m_rawObjectPool = nullptr;

		setSize(&m_objectPool, &m_rawObjectPool, &m_firstAvailable, m_poolSize);
	}
	~ObjectPool()
	{
		delete[] m_objectPool;
	}

	inline void init(size_t p_poolSize)
	{
		if(p_poolSize > 0)
		{
			// Deallocate the old pools
			free(m_objectPool);
			free(m_rawObjectPool);

			m_poolSize = p_poolSize;
			setSize(&m_objectPool, &m_rawObjectPool, &m_firstAvailable, m_poolSize);
		}
	}

	// Finds an unused object, marks it for usage and returns it; returns nullptr if pool is full
	// This might be faster for bigger objects, than using add(), as instead of 
	// using a copy operator, it just returns the object, so the caller can initialize the object themselves
	// Also useful if the caller needs to store the object index after creating it (used for removal)
	inline Object *newObject()
	{
		if(m_firstAvailable != nullptr)
		{
			Object *newObject = m_firstAvailable;
			m_firstAvailable = newObject->getNext();

			// Mark the object as allocated
			newObject->m_allocated = true;

			// Make this object the last one added
			m_lastAddedObject = newObject;
			
			// Increment the total number of allocated objects
			m_numAllocatedObjects++;

			// Return the newly allocated object
			return newObject;
		}

		// If this point is reached, object couldn't be allocated, so return a null pointer instead
		return nullptr;
	}

	// Finds an unused object, uses copy constructor to initialize it; returns an error code if pool is full
	inline ErrorCode addOld(T_Object &&p_object)
	{
		ErrorCode returnError = ErrorCode::Success;

		// If the pool is not full
		if(m_firstAvailable != nullptr)
		{
			// Assign the next available object
			Object *newObject = m_firstAvailable;
			m_firstAvailable = newObject->getNext();

			// Allocate the template object
			//new (newObject->m_object) T_Object(p_object);
			//newObject->m_object = new T_Object(p_object);

			new (static_cast<void*>(newObject->m_object)) T_Object(std::move(p_object));

			//*newObject->m_object = std::move(p_object);

			newObject->m_allocated = true;

			// Make this object the last one added
			m_lastAddedObject = newObject;

			// Increment the total number of allocated objects
			m_numAllocatedObjects++;

			//memcpy(newObject->m_object, &p_object, sizeof(T_Object));
		}
		else
			returnError = ErrorCode::ObjectPool_full;

		return returnError;
	}

	// Takes only the constructor arguments and initializes the template object by calling the constructor
	// internally. This way, it eliminates the need for temporary variables; returns an ErrorCode if pool is full
	template<class... T_Args>
	inline ErrorCode add(T_Args&&... p_args)
	{
		// If the pool is not full
		if(m_firstAvailable != nullptr)
		{
			// Assign the next available object
			Object *newObject = m_firstAvailable;
			m_firstAvailable = newObject->getNext();

			// Allocate the template object
			::new (static_cast<void*>(newObject->m_object)) T_Object(std::forward<T_Args>(p_args)...);

			// Mark object as allocated
			newObject->m_allocated = true;

			// Make this object the last one added
			m_lastAddedObject = newObject;

			// Increment the total number of allocated objects
			m_numAllocatedObjects++;

			return ErrorCode::Success;
		}
		else
			return ErrorCode::ObjectPool_full;
	}

	// Takes in a unique object index (that corresponds to the actual index in the pool array),
	// and marks the object as not in use anymore
	inline void remove(size_t p_index)
	{
		// If the index is not out of bounds
		if((p_index >= 0) && (p_index < m_poolSize))
			// If the object is allocated
			if(m_objectPool[p_index].m_allocated == true)
			{
				// Set is as a new first available and deallocate it
				m_objectPool[p_index].setNext(m_firstAvailable);
				m_firstAvailable = &m_objectPool[p_index];
				m_objectPool[p_index].m_allocated = false;

				// Decrement the total number of allocated objects
				m_numAllocatedObjects--;
			}
	}

	// Resizes the pool to the new given size; retains however many objects fit in the new pool size; very expensive
	inline void resize(size_t p_poolSize)
	{
		// If the pool is not the given size already
		if(p_poolSize != m_poolSize)
		{
			Object *newObjectPool;
			T_Object *newRawObjectPool;
			
			setSize(newObjectPool, newRawObjectPool, m_firstAvailable, p_poolSize);

			m_firstAvailable = nullptr;

			// Iterate over all pool elements and find the first available element; early bail when found
			for(decltype(p_poolSize) i = 0; i < p_poolSize; i++)
			{
				if(!newObjectPool[i].allocated())
				{
					m_firstAvailable = &newObjectPool[i];
					break;
				}
			}

			// Add the objects from the old pool
			for(decltype(m_poolSize) i = 0; i < m_poolSize; i++)
				if(m_objectPool[i].allocated())
					add(m_objectPool[i].getObject());

			// Deallocate the old pools
			free(m_objectPool);
			free(m_rawObjectPool);

			// Assign the new pools
			m_objectPool = newObjectPool;
			m_rawObjectPool = newRawObjectPool;
			m_poolSize = p_poolSize;

			// Reset the total number of allocated objects variable
			m_numAllocatedObjects = 0;

			// Freshly calculate the total number of allocated objects
			for(decltype(m_poolSize) i = 0; i < m_poolSize; i++)
				if(m_objectPool[i].allocated())
					m_numAllocatedObjects++;
		}
	}

	// Returns the actual template object; returns nullptr if index is invalid or object is not in use
	inline T_Object *getRawObject(size_t p_index)
	{
		T_Object *returnObject = nullptr;

		if((p_index >= 0) && (p_index < m_poolSize))
			if(m_objectPool[p_index].m_allocated == true)
				returnObject = m_objectPool[p_index].m_object;

		return returnObject;
	}

	// Returns the actual template object; does not have any validity checks, trading safety for performance
	inline T_Object *getRawObjectUnsafe(size_t p_index) { return m_rawObjectPool[p_index]; }

	// Returns the object wrapper; returns nullptr if index is invalid or object is not in use
	inline Object *getObject(size_t p_index)
	{
		Object *returnObject = nullptr;

		if((p_index >= 0) && (p_index < m_poolSize))
			if(m_objectPool[p_index].m_allocated == true)
				returnObject = &m_objectPool[p_index];

		return returnObject;
	}

	// Returns the object wrapper; does not have any validity checks, trading safety for performance
	inline Object *getObjectUnsafe(size_t p_objectIndex) { return m_objectPool[p_objectIndex]; }

	// Returns a const pointer to the raw template object pool; use only for performance critical code
	const inline T_Object *getRawObjectPool() const { return m_rawObjectPool; }

	// Returns a const pointer to the object-wrapper pool; use only for performance critical code
	const inline Object *getObjectPool() const { return m_objectPool; }

	// Returns an object that was the last one to be successfully added to the pool
	inline Object *getLastObject() const { return m_lastAddedObject; }

	// Returns a template object that was the last one to be successfully added to the pool
	inline T_Object *getLastRawObject() const { return m_lastAddedObject->m_object; }

	// Returns current size of the object pool
	inline size_t getPoolSize() const { return m_poolSize; }

	// Returns the total number of allocated objects
	inline size_t getNumAllocated() const { return m_numAllocatedObjects; }

	// Array subscription operator; unsafe - does not check for index being out of bounds
	inline Object &operator[] (size_t p_index) const { return m_objectPool[p_index]; }

private:
	// Meant to be called for setting the size of the pool for the first time,
	// as it does not preserve the old object pool
	inline void setSize(Object *p_objectPool[], T_Object *p_rawObjectPool[], Object **p_firstAvailable, size_t p_poolSize)
	{
		// Create the new pool of the given size
		*p_objectPool = (Object*)malloc(sizeof(Object) * p_poolSize);

		// Allocate memory for the raw object pool
		*p_rawObjectPool = (T_Object*)malloc(sizeof(T_Object) * p_poolSize);

		// Iterate over all objects, allocate using placement new (calls constructor) and initialize values
		for(decltype(p_poolSize) i = 0; i < p_poolSize; i++)
		{
			new (&(*p_objectPool)[i]) Object(i);
			(*p_objectPool)[i].setRawObject(&(*p_rawObjectPool)[i]);
			(*p_objectPool)[i].setNext(&(*p_objectPool)[i + 1]);
		}

		// Initialize the last object's next available to null
		(*p_objectPool)[p_poolSize - 1].setNext(nullptr);

		// Set the first available object
		*p_firstAvailable = &(*p_objectPool)[0];
	}

	// Array for template object wrappers
	Object *m_objectPool;

	// Next yet unallocated object
	Object *m_firstAvailable;

	// Last object that was added to the pool
	Object *m_lastAddedObject;

	// Template objects held in contiguous memory
	T_Object *m_rawObjectPool;

	size_t m_poolSize;
	size_t m_numAllocatedObjects;
};
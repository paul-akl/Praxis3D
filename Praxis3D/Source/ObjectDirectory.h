#pragma once

#include <string>
#include <vector>

#include "System.h"

// A directory of all system objects, used to quickly get the name or the object itself.
// Every newly created system object registers itself with the directory.
class ObjectDirectory
{
public:

	// Initialize the service locator
	inline static ErrorCode init() 
	{
		m_nullName = "Null Object";

		// Reserve the first ID for null object
		registerObject(nullptr);

		return ErrorCode::Success; 
	}

	// Register a system object, so it gets added to the directory. Returns a unique object ID, that can be used to retrieve the object later.
	inline static size_t registerObject(const SystemObject *p_systemObject)
	{
		auto uniqueID = m_systemObjectList.size();

		m_systemObjectList.push_back(p_systemObject);

		return uniqueID;
	}

	// Returns the name of an object found by its ID. If the ID is incorrect or the object is not found, returns a null name
	const inline std::string &getObjectName(size_t p_objectID)
	{
		if(p_objectID < m_systemObjectList.size())
		{
			const SystemObject *object = m_systemObjectList[p_objectID];

			if(object != nullptr)
				return object->getName();
		}

		return m_nullName;
	}

	// Returns object ID found by its name.
	// This is slow, as it traverses through all the objects and compares names.
	// Note: to find the exact object, its name has to be unique.
	inline static size_t getSystemObjectID(const std::string &p_objectName)
	{
		// Check if the name isn't empty
		if(p_objectName.size() > 0)
		{
			// Go through all the objects
			for(decltype(m_systemObjectList.size()) i = 0, size = m_systemObjectList.size(); i < size; i++)
			{
				// Check if the object is valid and its name matches
				if(m_systemObjectList[i] != nullptr && m_systemObjectList[i]->getName() == p_objectName)
					return i;
			}
		}

		return 0;
	}

private:
	// System object directory
	static std::vector<const SystemObject*> m_systemObjectList;

	// Null name that gets returned if no object is found
	static std::string m_nullName;
};
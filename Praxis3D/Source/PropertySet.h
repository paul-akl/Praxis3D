#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include "Config.h"
#include "Math.h"
#include "Utilities.h"

class PropertySet;

// Stores a single value of a variable type. Has a getter function for each type. If a getter of
// incorrect type is called, it deals with it by converting the internal value to a type of the
// getter. Has a propertyID tag, to distinguish between other instances.
class Property
{
	friend class PropertySet;
public:
	// Type can be determined by overloaded constructor call
	Property() : m_propertyID(Properties::Null), m_variableType(Type_null) { }
	Property(const Properties::PropertyID p_propertyID) : m_propertyID(p_propertyID), m_variableType(Type_null) { }
	Property(const Properties::PropertyID p_propertyID, const bool p_variable) : m_propertyID(p_propertyID), m_variableType(Type_bool)
	{ 
		m_variable.m_bool = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const int p_variable) : m_propertyID(p_propertyID), m_variableType(Type_int)
	{
		m_variable.m_int = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const float p_variable) : m_propertyID(p_propertyID), m_variableType(Type_float)
	{
		m_variable.m_float = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const double p_variable) : m_propertyID(p_propertyID), m_variableType(Type_double)
	{
		m_variable.m_double = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const Math::Vec2i &p_variable) : m_propertyID(p_propertyID), m_variableType(Type_vec2i)
	{
		m_variable.m_vec2i = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const Math::Vec2f &p_variable) : m_propertyID(p_propertyID), m_variableType(Type_vec2f)
	{
		m_variable.m_vec2f = p_variable;
		//m_vec2f = new Math::Vec2f(p_variable);
	}
	Property(const Properties::PropertyID p_propertyID, const Math::Vec3f &p_variable) : m_propertyID(p_propertyID), m_variableType(Type_vec3f)
	{
		m_variable.m_vec3f = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const Math::Vec4f &p_variable) : m_propertyID(p_propertyID), m_variableType(Type_vec4f)
	{
		m_variable.m_vec4f = p_variable;
	}
	Property(const Properties::PropertyID p_propertyID, const std::string &p_variable) : m_propertyID(p_propertyID), m_variableType(Type_string)
	{
		new (&m_variable.m_string) std::string(p_variable);
	}
	Property(const Properties::PropertyID p_propertyID, const Properties::PropertyID p_variable) : m_propertyID(p_propertyID), m_variableType(Type_propertyID)
	{
		m_variable.m_ID = p_variable;
	}
	Property(const Property &p_property) : m_propertyID(p_property.m_propertyID), m_variableType(p_property.m_variableType)
	{
		switch(m_variableType)
		{
		case Property::Type_bool:
			m_variable.m_bool = p_property.m_variable.m_bool;
			break;
		case Property::Type_int:
			m_variable.m_int = p_property.m_variable.m_int;
			break;
		case Property::Type_float:
			m_variable.m_float = p_property.m_variable.m_float;
			break;
		case Property::Type_double:
			m_variable.m_double = p_property.m_variable.m_double;
			break;
		case Property::Type_vec2i:
			m_variable.m_vec2i = p_property.m_variable.m_vec2i;
			break;
		case Property::Type_vec2f:
			m_variable.m_vec2f = p_property.m_variable.m_vec2f;
			break;
		case Property::Type_vec3f:
			m_variable.m_vec3f = p_property.m_variable.m_vec3f;
			break;
		case Property::Type_vec4f:
			m_variable.m_vec4f = p_property.m_variable.m_vec4f;
			break;
		case Property::Type_string:
			new (&m_variable.m_string) std::string(p_property.m_variable.m_string);
			break;
		case Property::Type_propertyID:
			m_variable.m_ID = p_property.m_variable.m_ID;
			break;
		}
	}
	//~Property() { }

	// Getters for each type
	const inline bool getBool() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return false;
			break;
		case Property::Type_bool:
			return m_variable.m_bool;
			break;
		case Property::Type_int:
			return m_variable.m_int == 0 ? false : true;
			break;
		case Property::Type_float:
			return m_variable.m_float == 0.0f ? false : true;
			break;
		case Property::Type_double:
			return m_variable.m_double == 0.0 ? false : true;
			break;
		case Property::Type_vec2i:
			return m_variable.m_vec2i.x == 0 && m_variable.m_vec2i.y == 0 ? false : true;
			break;
		case Property::Type_vec2f:
			return m_variable.m_vec2f.x == 0.0f && m_variable.m_vec2f.y == 0.0f ? false : true;
			break;
		case Property::Type_vec3f:
			return m_variable.m_vec3f.x == 0.0f && m_variable.m_vec3f.y == 0.0f
				&& m_variable.m_vec3f.z == 0.0f ? false : true;
			break;
		case Property::Type_vec4f:
			return m_variable.m_vec4f.x == 0.0f && m_variable.m_vec4f.y == 0.0f
				&& m_variable.m_vec4f.z == 0.0f && m_variable.m_vec4f.w == 0.0f ? false : true;
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? false : true;
			break;
		case Property::Type_propertyID:
			return m_variable.m_ID == Properties::Null ? false : true;
			break;
		}
	}
	const inline int getInt() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return 0;
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? 0 : 1;
			break;
		case Property::Type_int:
			return m_variable.m_int;
			break;
		case Property::Type_float:
			return (int)m_variable.m_float;
			break;
		case Property::Type_double:
			return (int)m_variable.m_double;
			break;
		case Property::Type_vec2i:
			return m_variable.m_vec2i.x;
			break;
		case Property::Type_vec2f:
			return (int)m_variable.m_vec2f.x;
			break;
		case Property::Type_vec3f:
			return (int)m_variable.m_vec3f.x;
			break;
		case Property::Type_vec4f:
			return (int)m_variable.m_vec4f.x;
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? 0 : (int)(m_variable.m_string)[0];
			break;
		case Property::Type_propertyID:
			return (int)m_variable.m_ID;
			break;
		}
	}
	const inline float getFloat() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return 0.0f;
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? 0.0f : 1.0f;
			break;
		case Property::Type_int:
			return (float)m_variable.m_int;
			break;
		case Property::Type_float:
			return m_variable.m_float;
			break;
		case Property::Type_double:
			return (float)m_variable.m_double;
			break;
		case Property::Type_vec2i:
			return (float)m_variable.m_vec2i.x;
			break;
		case Property::Type_vec2f:
			return m_variable.m_vec2f.x;
			break;
		case Property::Type_vec3f:
			return m_variable.m_vec3f.x;
			break;
		case Property::Type_vec4f:
			return m_variable.m_vec4f.x;
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? 0.0f : (float)(m_variable.m_string)[0];
			break;
		case Property::Type_propertyID:
			return (float)m_variable.m_ID;
			break;
		}
	}
	const inline double getDouble() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return 0.0;
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? 0.0 : 1.0;
			break;
		case Property::Type_int:
			return (double)m_variable.m_int;
			break;
		case Property::Type_float:
			return (double)m_variable.m_float;
			break;
		case Property::Type_double:
			return m_variable.m_double;
			break;
		case Property::Type_vec2i:
			return (double)m_variable.m_vec2i.x;
			break;
		case Property::Type_vec2f:
			return (double)m_variable.m_vec2f.x;
			break;
		case Property::Type_vec3f:
			return (double)m_variable.m_vec3f.x;
			break;
		case Property::Type_vec4f:
			return (double)m_variable.m_vec4f.x;
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? 0.0 : (double)(m_variable.m_string)[0];
			break;
		case Property::Type_propertyID:
			return (double)m_variable.m_ID;
			break;
		}
	}
	const inline Math::Vec2i getVec2i() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return Math::Vec2i(0);
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? Math::Vec2i(0) : Math::Vec2i(1);
			break;
		case Property::Type_int:
			return Math::Vec2i(m_variable.m_int);
			break;
		case Property::Type_float:
			return Math::Vec2i((int)m_variable.m_float);
			break;
		case Property::Type_double:
			return Math::Vec2i((int)m_variable.m_double);
			break;
		case Property::Type_vec2i:
			return m_variable.m_vec2i;
			break;
		case Property::Type_vec2f:
			return Math::Vec2i((int)m_variable.m_vec2f.x, (int)m_variable.m_vec2f.y);
			break;
		case Property::Type_vec3f:
			return Math::Vec2i((int)m_variable.m_vec3f.x, (int)m_variable.m_vec3f.y);
			break;
		case Property::Type_vec4f:
			return Math::Vec2i((int)m_variable.m_vec4f.x, (int)m_variable.m_vec4f.y);
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? Math::Vec2i(0) : Math::Vec2i((int)(m_variable.m_string)[0]);
			break;
		case Property::Type_propertyID:
			return Math::Vec2i((int)m_variable.m_ID);
			break;
		}
	}
	const inline Math::Vec2f getVec2f() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return Math::Vec2f(0.0f);
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? Math::Vec2f(0.0f) : Math::Vec2f(1.0f);
			break;
		case Property::Type_int:
			return Math::Vec2f((float)m_variable.m_int);
			break;
		case Property::Type_float:
			return Math::Vec2f(m_variable.m_float);
			break;
		case Property::Type_double:
			return Math::Vec2f((float)m_variable.m_double);
			break;
		case Property::Type_vec2i:
			return Math::Vec2f((float)m_variable.m_vec2i.x, (float)m_variable.m_vec2i.y);
			break;
		case Property::Type_vec2f:
			return m_variable.m_vec2f;
			break;
		case Property::Type_vec3f:
			return Math::Vec2f(m_variable.m_vec3f);
			break;
		case Property::Type_vec4f:
			return Math::Vec2f(m_variable.m_vec4f);
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? Math::Vec2f(0.0f) : Math::Vec2f((float)(m_variable.m_string)[0]);
			break;
		case Property::Type_propertyID:
			return Math::Vec2f((float)m_variable.m_ID);
			break;
		}
	}
	const inline Math::Vec3f getVec3f() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return Math::Vec3f(0.0f);
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? Math::Vec3f(0.0f) : Math::Vec3f(1.0f);
			break;
		case Property::Type_int:
			return Math::Vec3f((float)m_variable.m_int);
			break;
		case Property::Type_float:
			return Math::Vec3f(m_variable.m_float);
			break;
		case Property::Type_double:
			return Math::Vec3f((float)m_variable.m_double);
			break;
		case Property::Type_vec2i:
			return Math::Vec3f((float)m_variable.m_vec2i.x, (float)m_variable.m_vec2i.y, 0.0f);
			break;
		case Property::Type_vec2f:
			return Math::Vec3f(m_variable.m_vec2f, 0.0f);
			break;
		case Property::Type_vec3f:
			return m_variable.m_vec3f;
			break;
		case Property::Type_vec4f:
			return Math::Vec3f(m_variable.m_vec4f);
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? Math::Vec3f(0.0f) : Math::Vec3f((float)(m_variable.m_string)[0]);
			break;
		case Property::Type_propertyID:
			return Math::Vec3f((float)m_variable.m_ID);
			break;
		}
	}
	const inline Math::Vec4f getVec4f() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return Math::Vec4f(0.0f);
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? Math::Vec4f(0.0f) : Math::Vec4f(1.0f);
			break;
		case Property::Type_int:
			return Math::Vec4f((float)m_variable.m_int);
			break;
		case Property::Type_float:
			return Math::Vec4f(m_variable.m_float);
			break;
		case Property::Type_double:
			return Math::Vec4f((float)m_variable.m_double);
			break;
		case Property::Type_vec2i:
			return Math::Vec4f((float)m_variable.m_vec2i.x, (float)m_variable.m_vec2i.y, 0.0f, 0.0f);
			break;
		case Property::Type_vec2f:
			return  Math::Vec4f(m_variable.m_vec2f, 0.0f, 0.0f);
			break;
		case Property::Type_vec3f:
			return Math::Vec4f(m_variable.m_vec3f, 0.0f);
			break;
		case Property::Type_vec4f:
			return m_variable.m_vec4f;
			break;
		case Property::Type_string:
			return m_variable.m_string == "" ? Math::Vec4f(0.0f) : Math::Vec4f((float)(m_variable.m_string)[0]);
			break;
		case Property::Type_propertyID:
			return Math::Vec4f((float)m_variable.m_ID);
			break;
		}
	}
	const inline std::string getString() const
	{
		switch(m_variableType)
		{
		default:
		case Property::Type_null:
			return "";
			break;
		case Property::Type_bool:
			return m_variable.m_bool == false ? "0" : "1";
			break;
		case Property::Type_int:
			return Utilities::toString(m_variable.m_int);
			break;
		case Property::Type_float:
			return Utilities::toStringCpp(m_variable.m_float);
			break;
		case Property::Type_double:
			return Utilities::toString(m_variable.m_double);
			break;
		case Property::Type_vec2i:
			return Utilities::toString(m_variable.m_vec2i.x) + ", " + Utilities::toString(m_variable.m_vec2i.y);
			break;
		case Property::Type_vec2f:
			return Utilities::toStringCpp(m_variable.m_vec2f.x) + ", " + Utilities::toStringCpp(m_variable.m_vec2f.y);
			break;
		case Property::Type_vec3f:
			return Utilities::toStringCpp(m_variable.m_vec3f.x)
				+ ", " + Utilities::toStringCpp(m_variable.m_vec3f.y)
				+ ", " + Utilities::toStringCpp(m_variable.m_vec3f.z);
			break;
		case Property::Type_vec4f:
			return Utilities::toStringCpp(m_variable.m_vec4f.x)
				+ ", " + Utilities::toStringCpp(m_variable.m_vec4f.y)
				+ ", " + Utilities::toStringCpp(m_variable.m_vec4f.z)
				+ ", " + Utilities::toStringCpp(m_variable.m_vec4f.w);
			break;
		case Property::Type_string:
			return m_variable.m_string;
			break;
		case Property::Type_propertyID:
			return GetString(m_variable.m_ID);
			break;
		}
	}
	const inline Properties::PropertyID getID() const
	{
		if(m_variableType == Type_propertyID)
			return m_variable.m_ID;
		else
			return Properties::Null;
	}

	// Returns the property ID of Property class, NOT the property ID stored in enum (as property value)
	const inline Properties::PropertyID getPropertyID() const				{ return m_propertyID;			}
	const inline void setPropertyID(Properties::PropertyID &p_propertyID)	{ m_propertyID = p_propertyID;	}

	// Equal and less than operators accepting property ID enum
	const inline bool operator==(const Properties::PropertyID &p_propertyID) const { return (m_propertyID == p_propertyID); }
	const inline bool operator<(const Properties::PropertyID &p_propertyID) const { return (m_propertyID < p_propertyID); }

	// Equal and less than operators accepting property class
	const inline bool operator==(const Property &p_property) const { return (m_propertyID == p_property.m_propertyID); }
	const inline bool operator<(const Property &p_property) const { return (m_propertyID < p_property.m_propertyID); }

	// Assignment operator
	Property &Property::operator=(const Property &p_property)
	{
		m_propertyID = p_property.m_propertyID;
		m_variableType = p_property.m_variableType;

		switch(m_variableType)
		{
		case Property::Type_bool:
			m_variable.m_bool = p_property.m_variable.m_bool;
			break;
		case Property::Type_int:
			m_variable.m_int = p_property.m_variable.m_int;
			break;
		case Property::Type_float:
			m_variable.m_float = p_property.m_variable.m_float;
			break;
		case Property::Type_double:
			m_variable.m_double = p_property.m_variable.m_double;
			break;
		case Property::Type_vec2i:
			m_variable.m_vec2i = p_property.m_variable.m_vec2i;
			break;
		case Property::Type_vec2f:
			m_variable.m_vec2f = p_property.m_variable.m_vec2f;
			break;
		case Property::Type_vec3f:
			m_variable.m_vec3f = p_property.m_variable.m_vec3f;
			break;
		case Property::Type_vec4f:
			m_variable.m_vec4f = p_property.m_variable.m_vec4f;
			break;
		case Property::Type_string:
			new (&m_variable.m_string) std::string(p_property.m_variable.m_string);
			break;
		case Property::Type_propertyID:
			m_variable.m_ID = p_property.m_variable.m_ID;
			break;
		}

		return *this;
	}

	// Bool operator; returns true if the property is valid and false if the property is null
	inline explicit operator bool() const { return m_propertyID != Properties::Null; }

	// Converts the property to text (used for saving properties to text files)
	const inline std::string toString() { return "\"" + std::string(GetString(m_propertyID)) + "\": \"" + getString() + "\""; }
private:
	enum VariableType
	{
		Type_null,
		Type_bool,
		Type_int,
		Type_float,
		Type_double,
		Type_vec2i,
		Type_vec2f,
		Type_vec3f,
		Type_vec4f,
		Type_string,
		Type_propertyID
	} m_variableType;
	union VariableUnion
	{
		VariableUnion() { }
		~VariableUnion() { }

		bool m_bool;
		int m_int;
		float m_float;
		double m_double;
		Math::Vec2i m_vec2i;
		Math::Vec2f m_vec2f;
		Math::Vec3f m_vec3f;
		Math::Vec4f m_vec4f;
		std::string m_string;
		Properties::PropertyID m_ID;
	} m_variable;

	Properties::PropertyID m_propertyID;
};

// Holds an array of Property classes and an array of PropertySet classes
class PropertySet
{
public:
	PropertySet(Properties::PropertyID p_propertyID = Properties::Null)
		: m_propertyID(p_propertyID), m_optimizedForSearch(false), m_numPropertySets(0), m_numProperties(0) { }
	~PropertySet() { }

	// Adds an instance of property to internal array; takes only the arguments of property
	template<class... T_Args>
	inline void addProperty(T_Args&&... p_args)
	{
		m_properties.emplace_back(std::forward<T_Args>(p_args)...);
		m_numProperties++;
		m_optimizedForSearch = false;
	}

	// Adds an instance of property set to internal array; takes only the arguments of property set
	template<class... T_Args>
	inline PropertySet &addPropertySet(T_Args&&... p_args)
	{
		m_propertySets.emplace_back(std::forward<T_Args>(p_args)...);
		m_numPropertySets++;
		m_optimizedForSearch = false;
		return m_propertySets[m_numPropertySets - 1];
	}
	
	// Adds an instance of property to internal array
	inline void addProperty(Property &&p_property)
	{
		m_properties.push_back(p_property);
		m_numProperties++;
		m_optimizedForSearch = false;
	}
	
	// Adds an instance of property set to internal array
	inline PropertySet &addPropertySet(PropertySet &&p_propertySet)
	{
		m_propertySets.push_back(p_propertySet);
		m_numPropertySets++;
		m_optimizedForSearch = false;
		return m_propertySets[m_numPropertySets - 1];
	}

	// Optimizes for faster search (getting properties by ID), by sorting the internal arrays.
	// Should only be called after all the property elements have been added (for performance reasons).
	// Required to be able to use the getPropertyByIDFast and getPropertySetByIDFast methods.
	inline void optimizeForSearch()
	{
		for(decltype(m_numPropertySets) i = 0; i < m_numPropertySets; i++)
			m_propertySets[i].optimizeForSearch();

		// Sort all the properties
		std::sort(m_properties.begin(), m_properties.end());

		// Sort all the property sets
		std::sort(m_propertySets.begin(), m_propertySets.end());

		m_optimizedForSearch = true;
	}

	// Finds a property with a matching ID and returns it. Uses faster search
	// of the property set is optimized for search (by calling optimizeForSearch())
	const inline Property &getPropertyByID(Properties::PropertyID p_propertyID) const
	{
		if(m_optimizedForSearch)
		{
			// Find the property by comparing the property ID
			auto returnProperty = std::lower_bound(m_properties.begin(), m_properties.end(), p_propertyID);

			// If the property ID matches (property is found), return it; otherwise return a null property
			if(returnProperty != m_properties.end() && *returnProperty == p_propertyID)
				return *returnProperty;
		}
		else
		{
			// Iterate over all properties and match the propertyID
			for(decltype(m_numProperties) i = 0; i < m_numProperties; i++)
				if(m_properties[i] == p_propertyID)
					return m_properties[i];

		}

		// If this point is reached, no match was found, so return a null property
		return m_nullProperty;
	}
	
	// Finds a property set with a matching ID and returns it. Uses faster search
	// of the property set is optimized for search (by calling optimizeForSearch())
	const inline PropertySet &getPropertySetByID(Properties::PropertyID p_propertyID) const
	{
		if(m_optimizedForSearch)
		{
			// Find the property by comparing the property ID const PropertySet &returnPropertySet
			auto returnPropertySet = std::lower_bound(m_propertySets.begin(), m_propertySets.end(), p_propertyID);

			// If the property ID matches (property is found), return it; otherwise return a null property
			if(returnPropertySet != m_propertySets.end() && *returnPropertySet == p_propertyID)
				return *returnPropertySet;
		}
		else
		{
			// Iterate over all properties sets and match the propertyID
			for(decltype(m_numPropertySets) i = 0; i < m_numPropertySets; i++)
				if(m_propertySets[i] == p_propertyID)
					return m_propertySets[i];
		}

		// If this point is reached, no match was found, so return a null property set
		return m_nullPropertySet;
	}
	
	// Returns a property by index; checks for out of bounds index and returns null property in that case
	const inline Property &getProperty(size_t p_index) const
	{
		// Check if index is in bounds
		if(p_index >= 0 && p_index < m_numProperties)
			return m_properties[p_index];
		else
			return m_nullProperty;
	}

	// Returns a property set by index; checks for out of bounds index and returns null property set in that case
	const inline PropertySet &getPropertySet(size_t p_index) const
	{
		// Check if index is in bounds
		if(p_index >= 0 && p_index < m_numPropertySets)
			return m_propertySets[p_index];
		else
			return m_nullPropertySet;
	}

	// Returns a property by index without checking if index is valid.
	// Unsafe - can go out of bounds, use for performance critical code only
	const inline Property &getPropertyUnsafe(size_t p_index) const { return m_properties[p_index]; }

	// Returns a property set by index without checking if index is valid.
	// Unsafe - can go out of bounds, use for performance critical code only
	const inline PropertySet &getPropertySetUnsafe(size_t p_index) const { return m_propertySets[p_index]; }

	// Array subscription operator; retrieves an individual property.
	// Unsafe - can go out of bounds, use for performance critical code only
	const inline Property &operator[] (size_t p_index) const { return m_properties[p_index]; }

	// Equal and less than operators accepting property ID enum
	const inline bool operator==(const Properties::PropertyID &p_propertyID) const { return (m_propertyID == p_propertyID); }
	const inline bool operator<(const Properties::PropertyID &p_propertyID) const { return (m_propertyID < p_propertyID); }

	// Equal and less than operators accepting property set class
	const inline bool operator==(const PropertySet &p_propertySet) const { return (m_propertyID == p_propertySet.m_propertyID); }
	const inline bool operator<(const PropertySet &p_propertySet) const { return (m_propertyID < p_propertySet.m_propertyID); }

	// Setters
	const inline void setPropertyID(Properties::PropertyID p_propertyID) { m_propertyID = p_propertyID; }

	// Getters
	inline size_t getNumProperties() const { return m_numProperties; }
	inline size_t getNumPropertySets() const { return m_numPropertySets; }
	const inline Properties::PropertyID getPropertyID() const { return m_propertyID; }

	// Bool operator; returns true if the property is valid and false if the property is null
	inline explicit operator bool() const { return m_propertyID != Properties::Null; }

	// Converts the property set to text (used for saving properties to text files)
	const inline std::string toString()
	{

	}

private:
	struct PropertyContainer
	{
		PropertyContainer() : m_type(PropertyContainerType::Type_Null) { }
		PropertyContainer(Property *p_property) : m_type(PropertyContainerType::Type_SingleProperty) 
		{ 
			m_propertySet.m_property = p_property; 
		}
		PropertyContainer(std::vector<Property> *p_propertyArray) : m_type(PropertyContainerType::Type_PropertyArray) 
		{ 
			m_propertySet.m_propertyArray = p_propertyArray; 
		}
		~PropertyContainer()
		{
			switch(m_type)
			{
			case PropertySet::PropertyContainer::Type_SingleProperty:
				delete m_propertySet.m_property;
				break;
			case PropertySet::PropertyContainer::Type_PropertyArray:
				delete m_propertySet.m_propertyArray;
				break;
			}
		}

		enum PropertyContainerType
		{
			Type_Null,
			Type_SingleProperty,
			Type_PropertyArray
		} m_type;

		union PropertyUnion
		{
			Property *m_property;
			std::vector<Property> *m_propertyArray;
		} m_propertySet;
	};
	
	bool m_optimizedForSearch;

	Properties::PropertyID m_propertyID;

	std::vector<Property> m_properties;
	std::vector<PropertySet> m_propertySets;

	size_t m_numProperties;
	size_t m_numPropertySets;

	static Property m_nullProperty;
	static PropertySet m_nullPropertySet;
};
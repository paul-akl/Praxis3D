#pragma once

#include "PropertySet.h"

class PropertyLoader
{
public:
	PropertyLoader(std::string p_filename = "") : m_filename(p_filename) {}
	~PropertyLoader() { }

	ErrorCode loadFromFile(std::string p_filename = "");
	ErrorCode saveToFile(PropertySet &p_propertySet, std::string p_filename = "");

	const std::string &getFilename() const { return m_filename; }
	const PropertySet &getPropertySet() const { return m_propertySets; }

private:
	// Gets the next piece of string contained between quotation marks
	std::string getNextField(size_t &p_charIndex, const std::string &p_string);

	// Loads the string into a property set
	void loadPropertySet(PropertySet &p_propertySet, size_t &p_charIndex, const std::string &p_string);

	// Loads the string into a property
	void loadProperty(PropertySet &p_propertySet, const Properties::PropertyID p_propID, const std::string &p_string);

	// Converts a property to string (for saving to a text file)
	std::string toString(const Property &p_property, const std::string &p_prefix);
	// Converts a property set to string (for saving to a text file)
	std::string toString(const PropertySet &p_propertySet, const std::string &p_prefix);

	std::string m_filename;
	PropertySet m_propertySets;
};
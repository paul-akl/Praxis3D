
#include <fstream>
//#include <sstream>

#include "ErrorHandlerLocator.h"
#include "PropertyLoader.h"

ErrorCode PropertyLoader::loadFromFile(std::string p_filename)
{
	// If passed string is not empty, assign it as a filename
	if(!p_filename.empty())
		m_filename = p_filename;
	else
		// If both passed string and filename string are empty, return an error
		if(m_filename.empty())
			return ErrorCode::Filename_empty;

	std::ifstream file;
	std::string singleLine, parsedString, processedString;

	// Open the file stream
	file.open(m_filename, std::ios::in);

	// If file is not valid, close it and return an error
	if(file.fail())
	{
		file.close();
		return ErrorCode::Ifstream_failed;
	}

	// Iterate over each line, until reaching end-of-line
	while(!file.eof())
	{
		// Get single line
		std::getline(file, singleLine);

		// If it contains commentary marks, discard the whole line
		if(singleLine.size() >= 2 && singleLine[0] == '/' && singleLine[1] == '/')
			continue;

		// Add each line to the whole file string
		parsedString += singleLine;
	}

	// String is parsed, close the file
	file.close();

	// Reserve space in the processed string, so it doesn't have to reallocate memory internally while filling it
	processedString.reserve(parsedString.size());

	// Iterate over each character in the parsed string and process it by removing
	// all spaces and horizontal tabs, except from in between quotation marks (values)
	for(decltype(parsedString.size()) i = 0, size = parsedString.size(); i < size; i++)
	{
		if(parsedString[i] == '"')					// If character is a quotation mark
		{
			processedString += parsedString[i];		// put the first quotation mark in the final string
			for(i++; parsedString[i] != '"'; i++)	// iterate to find the second quotation mark
			{
				// If end of string is reached without second quotation mark, log an error and exit loop
				if(i >= size)
				{	
					ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Missing quotation mark");
					break;
				}

				// Put every char (that's in between quotation marks) in the final string
				processedString += parsedString[i];	
			}
		}

		if(parsedString[i] != ' ' && parsedString[i] != '\t')	// If the char is not a space or tab
			processedString += parsedString[i];					// Copy it to the final string
	}

	// Character index, holds the position of the part of string that is being processed
	decltype(processedString.size()) charIndex = 0;

	// Find the first open curled bracket
	for(; processedString[charIndex] != '{'; charIndex++);

	do
	{
		loadPropertySet(m_propertySets, charIndex, processedString);
	} while(processedString[charIndex] == ',');

	// Optimize property sets for fast search, since they won't be modified anymore
	m_propertySets.optimizeForSearch();

	return ErrorCode::Success;
}

ErrorCode PropertyLoader::saveToFile(PropertySet &p_propertySet, std::string p_filename)
{
	// If passed string is not empty, assign it as a filename
	if(!p_filename.empty())
		m_filename = p_filename;
	else
		// If both passed string and filename string are empty, return an error
		if(m_filename.empty())
			return ErrorCode::Property_no_filename;

	std::ofstream file;
	//std::string singleLine, parsedString, processedString;

	// Open the file stream
	file.open(m_filename, std::ios::out);

	// If file is not valid, close it and return an error
	if(file.fail())
	{
		file.close();
		return ErrorCode::Ifstream_failed;
	}

	// Export property sets to the file
	file << toString(p_propertySet, std::string());

	// Close the file after we are done writing to it
	file.close();

	return ErrorCode::Success;
}

std::string PropertyLoader::getNextField(size_t &p_charIndex, const std::string &p_string)
{
	// String of the field to be returned
	std::string returnName;

	// Get the size of the string once
	auto size = p_string.size();

	// Search for the 1st quotation marks
	for(; p_charIndex < size && p_string[p_charIndex] != '"'; p_charIndex++);

	// Check if it's the end of the string, to not cause out of bounds error
	if(p_charIndex < size)
		p_charIndex++;	// Increment, so the search for 2nd quotation marks won't find the 1st quotation marks instead

	// Search for the 2nd quotation marks
	for(; p_charIndex < size && p_string[p_charIndex] != '"'; p_charIndex++)
		returnName += p_string[p_charIndex];

	// Check if it's the end of the string, to not cause out of bounds error
	if(p_charIndex < size)
		p_charIndex++;	// Increment, so index is not left on the 2nd quotation marks
	else
		p_charIndex--;	// Decrement, so the last character ( "}" ) can be found, by the caller

	// Return the field
	return returnName;
}

void PropertyLoader::loadPropertySet(PropertySet &p_propertySet, size_t &p_charIndex, const std::string &p_string)
{
	// Get the property ID of the next field (name equivalent)
	Properties::PropertyID propID = Properties::toPropertyID(getNextField(p_charIndex, p_string));

	// Determine the type of entry (depending on the symbol that's following ":")
	if(p_string[p_charIndex] == ':')
	{
		p_charIndex++;

		switch(p_string[p_charIndex])
		{
		// Quotation marks means it only has one entry, with a value in it, which
		// means it is a property. Get the value string and pass it to loadProperty
		case '"':
		{
			loadProperty(p_propertySet, propID, getNextField(p_charIndex, p_string));
			break;
		}

		// Curly brackets means there are more than one entry to follow (parent node)
		// They are contained within property set
		case '{':
		{
			auto &propertySet = p_propertySet.addPropertySet(propID);

			do // Repeat for every element in the current entry
			{
				p_charIndex++;

				// Check if the curly bracket scope is not empty
				if(p_string[p_charIndex] != '}')
					loadPropertySet(propertySet, p_charIndex, p_string);

			} while(p_string[p_charIndex] == ',');

			p_charIndex++;
			break;
		}

		// Array brackets means there are array-like entries to follow. Property sets
		// acts as array entries, with each entry having the propertyID of the parent node
		case '[':
		{
			// Add a property set to contain all arrays entry
			auto &propertySet = p_propertySet.addPropertySet(propID);
			p_charIndex++;

			// Check if the array bracket scope is not empty
			if(p_string[p_charIndex] != ']')
			{
				do // Repeat for every array entry
				{
					// Add an array entry to contain all entry elements
					auto &arrayEntry = propertySet.addPropertySet(propID);

					do // Repeat for every element in the array entry
					{
						p_charIndex++;

						// Add individual entry elements to the array entry
						loadPropertySet(arrayEntry, p_charIndex, p_string);

					} while(p_string[p_charIndex] == ',');
					p_charIndex++;

				} while(p_string[p_charIndex] == ',');
			}
			
			// Next character should be closing array brackets, if so, increment char index to skip it
			if(p_string[p_charIndex] == ']')
			{
				p_charIndex++;
			}
			// If closing array brackets are missing, log a warning (must be syntax error)
			else
			{
				ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Missing \" ] \" sign");
			}

			break;
		}

			// If this point is reached, the correct sign is missing. Log a warning
		default:
			ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Unknown sign "
									 + Utilities::toString(p_string[p_charIndex])
									 + ". Should be \" \" \", \" { \" or \" [ \"");
			break;
		}
	}
	else
	{
		ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Missing \" : \" sign");
	}
}

void PropertyLoader::loadProperty(PropertySet &p_propertySet, const Properties::PropertyID p_propID, const std::string &p_string)
{
	// If string is empty, return a null property
	if(p_string.empty())
	{
		p_propertySet.addProperty(p_propID);
		return;
	}

	// _________________
	//|	  Bool value	|
	//|_________________|
	if(p_string == "false" || p_string == "FALSE")
	{
		p_propertySet.addProperty(p_propID, false);
		return;
	}
	if(p_string == "true" || p_string == "TRUE")
	{
		p_propertySet.addProperty(p_propID, true);
		return;
	}

	unsigned int commas = 0;
	unsigned int decimalPoints = 0;
	bool letterPresent = false;

	// Iterate over the string character by character and count commas and decimal points
	for(decltype(p_string.size()) i = 0, size = p_string.size(); i < size; i++)
	{
		// _________________
		//|	 String value	|
		//|_________________|
		// If current character is alphabetic but not a space or a minus sign;
		// check for certain characters that are allowed in number representations and do not set the property type to string because of them
		if(!isdigit(p_string.c_str()[i]) && p_string[i] != ' ' && p_string[i] != '-')
		{
			// Allow dots between digits
			if(!(p_string[i] == '.' && i + 1 < size && isdigit(p_string.c_str()[i + 1]) && i - 1 >= 0 && isdigit(p_string.c_str()[i - 1])))
			{
				// Allow commas that separate numbers in vector variables (can be between digits, letter "f" of a float or a space)
				if(!(p_string[i] == ',' && i - 1 >= 0 && (isdigit(p_string.c_str()[i - 1]) || p_string[i - 1] == 'f') && i + 1 < size && (isdigit(p_string.c_str()[i + 1]) || p_string[i + 1] == ' ')))
				{
					// Allow letter "f" at the end of a float (can be between a digit and a comma or a space)
					//if(!(p_string[i] == 'f' && i == size - 1 && i + 1 < size && (p_string[i + 1] == ',' || p_string[i + 1] == ' ')))
					if(!(p_string[i] == 'f' && i - 1 >= 0 && isdigit(p_string.c_str()[i - 1])))
					{
						// Allow letter "e" for exponential notation in a float (must be followed by a minus or a plus sign)
						if(!(p_string[i] == 'e' && (i + 1 < size && (p_string[i + 1] == '-' || p_string[i + 1] == '+'))))
						{
							// If this point is reached, property is of a string type

							// Convert string to property ID
							Properties::PropertyID propertyID = Properties::toPropertyID(p_string);

							// If conversion was successful, assign property with a propertyID value
							// otherwise assign property with a string value
							if(propertyID != Properties::Null)
								p_propertySet.addProperty(p_propID, propertyID);
							else
								p_propertySet.addProperty(p_propID, p_string);

							return;
						}
					}
				}
			}

			letterPresent = true;
		}

		// Count decimal points and commas
		if(p_string[i] == '.')
			decimalPoints++;
		if(p_string[i] == ',')
			commas++;
	}

	// _________________
	//|	 Integer value	|
	//|_________________|
	if(!letterPresent && decimalPoints == 0)
	{
		p_propertySet.addProperty(p_propID, std::stoi(p_string));
		return;
	}

	// _________________
	//|	  Vec4 value	|
	//|_________________|
	if(decimalPoints <= 4 && commas == 3)
	{
		std::string xString, yString, zString, wString;

		decltype(p_string.size()) index = 0;
		decltype(p_string.size()) size = p_string.size();

		for(; p_string[index] != ','; index++)
			xString += p_string[index];	index++;
		for(; p_string[index] != ','; index++)
			yString += p_string[index];	index++;
		for(; p_string[index] != ','; index++)
			zString += p_string[index];	index++;
		for(; index < size; index++)
			wString += p_string[index];

		p_propertySet.addProperty(p_propID, glm::vec4(std::stof(xString), std::stof(yString), 
														std::stof(zString), std::stof(wString)) );
		return;
	}

	// _________________
	//|	  Vec3 value	|
	//|_________________|
	if(decimalPoints <= 3 && commas == 2)
	{
		std::string xString, yString, zString;

		decltype(p_string.size()) index = 0;
		decltype(p_string.size()) size = p_string.size();

		for(; p_string[index] != ','; index++)
			xString += p_string[index];	index++;
		for(; p_string[index] != ','; index++)
			yString += p_string[index];	index++;
		for(; index < size; index++)
			zString += p_string[index];

		p_propertySet.addProperty(p_propID, glm::vec3(std::stof(xString), std::stof(yString), std::stof(zString)));
		return;
	}

	// _________________
	//|	  Vec2 value	|
	//|_________________|
	if(decimalPoints <= 2 && commas == 1)
	{
		std::string xString, yString;

		decltype(p_string.size()) index = 0;
		decltype(p_string.size()) size = p_string.size();

		for(; p_string[index] != ','; index++)
			xString += p_string[index];	index++;
		for(; index < size; index++)
			yString += p_string[index];

		p_propertySet.addProperty(p_propID, glm::vec2(std::stof(xString), std::stof(yString)));
		return;
	}

	if(decimalPoints <= 1 && commas == 0)
	{
		// _________________
		//|	  Float value	|
		//|_________________|
		if(p_string[p_string.size() - 1] == 'f')
		{
			p_propertySet.addProperty(p_propID, std::stof(p_string));
			return;
		}
		// _________________
		//|	  Double value	|
		//|_________________|
		else
		{
			p_propertySet.addProperty(p_propID, std::stod(p_string));
			return;
		}
	}

	// This point should not be reached, but if it is, return a null property
	p_propertySet.addProperty(p_propID);
	return;
}

inline std::string PropertyLoader::toString(const Property &p_property, const std::string &p_prefix)
{
	return p_prefix + "\"" + Utilities::toString(GetString(p_property.getPropertyID())) + "\": \"" + p_property.getString() + "\"";
}

std::string PropertyLoader::toString(const PropertySet &p_propertySet, const std::string &p_prefix)
{
	std::string returnString;
	std::string prefix = p_prefix;

	// Add property ID only if it's valid (i.e. not Default or Array Entry)
	if(p_propertySet.getPropertyID() != Properties::ArrayEntry &&
		p_propertySet.getPropertyID() != Properties::Default)
	{
		returnString += prefix + "\"" + std::string(GetString(p_propertySet.getPropertyID())) + "\": \n";
	}

	if(p_propertySet.getNumProperties() == 0 && p_propertySet.getNumPropertySets() > 0 &&
	   p_propertySet.getPropertySet(0).getPropertyID() == Properties::ArrayEntry)
	{
		returnString += prefix + "[\n";
		prefix += "\t";

		// Iterate over every Property Set and convert them to text
		for(decltype(p_propertySet.getNumPropertySets()) i = 0, size = p_propertySet.getNumPropertySets(); i < size; i++)
			if(p_propertySet.getPropertySetUnsafe(i).getPropertyID() != Properties::Null)
				returnString += toString(p_propertySet.getPropertySetUnsafe(i), prefix) + ",\n";

		returnString.pop_back();
		returnString.pop_back();
		returnString += "\n" + p_prefix + "]";
	}
	else
	{
		// Add opening bracket, and increment the prefix
		returnString += p_prefix + "{\n";
		prefix += "\t";

		// Iterate over every Property and convert them to text
		for(decltype(p_propertySet.getNumProperties()) i = 0, size = p_propertySet.getNumProperties(); i < size; i++)
			if(p_propertySet[i].getPropertyID() != Properties::Null)
				returnString += toString(p_propertySet[i], prefix) + ",\n";

		// If there are any Property Sets, process them
		if(p_propertySet.getNumPropertySets() > 0)
		{
			// Declare a new prefix for Property Sets
			std::string newPrefix = prefix;

			// Iterate over every Property Set and convert them to text
			for(decltype(p_propertySet.getNumPropertySets()) i = 0, size = p_propertySet.getNumPropertySets(); i < size; i++)
				if(p_propertySet.getPropertySetUnsafe(i).getPropertyID() != Properties::Null)
					returnString += toString(p_propertySet.getPropertySetUnsafe(i), prefix) + ",\n";

			// Remove the comma form the last Property Set entry
			returnString.pop_back();
			returnString.pop_back();
			returnString += "\n";

		}
		else
		{
			// If there are no Property Sets, but there are Properties, delete the last comma
			if(p_propertySet.getNumProperties() > 0)
			{
				returnString.pop_back();
				returnString.pop_back();
				returnString += "\n";
			}
		}

		// Add closing bracket
		returnString += p_prefix + "}";
	}

	

	/*/ If there are any Property Sets, process them
	if(p_propertySet.getNumPropertySets() > 0)
	{
		// Declare a new prefix for Property Sets
		std::string newPrefix = prefix;
		bool arrayEntry = false;

		// If the first Property Set is an array entry, assume the root property set is an array
		if(p_propertySet.getPropertySet(0).getPropertyID() == Properties::ArrayEntry)
		{
			returnString += prefix + "[\n";
			newPrefix += "\t";
			arrayEntry = true;
		}

		// Iterate over every Property Set and convert them to text
		for(decltype(p_propertySet.getNumPropertySets()) i = 0, size = p_propertySet.getNumPropertySets(); i < size; i++)
			if(p_propertySet.getPropertySetUnsafe(i).getPropertyID() != Properties::Null)
				returnString += toString(p_propertySet.getPropertySetUnsafe(i), newPrefix) + ",\n";

		// Remove the comma form the last Property Set entry
		returnString.pop_back();
		returnString.pop_back();
		returnString += "\n";

		// If this was an array, add closing array bracket
		if(arrayEntry)
			returnString += prefix + "]\n";
	}
	else
	{
		// If there are no Property Sets, but there are Properties, delete the last comma
		if(p_propertySet.getNumProperties() > 0)
		{
			returnString.pop_back();
			returnString.pop_back();
			returnString += "\n";
		}
	}

	// Add closing bracket
	returnString += p_prefix + "}";*/

	return returnString;
}

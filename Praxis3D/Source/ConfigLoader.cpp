#include "ConfigLoader.h"
#include "ErrorHandlerLocator.h"

ConfigFile::NullValue ConfigFile::NodeBase::m_nullValue;
std::vector<ConfigFile::NodeBase*> ConfigFile::NodeBase::m_nullArray;
std::vector<std::vector<ConfigFile::NodeBase*>*> ConfigFile::NodeBase::m_nullArrayOfArray;
ConfigFile::NodeBase NodeIterator::m_nullNode("", 0);

ConfigFile::ConfigFile()
{

}
ConfigFile::~ConfigFile()
{

}

ErrorCode ConfigFile::import(std::string p_filename, std::unordered_map<std::string, int> &p_hashMap)
{
	//m_configString = p_filename;
	ErrorCode returnCode = ErrorCode::Success;

	// Load the config from a text file
	returnCode = loadFromFile(p_filename);

	// Check for errors
	if(returnCode != ErrorCode::Success)
		return returnCode;
	else
	{
		// Propagate the node structure
		returnCode = generateNodes(p_hashMap);

		// Check for errors
		if(returnCode != ErrorCode::Success)
			return returnCode;
	}

	return returnCode;
}
NodeIterator ConfigFile::getRootNode()
{
	if(m_rootNode)
		return NodeIterator(m_rootNode->getChildren());

	return NodeIterator();
}

ErrorCode ConfigFile::loadFromFile(std::string p_filename)
{
	ErrorCode returnError = ErrorCode::Success;

	std::ifstream configFile;
	std::string singleLine, tempConfigString;

	configFile.open(p_filename, std::ios::in);

	if(configFile.fail())
	{
		configFile.close();
		//throw Message::messageCode(MSG_FATAL_ERROR, MSG_OBJECT, configFileName + ": Has failed to load.");
		returnError = ErrorCode::Ifstream_failed;
	}
	else
	{
		while(!configFile.eof())
		{
			// Get every line, to be processed
			std::getline(configFile, singleLine);

			// If it contains commentary marks, discard the whole line
			if(singleLine.size() >= 2 && singleLine[0] == '/' && singleLine[1] == '/')
				continue;

			// Add each line to the whole file string
			tempConfigString += singleLine;
		}

		// Remove all spaces and horizontal tabs, except from in between quotation marks
		for(std::vector<std::string>::size_type i = 0; i < tempConfigString.size(); i++)
		{
			if(tempConfigString[i] == '"')					// If it's a quotation mark
			{
				m_configString += tempConfigString[i];		// put the first quotation mark in the final string
				for(i++; tempConfigString[i] != '"'; i++)	// iterate to find the second quotation mark
					m_configString += tempConfigString[i];	// put every char (that is in between quotation marks) in the final string
			}

			if(tempConfigString[i] != ' ' && tempConfigString[i] != '\t')	// If the char is not space or tab
				m_configString += tempConfigString[i];						// Copy it to the final string
		}
		configFile.close();
	}

	return returnError;
}
ErrorCode ConfigFile::generateNodes(std::unordered_map<std::string, int> &p_hashMap)
{
	ErrorCode returnError = ErrorCode::Success;

	std::vector<std::string>::size_type index = 0;
	m_rootNode = new ConfigFile::ParentNode("root", p_hashMap["root"]);

	// Find the first open curled bracket
	for(; m_configString[index] != '{'; index++);

	do
	{
		m_rootNode->addChild(getNextNode(index, p_hashMap));
	} while(m_configString[index] == ',');

	return returnError;
}

std::string ConfigFile::getNextField(std::vector<std::string>::size_type &p_index)
{
	std::string returnName;

	// Search for the first quotation marks
	for(; p_index < m_configString.size() && m_configString[p_index] != '"'; p_index++);

	// Check if it's end of the string, to not couse out of bounds error
	if(p_index < m_configString.size())
		p_index++;	// Increment, so search for second quotation marks won't find first quotation marks

	// Search for the second quotation marks
	for(; p_index < m_configString.size() && m_configString[p_index] != '"'; p_index++)
		returnName += m_configString[p_index];

	// Check if it's the end of the string, to not couse out of bounds error
	if(p_index < m_configString.size())
		p_index++;	// Increment, so index is not on second quotation marks
	else
		p_index--;	// Decrement, so end character ( "}" ) can be found, by main while loop

	return returnName;
}

ConfigFile::NodeBase *ConfigFile::getNextNode(std::vector<std::string>::size_type &p_index, std::unordered_map<std::string, int> &p_hashMap)
{
	NodeBase *returnNode = nullptr;

	// Get the name of the node
	std::string nodeName = getNextField(p_index);

	// Determine what kind of node to parse, depending on the symbol that's following ":"
	if(m_configString[p_index] == ':')
	{
		p_index++;
		switch(m_configString[p_index])
		{
		// Quotation marks means it only has one entry, with a value in it - hence value node
		// Just parse the value and convert it to one of the baseValues
		case '"':
			returnNode = new ValueNode(nodeName, p_hashMap[nodeName]);
			returnNode->setValue(getValueNode(getNextField(p_index)));
			break;

		// Curly brackets means there are more than one entry to follow, which means it's a parent node
		// Parse every child recursively
		case '{':
			returnNode = new ParentNode(nodeName, p_hashMap[nodeName]);

			do
			{
				p_index++;

				// Check if the curly bracket scope is not empty
				if(m_configString[p_index] != '}')
					returnNode->addChild(getNextNode(p_index, p_hashMap));

			} while(m_configString[p_index] == ',');
			p_index++;
			break;

		// Array brackets means there are array-like entries to follow
		// Go over each entry and add it as a child, recursively
		case '[':
			returnNode = new ArrayNode(nodeName, p_hashMap[nodeName]);
			p_index++;

			// Check if the array bracket scope is not empty
			if(m_configString[p_index] != ']')
			{
				do
				{
					std::vector<NodeBase*> *arrayNodes = new std::vector<NodeBase*>;

					do
					{
						p_index++;
						arrayNodes->push_back(getNextNode(p_index, p_hashMap));
					} while(m_configString[p_index] == ',');
					p_index++;

					returnNode->addArrayEntry(arrayNodes);

				} while(m_configString[p_index] == ',');
			}
			if(m_configString[p_index] != ']')
			{
				ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Missing \" ] \" sign");
			}
			else
				p_index++;
			break;

			returnNode = new NodeBase(nodeName, p_hashMap[nodeName]);
			ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Unknown sign " + Utilities::toString(m_configString[p_index]) + ". Should be \" \" \", \" { \" or \" [ \"");
		}
	}
	// If the colon symbol was missing, that means there's a syntax mistake in the script
	// Return a "null" node instead, with an error, so it's save to load a script even with mistakes
	else
	{
		returnNode = new NodeBase(nodeName, p_hashMap[nodeName]);
		ErrHandlerLoc::get().log(Warning, Source_FileLoader, "Missing \" : \" sign");
	}

	return returnNode;
}

ConfigFile::BaseValue *ConfigFile::getValueNode(std::string p_value)
{
	if(p_value == "true" || p_value == "false")	// If value is bool
		return new BoolValue(p_value);

	int decimalPoints = 0, commas = 0;
	for(std::vector<std::string>::size_type i = 0; i < p_value.size(); i++)
	{
		if(isalpha(p_value.c_str()[i]) && (i < p_value.size() || p_value[i] != 'f'))	// If value is string
			return new StringValue(p_value);

		if(p_value[i] == '.')
			decimalPoints++;
		if(p_value[i] == ',')
			commas++;
	}

	if(decimalPoints == 0)	// If value is integer
	{
		return new IntValue(p_value);
	}
	else
	{
		if(decimalPoints == 4 && commas == 3)	// If value is vector of 4 floats
		{
			return new Vec4fValue(p_value);
		}
		else
		{
			if(decimalPoints == 3 && commas == 2)	// If value is vector of 3 floats
			{
				return new Vec3fValue(p_value);
			}
			else
			{
				if(decimalPoints == 2 && commas == 1)	// If value is vector of 2 floats
				{
					return new Vec2fValue(p_value);
				}
				else
				{
					if(decimalPoints == 1 && commas == 0)
					{
						if(p_value[p_value.size() - 1] == 'f')	// If value is float
							return new FloatValue(p_value);
						else									// If value is double (not implemented yet)
							return new DoubleValue(p_value);
					}
					else
						return new BoolValue("false");	// If value is incorrect or cannot be read, just create a bool and set it to false
				}
			}
		}
	}
}
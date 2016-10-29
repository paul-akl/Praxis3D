#pragma once

#include <sstream>
#include <string>

#include "Scancodes.h"

namespace Utilities
{
	// Static functions to convert simple types to string

	static std::string toString(const int p_value)
	{
		std::stringstream stringstream_ret;
		stringstream_ret << p_value;
		return stringstream_ret.str();
	}
	static std::string toString(const unsigned int p_value)
	{
		std::stringstream stringstream_ret;
		stringstream_ret << p_value;
		return stringstream_ret.str();
	}
	static std::string toString(const double p_value)
	{
		std::stringstream stringstream_ret;
		stringstream_ret << p_value;
		return stringstream_ret.str();
	}
	static std::string toString(const float p_value)
	{
		std::stringstream stringstream_ret;
		stringstream_ret << p_value;
		return stringstream_ret.str();
	}
	static std::string toString(const bool p_value)
	{
		return (p_value) ? "true" : "false";
	}
	static std::string toString(const char p_value)
	{
		std::string stringRet;
		stringRet += p_value;
		return stringRet;
	}
	static std::string toString(const char *p_value)
	{
		return std::string(p_value);
	}

	// Returns a float as being declared in C++ (with period and an "f" at the end)
	static std::string toStringCpp(const float p_value)
	{
		std::stringstream stringstream_ret;
		stringstream_ret << p_value;
		//std::string floatString  = stringstream_ret.str();

		if(rintf(p_value) == p_value)
			return stringstream_ret.str() + ".0f";
		else
			return stringstream_ret.str() + "f";

		/*for(decltype(floatString.size()) i = 0, size = floatString.size(); i < size; i++)
			if(floatString[i] == '.')
			{
				floatString += "f";
				return floatString;
			}

		floatString += ".0f";
		return floatString;*/
	}
	

	// Static functions to convert a few simple types to Scancodes.
	// Safe - checks if value in bounds. No conversion from string should be provided

	static Scancode toScancode(const int p_value)
	{
		// If the passed value is within enum range, static cast it to a scancode, if not, return invalid scancode
		if(p_value > Scancode::Key_Invalid && p_value < Scancode::NumberOfScancodes)
			return static_cast<Scancode>(p_value);
		else
			return Scancode::Key_Invalid;
	}
	static Scancode toScancode(const float p_value)
	{
		// If the passed value is within enum range, static cast it to a scancode, if not, return invalid scancode
		if(p_value > Scancode::Key_Invalid && p_value < Scancode::NumberOfScancodes)
			return static_cast<Scancode>((int)p_value);
		else
			return Scancode::Key_Invalid;
	}


	// A very simplistic but fast hash key function, converts string into an unsigned int
	static unsigned int getHashKey(const std::string &p_string)
	{
		unsigned int hash = 5381;
		for(decltype(p_string.size()) i = 0, size = p_string.size(); i < size; i++)
		{
			hash = ((hash << 5) + hash) + p_string[i];
		}
		return hash;
	}
}
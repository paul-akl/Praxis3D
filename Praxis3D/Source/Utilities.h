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
	}

	// Replaces a word with a different word in a given string
	static std::string replace(const std::string &p_fullText, const std::string &p_wordToReplace, const std::string &p_wordReplaceWith)
	{
		std::string returnString = p_fullText;
		
		// Search for the given character set and do a replace
		while(returnString.find(p_wordToReplace) != std::string::npos)
		{
			returnString.replace(returnString.find(p_wordToReplace), p_wordToReplace.size(), p_wordReplaceWith.c_str());
			break;
		}

		return returnString;
	}	
	
	// Replaces all matched words with a different word in a given string; given word cannot contain the word to be replaced
	static std::string replaceAll(const std::string &p_fullText, const std::string &p_wordToReplace, const std::string &p_wordReplaceWith)
	{
		std::string returnString = p_fullText;

		// Check if the replacement word does not contain the character set to be replaced, so it does not go into an infinite loop
		while(p_wordReplaceWith.find(p_wordToReplace) != std::string::npos)
		{
			return returnString;
		}

		// Search for the given character set and do a replace
		while(returnString.find(p_wordToReplace) != std::string::npos)
		{
			returnString.replace(returnString.find(p_wordToReplace), p_wordToReplace.size(), p_wordReplaceWith.c_str());
		}

		return returnString;
	}

	// Strip and return the filename from a full file path in Windows environment
	static std::string stripFilename(const std::string &p_fullPath)
	{
		std::string returnFilename = p_fullPath;

		// Make sure full path string is not empty
		if(!p_fullPath.empty())
		{
			// Separate the directory from the scene name
			for(decltype(p_fullPath.size()) i = p_fullPath.size() - 1; i > 0; i--)
			{
				// Find the start (or end of directory) of the filename string
				if(p_fullPath[i] == '\\' || p_fullPath[i] == '/')
				{
					// Cut the filename only and return it
					returnFilename = p_fullPath.substr(i + 1, p_fullPath.size());
					break;
				}
			}
		}

		return returnFilename;
	}

	// Strip and return the directory path from a full file path in Windows environment
	static std::string stripFilePath(const std::string &p_fullPath)
	{
		std::string returnFilename = p_fullPath;

		// Make sure full path string is not empty
		if(!p_fullPath.empty())
		{
			// Separate the directory from the scene name
			for(decltype(p_fullPath.size()) i = p_fullPath.size() - 1; i > 0; i--)
			{
				// Find the start (or end of directory) of the filename string
				if(p_fullPath[i] == '\\' || p_fullPath[i] == '/')
				{
					// Cut the path only and return it
					returnFilename = p_fullPath.substr(0, i + 1);
					break;
				}
			}
		}

		return returnFilename;
	}

	// Removes the extension of a filename (removes everything after the last "." (dot) in the string, if it is present)
	static std::string removeExtension(const std::string &p_filename)
	{
		std::string returnFilename = p_filename;
		
		// Make sure full path string is not empty
		if(!p_filename.empty())
		{
			// Go over each character from end to start
			for(decltype(p_filename.size()) i = p_filename.size() - 1; i > 0; i--)
			{
				// If it is a dot, meaning the last dot character in the string
				if(p_filename[i] == '.')
				{
					// Remove everything after the dot (including the dot itself); break the "for" loop
					returnFilename = p_filename.substr(0, i);
					break;
				}
			}
		}

		return returnFilename;
	}

	// Converts all slashes present in the string to backslashes ( '/' -> '\' )
	static std::string slashToBackslash(const std::string& p_textWithSlashes)
	{
		std::string returnText;
			
		// Make sure full path string is not empty
		if(!p_textWithSlashes.empty())
		{
			returnText = p_textWithSlashes;

			// Separate the directory from the scene name
			for(decltype(returnText.size()) i = 0, size = returnText.size(); i < size; i++)
			{
				if(returnText[i] == '/')
					returnText[i] = '\\';
			}
		}
		return returnText;
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

	// Template std::pair comparator, only compares the first element
	template<class T1, class T2, class Pred = std::less<T2>>
	struct sort_pair_first 
	{
		bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) 
		{
			Pred p;
			return p(left.first, right.first);
		}
	};

	// Template std::pair comparator, only compares the second element
	template<class T1, class T2, class Pred = std::less<T2>>
	struct sort_pair_second 
	{
		bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) 
		{
			Pred p;
			return p(left.second, right.second);
		}
	};

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

	// Calculates value with a "1" bit at the given bit position (i.e. bit-shifting "1" by the given number of places)
	template<class T>
	T getBitmask(T p_bitShiftPosition)
	{
		return ((T)1 << p_bitShiftPosition);
	}

	// Calculates value with a "1" bit at the given bit position (i.e. bit-shifting "1" by the given number of places)
	template<class T>
	T getBitmask(int p_bitShiftPosition)
	{
		return ((T)1 << p_bitShiftPosition);
	}
}
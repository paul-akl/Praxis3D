#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Utilities.h"

// General file and file-system operations
class Filesystem
{
public:
	// Creates directory tree up to the (and including) the given directory. Required full path. Returns true if successful
	static bool createDirectories(const std::string &p_dirPathAndName)
	{
		return std::filesystem::create_directories(p_dirPathAndName);
	}
	
	// Copies a single file. Returns true if successful
	static bool copyFile(const std::string &p_sourceFile, const std::string &p_destinationFile)
	{
		if(exists(p_destinationFile))
			std::filesystem::remove(p_destinationFile);
		return std::filesystem::copy_file(p_sourceFile, p_destinationFile);
	}

	// Creates a destination directory tree and copies a single file
	static bool copyFileCreateDirectory(const std::string &p_sourceFile, const std::string &p_destinationFile)
	{
		// Create the directory while also stripping down the file path
		if(createDirectories(Utilities::stripFilePath(p_destinationFile)))
		{
			return copyFile(p_sourceFile, p_destinationFile);
		}

		return false;
	}

	// Write text (string) to file. Returns true if successful, false if not
	static bool writeToFile(const std::string &p_text, const std::string &p_filename)
	{
		// Open file stream
		std::ofstream exportFile(p_filename);

		// Check if file stream is open
		if(exportFile.is_open())
		{
			// Write text to file
			exportFile << p_text << std::endl;
			return true;
		}

		// If this point is reached, write operation failed
		return false;
	}

	// Checks if file or directory exists, returns true if it does
	static bool exists(const std::string &p_file)
	{
		return std::filesystem::exists(p_file);
	}

	// Returns the current working directory
	static std::string getCurrentDirectory()
	{
		std::filesystem::path path = std::filesystem::current_path();
		return path.string();
	}
};
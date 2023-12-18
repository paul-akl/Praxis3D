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

	// Read text (string) from file. Returns true if successful, false if not
	static bool readTextFromFile(const std::string &p_filename, std::string &p_text)
	{
		// Open file
		std::ifstream importFile(p_filename, std::ios::in);

		// Check if the file opened successfully
		if(importFile)
		{
			// Seek to the end of the imported file
			importFile.seekg(0, std::ios::end);

			// Resize the output string to accommodate the whole import file
			p_text.resize(importFile.tellg());

			// Seek to the beginning of the imported file
			importFile.seekg(0, std::ios::beg);

			// Set the imported files content to the output string
			importFile.read(&p_text[0], p_text.size());

			// Close the import file
			importFile.close();

			// Read operation was successful
			return true;
		}

		// If this point is reached, read operation failed
		return false;
	}

	// Write text (string) to file. Any contents that existed in the file before it is open are discarded
	// Returns true if successful, false if not
	static bool writeTextToFile(const std::string &p_filename, const std::string &p_text)
	{
		// Open file stream
		std::ofstream exportFile(p_filename, std::ofstream::trunc);

		// Check if file stream is open
		if(exportFile.is_open())
		{
			// Write text to file
			exportFile << p_text;

			// Close the export file
			exportFile.close();

			// Write operation was successful
			return true;
		}

		// If this point is reached, write operation failed
		return false;
	}	
	
	// Appends text (string) to file. Adds all text to the end of the file, appending to its existing contents
	// Returns true if successful, false if not
	static bool addTextToFile(const std::string &p_filename, const std::string &p_text)
	{
		// Open file stream
		std::ofstream exportFile(p_filename, std::ofstream::app);

		// Check if file stream is open
		if(exportFile.is_open())
		{
			// Write text to file
			exportFile << p_text;

			// Close the export file
			exportFile.close();

			// Write operation was successful
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
#pragma once

#include <iostream>
#include <string>

#include "Config.h"
#include "ErrorCodes.h"

class ErrorHandlerBase
{
public:
	ErrorHandlerBase() { }
	~ErrorHandlerBase() { }

	virtual ErrorCode init() = 0;

	// Get a pre-define text string of an error code
	virtual std::string getErrorString(ErrorCode p_error) = 0;
	// Get a pre-defined error type of an error code
	virtual ErrorType getErrorType(ErrorCode p_error) = 0;

	// Error logging functions, passes the error to the apropriate "console" class
	virtual void log(ErrorCode p_errorCode) = 0;
	virtual void log(ErrorCode p_errorCode, ErrorSource p_errorSource) = 0;
	virtual void log(ErrorType p_errorType, ErrorSource p_errorSource, std::string p_error) = 0;
	virtual void log(ErrorCode p_errorCode, ErrorSource p_errorSource, std::string p_error) = 0;
	virtual void log(const ErrorCode p_errorCode, const std::string &p_objectName, const ErrorSource p_errorSource) = 0;

	// This should be used when trying to pass an error code and an error string as a return. So instead of logging
	// the error immediately, it is cached and then can be retrieved later (presumably from higher scope / parent, etc).
	// Note: Unusable at the time - needs to be updated to be thread safe (using TLS for example)
	//virtual ErrorCode cacheError(ErrorCode p_errorCode, std::string p_error) = 0;

	// Assigns the p_errorCode to the p_returnCode; Returns true if the p_errorCode was successful
	const inline bool ifSuccessful(ErrorCode p_errorCode, ErrorCode &p_returnCode) const { p_returnCode = p_errorCode; return p_errorCode == ErrorCode::Success; }
};

class ErrorHandler : public ErrorHandlerBase
{
public:
	ErrorHandler();
	~ErrorHandler();

	ErrorCode init();
	inline std::string getErrorString(ErrorCode p_error) { return (p_error < ErrorCode::NumberOfErrorCodes) ? m_errorData[p_error].m_errorString : "Invalid error code."; }
	inline ErrorType getErrorType(ErrorCode p_error) { return (p_error < ErrorCode::NumberOfErrorCodes) ? m_errorData[p_error].m_errorType : ErrorType::Info; }

	void log(ErrorCode p_errorCode);
	void log(ErrorCode p_errorCode, ErrorSource p_errorSource);
	void log(ErrorType p_errorType, ErrorSource p_errorSource, std::string p_error);
	void log(ErrorCode p_errorCode, ErrorSource p_errorSource, std::string p_error);
	void log(const ErrorCode p_errorCode, const std::string &p_objectName, const ErrorSource p_errorSource);

	// Note: Unusable at the time - needs to be updated to be thread safe (using TLS for example)
	//inline ErrorCode cacheError(ErrorCode p_errorCode, std::string p_error) { m_cachedError.cache(p_errorCode, p_error); return ErrorCode::CachedError; }

private:
	struct ErrorData
	{
		ErrorData() : m_errorType(ErrorType::Info), m_errorName(""), m_errorString("") { }
		inline void setTypeAndString(ErrorType p_errorType, std::string p_errorString)
		{
			m_errorType = p_errorType;
			//m_errorSource = p_errorSource;
			m_errorString = p_errorString;
		}

		ErrorType	m_errorType;
		//ErrorSource m_errorSource;
		std::string m_errorName;
		std::string m_errorString;
	};
	struct ErrorCache
	{
		bool m_errorPresent;
		ErrorCode m_errorCode;
		std::string m_errorString;

		ErrorCache()
		{
			m_errorString = "";
			m_errorPresent = false;
			m_errorCode = ErrorCode::Undefined;
		}
		void cache(ErrorCode p_errorCode, std::string p_errorString)
		{
			m_errorPresent = true;
			m_errorCode = p_errorCode;
			m_errorString = p_errorString;
		}
		void clear() { m_errorPresent = false; }
		bool errorPresent() { return m_errorPresent; }
	};

	// "Consoles" are where all the error messages eventually go. They can be extended, and can process 
	// messages differently (for example it could output to cout/fprint, to game console, log to file, etc).
	class ConsoleBase
	{
	public:
		ConsoleBase() { }
		~ConsoleBase() { }

		virtual void displayMessage(std::string p_message) { }
	};

	class CoutConsole : public ConsoleBase
	{
	public:
		CoutConsole();
		~CoutConsole() 
		{ 
			restoreConsole();
		}

		void displayMessage(std::string p_message) { printf("%s\n", p_message.c_str()); }

	private:
		void setupConsole();
		void restoreConsole();

		void	*m_stdoutHandle,
				*m_stdinHandle;

		unsigned long	m_outModeInit,
						m_inModeInit;
	};
	
	ErrorData m_errorData[ErrorCode::NumberOfErrorCodes];

	std::string m_errorTypeStrings[NumberOfErrorTypes];
	std::string m_errorSources[Source_NumberOfErrorSources];

	std::unordered_map<std::string, int> m_errHashmap;

	ErrorCache m_cachedError;
	ConsoleBase *m_console;
};

class NullErrorHandler : public ErrorHandlerBase
{
public:
	NullErrorHandler() { }
	~NullErrorHandler() { }

	ErrorCode init() { return ErrorCode::Success; }

	std::string getErrorString(ErrorCode p_error) { return ""; }
	ErrorType getErrorType(ErrorCode p_error) { return ErrorType::Warning; }

	void log(ErrorCode p_errorCode) { printf("Error: %i.\n", p_errorCode); }
	void log(ErrorCode p_errorCode, ErrorSource p_errorSource) { printf("Error: %i.\n", p_errorCode); }
	void log(ErrorType p_errorType, ErrorSource p_errorSource, std::string p_error) { printf("Error: %s.\n", p_error.c_str()); }
	void log(ErrorCode p_errorCode, ErrorSource p_errorSource, std::string p_error) { printf("Error: %i.\n", p_errorCode); }
	void log(const ErrorCode p_errorCode, const std::string &p_objectName, const ErrorSource p_errorSource) { printf("Error: %s: %i.\n", p_objectName.c_str(), p_errorCode); }

	bool ifSuccessful(ErrorCode p_errorCode, ErrorCode &p_returnCode) { p_returnCode = p_errorCode; return p_errorCode == ErrorCode::Success; }
	//ErrorCode cacheError(ErrorCode p_errorCode, std::string p_error) { return ErrorCode::Undefined; }
};
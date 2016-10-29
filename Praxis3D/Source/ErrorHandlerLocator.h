#pragma once

#include "ErrorHandler.h"

// Service locator provides global access to specific intanciated classes (i.e. services),
// but retains the ability to switch instances, and has fail-proof mechanism that returns 
// an empty (null) service if it hasn't been initialized. Uses dependency injection pattern.
class ErrHandlerLoc
{
public:
	// Get the services
	inline static ErrorHandlerBase &get() { return *m_errorHandler; }

	// Initialize the service locator to use null services
	inline static ErrorCode init() { m_errorHandler = &m_nullErrorHandler; return ErrorCode::Success; }

	// If the passed service is null, assign current service as a null service, otherwise assign the passed service as current
	inline static void provide(ErrorHandlerBase *p_service) { m_errorHandler = (p_service == nullptr) ? &m_nullErrorHandler : p_service; }

private:
	static ErrorHandlerBase *m_errorHandler;
	static NullErrorHandler m_nullErrorHandler;
};
#include "ServiceLocators/Include/ErrorHandlerLocator.hpp"

NullErrorHandler ErrHandlerLoc::m_nullErrorHandler;
ErrorHandlerBase *ErrHandlerLoc::m_errorHandler = &m_nullErrorHandler;
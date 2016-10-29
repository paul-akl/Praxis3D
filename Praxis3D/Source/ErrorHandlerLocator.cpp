#include "ErrorHandlerLocator.h"

NullErrorHandler ErrHandlerLoc::m_nullErrorHandler;
ErrorHandlerBase *ErrHandlerLoc::m_errorHandler = &m_nullErrorHandler;
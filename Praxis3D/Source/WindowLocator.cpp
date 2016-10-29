#include "WindowLocator.h"

WindowLocator::WindowWrapperBase WindowLocator::m_nullWindowWrapper;
WindowLocator::WindowWrapperBase *WindowLocator::m_windowWrapper = &m_nullWindowWrapper;
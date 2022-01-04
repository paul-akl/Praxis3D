#include "GUIHandlerLocator.h"

GUIHandlerNull GUIHandlerLocator::m_nullGUIHandler;
GUIHandlerBase *GUIHandlerLocator::m_GUIHandler = &m_nullGUIHandler;
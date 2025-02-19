#include "ServiceLocators/Include/ClockLocator.hpp"

ClockNull ClockLocator::m_nullClock;
ClockBase *ClockLocator::m_clock = &m_nullClock;
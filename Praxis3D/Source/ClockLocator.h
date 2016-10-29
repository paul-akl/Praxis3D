#pragma once

#include "Clock.h"

// Service locator provides global access to specific instantiated classes (i.e. services),
// but retains the ability to switch instances, and has fail-proof mechanism that returns 
// an empty (null) service if it hasn't been initialized. Uses dependency injection pattern.
class ClockLocator
{
public:
	// Get the services
	inline static ClockBase &get() { return *m_clock; }

	// Initialize the service locator to use null services
	inline static ErrorCode init() { m_clock = &m_nullClock; return ErrorCode::Success; }

	// If the passed service is null, assign current service as a null service, otherwise assign the passed service as current
	inline static void provide(ClockBase *p_clock) { m_clock = (p_clock == nullptr) ? &m_nullClock : p_clock; }

private:
	static ClockBase *m_clock;
	static ClockNull m_nullClock;
};
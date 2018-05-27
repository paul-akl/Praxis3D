#pragma once

#include <iostream>
#include <Windows.h>

#include "Config.h"
#include "ErrorCodes.h"

// Base class from which real and null implementations must inherit
// Does not contain some core methods, to restrict access to however can only call the base class, and whoever has
// the real implementation is the "owner" and can access all methods. Designed with inention of using for service locator.
class ClockBase
{
public:
	// Returns a double of the last frame in milliseconds
	const virtual double getDeltaMS() const = 0;
	// Returns a float of the last frame in milliseconds
	const virtual float getDeltaMSF() const = 0;

	// Returns a double of the last frame in seconds
	const virtual double getDeltaSeconds() const = 0;
	// Returns a float of the last frame in seconds
	const virtual float getDeltaSecondsF() const = 0;

	// Returns a double of the elapsed seconds since first update
	const virtual double getElapsedSeconds() const { return 0.0; }
	// Returns a float of the elapsed seconds since first update
	const virtual float getElapsedSecondsF() const { return 0.0f; }

	// Returns the number of elapsed frames since the first update call
	const virtual size_t getElapsedNumFrames() = 0;

	// Returns current number Frames per Second
	const virtual float getFPS() = 0;

protected:
	virtual const double getCurrentTime() = 0;
	virtual const float getCurrentTimeF() = 0;
};

// Real implementation. init() and update() methods are not in base class, to restrict access when used in service locator.
class Clock : public ClockBase
{
private:
	LARGE_INTEGER m_timeCurrent;
	LARGE_INTEGER m_timeLast;

	size_t	m_currentNumFrames;
	size_t	m_lastNumFrames;

	float	m_currentFPS,
			m_tickSum,
			*m_tickList;

	double	m_currentMS,
			m_frequency,
			m_elapsedSeconds;

	size_t	m_tickSamples,
			m_currentTickIndex;

public:
	Clock()
	{
		m_timeCurrent.u.HighPart = 0;
		m_timeCurrent.u.LowPart = 0;
		m_timeLast.u.HighPart = 0;
		m_timeLast.u.LowPart = 0;

		m_currentNumFrames = 0;
		m_lastNumFrames = 0;
		m_currentFPS = 0.0f;

		m_currentMS = 0.0;
		m_frequency = 0.0;

		m_elapsedSeconds = 0.0;

		m_tickSum = 0.0f;
		m_tickSamples = 0;
		m_currentTickIndex = 0;

		m_tickList = nullptr;
	}
	~Clock() { }

	ErrorCode init()
	{
		LARGE_INTEGER ticksPerSec;

		// Get CPU's ticks per second
		if(QueryPerformanceFrequency(&ticksPerSec))
		{
			// Get current time
			QueryPerformanceCounter(&m_timeCurrent);
			m_timeLast = m_timeCurrent;

			// Divide ticks per second by 1000 (to get MS instead of seconds) and cast to double
			m_frequency = double(ticksPerSec.QuadPart) / 1000.0;

			m_tickSamples = 100;// Config::engineVar().smoothing_tick_samples;

			m_tickList = new float[m_tickSamples]();

			for(unsigned int i = 0; i < m_tickSamples; i++)
				m_tickList[i] = 0.0f;

			return ErrorCode::Success;
		}
		else
		{
			return ErrorCode::Clock_QueryFrequency;
		}
	}

	// Must only be called once per frame
	inline void update()
	{
		// Increment the frame counter
		m_currentNumFrames++;

		// Get current time
		QueryPerformanceCounter(&m_timeCurrent);

		// Calculate current delta time milliseconds
		m_currentMS = double(m_timeCurrent.QuadPart - m_timeLast.QuadPart) / m_frequency;

		// Add current frame delta time to total elapsed time
		m_elapsedSeconds += (m_currentMS / 1000.0);

		m_timeLast = m_timeCurrent;
		
		// Increment tick index and reset it when it goes out of bounds
		m_currentTickIndex++;
		if(m_currentTickIndex >= m_tickSamples)
			m_currentTickIndex = 0;

		// Add the current tick's ms to the list
		m_tickList[m_currentTickIndex] = (float)m_currentMS / 1000.0f;

		// Add all the ticks
		m_tickSum = 0.0f;
		for(decltype(m_tickSamples) i = 0; i < m_tickSamples; i++)
			m_tickSum += m_tickList[i];

		// Average out the ticks and calculate FPS
		m_currentFPS = 1.0f / (m_tickSum / m_tickSamples);
	}

	// Returns a double of the last frame in milliseconds
	const double getDeltaMS() const { return m_currentMS; }

	// Returns a float of the last frame in milliseconds
	const float getDeltaMSF() const { return (float)m_currentMS; }

	// Returns a double of the last frame in seconds
	const double getDeltaSeconds() const { return m_currentMS / 1000.0; }

	// Returns a float of the last frame in seconds
	const float getDeltaSecondsF() const { return (float)(m_currentMS / 1000.0); }

	// Returns a double of the elapsed seconds since first update
	const double getElapsedSeconds() const { return m_elapsedSeconds; }

	// Returns a float of the elapsed seconds since first update
	const float getElapsedSecondsF() const { return (float)m_elapsedSeconds; }

	// Number of elapsed frames since the first update call
	// update() needs to be called once per frame to work accurately
	const size_t getElapsedNumFrames() { return m_currentNumFrames; }

	// Returns current number Frames per Second
	const float getFPS() { return m_currentFPS; }

protected:
	const double getCurrentTime()
	{
		// Get current time
		QueryPerformanceCounter(&m_timeCurrent);

		return double(m_timeCurrent.QuadPart) / m_frequency;
	}
	const float getCurrentTimeF() { return (float)getCurrentTime(); }
};

// Null version of the clock class, returning "0" from all functions
class ClockNull : public ClockBase
{
public:
	const double getDeltaMS() const { return 0.0; }
	const float getDeltaMSF() const { return 0.0f; }

	const double getDeltaSeconds() const { return 0.0; }
	const float getDeltaSecondsF() const { return 0.0f; }

	// Returns a double of the elapsed seconds since first update
	const double getElapsedSeconds() const { return 0.0; }

	// Returns a float of the elapsed seconds since first update
	const float getElapsedSecondsF() const { return 0.0f; }

	const double getCurrentTime() { return 0.0; }
	const float getCurrentTimeF() { return 0.0f; }

	const size_t getElapsedNumFrames() { return 0; }

	const float getFPS() { return 0; }
};
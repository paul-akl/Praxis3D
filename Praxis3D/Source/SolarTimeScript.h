#pragma once

#include "BaseScriptObject.h"
#include "ClockLocator.h"
#include "RendererScene.h"
#include "KeyCommand.h"

// Amount of seconds contained in one hour
#define SECONDS_IN_HOUR 82800.0f

// Provides bare user interface functionality, like button presses to toggle features (for debugging)
class SolarTimeScript : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	SolarTimeScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::SolarTimeScript)
	{
		m_dayOfYear = 1;
		m_hours = 0;
		m_minutes = 0;
		m_seconds = 0.0f;
		m_elaspedSeconds = 0.0f;
		m_timeMultiplier = 1.0f;
		m_offsetPosition = 10.0f;
		m_latitude = 0.0f;
		m_longitude = 0.0f;
	}
	~SolarTimeScript()
	{
		// Unbind all the keys
		m_forwardKey.unbindAll();
	}

	virtual ErrorCode init()
	{

		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		// Nothing to load
	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::SolarTimeScript);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Hours, m_hours);
		propertySet.addProperty(Properties::Seconds, m_seconds);
		propertySet.addProperty(Properties::Latitude, m_latitude);
		propertySet.addProperty(Properties::Longitude, m_latitude);
		propertySet.addProperty(Properties::DayOfYear, m_dayOfYear);
		propertySet.addProperty(Properties::TimeMultiplier, m_timeMultiplier);
		propertySet.addProperty(Properties::OffsetPosition, m_offsetPosition);

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		/* Not used at this time
		// _____________________________________________________________________________________
		// Calculate hours, minutes and seconds
		m_hours = (int)(m_elaspedSeconds / 3600.0f);
		m_minutes = (int)((m_elaspedSeconds - (m_hours * 3600.0f)) / 60.0f);
		m_seconds = (m_elaspedSeconds - (m_hours * 3600.0f) - (m_minutes * 60.0f));

		// Reset the hours when it reaches 24
		if(m_hours > 23)
		{
			m_elaspedSeconds -= m_hours * 3600.0f;
			m_hours = 0;
		}
		// _____________________________________________________________________________________*/

		// Add delta seconds to elapsed time
		m_elaspedSeconds += ClockLocator::get().getDeltaSecondsF() * m_timeMultiplier;

		// Reset the hours when it reaches 24
		if(m_elaspedSeconds > SECONDS_IN_HOUR)
			m_elaspedSeconds -= SECONDS_IN_HOUR;

		// Calculate a crude approximation of sun angle based on time of day
		float angle = (m_elaspedSeconds / SECONDS_IN_HOUR) * (float)(2.0*PI) + (float)(1.5*PI);

		// Set the sun direction
		m_sunDirection = Math::normalize(Math::Vec3f(cosf(angle), sinf(angle), sinf(angle)));

		m_sunPosition = (-m_sunDirection * m_offsetPosition) + m_originPosition;

		//postChanges(Systems::Changes::Spacial::Position | Systems::Changes::Spacial::Rotation);
	}

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Spacial::All; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spacial::Position)
			m_originPosition = p_subject->getVec3(this, Systems::Changes::Spacial::Position);

		if(p_changeType & Systems::Changes::Spacial::Rotation)
			m_sunDirection = p_subject->getVec3(this, Systems::Changes::Spacial::Rotation);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_sunPosition;
			break;
		case Systems::Changes::Spacial::Rotation:
			return m_sunDirection;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}
	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Graphics::Lighting:

			break;
		}

		return ObservedSubject::getBool(p_observer, p_changedBits);
	}

	// Setters
	const inline void setHours(const int p_hours)							{ m_hours = p_hours;					}
	const inline void setMinutes(const int p_minutes)						{ m_minutes = p_minutes;				}
	const inline void setSeconds(const float p_seconds)						{ m_seconds = p_seconds;				}
	const inline void setLatitude(const float p_latitude)					{ m_latitude = p_latitude;				}
	const inline void setLongitude(const float p_longitude)					{ m_longitude = p_longitude;			}
	const inline void setDayOfYear(const int p_dayOfYear)					{ m_dayOfYear = p_dayOfYear;			}
	const inline void setTimeMultiplier(const float p_timeMultiplier)		{ m_timeMultiplier = p_timeMultiplier;	}
	const inline void setOffsetPosition(const float p_offSetPosition)		{ m_offsetPosition = p_offSetPosition;	}
	const inline void setOriginPosition(const Math::Vec3f p_originPosition) { m_originPosition = p_originPosition;	}

	// Setters for the key binds:
	inline void setForwardKey(Scancode p_key)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_key);
	}
	inline void setForwardKey(std::string &p_string)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_string);
	}

	inline Math::Vec2f calcSunPosition() const
	{
		// Get the fractional year in radians
		float fracYear = (2.0f * (float)PI / 365.0f) * (m_dayOfYear - 1 + (m_hours - 12.0f) / 24.0f);

		// Get the equation of time in minutes
		float equatOfTime = 229.18f * (0.000075f + 0.001868f * cosf(fracYear) - 
									   0.032077f * sinf(fracYear) - 
									   0.014615f * cosf(2 * fracYear) - 
									   0.040849f * sinf(2.0f * fracYear));

		// Get the solar declination angle in radians
		float declinAngle = 0.006918f - 
							0.399912f * cosf(fracYear) + 
							0.070257f * sinf(fracYear) - 
							0.006758f * cosf(2.0f * fracYear) + 
							0.000907f * sinf(2.0f * fracYear) - 
							0.002697f * cosf(3.0f * fracYear) + 
							0.00148f * sinf(3.0f * fracYear);

		// Calculate true solar time in minutes
		float timeOffset = equatOfTime - 4.0f * m_longitude;
		float solarTime = m_hours * 60 + m_minutes + (m_seconds * (1.0f / 60.0f)) + timeOffset;

		// Calculate solar hour angle in degrees
		float solarHour = solarTime / 4.0f - 180.0f;

		// Calculate solar zenith and azimuth angles
		float solarZenith = sinf(m_latitude) * sinf(declinAngle) + cosf(m_latitude) * cosf(declinAngle) * cosf(solarHour);
		float solarAzimuth = -(sinf(m_latitude) * cosf(solarZenith) - sinf(declinAngle)) / (cosf(m_latitude) * sinf(solarZenith));

		return Math::Vec2f(solarZenith, solarAzimuth);
	}

private:
	KeyCommand  m_forwardKey;

	int	m_dayOfYear,
		m_hours,
		m_minutes;

	float	m_seconds,
			m_elaspedSeconds,
			m_timeMultiplier,
			m_latitude,
			m_longitude,
			m_offsetPosition;	// Used as an offset from origin when calculating celestial bodies positions

	Math::Vec3f	m_originPosition,
				m_sunPosition,
				m_sunDirection;
};
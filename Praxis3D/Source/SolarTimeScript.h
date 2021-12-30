#pragma once

#include "BaseScriptObject.h"
#include "ClockLocator.h"
#include "Math.h"
#include "RendererScene.h"
#include "KeyCommand.h"

// Amount of seconds contained in one hour
#define SECONDS_IN_HOUR		82800.0f

// Declaration of some constants
#define dEarthMeanRadius     6371.01	// In km
#define dAstronomicalUnit    149597890	// In km

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
		m_timeZone = 0;
		m_year = 2018;
		m_month = 5;
		m_day = 14;

		m_azimuthAngle = 0.0;
		m_zenithAngle = 0.0;
		m_elevationAngle = 0.0;
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
		// Add delta seconds to elapsed time
		m_elaspedSeconds += ClockLocator::get().getDeltaSecondsF() * m_timeMultiplier;

		// Calculate hours, minutes and seconds
		m_hours = (int)(m_elaspedSeconds / 3600.0f);
		m_minutes = (int)((m_elaspedSeconds - (m_hours * 3600.0f)) / 60.0f);
		m_seconds = (m_elaspedSeconds - (m_hours * 3600.0f) - (m_minutes * 60.0f));

		// Increment date counters and reset the hours when it reaches 24
		if(m_hours > 23)
		{
			m_elaspedSeconds -= m_hours * 3600.0f;
			m_hours = 0;
			m_day++;

			if(m_day > 28)
			{
				switch(m_day)
				{
				case 29:

					// If the month has 28 days (February)
					if(m_month == 2)
					{
						m_month++;
						m_day = 1;
					}

					break;
				case 31:

					// If the month has 30 days
					if(m_month == 4 || m_month == 6 || m_month == 9 || m_month == 11)
					{
						m_month++;
						m_day = 1;
					}

					break;
				case 32:

					// If the month has 31 days
					if(m_month == 1 || m_month == 3 || m_month == 5 || m_month == 7 || m_month == 8 || m_month == 10 || m_month == 12)
					{
						m_month++;
						m_day = 1;
					}

					break;
				}
			}
		}
				
		// Calculate sun position
		calcSunPosition();

		// Transform angle values into radian
		m_azimuthAngle = Math::toRadian(m_azimuthAngle);
		m_zenithAngle = Math::toRadian(m_zenithAngle);
		m_elevationAngle = Math::toRadian(m_elevationAngle);
		
		/*
		m_sunDirection = Math::Vec3f(
			cos(m_azimuthAngle) * sin(m_zenithAngle),
			cos(m_zenithAngle),
			sin(m_azimuthAngle) * sin(m_zenithAngle));

		m_sunDirection = Math::Vec3f(
			sin(m_azimuthAngle) * sin(altitude),
			cos(m_azimuthAngle) * sin(altitude),
			sin(altitude));

		m_sunDirection = Math::Vec3f(
			sin(m_zenithAngle) * cos(m_azimuthAngle),
			sin(m_zenithAngle) * sin(m_azimuthAngle),
			cos(m_zenithAngle));

		m_sunDirection.z = cos(m_zenithAngle) * cos(m_azimuthAngle) * radius;
		m_sunDirection.x = sin(m_zenithAngle) * cos(m_azimuthAngle) * radius;
		m_sunDirection.y = sin(m_azimuthAngle) * radius;

		m_sunDirection.x = cos(m_zenithAngle) * sin(m_azimuthAngle) * radius;
		m_sunDirection.y = sin(m_zenithAngle) * sin(m_azimuthAngle) * radius;
		m_sunDirection.z = cos(m_azimuthAngle) * radius;

		m_sunDirection.x = sin(m_azimuthAngle) * cos(m_zenithAngle) * radius;
		m_sunDirection.y = sin(m_azimuthAngle) * sin(m_zenithAngle) * radius;
		m_sunDirection.z = cos(m_azimuthAngle) * radius;
		
		m_sunDirection.x = sin(m_azimuthAngle) * cos(altitude);
		m_sunDirection.y = cos(m_azimuthAngle) * cos(altitude);
		m_sunDirection.z = sin(altitude);
		
		m_sunDirection.x = 0.0f;// sin(m_azimuthAngle) * cos(m_elevationAngle);
		m_sunDirection.y = cos(m_azimuthAngle);// *cos(m_elevationAngle);
		m_sunDirection.z = sin(m_elevationAngle);
		
		m_sunDirection.x = sin(m_azimuthAngle);
		m_sunDirection.y = cos(m_azimuthAngle) * cos(m_elevationAngle);
		m_sunDirection.z = sin(m_elevationAngle);*/
		
		// Translate sun spherical coordinates into cartesian direction vector
		m_sunDirection.x = static_cast<float>(sin(m_azimuthAngle) * cos(m_elevationAngle));
		m_sunDirection.y = static_cast<float>(sin(m_elevationAngle));
		m_sunDirection.z = static_cast<float>(cos(m_azimuthAngle) * cos(m_elevationAngle));
		
		// Normalize sun direction
		m_sunDirection.normalize();

		// Calculate sun position in the sky
		m_sunPosition = (-m_sunDirection * m_offsetPosition) + m_originPosition;

		// Notify observers of the changes
		postChanges(Systems::Changes::Spatial::LocalPosition | Systems::Changes::Spatial::LocalRotation);
	}

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::All; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spatial::LocalPosition)
			m_originPosition = p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition);

		if(p_changeType & Systems::Changes::Spatial::LocalRotation)
			m_sunDirection = p_subject->getVec3(this, Systems::Changes::Spatial::LocalRotation);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_sunPosition;
			break;
		case Systems::Changes::Spatial::LocalRotation:
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
	const inline void setHours(const int p_hours)
	{
		m_hours = p_hours; 
		m_elaspedSeconds += p_hours * 3600;
	}
	const inline void setMinutes(const int p_minutes)
	{
		m_minutes = p_minutes; 
		m_elaspedSeconds += p_minutes * 60;
	}
	const inline void setSeconds(const float p_seconds)
	{
		m_seconds = p_seconds; 
		m_elaspedSeconds += p_seconds;
	}
	const inline void setLatitude(const float p_latitude)					{ m_latitude = p_latitude;				}
	const inline void setLongitude(const float p_longitude)					{ m_longitude = p_longitude;			}
	const inline void setDayOfYear(const int p_dayOfYear)					{ m_dayOfYear = p_dayOfYear;			}
	const inline void setYear(const int p_year)								{ m_year = p_year;						}
	const inline void setMonth(const int p_month)							{ m_month = p_month;					}
	const inline void setDay(const int p_day)								{ m_day = p_day;						}
	const inline void setTimeZone(const int p_timeZone)						{ m_timeZone = p_timeZone;				}
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
		m_forwardKey.bindByKeyName(p_string);
	}
	
	// PSA Sun position algorithm
	void calcSunPosition()
	{
		// Main variables
		double dElapsedJulianDays;
		double dDecimalHours;
		double dEclipticLongitude;
		double dEclipticObliquity;
		double dRightAscension;
		double dDeclination;

		// Auxiliary variables
		double dY;
		double dX;

		// Calculate difference in days between the current Julian Day 
		// and JD 2451545.0, which is noon 1 January 2000 Universal Time
		{
			double dJulianDate;
			long int liAux1;
			long int liAux2;

			// Calculate time of the day in UT decimal hours
			dDecimalHours = (m_hours + (m_minutes + m_seconds / 60.0) / 60.0) - m_timeZone;

			// Calculate current Julian Day
			liAux1 = (m_month - 14) / 12;
			liAux2 = (1461 * (m_year + 4800 + liAux1)) / 4 
				+ (367 * (m_month - 2 - 12 * liAux1)) / 12 
				- (3 * ((m_year + 4900 + liAux1) / 100)) / 4 
				+ m_day - 32075;

			dJulianDate = (double)(liAux2)-0.5 + dDecimalHours / 24.0;

			// Calculate difference between current Julian Day and JD 2451545.0 
			dElapsedJulianDays = dJulianDate - 2451545.0;
		}

		// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the 
		// ecliptic in radians but without limiting the angle to be less than 2*Pi 
		// (i.e., the result may be greater than 2*Pi)
		{
			double dMeanLongitude;
			double dMeanAnomaly;
			double dOmega;
			dOmega = 2.1429 - 0.0010394594*dElapsedJulianDays;
			dMeanLongitude = 4.8950630 + 0.017202791698*dElapsedJulianDays; // Radians
			dMeanAnomaly = 6.2400600 + 0.0172019699*dElapsedJulianDays;
			dEclipticLongitude = dMeanLongitude 
				+ 0.03341607 * sin(dMeanAnomaly) 
				+ 0.00034894 * sin(2 * dMeanAnomaly) 
				- 0.0001134 - 0.0000203 * sin(dOmega);
			dEclipticObliquity = 0.4090928 - 6.2140e-9 * dElapsedJulianDays + 0.0000396 * cos(dOmega);
		}

		// Calculate celestial coordinates ( right ascension and declination ) in radians 
		// but without limiting the angle to be less than 2*Pi (i.e., the result may be 
		// greater than 2*Pi)
		{
			double dSin_EclipticLongitude;
			dSin_EclipticLongitude = sin(dEclipticLongitude);
			dY = cos(dEclipticObliquity) * dSin_EclipticLongitude;
			dX = cos(dEclipticLongitude);
			dRightAscension = atan2(dY, dX);
			if(dRightAscension < 0.0) 
				dRightAscension = dRightAscension + TWOPI;
			dDeclination = asin(sin(dEclipticObliquity) * dSin_EclipticLongitude);
		}

		// Calculate local coordinates ( azimuth and zenith angle ) in degrees
		{
			double dGreenwichMeanSiderealTime;
			double dLocalMeanSiderealTime;
			double dLatitudeInRadians;
			double dHourAngle;
			double dCos_Latitude;
			double dSin_Latitude;
			double dCos_HourAngle;
			double dParallax;
			dGreenwichMeanSiderealTime = 6.6974243242 + 0.0657098283 * dElapsedJulianDays + dDecimalHours;
			dLocalMeanSiderealTime = (dGreenwichMeanSiderealTime * 15 + m_longitude) * RAD;
			dHourAngle = dLocalMeanSiderealTime - dRightAscension;
			dLatitudeInRadians = m_latitude * RAD;
			dCos_Latitude = cos(dLatitudeInRadians);
			dSin_Latitude = sin(dLatitudeInRadians);
			dCos_HourAngle = cos(dHourAngle);
			m_zenithAngle = (acos(dCos_Latitude * dCos_HourAngle * cos(dDeclination) + sin(dDeclination) * dSin_Latitude));
			dY = -sin(dHourAngle);
			dX = tan(dDeclination)*dCos_Latitude - dSin_Latitude * dCos_HourAngle;
			m_azimuthAngle = atan2(dY, dX);
			if(m_azimuthAngle < 0.0)
				m_azimuthAngle = m_azimuthAngle + TWOPI;
			m_azimuthAngle = m_azimuthAngle / RAD;

			// Parallax Correction
			dParallax = (dEarthMeanRadius / dAstronomicalUnit) * sin(m_zenithAngle);
			m_zenithAngle = (m_zenithAngle + dParallax) / RAD;
			m_elevationAngle = 90.0f - m_zenithAngle;
		}
	}

	void calcSunPositionOld()
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
		m_zenithAngle = (double)(sinf(m_latitude) * sinf(declinAngle) + cosf(m_latitude) * cosf(declinAngle) * cosf(solarHour));
		m_azimuthAngle = (double)(-(sinf(m_latitude) * cosf((float)m_zenithAngle) - sinf(declinAngle)) / (cosf(m_latitude) * sinf((float)m_zenithAngle)));
	}
	
private:
	KeyCommand m_forwardKey;

	int	m_dayOfYear,
		m_hours,
		m_minutes,
		m_year,
		m_month,
		m_day,
		m_timeZone;

	float	m_seconds,
			m_elaspedSeconds,
			m_timeMultiplier,
			m_latitude,
			m_longitude,
			m_offsetPosition;	// Used as an offset from origin when calculating celestial bodies positions
	
	double	m_azimuthAngle,
			m_zenithAngle,
			m_elevationAngle;

	Math::Vec3f	m_originPosition,
				m_sunPosition,
				m_sunDirection;
};
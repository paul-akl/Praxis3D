#pragma once

#include "Containers.h"
#include "Math.h"
#include "NullSystemObjects.h"
#include "System.h"

class GameLogicObject : public SystemObject
{
public:
	GameLogicObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::Script)
	{


	}

	~GameLogicObject()
	{

	}

	// System type is Graphics
	BitMask getSystemType() { return Systems::Script; }

	void update(const float p_deltaTime)
	{
		if(isUpdateNeeded())
		{
			// Mark as updated
			updatePerformed();
		}
	}

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::AllWorld | Systems::Changes::Graphics::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::WorldTransform; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// Get all of the world spatial data, include the transform matrix; add up the bit-mask of changed data;
		if(p_changeType & Systems::Changes::Spatial::AllWorld)
		{
			newChanges = newChanges | Systems::Changes::Spatial::AllWorld;
		}
		else
		{
			setUpdateNeeded(true);
		}

		// If any new data has been left, pass it to the components
		if(newChanges != Systems::Changes::None)
		{

		}
		//postChanges(newChanges);
	}

private:

};
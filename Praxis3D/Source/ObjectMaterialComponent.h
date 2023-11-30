#pragma once

#include "CommonDefinitions.h"
#include "InheritanceObjects.h"

class ObjectMaterialComponent : public SystemObject
{
	friend class WorldScene;
public:
	struct ObjectMaterialComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		ObjectMaterialComponentConstructionInfo()
		{
			m_materialType = ObjectMaterialType::Concrete;
		}
		ObjectMaterialType m_materialType;
	};

	ObjectMaterialComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::ObjectMaterialComponent, p_entityID)
	{
		m_materialType = ObjectMaterialType::Concrete;
	}
	~ObjectMaterialComponent()
	{

	}
	ErrorCode init() final override
	{
		m_initialized = true;
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		setActive(true);
	}

	void update(const float p_deltaTime)
	{

	}

	BitMask getSystemType() final override { return Systems::World; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::None; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::World::ObjectMaterialType))
		{
			// Get the new material type from the observed subject
			auto newMaterialType = p_subject->getUnsignedInt(this, Systems::Changes::World::ObjectMaterialType);

			// If the new material type number is within the bounds of the ObjectMaterialType enum, 
			// cast the number to the enum and assign it as the material type of this component
			if(newMaterialType < ObjectMaterialType::NumberOfMaterialTypes)
				m_materialType = static_cast<ObjectMaterialType>(newMaterialType);
		}
	}

	inline ObjectMaterialType getObjectMaterialType() const noexcept { return m_materialType; }

private:
	ObjectMaterialType m_materialType;
};
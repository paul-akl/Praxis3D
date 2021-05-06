#pragma once

#include "Config.h"
#include "Containers.h"
#include "PropertySet.h"
#include "System.h"

class GraphicsObject;

class BaseGraphicsComponent// : SystemObject
{
public:
	BaseGraphicsComponent() : m_empty(true), m_loaded(false) { }
	~BaseGraphicsComponent() { }

	virtual void load(PropertySet &p_properties) { }
	virtual PropertySet export() { return PropertySet(); }

	virtual void update(GraphicsObject &p_parentObject);
	
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }
	
	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::None; };

	const inline bool empty() const { return m_empty; }
	const inline bool loaded() const { return m_loaded; }

protected:
	inline void setEmpty(bool p_empty) { m_empty = p_empty; };
	inline void setLoaded(bool p_loaded) { m_loaded = p_loaded; };

private:
	bool m_empty;
	bool m_loaded;
};
#pragma once

#include <string>

#include "Config.h"
#include "Scancodes.h"

class KeyCommand
{
public:
	KeyCommand() : m_active(false) { }
	~KeyCommand() { unbindAll(); }

	inline void activate() { m_active = true; }
	inline void deactivate() { m_active = false; }
	const inline bool isActivated() const { return m_active; }

	inline void addScancode(Scancode p_scancode) { m_scancodes.push_back(p_scancode); }
	inline void removeScancode(Scancode p_scancode)
	{
		for(decltype(m_scancodes.size()) i = 0, size = m_scancodes.size(); i < size; i++)
		{
			if(m_scancodes[i] == p_scancode)
			{
				std::swap(m_scancodes[i], m_scancodes.back());
				m_scancodes.pop_back();
				return;
			}
		}
	}
	inline void removeAllScancodes()
	{
		std::vector<Scancode>().swap(m_scancodes);
	}

	void bind(std::string &p_keyName);
	void bind(Scancode p_scancode);
	void unbind(Scancode p_scancode);
	void unbindAll();

	inline size_t getNumBindings() const { return m_scancodes.size(); }

	// Bool operator, returns true if the key is activated
	inline operator bool() const { return isActivated(); }

	// Compares the memory address of the instances (i.e. pointer equality)
	const inline bool operator==(const KeyCommand &p_command) const	{ return this == &p_command; }
	
	// Array subscription operator; returns a scancode from an internal array
	const inline Scancode operator[](const size_t p_index) const { return m_scancodes[p_index]; }

	// Returns the scancode of the first binding; if no bindings exist, returns a null scancode
	const inline Scancode getFirstBinding() { return m_scancodes.empty() ? Key_Invalid : m_scancodes[0]; }

private:
	bool m_active;
	std::vector<Scancode> m_scancodes;
};
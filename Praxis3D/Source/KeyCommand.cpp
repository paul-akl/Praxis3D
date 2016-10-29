
#include "KeyCommand.h"
#include "WindowLocator.h"

void KeyCommand::bind(std::string &p_keyName)
{
	// Get the scancode by the key name, and call bind
	bind(WindowLocator::get().getScancode(p_keyName));
}

void KeyCommand::bind(Scancode p_scancode)
{
	// If the scancode is valid, bind this key command to it
	if(p_scancode != Scancode::Key_Invalid)
	{
		WindowLocator::get().bindCommand(p_scancode, this);
	}
}

void KeyCommand::unbind(Scancode p_scancode)
{
	// Iterate over all scancodes, if they match, unbind the command and remove the scancode
	for(decltype(m_scancodes.size()) i = 0, size = m_scancodes.size(); i < size; i++)
		if(m_scancodes[i] == p_scancode)
		{
			WindowLocator::get().unbindCommand(p_scancode, this);
			std::swap(m_scancodes[i], m_scancodes.back());
			m_scancodes.pop_back();
			return;
		}
}

void KeyCommand::unbindAll()
{
	for(decltype(m_scancodes.size()) i = 0, size = m_scancodes.size(); i < size; i++)
		WindowLocator::get().unbindCommand(m_scancodes[i], this);

	removeAllScancodes();
}
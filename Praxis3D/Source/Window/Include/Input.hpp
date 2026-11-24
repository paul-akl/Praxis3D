#pragma once

#include "Window/Include/KeyCommand.hpp"
#include "Window/Include/Window.hpp"

class InputNull
{
public:
	InputNull() { }
	~InputNull() { }

	virtual void bindCommand(std::string &p_keyName, KeyCommand *p_command) { }
	virtual void bindCommand(Scancode p_scancode, KeyCommand *p_command) { }

	virtual void unbindCommand(std::string &p_keyName, KeyCommand *p_command) { }
	virtual void unbindCommand(Scancode p_scancode, KeyCommand *p_command) { }

	virtual void unbindAll(std::string &p_keyName) { }
	virtual void unbindAll(Scancode p_scancode) { }

private:

};

class Input : public InputNull
{
public:
	Input(Window &p_window) : m_window(p_window) { }
	~Input() { }

	void bindCommand(std::string &p_keyName, KeyCommand *p_command) { m_window.bindCommand(p_keyName, p_command); }
	void bindCommand(Scancode p_scancode, KeyCommand *p_command) { m_window.bindCommand(p_scancode, p_command); }

	void unbindCommand(std::string &p_keyName, KeyCommand *p_command) { m_window.unbindCommand(p_keyName, p_command); }
	void unbindCommand(Scancode p_scancode, KeyCommand *p_command) { m_window.unbindCommand(p_scancode, p_command); }

	void unbindAll(std::string &p_keyName) { m_window.unbindAll(p_keyName); }
	void unbindAll(Scancode p_scancode) { m_window.unbindAll(p_scancode); }

private:
	Window &m_window;
};
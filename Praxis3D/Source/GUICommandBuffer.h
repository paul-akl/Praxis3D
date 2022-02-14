#pragma once

#include <string>

enum GUICommandType : unsigned int
{
	GUICommandType_none = 0,
	GUICommandType_begin,
	GUICommandType_end,
	GUICommandType_text,
	GUICommandType_button,
	GUICommandType_color4,
	GUICommandType_numOfCommands
};

struct GUICommandBegin
{
	std::string m_text;
	bool m_active;
	int m_flags;
};
struct GUICommandEnd
{

};
struct GUICommandText
{
	std::string m_text;
};
struct GUICommandButton
{
	std::string m_text;
};
struct GUICommandColor4
{
	std::string m_text;
};

struct GUICommand
{

	GUICommand(const std::string &p_text, const bool p_active, const int p_flags)
	{
		m_command.m_GUICommandBegin.m_text = p_text;
		m_command.m_GUICommandBegin.m_active = p_active;
		m_command.m_GUICommandBegin.m_flags = p_flags;
		m_commandType = GUICommandType::GUICommandType_begin;
	}
	GUICommand()
	{
		m_commandType = GUICommandType::GUICommandType_end;
	}
	GUICommand(const std::string &p_text)
	{
		m_command.m_GUICommandText.m_text = p_text;
		m_commandType = GUICommandType::GUICommandType_text;
	}

	union GUICommandUnion
	{
		GUICommandUnion() { }
		~GUICommandUnion() { }

		GUICommandBegin m_GUICommandBegin;
		GUICommandEnd m_GUICommandEnd;
		GUICommandText m_GUICommandText;
	};

	GUICommandUnion m_command;
	GUICommandType m_commandType;
};

class GUICommandBuffer
{
public:
	GUICommandBuffer() { }
	~GUICommandBuffer() { }

private:
};
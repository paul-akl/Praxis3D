#pragma once

class SpinWait
{
	friend class Lock;
public:
	SpinWait();
	~SpinWait();

	class Lock
	{
	public:
		Lock(SpinWait &p_spinWait, bool p_readOnly = false);
		~Lock();

		Lock& operator = (const Lock &p_lock)
		{

		}

	protected:
		SpinWait &m_spinWait;
	};

protected:
	unsigned int m_lock[8];
};



#define TBB_SUPPRESS_DEPRECATED_MESSAGES 1

#include <tbb\tbb.h>

#include "Multithreading/Include/SpinWait.hpp"

SpinWait::SpinWait()
{
	// IMPLEMENTATION NOTE
	// In cases when there is no oversubscription the reasonable value of the spin 
	// count would be 50000 - 100000 (25 - 50 microseconds on modern CPUs).
	// Unfortunately the spinning in Windows critical section is power inefficient
	// so the value is a traditional 1000 (approx. the cost of kernel mode transition)
	//
	// To achieve maximal locking efficiency use TBB spin_mutex (which employs 
	// exponential backoff technique, and supports cooperative behavior in case 
	// of oversubscription)

	//auto result = ::InitializeCriticalSectionAndSpinCount(reinterpret_cast<LPCRITICAL_SECTION>(m_lock), 1000);
	auto result = ::InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
}
SpinWait::~SpinWait()
{
	//::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(m_lock));
	::DeleteCriticalSection(&m_lock);
}

SpinWait::Lock::Lock(SpinWait &p_spinWait, bool p_readOnly) : m_spinWait(p_spinWait)
{
	//::EnterCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(m_spinWait.m_lock));
	::EnterCriticalSection(&m_spinWait.m_lock);
}
SpinWait::Lock::~Lock()
{
	//::LeaveCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(m_spinWait.m_lock));
	::LeaveCriticalSection(&m_spinWait.m_lock);
}

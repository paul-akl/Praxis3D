#include "ServiceLocators/Include/ObjectDirectory.hpp"

tbb::concurrent_vector<const SystemObject*> ObjectDirectory::m_systemObjectPool;
tbb::concurrent_queue<ObjectDirectory::ObjectListIndexType> ObjectDirectory::m_emptyObjectListElements;

NullSystemObject ObjectDirectory::m_nullObject;
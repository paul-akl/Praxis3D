
#include "NullObjects.h"
#include "Math.h"
#include "ObserverBase.h"
#include "SpatialDataManager.h"

// An empty observer class, used only to construct the SpatialDataManager
class NullObserver : public Observer
{
public:
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }
};

constexpr NullObserver nullObserver;

const Math::Quaternion		NullObjects::NullQuaterion = Math::Quaternion();
const Math::Vec3f			NullObjects::NullVec3f = Math::Vec3f(1.0f);
const Math::Vec4f			NullObjects::NullVec4f = Math::Vec4f(1.0f);
const Math::Mat4f			NullObjects::NullMat4f = Math::Mat4f();
const bool					NullObjects::NullBool = false;
const int					NullObjects::NullInt = 0;
const float					NullObjects::NullFloat = 0.0f;
const double				NullObjects::NullDouble = 0.0;
const std::string			NullObjects::NullString;
const SpatialData			NullObjects::NullSpacialData;
const SpatialTransformData	NullObjects::NullSpacialTransformData;
const SpatialDataManager	NullObjects::NullSpatialDataManager = SpatialDataManager(nullObserver);

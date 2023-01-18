
#include "CommonDefinitions.h"
#include "GUIDataManager.h"
#include "NullObjects.h"
#include "Math.h"
#include "ObserverBase.h"
#include "PhysicsDataManager.h"
#include "SpatialDataManager.h"

// An empty observer class, used only to construct the SpatialDataManager
class NullObserver : public Observer
{
public:
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }
};

constexpr NullObserver nullObserver;

const glm::quat				NullObjects::NullQuaterion = glm::quat();
const glm::vec3				NullObjects::NullVec3f = glm::vec3(1.0f);
const glm::vec4				NullObjects::NullVec4f = glm::vec4(1.0f);
const glm::mat3				NullObjects::NullMat3f = glm::mat3();
const glm::mat4				NullObjects::NullMat4f = glm::mat4();
const bool					NullObjects::NullBool = false;
const int					NullObjects::NullInt = 0;
const float					NullObjects::NullFloat = 0.0f;
const double				NullObjects::NullDouble = 0.0;
const std::string			NullObjects::NullString;
const SpatialData			NullObjects::NullSpacialData;
const SpatialTransformData	NullObjects::NullSpacialTransformData;
const SpatialDataManager	NullObjects::NullSpatialDataManager = SpatialDataManager(nullObserver);
const GUIDataManager		NullObjects::NullGUIDataManager = GUIDataManager(nullObserver);
const PhysicsDataManager	NullObjects::NullPhysicsDataManager = PhysicsDataManager(nullObserver);
const Functors				NullObjects::NullFunctors;

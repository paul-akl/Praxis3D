#include "Math.h"

/*namespace Math
{

}*/

const glm::mat4 Math::createTransformMat(const glm::vec3 &p_position, const glm::vec3 &p_rotation, const glm::vec3 &p_scale)
{
	glm::mat4 returnMatrix(1.0f);

	returnMatrix = glm::translate(returnMatrix, p_position);

	//glm::quat quaternion(glm::radians(p_rotation));
	//returnMatrix *= glm::toMat4(quaternion);

	glm::quat yawQ = glm::quat(glm::vec3(0.0f, glm::radians(p_rotation.y), 0.0f));
	yawQ = glm::normalize(yawQ);
	glm::mat4 yawMat = glm::mat4_cast(yawQ);

	glm::quat pitchQ = glm::quat(glm::vec3(glm::radians(p_rotation.x), 0.0f, 0.0f));
	pitchQ = glm::normalize(pitchQ);
	glm::mat4 pitchMat = glm::mat4_cast(pitchQ);

	glm::quat rollQ = glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(p_rotation.z)));
	rollQ = glm::normalize(rollQ);
	glm::mat4 rollMat = glm::mat4_cast(rollQ);

	glm::quat qPitch = glm::angleAxis(glm::radians(p_rotation.x), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(p_rotation.y), glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(glm::radians(p_rotation.z), glm::vec3(0, 0, 1));

	glm::quat orientation = glm::normalize(qPitch * qYaw * qRoll);

	returnMatrix *= glm::mat4_cast(orientation);

	//returnMatrix *= pitchMat * yawMat * rollMat;

	returnMatrix = glm::scale(returnMatrix, p_scale);
	//returnMatrix *= glm::orientate4(glm::vec3(p_rotation.y, p_rotation.z, p_rotation.x));
	//returnMatrix = glm::rotate(returnMatrix, rotAngle, Rotation);

	return returnMatrix;
}
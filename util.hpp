#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace util 
{
	void PrintVec3(const glm::vec3& vec3);
	void PrintMat3(const glm::mat3& mat3);
	void PrintQuat(const glm::quat& quat);
}

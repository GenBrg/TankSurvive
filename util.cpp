#include "util.hpp"

#include <iostream>

namespace util 
{
	void PrintVec3(const glm::vec3& vec3)
	{
		for (int i = 0; i < 3; ++i) {
			std::cout << vec3[i] << " ";
		}
		std::cout << std::endl;
	}

	void PrintMat3(const glm::mat3& mat3)
	{
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				std::cout << mat3[j][i] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}
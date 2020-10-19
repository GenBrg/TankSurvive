#pragma once

#include "Scene.hpp"

#include <glm/glm.hpp>

class TankShell {
	public:
		TankShell(const glm::vec3& initial_pos, const glm::vec3& initial_velocity);
		bool Update(float elapsed);
		void Draw(Scene& scene);

	private:
		Scene::Transform transform_;
		Scene::Transform rotation_;
		Scene::Drawable drawable_;
		glm::vec3 velocity_;
};

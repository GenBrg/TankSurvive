#pragma once

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>

class Tank;

class TankShell {
	public:
		TankShell(const glm::vec3& initial_pos, const glm::vec3& initial_velocity);
		void Update(float elapsed);
		void Draw(Scene& scene);
		bool IsColliding(const std::vector<Tank*>& tanks);
		glm::vec3 GetPosition() const { return transform_.position; }

	private:
		Scene::Transform transform_;
		Scene::Transform rotation_;
		Scene::Drawable drawable_;
		glm::vec3 velocity_;
};

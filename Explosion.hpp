#pragma once

#include "Scene.hpp"

class Tank;

class Explosion {
	public:
		Explosion(const glm::vec3& pos, Scene& scene, float power, float radius);
		bool Update(float elapsed);
		void Draw();
		void ApplyDamage(std::vector<Tank*>& tanks);

	private:
		inline static constexpr float kExplodeSpeed { 15.0f };
		float scale_ { 1.0f };
		float power_;
		float radius_;

		Scene& scene_;
		Scene::Drawable drawable_;
		Scene::Transform transform_;
};

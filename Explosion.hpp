#pragma once

#include "Scene.hpp"

class Explosion {
	public:
		Explosion(const glm::vec3& pos, Scene& scene);
		bool Update(float elapsed);
		void Draw();

	private:
		inline static constexpr float kExplodeScale { 5.0f };
		inline static constexpr float kExplodeSpeed { 15.0f };
		float scale_ { 1.0f };

		Scene& scene_;
		Scene::Drawable drawable_;
		Scene::Transform transform_;
};

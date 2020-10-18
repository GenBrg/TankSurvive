#pragma once

#include "Scene.hpp"
#include "WalkMesh.hpp"

class Tank {
	private:
		Scene::Drawable tank_body_;
		Scene::Transform root_transform_;
		Scene::Transform root_rotation_;
		float body_yaw_ { 0.0f };
		WalkPoint at_;
		
		Scene::Drawable tank_turret_;
		Scene::Transform turret_rotation_;
		float turret_yaw_ { 0.0f };

		Scene::Drawable tank_barrel_;
		Scene::Transform barrel_pitch_;
		Scene& scene_;

		float velocity_ {0.0f};
		float rotation_force_ {0.0f};
		float external_force_ {0.0f};

		void SetPosition();

	public:
		inline static constexpr float kMaxSpeed = 10.0f;
		inline static constexpr float kFraction = 5.0f;

		Tank(Scene& scene, const glm::vec3& initial_pos);

		void TargetTurret(const glm::vec3& target_pos);
		void AdjustBarrel();
		
		void MoveForward() { external_force_ = 10.0f; }
		void MoveBackward() { external_force_ = -10.0f; }
		void TurnLeft() { rotation_force_ = glm::radians(20.0f); }
		void TurnRight() { rotation_force_ = glm::radians(-20.0f); }
		void SlideTurnLeft() { rotation_force_ = (velocity_ >= 0.0f) ? glm::radians(20.0f) : glm::radians(-20.0f); }
		void SlideTurnRight() { rotation_force_ = (velocity_ >= 0.0f) ? glm::radians(-20.0f) : glm::radians(20.0f); }

		void AttachCamera(Scene::Camera* camera);

		void Fire();

		void Update(float elapsed);
		void Draw();

		glm::vec3 GetUpVector() const { return walkmesh->to_world_smooth_normal(at_); }
		glm::vec3 GetFaceVector() const;
};

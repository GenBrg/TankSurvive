#pragma once

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "CollisionDetection.hpp"

#include <chrono>

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
		Scene::Transform barrel_rotation_;
		float barrel_pitch_ { 0.0f };
		Scene& scene_;

		float velocity_ {0.0f};
		float rotation_force_ {0.0f};
		float external_force_ {0.0f};

		float target_turret_yaw_ { 0.0f };
		float target_barrel_pitch_ { 0.0f };

		float hp_ { kMaxHP };

		std::chrono::high_resolution_clock::time_point last_fire_time;

	public:
		inline static constexpr float kMaxSpeed = 10.0f;
		inline static constexpr float kFraction = 5.0f;
		inline static constexpr std::chrono::milliseconds kFireCoolDown{ 3000 };
		inline static constexpr float kTurretRotationSpeed = glm::radians(20.0f);
		inline static constexpr float kBarrelRotationSpeed = glm::radians(15.0f);
		inline static constexpr float kMaxBarrelPitch = glm::radians(15.0f);
		inline static constexpr float kMinBarrelPitch = glm::radians(0.0f);
		inline static constexpr float kMaxHP = 100.0f;

		Tank(Scene& scene, const glm::vec3& initial_pos, bool is_player);

		void TargetTurret(float target_yaw) { target_turret_yaw_ = target_yaw; }
		void AdjustBarrel(float target_pitch) { target_barrel_pitch_ = glm::clamp(target_pitch, kMinBarrelPitch, kMaxBarrelPitch); }
		
		void MoveForward() { external_force_ = 10.0f; }
		void MoveBackward() { external_force_ = -10.0f; }
		void TurnLeft() { rotation_force_ = glm::radians(20.0f); }
		void TurnRight() { rotation_force_ = glm::radians(-20.0f); }
		void SlideTurnLeft() { rotation_force_ = (velocity_ >= 0.0f) ? glm::radians(20.0f) : glm::radians(-20.0f); }
		void SlideTurnRight() { rotation_force_ = (velocity_ >= 0.0f) ? glm::radians(-20.0f) : glm::radians(20.0f); }

		void AttachCamera(Scene::Camera* camera);

		void Fire(float initial_speed);
		bool CanFire() const;

		void Update(float elapsed);
		void Draw();

		void CollisionResolution(Tank* tank);
		bool IsPointInTank(const glm::vec3& point);
		void ApplyDamage(const glm::vec3& origin, float power, float radius);

		glm::vec3 GetUpVector() const { return walkmesh->to_world_smooth_normal(at_); }
		glm::vec3 GetFaceVector() const;
		glm::vec3 GetPosition() const { return root_transform_.position; }
		ConvexPolygon GetBoundingBox() const;
		float GetHp() const { return hp_; }
		float GetHpPercentage() const { return hp_ / kMaxHP; }
		float GetBodyYaw() const { return body_yaw_; }
		float GetTurretYaw() const { return turret_yaw_; }
		float GetTotalYaw() const { return body_yaw_ + turret_yaw_; }
		float GetBarrelPitch() const { return barrel_pitch_; }
		glm::vec3 GetFirePosition() const;

		void Reset(const glm::vec3& initial_pos);
};

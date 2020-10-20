#include "TankAI.hpp"

#include <stdexcept>
#include <iostream>
#include <algorithm>

TankAI::TankAI(Scene& scene, const glm::vec3& initial_pos) :
tank_(scene, initial_pos, false),
scene_(scene)
{}

void TankAI::Update(float elapsed, const glm::vec3& player_pos)
{
	switch (state_) {
		case State::MOVE:
			state_ = State::AIM_AND_FIRE;
		break;
		case State::AIM_AND_FIRE: {
			constexpr float barrel_pitch { glm::radians(5.0f) };

			// Move Turret and Barrel

			tank_.AdjustBarrel(barrel_pitch);

			float target_yaw = CalcYaw(tank_.GetPosition(), player_pos) - tank_.GetBodyYaw();
			
			if (std::abs(target_yaw - tank_.GetTurretYaw()) >= glm::radians(3.0f)) {
				tank_.TargetTurret(target_yaw);
			} else if (tank_.CanFire()) {
				float shell_speed = CalcShellSpeed(tank_.GetFirePosition(), player_pos, tank_.GetBarrelPitch());
				if (shell_speed > 0.0f) {
					tank_.Fire(shell_speed);
					state_ = State::MOVE;
				}
			}

			break;
		}
			
		default:
		throw std::runtime_error("Unknown state");
	}

	tank_.Update(elapsed);
}

void TankAI::Draw()
{
	tank_.Draw();
}

float TankAI::CalcYaw(const glm::vec2& pos1, const glm::vec2& pos2)
{
	auto dir = pos2 - pos1;
	return std::atan2(dir.y, dir.x) - glm::radians(90.0f);
}

float TankAI::CalcShellSpeed(const glm::vec3& src_pos, const glm::vec3& dest_pos, float pitch)
{
	constexpr float gravity = 9.8f;
	glm::vec2 dir2D = dest_pos - src_pos;
	float dist = glm::length(dir2D);
	glm::vec2 target_pos {dist, dest_pos.z - src_pos.z};

	auto factor1 = target_pos.x * glm::tan(pitch) - target_pos.y;
	if (factor1 < 0)
		return -1.0f;
	float fly_time = std::sqrt((2.0f * factor1) / gravity);
	return target_pos.x / (glm::cos(pitch) * fly_time);
}

		
		
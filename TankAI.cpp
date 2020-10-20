#include "TankAI.hpp"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <random>

TankAI::TankAI(Scene& scene, const glm::vec3& initial_pos) :
tank_(scene, initial_pos, false)
{
	OnMoveEnter();
}

void TankAI::Update(float elapsed, const glm::vec3& player_pos)
{
	constexpr float barrel_pitch { glm::radians(5.0f) };
	switch (state_) {
		case State::MOVE: {
			sub_movement_.Update(elapsed, tank_);

			tank_.AdjustBarrel(barrel_pitch);
			float target_yaw = CalcYaw(tank_.GetPosition(), player_pos) - tank_.GetBodyYaw();
			
			if (std::abs(target_yaw - tank_.GetTurretYaw()) >= glm::radians(3.0f)) {
				tank_.TargetTurret(target_yaw);
			}

			move_time_ -= elapsed;
			if (move_time_ <= 0.0f) {
				state_ = State::AIM_AND_FIRE;
			}
			break;
		}
		case State::AIM_AND_FIRE: {
			// Move Turret and Barrel
			tank_.AdjustBarrel(barrel_pitch);
			float target_yaw = CalcYaw(tank_.GetPosition(), player_pos) - tank_.GetBodyYaw();
			
			if (std::abs(target_yaw - tank_.GetTurretYaw()) >= glm::radians(3.0f)) {
				tank_.TargetTurret(target_yaw);
			} else if (tank_.CanFire()) {
				float shell_speed = CalcShellSpeed(tank_.GetFirePosition(), player_pos, tank_.GetBarrelPitch());
				if (shell_speed > 0.0f) {
					tank_.Fire(shell_speed);
					OnMoveEnter();
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

void TankAI::OnMoveEnter()
{
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> move_time_dist(3.0f, 10.0f);

	move_time_ = move_time_dist(mt);
	sub_movement_.Initialize();
}
		

void TankAI::SubMovement::Update(float elapsed, Tank& tank)
{
	time_ -= elapsed;
	if (time_ < 0.0f) {
		Initialize();
	}

	if (move_forward_) {
		tank.MoveForward();
	}

	if (move_backward_) {
		tank.MoveBackward();
	}

	if (turn_left_) {
		tank.TurnLeft();
	}

	if (turn_right_) {
		tank.TurnRight();
	}
}

void TankAI::SubMovement::Initialize()
{
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_int_distribution<int> dice_dist(0, 2);
	static std::uniform_real_distribution<float> time_dist(1.0f, 3.0f);

	time_ = time_dist(mt);

	int dice = dice_dist(mt);
	switch (dice) {
		case 0:
		turn_left_ = false;
		turn_right_ = false;
		break;
		case 1:
		turn_left_ = true;
		turn_right_ = false;
		break;
		case 2:
		turn_left_ = false;
		turn_right_ = true;
		break;
	}

	dice = dice_dist(mt);
	switch (dice) {
		case 0:
		move_forward_ = false;
		move_backward_ = false;
		break;
		case 1:
		move_forward_ = true;
		move_backward_ = false;
		break;
		case 2:
		move_forward_ = false;
		move_backward_ = true;
		break;
	}
}

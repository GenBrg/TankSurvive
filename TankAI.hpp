#pragma once

#include "Tank.hpp"

class TankAI {
public:
	TankAI(Scene& scene, const glm::vec3& initial_pos);
	void Update(float elapsed, const glm::vec3& player_pos);
	void Draw();

	Tank* GetTank() { return &tank_; }

private:
	Tank tank_;
	Scene& scene_;

	float move_time_ { 0.0f };

	struct SubMovement {
		float time_;
		bool turn_right_;
		bool turn_left_;
		bool move_forward_;
		bool move_backward_;

		void Update(float elapsed, Tank& tank);
		void Initialize();
	} sub_movement_;

	enum class State : uint8_t {
		MOVE = 0,
		AIM_AND_FIRE,
	} state_ { State::MOVE };

	static float CalcYaw(const glm::vec2& pos1, const glm::vec2& pos2);
	static float CalcShellSpeed(const glm::vec3& src_pos, const glm::vec3& dest_pos, float pitch);

	void OnMoveEnter();
};
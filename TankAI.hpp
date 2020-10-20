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

	enum class State : uint8_t {
		MOVE = 0,
		AIM_AND_FIRE,
	} state_ { State::MOVE };

	static float CalcYaw(const glm::vec2& pos1, const glm::vec2& pos2);
	static float CalcShellSpeed(const glm::vec3& src_pos, const glm::vec3& dest_pos, float pitch);
};
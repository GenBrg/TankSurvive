#pragma once

#include "Tank.hpp"

class TankAI {
public:
	TankAI(Scene& scene, const glm::vec3& initial_pos);
	void Update(float elapsed);
	void Draw();

	Tank* GetTank() { return &tank_; }

private:
	Tank tank_;

	enum class State : uint8_t {
		MOVE = 0,
		AIM_AND_FIRE,
	} state_;
};
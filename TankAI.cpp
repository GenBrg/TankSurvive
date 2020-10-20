#include "TankAI.hpp"

TankAI::TankAI(Scene& scene, const glm::vec3& initial_pos) :
tank_(scene, initial_pos, false)
{}

void TankAI::Update(float elapsed)
{
	tank_.Update(elapsed);
}

void TankAI::Draw()
{
	tank_.Draw();
}

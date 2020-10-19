#include "TankShell.hpp"
#include "LitColorTextureProgram.hpp"

#include "util.hpp"

#include <glm/gtx/quaternion.hpp>

constexpr glm::vec3 kGravity { 0.0f, 0.0f, -9.8f };

TankShell::TankShell(const glm::vec3& initial_pos, const glm::vec3& initial_velocity) :
velocity_(initial_velocity)
{
	drawable_.pipeline = lit_color_texture_program_pipeline;
	drawable_.pipeline.mesh = &tank_survive_meshes->lookup("TankShell");
	rotation_.parent = &transform_;
	drawable_.transform = &rotation_;
	transform_.position = initial_pos;
	rotation_.rotation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(velocity_));
}

bool TankShell::Update(float elapsed)
{
	glm::vec3 old_velocity = velocity_;
	velocity_ += kGravity * elapsed;

	// Update shell location
	transform_.position += elapsed * (old_velocity + velocity_) / 2.0f;

	// Update shell rotation
	rotation_.rotation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(velocity_));

	// Collision detection
	// Hit ground
	if (transform_.position.z <= 0.0f) {
		return true;
	}

	// Hit mesh

	// Hit tank

	return false;
}

void TankShell::Draw(Scene& scene)
{
	scene.dynamic_drawables.emplace_back(drawable_);
}
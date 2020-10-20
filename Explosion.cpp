#include "Explosion.hpp"
#include "LitColorTextureProgram.hpp"

Explosion::Explosion(const glm::vec3& pos, Scene& scene) :
scene_(scene)
{
	drawable_.pipeline = lit_color_texture_program_pipeline;
	drawable_.pipeline.mesh = &tank_survive_meshes->lookup("Explosion");
	drawable_.transform = &transform_;

	transform_.position = pos;
}

bool Explosion::Update(float elapsed)
{
	bool finished = false;
	scale_ += elapsed * kExplodeSpeed;

	if (scale_ >= kExplodeScale) {
		scale_ = kExplodeScale;
		finished = true;
	}

	transform_.scale = glm::vec3(scale_);

	return finished;
}

void Explosion::Draw()
{
	scene_.dynamic_drawables.emplace_back(drawable_);
}

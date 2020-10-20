#include "Explosion.hpp"
#include "LitColorTextureProgram.hpp"

#include "Tank.hpp"

Explosion::Explosion(const glm::vec3& pos, Scene& scene, float power, float radius) :
scene_(scene),
power_(power),
radius_(radius)
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

	if (scale_ >= radius_) {
		scale_ = radius_;
		finished = true;
	}

	transform_.scale = glm::vec3(scale_);

	return finished;
}

void Explosion::Draw()
{
	scene_.dynamic_drawables.emplace_back(drawable_);
}

void Explosion::ApplyDamage(std::vector<Tank*>& tanks)
{
	for (Tank* tank : tanks) {
		tank->ApplyDamage(transform_.position, power_, radius_);
	}
}
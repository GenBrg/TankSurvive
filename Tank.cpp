#include "Tank.hpp"
#include "LitColorTextureProgram.hpp"
#include "TankShell.hpp"
#include "util.hpp"

#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <iostream>

Tank::Tank(Scene& scene, const glm::vec3& initial_pos) :
scene_(scene)
{
	tank_body_.pipeline = lit_color_texture_program_pipeline;
	tank_body_.pipeline.mesh = &tank_survive_meshes->lookup("TankBody");
	root_rotation_.parent = &root_transform_;
	tank_body_.transform = &root_rotation_;

	tank_turret_.pipeline = lit_color_texture_program_pipeline;
	tank_turret_.pipeline.mesh = &tank_survive_meshes->lookup("TankTurret");
	turret_rotation_.parent = &root_rotation_;
	turret_rotation_.position = glm::vec3(0.0f, -0.2f, 1.5f);
	tank_turret_.transform = &turret_rotation_;

	tank_barrel_.pipeline = lit_color_texture_program_pipeline;
	tank_barrel_.pipeline.mesh = &tank_survive_meshes->lookup("TankBarrel");
	barrel_rotation_.parent = &turret_rotation_;
	barrel_rotation_.position = glm::vec3(0.0f, 0.4f, 0.0f);
	tank_barrel_.transform = &barrel_rotation_;

	at_ = walkmesh->nearest_walk_point(initial_pos);
	root_transform_.position = walkmesh->to_world_point(at_);
}

void Tank::Draw() {
	scene_.dynamic_drawables.emplace_back(tank_body_);
	scene_.dynamic_drawables.emplace_back(tank_turret_);
	scene_.dynamic_drawables.emplace_back(tank_barrel_);
}

void Tank::AttachCamera(Scene::Camera* camera)
{
	assert(camera);
	camera->transform->parent = &root_rotation_;
	camera->transform->position = glm::vec3(0.0f, 0.0f, 2.5f);

	camera->up = walkmesh->to_world_smooth_normal(at_);
	camera->yaw = 0.0f;
	camera->pitch = 0.0f;
	camera->UpdateCameraRotation();
}

glm::vec3 Tank::GetFaceVector() const
{
	glm::vec3 up = GetUpVector();
	auto rotation = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), up);
	return glm::angleAxis(body_yaw_, up) * rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void Tank::Update(float elapsed)
{
	float force = external_force_;
	float old_velocity = velocity_;

	// Apply fraction
	if (force == 0.0f) {
		if (velocity_ > 0.0f) {
			velocity_ = std::max(0.0f, velocity_ - kFraction * elapsed);
		} else if (velocity_ < 0.0f) {
			velocity_ = std::min(0.0f, velocity_ + kFraction * elapsed);
		}
	} else {
		velocity_ += force * elapsed;
	}

	body_yaw_ += rotation_force_ * elapsed;
	velocity_ = glm::clamp(velocity_, -kMaxSpeed, kMaxSpeed);

	// Move
	glm::vec3 step = GetFaceVector() * ((velocity_ + old_velocity) / 2.0f) * elapsed;
	if (walkmesh->walk(at_, step))
	{
		velocity_ = 0.0f;
	}

	root_transform_.position = walkmesh->to_world_point(at_);

	// Turn body
	root_rotation_.rotation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), GetFaceVector());

	external_force_ = 0.0f;
	rotation_force_ = 0.0f;

	// Turn turret
	float turn_amount = kTurretRotationSpeed * elapsed;
	if (std::abs(target_turret_yaw_ - turret_yaw_) <= turn_amount) {
		turret_yaw_ = target_turret_yaw_;
	} else {
		if (target_turret_yaw_ > turret_yaw_) {
			turret_yaw_ += turn_amount;
		} else {
			turret_yaw_ -= turn_amount;
		}
	}

	turret_rotation_.rotation = glm::angleAxis(turret_yaw_, glm::vec3(0.0f, 0.0f, 1.0f));

	// Turn barrel
	turn_amount = kBarrelRotationSpeed * elapsed;
	if (std::abs(barrel_pitch_ - target_barrel_pitch_) <= turn_amount) {
		barrel_pitch_ = target_barrel_pitch_;
	} else {
		if (target_barrel_pitch_ > barrel_pitch_) {
			barrel_pitch_ += turn_amount;
		} else {
			barrel_pitch_ -= turn_amount;
		}
	}

	barrel_rotation_.rotation = glm::angleAxis(barrel_pitch_, glm::vec3(1.0f, 0.0f, 0.0f));
}

bool Tank::CanFire() const
{
	return std::chrono::high_resolution_clock::now() - last_fire_time > kFireCoolDown;
}

void Tank::Fire(float initial_speed)
{
	if (CanFire()) {
		last_fire_time = std::chrono::high_resolution_clock::now();
	} else {
		return;
	}

	auto model_mat = barrel_rotation_.make_local_to_world();
	glm::vec3 fire_position = model_mat * glm::vec4(0.0f, 3.0f, 0.0f, 1.0f);
	glm::vec3 velocity = initial_speed * glm::normalize(fire_position - model_mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	
	std::cout << "Fire: " << initial_speed << std::endl;

	scene_.tank_shells.emplace_back(new TankShell(fire_position, velocity));
}

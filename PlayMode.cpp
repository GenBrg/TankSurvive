#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "util.hpp"
#include "TankShell.hpp"
#include "Explosion.hpp"
#include "TankAI.hpp"
#include "Sound.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

Load<Sound::Sample> explode_sfx_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("explosion.wav"));
});

PlayMode::PlayMode() : scene(*tank_survive_scene),
					   player(kPlayerInitialPos, scene)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::vector<glm::vec3> generated_positions(3);
	generated_positions.push_back(kPlayerInitialPos);

	for (int i = 0; i < 2; ++i)
	{
		glm::vec3 spawn_pos;

		while (true)
		{
			float x = std::uniform_real_distribution<float>(-70.0f, 40.0f)(mt);
			float y = std::uniform_real_distribution<float>(-50.0f, 30.0f)(mt);
			WalkPoint wp = walkmesh->nearest_walk_point(glm::vec3(x, y, 0.0f));
			spawn_pos = walkmesh->to_world_point(wp);

			for (const auto &generated_position : generated_positions)
			{
				if (glm::distance(generated_position, spawn_pos) < 20.0f)
				{
					continue;
				}
			}
			generated_positions.push_back(spawn_pos);
			break;
		}

		scene.enemy_tanks.push_back(new TankAI(scene, spawn_pos));
	}
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (SDL_GetRelativeMouseMode() == SDL_FALSE)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		else if (evt.button.button == SDL_BUTTON_LEFT && player.tank_.CanFire())
		{
			isPoweringUp = true;
			player.power_ = 0.0f;
		}
		return true;
	}
	else if (evt.type == SDL_MOUSEBUTTONUP)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE && evt.button.button == SDL_BUTTON_LEFT && isPoweringUp)
		{
			isPoweringUp = false;
			player.tank_.Fire(20.0f + player.power_ * 100.0f);
		}
		return true;
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y));

			player.camera->yaw = glm::clamp(player.camera->yaw - motion.x * player.camera->fovy, glm::radians(-179.0f), glm::radians(179.0f));
			;
			player.camera->pitch = glm::clamp(player.camera->pitch + motion.y * player.camera->fovy, glm::radians(-45.0f), glm::radians(45.0f));
			player.camera->up = player.tank_.GetUpVector();
			player.camera->UpdateCameraRotation();
			player.tank_.TargetTurret(player.camera->yaw);
			player.tank_.AdjustBarrel(player.camera->pitch);

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{
	if (left.pressed && !right.pressed)
	{
		if (!down.pressed && !up.pressed)
		{
			player.tank_.SlideTurnLeft();
		}
		else
		{
			(down.pressed && !up.pressed) ? player.tank_.TurnRight() : player.tank_.TurnLeft();
		}
	}

	if (!left.pressed && right.pressed)
	{
		if (!down.pressed && !up.pressed)
		{
			player.tank_.SlideTurnRight();
		}
		else
		{
			(down.pressed && !up.pressed) ? player.tank_.TurnLeft() : player.tank_.TurnRight();
		}
	}

	if (down.pressed && !up.pressed)
		player.tank_.MoveBackward();
	if (!down.pressed && up.pressed)
		player.tank_.MoveForward();

	for (auto it = scene.explosions.begin(); it != scene.explosions.end();)
	{
		if ((*it)->Update(elapsed))
		{
			auto tmp = it;
			++it;
			delete *tmp;
			scene.explosions.erase(tmp);
		}
		else
		{
			++it;
		}
	}

	std::vector<Tank*> all_tanks;

	player.tank_.Update(elapsed);
	all_tanks.push_back(&player.tank_);

	for (auto enemy_tank : scene.enemy_tanks)
	{
		enemy_tank->Update(elapsed, player.tank_.GetPosition());
		all_tanks.push_back(enemy_tank->GetTank());
	}

	// Solve collision
	for (auto enemy_tank : scene.enemy_tanks)
	{
		player.tank_.CollisionResolution(enemy_tank->GetTank());
	}

	for (auto it = scene.enemy_tanks.begin(); it != scene.enemy_tanks.end(); ++it)
	{
		for (auto it2 = it; it2 != scene.enemy_tanks.end(); ++it2)
		{
			if (it2 != it)
			{
				(*it)->GetTank()->CollisionResolution((*it2)->GetTank());
			}
		}
	}

	for (auto it = scene.tank_shells.begin(); it != scene.tank_shells.end();)
	{
		(*it)->Update(elapsed);
		if ((*it)->IsColliding(all_tanks))
		{
			Explosion* explosion = new Explosion((*it)->GetPosition(), scene, 20.0f, 5.0f);
			explosion->ApplyDamage(all_tanks);
			scene.explosions.emplace_back(explosion);
			Sound::play_3D(*explode_sfx_sample, 20.0f, (*it)->GetPosition(), 30.0f);
			auto tmp = it;
			++it;
			delete *tmp;
			scene.tank_shells.erase(tmp);
		}
		else
		{
			++it;
		}
	}

	// Destroy tanks
	for (auto it = scene.enemy_tanks.begin(); it != scene.enemy_tanks.end();)
	{
		if ((*it)->GetTank()->GetHp() <= 0.0f)
		{
			Explosion* explosion = new Explosion((*it)->GetTank()->GetPosition(), scene, 20.0f, 10.0f);
			auto dd = std::remove(all_tanks.begin(), all_tanks.end(), (*it)->GetTank());
			explosion->ApplyDamage(all_tanks);
			Sound::play_3D(*explode_sfx_sample, 100.0f, (*it)->GetTank()->GetPosition(), 50.0f);
			scene.explosions.emplace_back(explosion);
			auto tmp = it;
			++it;
			delete *tmp;
			scene.enemy_tanks.erase(tmp);
		}
		else
		{
			++it;
		}
	}

	Sound::listener.set_position_right(player.tank_.GetPosition(), glm::normalize(glm::cross(player.tank_.GetFaceVector(), player.tank_.GetUpVector())));

	if (isPoweringUp)
	{
		player.PowerUp(elapsed);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.53f, 0.8f, 0.98f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	player.tank_.Draw();

	scene.draw(*player.camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;
		constexpr glm::vec4 red{1.0f, 0.0f, 0.0f, 1.0f};
		constexpr glm::vec4 green{0.0f, 1.0f, 0.0f, 1.0f};

		// Power bar
		if (isPoweringUp)
		{
			lines.draw_text("Power Bar: ",
							glm::vec3(0.5f * aspect, -1.0 + 0.1f * H, 0.0),
							glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
							glm::u8vec4(0x00, 0x00, 0x00, 0x00));

			lines.draw_text("Power Bar: ",
							glm::vec3(0.5f * aspect + ofs, -1.0 + +0.1f * H + ofs, 0.0),
							glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
							glm::u8vec4(0xff, 0xff, 0xff, 0x00));

			glm::vec4 color = red + (green - red) * player.power_;
			lines.draw_quad(glm::vec4(aspect * 0.72f, -0.92f, aspect * (0.72f + 0.2f * player.power_), -0.98f), color * 255.0f);
		}

		lines.draw_text("HP: ",
						glm::vec3(-0.5f * aspect, 0.8f, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));

		lines.draw_text("HP: ",
						glm::vec3(-0.5f * aspect + ofs, 0.8f + ofs, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		glm::vec4 color = red + (green - red) * player.tank_.GetHpPercentage();
		lines.draw_quad(glm::vec4(aspect * -0.35f, 0.83f, aspect * (-0.4f + 0.35f * player.tank_.GetHpPercentage()), 0.88f), color * 255.0f);
	}
	GL_ERRORS();
}

PlayMode::Player::Player(const glm::vec3 &initial_pos, Scene &scene) : tank_(scene, initial_pos, true)
{
	// Create camera
	scene.transforms.emplace_back();
	scene.cameras.emplace_back(&scene.transforms.back());
	camera = &scene.cameras.back();

	// Attach the camera to tank
	tank_.AttachCamera(camera);
}

void PlayMode::Player::PowerUp(float elapsed)
{
	static float inc = 0.5f;

	power_ += inc * elapsed;

	if (power_ > 1.0f)
	{
		power_ -= power_ - 1.0f;
		inc = -0.5f;
	}
	else if (power_ < 0.0f)
	{
		power_ = -power_;
		inc = 0.5f;
	}
}

void PlayMode::Initialize()
{

}

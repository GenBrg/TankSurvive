#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "util.hpp"
#include "TankShell.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

PlayMode::PlayMode() : 
scene(*tank_survive_scene),
player(kPlayerInitialPos, scene)
{
	//create a player transform:
	// scene.transforms.emplace_back();
	// player.transform = &scene.transforms.back();

	//create a player camera attached to a child of the player transform:
	// scene.transforms.emplace_back();
	// scene.cameras.emplace_back(&scene.transforms.back());
	// player.camera = &scene.cameras.back();
	// player.camera->fovy = glm::radians(60.0f);
	// player.camera->near = 0.01f;
	// player.camera->transform->parent = player.transform;

	//player's eyes are 1.8 units above the ground:
	// player.camera->transform->position = glm::vec3(0.0f, 0.0f, 5.0f);

	//rotate camera facing direction (-z) to player facing direction (+y):
	// player.camera->transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//start player walking at nearest walk point:
	// player.at = walkmesh->nearest_walk_point(player.transform->position);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		} else if (evt.button.button == SDL_BUTTON_LEFT && player.tank_.CanFire()) {
			isPoweringUp = true;
			player.power_ = 0.0f;
		}
		return true;
	} else if (evt.type == SDL_MOUSEBUTTONUP) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE && evt.button.button == SDL_BUTTON_LEFT && isPoweringUp) {
			isPoweringUp = false;
			player.tank_.Fire(20.0f + player.power_ * 100.0f);
		}
		return true;
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);

			player.camera->yaw = glm::clamp(player.camera->yaw - motion.x * player.camera->fovy, glm::radians(-179.0f), glm::radians(179.0f));;
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

void PlayMode::update(float elapsed) {
	if (left.pressed && !right.pressed) {
		if (!down.pressed && !up.pressed) {
			player.tank_.SlideTurnLeft();
		} else {
			(down.pressed && !up.pressed) ? player.tank_.TurnRight() : player.tank_.TurnLeft();
		}
	}
	
	if (!left.pressed && right.pressed) {
		if (!down.pressed && !up.pressed) {
			player.tank_.SlideTurnRight();
		} else {
			(down.pressed && !up.pressed) ? player.tank_.TurnLeft() : player.tank_.TurnRight();
		}
	}

	if (down.pressed && !up.pressed) player.tank_.MoveBackward();
	if (!down.pressed && up.pressed) player.tank_.MoveForward();

	player.tank_.Update(elapsed);

	for (auto it = scene.tank_shells.begin(); it != scene.tank_shells.end();) {
		if ((*it)->Update(elapsed)) {
			auto tmp = it;
			++it;
			delete *tmp;
			scene.tank_shells.erase(tmp);
		} else {
			++it;
		}
	}

	if (isPoweringUp) {
		player.PowerUp(elapsed);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
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
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;

		// Power bar
		if (isPoweringUp) {
			lines.draw_text("Power Bar: ",
			glm::vec3(0.5f * aspect, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		
			lines.draw_text("Power Bar: ",
			glm::vec3(0.5f * aspect + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

			constexpr glm::vec4 red { 1.0f, 0.0f, 0.0f, 1.0f };
			constexpr glm::vec4 green { 0.0f, 1.0f, 0.0f, 1.0f };
			glm::vec4 color = red + (green - red) * player.power_;
			lines.draw_quad(glm::vec4(aspect * 0.72f, -0.92f, aspect * (0.72f + 0.2f * player.power_), -0.98f), color * 255.0f);
		}

		// glm::u8vec4 white_color { 0xff, 0xff, 0xff, 0xff };

		// for (const auto& triangle : walkmesh->triangles) {
		// 	auto& camera = *(scene.cameras.begin());
		// 	glm::mat4 vp = camera.make_projection() * glm::mat4(camera.transform->make_world_to_local());
		// 	glm::vec3 a = vp * glm::vec4(walkmesh->vertices[triangle[0]], 0);
		// 	glm::vec3 b = vp * glm::vec4(walkmesh->vertices[triangle[1]], 0);
		// 	glm::vec3 c = vp * glm::vec4(walkmesh->vertices[triangle[2]], 0);

		// 	util::PrintVec3(a);

		// 	lines.draw(a, b, white_color);
		// 	lines.draw(b, c, white_color);
		// 	lines.draw(c, a, white_color);
		// }
	}
	GL_ERRORS();
}

PlayMode::Player::Player(const glm::vec3& initial_pos, Scene& scene) :
tank_(scene, initial_pos)
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

	if (power_ > 1.0f) {
		power_ -= power_ - 1.0f;
		inc = -0.5f;
	} else if (power_ < 0.0f) {
		power_ = - power_;
		inc = 0.5f;
	}
}

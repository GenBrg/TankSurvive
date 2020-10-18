#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "util.hpp"

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
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);

			// player.camera->transform->rotation = glm::normalize(
			// 	player.camera->transform->rotation
			// 	* glm::angleAxis(-motion.x * player.camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
			// 	* glm::angleAxis(motion.y * player.camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			// );

			player.camera->yaw = glm::clamp(player.camera->yaw - motion.x * player.camera->fovy, glm::radians(-179.0f), glm::radians(179.0f));;
			player.camera->pitch = glm::clamp(player.camera->pitch + motion.y * player.camera->fovy, glm::radians(-45.0f), glm::radians(45.0f));
			player.camera->up = player.tank_.GetUpVector();
			player.camera->UpdateCameraRotation();
			player.tank_.TargetTurret(player.camera->yaw);
			player.tank_.AdjustBarrel(player.camera->pitch);
			// glm::vec3 up = player.tank_.GetUpVector();
			// player.camera->transform->rotation = glm::angleAxis(-motion.x * player.camera->fovy, up) * player.camera->transform->rotation;

			// float pitch = glm::pitch(player.camera->transform->rotation);
			// pitch += motion.y * player.camera->fovy;
			//camera looks down -z (basically at the player's feet) when pitch is at zero.
			// pitch = std::min(pitch, 0.95f * 3.1415926f);
			// pitch = std::max(pitch, 0.05f * 3.1415926f);
			// player.camera->transform->rotation = player.camera->transform->rotation * glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	//player walking:
	// {
	// 	//combine inputs into a move:
	// 	constexpr float PlayerSpeed = 20.0f;
	// 	glm::vec2 move = glm::vec2(0.0f);
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

	// 	//make it so that moving diagonally doesn't go faster:
	// 	if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

	// 	//get move in world coordinate system:
	// 	glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

	// 	//using a for() instead of a while() here so that if walkpoint gets stuck in
	// 	// some awkward case, code will not infinite loop:
	// 	for (uint32_t iter = 0; iter < 10; ++iter) {
	// 		if (remain == glm::vec3(0.0f)) break;
	// 		WalkPoint end;
	// 		float time;
	// 		walkmesh->walk_in_triangle(player.at, remain, &end, &time);
	// 		player.at = end;

	// 		if (time == 1.0f) {
	// 			//finished within triangle:
	// 			remain = glm::vec3(0.0f);
	// 			break;
	// 		}
	// 		//some step remains:
	// 		remain *= (1.0f - time);
	// 		//try to step over edge:
	// 		glm::quat rotation;
	// 		if (walkmesh->cross_edge(player.at, &end, &rotation)) {
	// 			//stepped to a new triangle:
	// 			player.at = end;
	// 			//rotate step to follow surface:
	// 			remain = rotation * remain;
	// 		} else {
	// 			//ran into a wall, bounce / slide along it:
	// 			glm::vec3 const &a = walkmesh->vertices[player.at.indices.x];
	// 			glm::vec3 const &b = walkmesh->vertices[player.at.indices.y];
	// 			glm::vec3 const &c = walkmesh->vertices[player.at.indices.z];
	// 			glm::vec3 along = glm::normalize(b-a);
	// 			glm::vec3 normal = glm::normalize(glm::cross(b-a, c-a));
	// 			glm::vec3 in = glm::cross(normal, along);

	// 			//check how much 'remain' is pointing out of the triangle:
	// 			float d = glm::dot(remain, in);
	// 			if (d < 0.0f) {
	// 				//bounce off of the wall:
	// 				remain += (-1.25f * d) * in;
	// 			} else {
	// 				//if it's just pointing along the edge, bend slightly away from wall:
	// 				remain += 0.01f * d * in;
	// 			}
	// 		}
	// 	}

	// 	if (remain != glm::vec3(0.0f)) {
	// 		std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
	// 	}

	// 	//update player's position to respect walking:
	// 	player.transform->position = walkmesh->to_world_point(player.at);

	// 	{ //update player's rotation to respect local (smooth) up-vector:
			
	// 		glm::quat adjust = glm::rotation(
	// 			player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
	// 			walkmesh->to_world_smooth_normal(player.at) //smoothed up vector at walk location
	// 		);
	// 		player.transform->rotation = glm::normalize(adjust * player.transform->rotation);
	// 	}

		/*
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		*/
	//}

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
		lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

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

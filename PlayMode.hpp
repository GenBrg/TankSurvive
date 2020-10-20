#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Tank.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	bool isPoweringUp { false };

	inline static constexpr glm::vec3 kPlayerInitialPos { -30.0f, 50.0f, 0.0f };

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//player info:
	struct Player {
		Tank tank_;

		float power_;

		//camera is at player's head and will be pitched by mouse up/down motion:
		Scene::Camera *camera = nullptr;

		Player(const glm::vec3& initial_pos, Scene& scene);

		void PowerUp(float elapsed);
	} player;

	void Initialize();
};

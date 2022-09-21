#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PipelineInfo {
	GLenum type = GL_TRIANGLES;
	GLuint start = 0;
	GLuint count = 0; 
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	enum GameState : uint16_t {
		START,
		IN_PROGRESS,
		END,
	};
	GameState gameState = GameState::START;
	float global_timer = 0.0f;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, up, down, inward, outward, hit;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	PipelineInfo clock_pipeline;
	PipelineInfo arm_pipeline;
	PipelineInfo gear_pipeline;
	PipelineInfo hitbox_pipeline;
	PipelineInfo note_pipeline;

	Scene::Transform *clock_transform = nullptr;
	Scene::Transform *arm_transform = nullptr;
	Scene::Transform *gear_transform = nullptr;
	Scene::Transform *hitbox_transform = nullptr;

	uint16_t score = 0;
	
	static const uint16_t num_notes = 60;
	Scene::Transform *note_transforms[num_notes];
	float note_hit_time[num_notes];
	std::vector<float> notes {
		6.0f, 
		12.0f, 14.0f, 15.0f, 17.0f, 18.0f, 20.0f, 21.0f,
		24.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 33.0f,
		36.0f, 38.0f, 39.0f, 41.0f, 42.0f, 44.0f, 45.0f,
		48.0f, 50.0f, 51.0f, 52.0f, 54.0f, 55.0f, 58.0f, 59.0f,
		60.0f, 63.0f, 65.0f, 66.0f, 69.0f,
		72.0f, 75.0f, 76.0f, 77.0f, 78.0f, 81.0f,
		84.0f, 87.0f, 90.0f, 92.0f, 93.0f, 95.0f,
		96.0f, 99.0f, 101.0f, 102.0f, 105.0f, 107.0f,
		108.0f, 109.0f, 110.0f, 111.0f, 112.0f, 113.0f, 114.0f};
	std::vector<float> notes_distance {
		0.7f, 
		0.3f, 0.4f, 0.5f, 0.8f, 0.7f, 0.5f, 0.5f,
		0.8f, 0.7f, 0.6f, 0.5f, 0.3f, 0.4f, 0.7f,
		0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.7f,
		0.6f, 0.5f, 0.5f, 0.4f, 0.3f, 0.4f, 0.5f, 0.5f,
		0.6f, 0.7f, 0.5f, 0.5f, 0.6f,
		0.7f, 0.8f, 0.8f, 0.7f, 0.6f, 0.5f,
		0.6f, 0.8f, 0.4f, 0.5f, 0.6f, 0.3f,
		0.4f, 0.6f, 0.6f, 0.7f, 0.5f, 0.4f,
		0.5f, 0.5f, 0.5f, 0.7f, 0.7f, 0.7f, 0.6f}; // [0.18, 0.84]
	virtual void hit_note();
	virtual void update_note();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > piano_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};

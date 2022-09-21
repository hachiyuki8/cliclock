#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <math.h>

const std::string CLOCK = "Clock";
const std::string ARM = "Arm";
const std::string GEAR = "Gear";
const std::string HITBOX = "Hitbox";
const std::string NOTE = "Note2";

const float HITBOX_MOVEMENT_OUTWARD_BOUND = -1.85f;
const float HITBOX_MOVEMENT_INWARD_BOUND = -0.4f;
const float HITBOX_MOVEMENT_SPEED = 1.5f;
const float NOTE_OUTWARD_BOUND = 0.84f;
const float NOTE_TO_HITBOX_RATIO = -NOTE_OUTWARD_BOUND/HITBOX_MOVEMENT_OUTWARD_BOUND;
const float MAX_NOTE_TO_HITBOX_DIST = 0.1f;

const float ROTATION_SPEED = 50.0f;
const float BEAT_SPEED = 60.0f/130;

const float NOTE_SCALE = 0.07f;
const float FADE_IN_TIME = 2.0f;
const float FADE_IN_SPEED = NOTE_SCALE/FADE_IN_TIME;
const float HOLD_TIME = 1.0f;
const float HIT_TIME = 0.2f;
const float FADE_OUT_TIME = 1.0f;
const float FADE_OUT_SPEED = NOTE_SCALE/FADE_OUT_TIME;
const float HIT_OUT_TIME = 0.3f;
const float HIT_OUT_SPEED = NOTE_SCALE/HIT_OUT_TIME;

GLuint clock_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > clock_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("clock.pnct"));
	clock_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > clock_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("clock.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = clock_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = clock_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > piano_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("untitled.opus"));
});

PlayMode::PlayMode() : scene(*clock_scene) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	for (auto &drawable : scene.drawables) {
		if (drawable.transform->name == CLOCK) {
			clock_pipeline.type = drawable.pipeline.type;
			clock_pipeline.start = drawable.pipeline.start;
			clock_pipeline.count = drawable.pipeline.count;
		} else if (drawable.transform->name == ARM) {
			arm_pipeline.type = drawable.pipeline.type;
			arm_pipeline.start = drawable.pipeline.start;
			arm_pipeline.count = drawable.pipeline.count;
		} else if (drawable.transform->name == GEAR) {
			gear_pipeline.type = drawable.pipeline.type;
			gear_pipeline.start = drawable.pipeline.start;
			gear_pipeline.count = drawable.pipeline.count;
		} else if (drawable.transform->name == HITBOX) {
			hitbox_pipeline.type = drawable.pipeline.type;
			hitbox_pipeline.start = drawable.pipeline.start;
			hitbox_pipeline.count = drawable.pipeline.count;
		} else if (drawable.transform->name == NOTE) {
			note_pipeline.type = drawable.pipeline.type;
			note_pipeline.start = drawable.pipeline.start;
			note_pipeline.count = drawable.pipeline.count;
		}
	}

	scene.drawables.clear();

	clock_transform = new Scene::Transform;
	clock_transform->name = CLOCK;
	clock_transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
	clock_transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);
	scene.drawables.emplace_back(clock_transform);
	Scene::Drawable &drawable = scene.drawables.back();
	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = clock_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = clock_pipeline.type;
	drawable.pipeline.start = clock_pipeline.start;
	drawable.pipeline.count = clock_pipeline.count;

	gear_transform = new Scene::Transform;
	gear_transform->name = GEAR;
	gear_transform->position = glm::vec3(0.0f, 0.0f, 0.03f);
	gear_transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);
	scene.drawables.emplace_back(gear_transform);
	Scene::Drawable &drawable2 = scene.drawables.back();
	drawable2.pipeline = lit_color_texture_program_pipeline;
	drawable2.pipeline.vao = clock_meshes_for_lit_color_texture_program;
	drawable2.pipeline.type = gear_pipeline.type;
	drawable2.pipeline.start = gear_pipeline.start;
	drawable2.pipeline.count = gear_pipeline.count;

	arm_transform = new Scene::Transform;
	arm_transform->name = ARM;
	arm_transform->position = glm::vec3(0.0f, 0.0f, 0.03f);
	arm_transform->rotation *= glm::angleAxis(
			glm::radians(90.0f),
			glm::vec3(1.0f, 0.0f, 0.0f)
		);
	arm_transform->scale = glm::vec3(0.01f, 0.01f, 0.45f);
	scene.drawables.emplace_back(arm_transform);
	Scene::Drawable &drawable3 = scene.drawables.back();
	drawable3.pipeline = lit_color_texture_program_pipeline;
	drawable3.pipeline.vao = clock_meshes_for_lit_color_texture_program;
	drawable3.pipeline.type = arm_pipeline.type;
	drawable3.pipeline.start = arm_pipeline.start;
	drawable3.pipeline.count = arm_pipeline.count;

	hitbox_transform = new Scene::Transform;
	hitbox_transform->name = HITBOX;
	hitbox_transform->position = glm::vec3(0.0f, 1.0f, HITBOX_MOVEMENT_OUTWARD_BOUND);
	hitbox_transform->rotation *= glm::angleAxis(
			glm::radians(90.0f),
			glm::vec3(1.0f, 0.0f, 0.0f)
		);
	hitbox_transform->scale = glm::vec3(7.5f, 0.15f, 200.0f);
	hitbox_transform->parent = arm_transform;
	scene.drawables.emplace_back(hitbox_transform);
	Scene::Drawable &drawable4 = scene.drawables.back();
	drawable4.pipeline = lit_color_texture_program_pipeline;
	drawable4.pipeline.vao = clock_meshes_for_lit_color_texture_program;
	drawable4.pipeline.type = hitbox_pipeline.type;
	drawable4.pipeline.start = hitbox_pipeline.start;
	drawable4.pipeline.count = hitbox_pipeline.count;

	// add all notes but keep them invisible
	for (uint16_t i = 0; i < num_notes; i++) {
		note_hit_time[i] = 0.0f;

		float radian = glm::radians(ROTATION_SPEED * notes[i] * BEAT_SPEED);
		float x = 0.0f;
		float y = notes_distance[i];
		// https://en.wikipedia.org/wiki/Rotation_matrix#Two_dimensions
		x = (float) y * sin((float) radian);
		y = (float) y * cos((float) radian);

		note_transforms[i] = new Scene::Transform;
		note_transforms[i]->name = NOTE;
		note_transforms[i]->position = glm::vec3(x, y, 0.03f);
		note_transforms[i]->scale = glm::vec3(0.00f, 0.00f, 1.0f);
		scene.drawables.emplace_back(note_transforms[i]);

		Scene::Drawable &d = scene.drawables.back();
		d.pipeline = lit_color_texture_program_pipeline;
		d.pipeline.vao = clock_meshes_for_lit_color_texture_program;
		d.pipeline.type = note_pipeline.type;
		d.pipeline.start = note_pipeline.start;
		d.pipeline.count = note_pipeline.count;
	}
}

PlayMode::~PlayMode() {
}

void PlayMode::update_note() {
	for (uint16_t i = 0; i < num_notes; i++) {
		if (global_timer < notes[i] * BEAT_SPEED - HOLD_TIME - FADE_IN_TIME) {
			break; // not time to show anything from this note and onward yet
		} else if (global_timer < notes[i] * BEAT_SPEED - HOLD_TIME) {
			float delta_time = global_timer - (notes[i] * BEAT_SPEED - HOLD_TIME - FADE_IN_TIME);
			note_transforms[i]->scale.x = delta_time * FADE_IN_SPEED;
			note_transforms[i]->scale.y = delta_time * FADE_IN_SPEED;
		} else {
			if (note_hit_time[i] > 0.0f) {
				float delta_time = global_timer - note_hit_time[i];
				note_transforms[i]->scale.x = std::max(0.0f, NOTE_SCALE - delta_time * HIT_OUT_SPEED);
				note_transforms[i]->scale.y = std::max(0.0f, NOTE_SCALE - delta_time * HIT_OUT_SPEED);
			} else {
				if (global_timer < notes[i] * BEAT_SPEED + HOLD_TIME) {
					note_transforms[i]->scale.x = NOTE_SCALE;
					note_transforms[i]->scale.y = NOTE_SCALE;
				} else if (global_timer < notes[i] * BEAT_SPEED + HOLD_TIME + FADE_OUT_TIME) {
					float delta_time = global_timer - (notes[i] * BEAT_SPEED + HOLD_TIME);
					note_transforms[i]->scale.x = NOTE_SCALE - delta_time * FADE_OUT_SPEED;
					note_transforms[i]->scale.y = NOTE_SCALE - delta_time * FADE_OUT_SPEED;
				} else {
					note_transforms[i]->scale.x = 0.0f;
					note_transforms[i]->scale.y = 0.0f;
				}
			}
		}
	}
}

void PlayMode::hit_note() {
	for (uint16_t i = 0; i < num_notes; i++) {
		float position_diff = notes_distance[i] + hitbox_transform->position.z * NOTE_TO_HITBOX_RATIO;
		if (notes[i] * BEAT_SPEED - HIT_TIME <= global_timer && global_timer <= notes[i] * BEAT_SPEED + HIT_TIME
			&& note_hit_time[i] == 0.0f && -MAX_NOTE_TO_HITBOX_DIST <= position_diff && position_diff <= MAX_NOTE_TO_HITBOX_DIST) {
			note_hit_time[i] = global_timer;
		}
	}
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
		} else if (evt.key.keysym.sym == SDLK_z) {
			outward.downs += 1;
			outward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			inward.downs += 1;
			inward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			if (gameState != GameState::IN_PROGRESS) {
				//start music loop playing:
				piano_loop = Sound::play(*piano_sample);
				gameState = GameState::IN_PROGRESS;
			}
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			if (gameState == GameState::IN_PROGRESS) {
				//start music loop playing:
				hit.pressed = true;
			}
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
		} else if (evt.key.keysym.sym == SDLK_z) {
			outward.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			inward.pressed = false;
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
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if (inward.pressed && !outward.pressed) {
		hitbox_transform->position.z += HITBOX_MOVEMENT_SPEED * elapsed;
		hitbox_transform->position.z = std::min(HITBOX_MOVEMENT_INWARD_BOUND, hitbox_transform->position.z);
	}
	if (!inward.pressed && outward.pressed) {
		hitbox_transform->position.z -= HITBOX_MOVEMENT_SPEED * elapsed;
		hitbox_transform->position.z = std::max(HITBOX_MOVEMENT_OUTWARD_BOUND, hitbox_transform->position.z);
	}

	if (gameState == GameState::IN_PROGRESS) {
		global_timer += elapsed;
		gear_transform->rotation *= glm::angleAxis(
			glm::radians(-ROTATION_SPEED*elapsed),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		arm_transform->rotation *= glm::angleAxis(
			glm::radians(-ROTATION_SPEED*elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		if (hit.pressed) {
			hit_note();
			hit.pressed = false;
		}
		update_note();
	}

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	outward.downs = 0;
	inward.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

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

	scene.draw(*camera);

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
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
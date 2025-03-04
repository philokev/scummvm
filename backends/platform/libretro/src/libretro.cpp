/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <libretro.h>
#include "audio/mixer_intern.h"
#include "base/main.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/fs.h"
#include "streams/file_stream.h"
#include "graphics/surface.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifndef _MSC_VER
/**
 * Include libgen.h for basename() and dirname().
 * @see http://linux.die.net/man/3/basename
 */
#include <libgen.h>
#endif

/**
 * Include base/internal_version.h to allow access to SCUMMVM_VERSION.
 * @see retro_get_system_info()
 */
#define INCLUDED_FROM_BASE_VERSION_CPP
#include "base/internal_version.h"

#include "backends/platform/libretro/include/libretro-threads.h"
#include "backends/platform/libretro/include/libretro-core-options.h"
#include "backends/platform/libretro/include/os.h"

retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static int retro_device = RETRO_DEVICE_JOYPAD;

// System analog stick range is -0x8000 to 0x8000
#define ANALOG_RANGE 0x8000
// Default deadzone: 15%
static int analog_deadzone = (int)(0.15f * ANALOG_RANGE);

static float gampad_cursor_speed = 1.0f;
static bool analog_response_is_quadratic = false;

static float mouse_speed = 1.0f;
static float gamepad_acceleration_time = 0.2f;

static bool speed_hack_is_enabled = false;

char cmd_params[20][200];
char cmd_params_num;

int adjusted_RES_W = 0;
int adjusted_RES_H = 0;

static uint32 audio_latency = 0;
static bool audio_buffer_status_support = false;
static bool mute=true;

static bool retro_audio_buff_active = false;
static unsigned retro_audio_buff_occupancy = 0;
static bool retro_audio_buff_underrun = false;

static uint16 fps = 0;
static uint16 sound_len = 0;                // length in samples per frame
static size_t sound_size = 0;

static int16_t *sound_buffer = NULL;       // pointer to output buffer

static void audio_buffer_init(uint16 sample_rate, uint16 frame_rate) {
	fps = 100.0 * frame_rate;
	sound_len = (sample_rate * 100 + (fps >> 1)) / fps;
	sound_size = sound_len << 2 * sizeof(int16_t);
	if (sound_buffer)
		sound_buffer = (int16_t *)realloc(sound_buffer, sound_size);
	else
		sound_buffer = (int16_t *)malloc(sound_size);
	if (sound_buffer)
		memset(sound_buffer, 0, sound_size);
	else
		log_cb(RETRO_LOG_ERROR, "audio_buffer_init error.\n");
}

static void update_variables(void) {
	struct retro_variable var;

	var.key = "scummvm_gamepad_cursor_speed";
	var.value = NULL;
	gampad_cursor_speed = 1.0f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		gampad_cursor_speed = (float)atof(var.value);
	}

	var.key = "scummvm_gamepad_cursor_acceleration_time";
	var.value = NULL;
	gamepad_acceleration_time = 0.2f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		gamepad_acceleration_time = (float)atof(var.value);
	}

	var.key = "scummvm_analog_response";
	var.value = NULL;
	analog_response_is_quadratic = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "quadratic") == 0)
			analog_response_is_quadratic = true;
	}

	var.key = "scummvm_analog_deadzone";
	var.value = NULL;
	analog_deadzone = (int)(0.15f * ANALOG_RANGE);
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		analog_deadzone = (int)(atoi(var.value) * 0.01f * ANALOG_RANGE);
	}

	var.key = "scummvm_mouse_speed";
	var.value = NULL;
	mouse_speed = 1.0f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mouse_speed = (float)atof(var.value);
	}

	var.key = "scummvm_speed_hack";
	var.value = NULL;
	speed_hack_is_enabled = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "enabled") == 0)
			speed_hack_is_enabled = true;
	}
}

void parse_command_params(char *cmdline) {
	int j = 0;
	int cmdlen = strlen(cmdline);
	bool quotes = false;

	/* Append a new line to the end of the command to signify it's finished. */
	cmdline[cmdlen] = '\n';
	cmdline[++cmdlen] = '\0';

	/* parse command line into array of arguments */
	for (int i = 0; i < cmdlen; i++) {
		switch (cmdline[i]) {
		case '\"':
			if (quotes) {
				cmdline[i] = '\0';
				strcpy(cmd_params[cmd_params_num], cmdline + j);
				cmd_params_num++;
				quotes = false;
			} else
				quotes = true;
			j = i + 1;
			break;
		case ' ':
		case '\n':
			if (!quotes) {
				if (i != j && !quotes) {
					cmdline[i] = '\0';
					strcpy(cmd_params[cmd_params_num], cmdline + j);
					cmd_params_num++;
				}
				j = i + 1;
			}
			break;
		}
	}
}

#if defined(WIIU) || defined(__SWITCH__) || defined(_MSC_VER) || defined(_3DS)
#include <stdio.h>
#include <string.h>
char *dirname(char *path) {
	char *p;
	if (path == NULL || *path == '\0')
		return ".";
	p = path + strlen(path) - 1;
	while (*p == '/') {
		if (p == path)
			return path;
		*p-- = '\0';
	}
	while (p >= path && *p != '/')
		p--;
	return p < path ? "." : p == path ? "/" : (*p = '\0', path);
}
#endif

#if (defined(GEKKO) && !defined(WIIU)) || defined(__CELLOS_LV2__)
int access(const char *path, int amode) {
	RFILE *f;
	int mode;

	switch (amode) {
	// we don't really care if a file exists but isn't readable
	case F_OK:
	case R_OK:
		mode = RETRO_VFS_FILE_ACCESS_READ;
		break;

	case W_OK:
		mode = RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING;
		break;

	default:
		return -1;
	}

	f = filestream_open(path, RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (f) {
		filestream_close(f);
		return 0;
	}

	return -1;
}
#endif

void retro_set_video_refresh(retro_video_refresh_t cb) {
	video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
	audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
	poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
	input_cb = cb;
}

void retro_set_environment(retro_environment_t cb) {
	environ_cb = cb;
	bool tmp = true;

	environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &tmp);
	libretro_set_core_options(environ_cb);
}

unsigned retro_api_version(void) {
	return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info) {
	info->library_name = CORE_NAME;
#if defined GIT_TAG
#define __GIT_VERSION GIT_TAG
#elif defined GIT_HASH
#define __GIT_VERSION GIT_HASH "-" SCUMMVM_VERSION
#else
#define __GIT_VERSION ""
#endif
	info->library_version = __GIT_VERSION;
	info->valid_extensions = "scummvm";
	info->need_fullpath = true;
	info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
	info->geometry.base_width = RES_W;
	info->geometry.base_height = RES_H;
	info->geometry.max_width = RES_W;
	info->geometry.max_height = RES_H;
	info->geometry.aspect_ratio = 4.0f / 3.0f;
	info->timing.fps = REFRESH_RATE;
	info->timing.sample_rate = SAMPLE_RATE;
}

void retro_init(void) {
	const char *sysdir;
	const char *savedir;

	struct retro_log_callback log;
	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
		log_cb = log.log;
	else
		log_cb = NULL;

	update_variables();

	cmd_params_num = 1;
	strcpy(cmd_params[0], "scummvm\0");

	struct retro_input_descriptor desc[] = {
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Mouse Cursor Left"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Mouse Cursor Up"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Mouse Cursor Down"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Mouse Cursor Right"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Right Mouse Button"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Left Mouse Button"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Esc"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "."},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Numpad 5"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "Backspace"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "Cursor Fine Control"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3, "F10"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3, "Numpad 0"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "ScummVM GUI"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Virtual Keyboard"},
		{0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT, "Left click"},
		{0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT, "Right click"},
		{0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Left Analog X"},
		{0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Left Analog Y"},
		{0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X, "Right Analog X"},
		{0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y, "Right Analog Y"},
		{0},
	};

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

	/* Get color mode: 32 first as VGA has 6 bits per pixel */
#if 0
	RDOSGFXcolorMode = RETRO_PIXEL_FORMAT_XRGB8888;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &RDOSGFXcolorMode)) {
		RDOSGFXcolorMode = RETRO_PIXEL_FORMAT_RGB565;
		if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &RDOSGFXcolorMode))
			RDOSGFXcolorMode = RETRO_PIXEL_FORMAT_0RGB1555;
	}
#endif

#ifdef FRONTEND_SUPPORTS_RGB565
	enum retro_pixel_format rgb565 = RETRO_PIXEL_FORMAT_RGB565;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
		log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 -will use that instead of XRGB1555.\n");
#endif

	retro_keyboard_callback cb = {retroKeyEvent};
	environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cb);

	if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sysdir))
		retroSetSystemDir(sysdir);
	else {
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "No System directory specified, using current directory.\n");
		retroSetSystemDir(".");
	}

	if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir))
		retroSetSaveDir(savedir);
	else {
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "No Save directory specified, using current directory.\n");
		retroSetSaveDir(".");
	}

	// Check RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK support
	audio_buffer_status_support = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK, NULL);

	audio_buffer_init(SAMPLE_RATE, REFRESH_RATE);

	g_system = retroBuildOS(speed_hack_is_enabled);
}

void retro_deinit(void) {
	free(sound_buffer);
}

void retro_set_controller_port_device(unsigned port, unsigned device) {
	if (port != 0) {
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "Invalid controller port %d.\n", port);
		return;
	}

	switch (device) {
	case RETRO_DEVICE_JOYPAD:
	case RETRO_DEVICE_MOUSE:
		retro_device = device;
		break;
	default:
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "Invalid controller device class %d.\n", device);
		break;
	}
}

bool retro_load_game(const struct retro_game_info *game) {
	if (!g_system) {
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to initialize platform driver.\n");
		return false;
	}

	if (game) {
		// Retrieve the game path.
		Common::FSNode detect_target = Common::FSNode(game->path);
		Common::FSNode parent_dir = detect_target.getParent();
		char target_id[400] = {0};
		char buffer[400];
		int test_game_status = TEST_GAME_KO_NOT_FOUND;

		struct retro_message_ext retro_msg;
		retro_msg.type = RETRO_MESSAGE_TYPE_NOTIFICATION;
		retro_msg.target = RETRO_MESSAGE_TARGET_OSD;
		retro_msg.duration = 3000;
		retro_msg.msg = "";

		const char *target_file_ext = ".scummvm";
		int target_file_ext_pos = strlen(game->path) - strlen(target_file_ext);

		// See if we are loading a .scummvm file.
		if (!(target_file_ext_pos < 0) && strstr(game->path + target_file_ext_pos, target_file_ext) != NULL) {
			// Open the file.
			RFILE *gamefile = filestream_open(game->path, RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);
			if (!gamefile) {
				log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to load given game file '%s'.\n", game->path);
				return false;
			}

			// Load the file data.
			if (filestream_gets(gamefile, target_id, sizeof(target_id)) == NULL) {
				filestream_close(gamefile);
				log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to load contents of game file '%s'.\n", game->path);
				return false;
			}
			filestream_close(gamefile);

			Common::String tmp = target_id;
			tmp.trim();
			strcpy(target_id, tmp.c_str());

			if (strlen(target_id) == 0) {
				log_cb(RETRO_LOG_ERROR, "[scummvm] Game file '%s' does not contain any target id.\n", game->path);
				return false;
			}

			test_game_status = retroTestGame(target_id, false);
		} else {
			if (detect_target.isDirectory()) {
				parent_dir = detect_target;
			} else {
				// If this node has no parent node, then it returns a duplicate of this node.
				if (detect_target.getPath().equals(parent_dir.getPath())) {
					log_cb(RETRO_LOG_ERROR, "[scummvm] Autodetect not possible. No parent directory detected in '%s'.\n", game->path);
					return false;
				}
			}

			test_game_status = retroTestGame(parent_dir.getPath().c_str(), true);
		}

		// Preliminary game scan results
		switch (test_game_status) {
		case TEST_GAME_OK_ID_FOUND:
			sprintf(buffer, "-p \"%s\" %s", parent_dir.getPath().c_str(), target_id);
			log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via target id and game dir\n");
			break;
		case TEST_GAME_OK_TARGET_FOUND:
			sprintf(buffer, "%s", target_id);
			log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via target id and scummvm.ini\n");
			break;
		case TEST_GAME_OK_ID_AUTODETECTED:
			sprintf(buffer, "-p \"%s\" --auto-detect", parent_dir.getPath().c_str());
			log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via autodetect\n");
			break;
		case TEST_GAME_KO_MULTIPLE_RESULTS:
			log_cb(RETRO_LOG_WARN, "[scummvm] Multiple targets found for '%s' in scummvm.ini\n", target_id);
			retro_msg.msg = "Multiple targets found";
			break;
		case TEST_GAME_KO_NOT_FOUND:
		default:
			log_cb(RETRO_LOG_WARN, "[scummvm] Game not found. Check path and content of '%s'\n", game->path);
			retro_msg.msg = "Game not found";
		}

		if (retro_msg.msg[0] != '\0') {
			environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &retro_msg);
		} else {
			parse_command_params(buffer);
		}
	}

	if (!retro_init_emu_thread()) {
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to initialize emulation thread!\n");
		return false;
	}
	return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) {
	return false;
}

void retro_run(void) {
	if (retro_emu_thread_exited())
		retro_deinit_emu_thread();

	if (!retro_emu_thread_initialized()) {
		environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
		return;
	}

	// Setting RA's video or audio driver to null will disable video/audio bits,
	int audio_video_enable = 0;
	environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &audio_video_enable);

	retro_switch_to_emu_thread();
	/* Mouse */
	if (g_system) {
		poll_cb();
		retroProcessMouse(input_cb, retro_device, gampad_cursor_speed, gamepad_acceleration_time, analog_response_is_quadratic, analog_deadzone, mouse_speed);



		/* Upload video: TODO: Check the CANDUPE env value */
		if (audio_video_enable & 1) {
			const Graphics::Surface &screen = getScreen();
			video_cb(screen.getPixels(), screen.w, screen.h, screen.pitch);
		} else {
			//check if it's the same avoiding the call to video_cb at all
			video_cb(NULL, 0, 0, 0); // Set to NULL to skip frame rendering
		}

		/* Upload audio */
		size_t count = 0;
		if (audio_video_enable & 2) {
			count = ((Audio::MixerImpl *)g_system->getMixer())->mixCallback((byte *)sound_buffer, sound_size);
		}
		mute = count ? false : true;
#if defined(_3DS)
		/* Hack: 3DS will produce static noise
		 * unless we manually send a zeroed
		 * audio buffer when no samples are
		 * available (i.e. when the overlay
		 * is shown) */
		if (mute) {
			audio_buffer_init(SAMPLE_RATE, REFRESH_RATE);
		}
#endif
		audio_batch_cb(mute ? NULL : sound_buffer, count); // Set to NULL to skip sound rendering

	}

	bool updated = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
		update_variables();
	}
}

void retro_unload_game(void) {
	if (!retro_emu_thread_initialized())
		return;
	while (!retro_emu_thread_exited()) {
		retroQuit();
		retro_switch_to_emu_thread();
	}
	retro_deinit_emu_thread();
	// g_system->destroy(); //TODO: This call causes "pure virtual method called" after frontend "Unloading core symbols". Check if needed at all.
}

void retro_reset(void) {
	retroReset();
}

// Stubs
void *retro_get_memory_data(unsigned type) {
	return 0;
}
size_t retro_get_memory_size(unsigned type) {
	return 0;
}
size_t retro_serialize_size(void) {
	return 0;
}
bool retro_serialize(void *data, size_t size) {
	return false;
}
bool retro_unserialize(const void *data, size_t size) {
	return false;
}
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned unused, bool unused1, const char *unused2) {}

unsigned retro_get_region(void) {
	return RETRO_REGION_NTSC;
}

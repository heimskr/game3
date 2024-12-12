#define MINIAUDIO_IMPLEMENTATION

#include "ui/MiniAudio.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <miniaudio/extras/miniaudio_libopus.h>
#include <miniaudio/extras/miniaudio_libvorbis.h>
#pragma GCC diagnostic pop

#include <format>

static ma_result ma_decoding_backend_init__libvorbis(void *, ma_read_proc on_read, ma_seek_proc on_seek, ma_tell_proc on_tell, void *read_seek_tell_user_data, const ma_decoding_backend_config *config,
                                                     const ma_allocation_callbacks *allocation_callbacks, ma_data_source **backend) {
	auto *vorbis = reinterpret_cast<ma_libvorbis *>(ma_malloc(sizeof(ma_libvorbis), allocation_callbacks));

	if (!vorbis)
		return MA_OUT_OF_MEMORY;

	if (ma_result result = ma_libvorbis_init(on_read, on_seek, on_tell, read_seek_tell_user_data, config, allocation_callbacks, vorbis); result != MA_SUCCESS) {
		ma_free(vorbis, allocation_callbacks);
		return result;
	}

	*backend = vorbis;
	return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libvorbis(void *, const char *file_path, const ma_decoding_backend_config *config, const ma_allocation_callbacks *allocation_callbacks, ma_data_source **backend) {
	auto *vorbis = reinterpret_cast<ma_libvorbis *>(ma_malloc(sizeof(ma_libvorbis), allocation_callbacks));

	if (!vorbis)
		return MA_OUT_OF_MEMORY;

	if (ma_result result = ma_libvorbis_init_file(file_path, config, allocation_callbacks, vorbis); result != MA_SUCCESS) {
		ma_free(vorbis, allocation_callbacks);
		return result;
	}

	*backend = vorbis;
	return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libvorbis(void *, ma_data_source *backend, const ma_allocation_callbacks *allocation_callbacks) {
	auto *vorbis = reinterpret_cast<ma_libvorbis *>(backend);
	ma_libvorbis_uninit(vorbis, allocation_callbacks);
	ma_free(vorbis, allocation_callbacks);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libvorbis = {
	ma_decoding_backend_init__libvorbis,
	ma_decoding_backend_init_file__libvorbis,
	nullptr, /* onInitFileW() */
	nullptr, /* onInitMemory() */
	ma_decoding_backend_uninit__libvorbis,
};

static ma_result ma_decoding_backend_init__libopus(void *, ma_read_proc on_read, ma_seek_proc on_seek, ma_tell_proc on_tell, void *read_seek_tell_user_data, const ma_decoding_backend_config *config,
                                                   const ma_allocation_callbacks *allocation_callbacks, ma_data_source **backend) {
	auto *opus = reinterpret_cast<ma_libopus *>(ma_malloc(sizeof(ma_libopus), allocation_callbacks));
	if (!opus)
		return MA_OUT_OF_MEMORY;

	if (ma_result result = ma_libopus_init(on_read, on_seek, on_tell, read_seek_tell_user_data ,config, allocation_callbacks, opus); result != MA_SUCCESS) {
		ma_free(opus, allocation_callbacks);
		return result;
	}

	*backend = opus;
	return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libopus(void *, const char *file_path, const ma_decoding_backend_config *config, const ma_allocation_callbacks *allocation_callbacks, ma_data_source **backend) {
	auto *opus = reinterpret_cast<ma_libopus *>(ma_malloc(sizeof(ma_libopus), allocation_callbacks));
	if (!opus)
		return MA_OUT_OF_MEMORY;

	if (ma_result result = ma_libopus_init_file(file_path, config, allocation_callbacks, opus); result != MA_SUCCESS) {
		ma_free(opus, allocation_callbacks);
		return result;
	}

	*backend = opus;
	return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libopus(void *, ma_data_source *backend, const ma_allocation_callbacks *allocation_callbacks) {
	auto *opus = reinterpret_cast<ma_libopus *>(backend);
	ma_libopus_uninit(opus, allocation_callbacks);
	ma_free(opus, allocation_callbacks);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libopus = {
	ma_decoding_backend_init__libopus,
	ma_decoding_backend_init_file__libopus,
	nullptr, /* onInitFileW() */
	nullptr, /* onInitMemory() */
	ma_decoding_backend_uninit__libopus,
};

namespace MiniAudio {
	AudioError::AudioError(const std::string &message, ma_result result_):
		std::runtime_error(std::format("{} ({})", message, static_cast<int>(result_))),
		result(result_) {}

	AudioError::AudioError(ma_result result_):
		std::runtime_error(std::format("MiniAudio error {}", static_cast<int>(result_))),
		result(result_) {}

	ma_result makeEngine(ma_engine &engine, ma_resource_manager &resource_manager, ma_decoding_backend_vtable * (*vtables)[2]) {
		ma_engine_config engine_config{};

		ma_decoding_backend_vtable *custom_backend_vtables[] = {
			&g_ma_decoding_backend_vtable_libopus,
			&g_ma_decoding_backend_vtable_libvorbis,
		};

		if (vtables) {
			(*vtables)[0] = custom_backend_vtables[0];
			(*vtables)[1] = custom_backend_vtables[1];
		}

		// Using custom decoding backends requires a resource manager.
		ma_resource_manager_config config = ma_resource_manager_config_init();
		config.ppCustomDecodingBackendVTables = custom_backend_vtables;
		config.customDecodingBackendCount     = sizeof(custom_backend_vtables) / sizeof(custom_backend_vtables[0]);
		config.pCustomDecodingBackendUserData = nullptr; // This will be passed in to the pUserData parameter of each function in the decoding backend vtables.

		if (ma_result result = ma_resource_manager_init(&config, &resource_manager); result != MA_SUCCESS)
			return result;

		// Once we have a resource manager we can create the engine.
		engine_config = ma_engine_config_init();
		engine_config.pResourceManager = &resource_manager;

		return ma_engine_init(&engine_config, &engine);
	}
}

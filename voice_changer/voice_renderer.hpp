#pragma once
#include "../vocaloid/audio_context.hpp"
#include "effect.hpp"
#include "factory.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

struct RenderData {
	char *file;
	AudioContext *ctx;
};

class VoiceRenderer {

private:
	map<string, RenderData> data_;
	thread *progress_thread_;
	mutex progress_mutex_;
public:

	void Render(const char* file, int effect_id, float* options) {
		
	}

	void Render(const char** files, int* effect_ids, float** effect_options, int count) {
		
	}

	void CancelRender(int index) {
		
	}

	void CancelRender(const char* file) {
		
	}

	void CancelRenderAll() {
		
	}
};
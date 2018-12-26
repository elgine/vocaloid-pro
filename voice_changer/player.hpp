#pragma once
#include "../vocaloid/audio_context.hpp"
#include "effect.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;
class Player {

private:
	AudioContext *ctx_;
	Effect *effect_;

	explicit Player(AudioContext *ctx) {
		ctx_ = ctx;
	}

	int Play(const char* source, int effect_id, int64_t** segments) {
		ctx_->Prepare();
	}
};
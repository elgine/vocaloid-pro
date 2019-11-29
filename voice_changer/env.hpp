#pragma once
#include "../vocaloid/gain_node.hpp"
#include "envs.h"
#include "../vocaloid/maths.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
namespace env {
	
	class Env {
	protected:
		Envs id_;
		BaseAudioContext *ctx_;
		GainNode *input_;
		GainNode *dry_;
		GainNode *wet_;
		GainNode *output_;
	public:

		static float MIX_DEFAULT;
		static float MIX_MIN;
		static float MIX_MAX;

		explicit Env(BaseAudioContext *ctx) {
			ctx_ = ctx;
			input_ = new GainNode(ctx);
			dry_ = new GainNode(ctx);
			wet_ = new GainNode(ctx);
			output_ = new GainNode(ctx);
			ctx->Connect(dry_, output_);
			ctx->Connect(wet_, output_);
			CrossFade(1.0);
		}

		virtual void Start() {}

		virtual void Resume() {}

		virtual void Stop() {}

		virtual void Dispose() {
			input_->Dispose();
			delete input_;
			input_ = nullptr;
			dry_->Dispose();
			delete dry_;
			dry_ = nullptr;
			wet_->Dispose();
			delete wet_;
			wet_ = nullptr;
			output_->Dispose();
			delete output_;
			output_ = nullptr;
		}

		virtual void SetOptions(float *opts, int16_t opt_count) {
			if (opt_count > 0) {
				CrossFade(Clamp(MIX_MIN, MIX_MAX, opts[0]));
			}
		}

		void CrossFade(float v) {
			v = Clamp(MIX_MIN, MIX_MAX, v);
			auto gain1 = cosf(v * 0.5 * M_PI);
			auto gain2 = cosf((1.0 - v) * 0.5 * M_PI);
			dry_->gain_->value_ = gain1;
			wet_->gain_->value_ = gain2;
		}

		Envs Id() {
			return id_;
		}

		AudioNode* Input() {
			return input_;
		}

		AudioNode* Output() {
			return output_;
		}
	};

	float Env::MIX_DEFAULT = 1.0;
	float Env::MIX_MIN = 0.0;
	float Env::MIX_MAX = 1.0;
}
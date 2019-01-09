#pragma once
#include "../vocaloid/jungle.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "effects.h"
#include "effect.hpp"
using namespace vocaloid::composite;
namespace effect {
	class Female: public Effect{
	private:
		Jungle *jungle_;
		DynamicsCompressorNode *compressor_;
	public:

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

		explicit Female(BaseAudioContext *ctx):Effect(ctx) {
			id_ = Effects::FEMALE;
			compressor_ = new DynamicsCompressorNode(ctx);
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(PITCH_OFFSET_DEFAULT);
			ctx_->Connect(jungle_->output_, compressor_);
			ctx_->Connect(compressor_, gain_);
		}

		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				jungle_->SetPitchOffset(Clamp(PITCH_OFFSET_MIN, PITCH_OFFSET_MAX, options[0]));
			}
			if (option_count > 1) {
				SetGain(options[1]);
			}
		}

		void Start() override {
			jungle_->Start();
		}

		void SetPitchOffset(float v) {
			jungle_->SetPitchOffset(v);
		}

		void Dispose() override {
			jungle_->Dispose();
			delete jungle_;
			jungle_ = nullptr;

			compressor_->Dispose();
			delete compressor_;
			compressor_ = nullptr;

			Effect::Dispose();
		}

		AudioNode* Input() {
			return jungle_->input_;
		}
	};

	float Female::PITCH_OFFSET_DEFAULT = 0.45;
	float Female::PITCH_OFFSET_MIN = -0.5f;
	float Female::PITCH_OFFSET_MAX = 1.0f;
}
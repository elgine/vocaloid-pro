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

		explicit Female(AudioContext *ctx):Effect(ctx) {
			id_ = Effects::FEMALE;
			compressor_ = new DynamicsCompressorNode(ctx);
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(0.45);
			ctx_->Connect(jungle_->output_, compressor_);
			ctx_->Connect(compressor_, gain_);
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
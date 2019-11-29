#pragma once
#include "../vocaloid/gain_node.hpp"
#include "roles.h"
using namespace vocaloid;
using namespace vocaloid::node;
namespace role {
	class Role {
	protected:
		Roles id_;
		BaseAudioContext *ctx_;
		GainNode *gain_;
	public:

		static float GAIN_DEFAULT;
		static float GAIN_MIN;
		static float GAIN_MAX;

		explicit Role(BaseAudioContext *ctx) {
			ctx_ = ctx;
			gain_ = new GainNode(ctx, GAIN_DEFAULT);
		}

		virtual void Dispose() {
			if (gain_ != nullptr) {
				gain_->Dispose();
				delete gain_;
				gain_ = nullptr;
			}
		}

		void SetGain(float gain) {
			gain_->gain_->value_ = Clamp(GAIN_MIN, GAIN_MAX, gain);
		}

		virtual void SetOptions(float* options, int option_count) {
			if (option_count < 0)return;
			SetGain(options[0]);
		}

		virtual float TimeScale() { return 1.0f; };
		virtual float Delay() { return 0.0f; }

		virtual AudioNode* Input() {
			return gain_;
		}

		virtual AudioNode* Output() {
			return gain_;
		}

		virtual void Resume() {}

		virtual void Start() {}

		Roles Id() {
			return id_;
		}
	};

	float Role::GAIN_DEFAULT = 1.0f;
	float Role::GAIN_MIN = 0.0f;
	float Role::GAIN_MAX = 4.0f;
}
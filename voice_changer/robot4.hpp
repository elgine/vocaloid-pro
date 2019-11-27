#pragma once
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "effects.h"
#include "effect.hpp"
using namespace vocaloid::node;
namespace effect {

	class Robot4Node : public AudioNode {

	public:
		explicit Robot4Node(BaseAudioContext *ctx) :AudioNode(ctx) {
		}

		int64_t ProcessFrame(bool flush = false) override {
			for (auto i = 0; i < channels_; i++) {
				float beepboopMod = 0, beepboop = 0, delay0 = 0, delay1 = 0, delay2 = 0, delay3 = 0;
				for (auto j = 0; j < frame_size_; j++) {
					if (j % 1700 == 0) { beepboopMod = rand() / 32767.0; }
					beepboop = sin(j * beepboopMod) * 0.05;
					//delay0 = Math.sin(j/80) * 100;
					delay1 = sin(j / 400.0) * 100.0;
					delay2 = sin(j / 200.0) * 100.0;
					delay3 = sin(j / 100.0) * 100.0;
					result_buffer_->Channel(i)->Data()[j] = summing_buffer_->Channel(i)->Data()[int(round(j - delay0 - delay1 - delay2))] * 0.9 + beepboop;
				}
			}
			return frame_size_;
		}
	};

	class Robot4 : public Effect {

	private:
		Robot4Node *process_;
	public:

		explicit Robot4(BaseAudioContext* ctx) :Effect(ctx) {
			id_ = Effects::ROBOT4;
			process_ = new Robot4Node(ctx);
			ctx_->Connect(process_, gain_);
		}

		void Dispose() override {
			if (process_) {
				process_->Dispose();
				delete process_;
				process_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode* Input() {
			return process_;
		}
	};
}
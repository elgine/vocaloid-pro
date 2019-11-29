#pragma once
#include <math.h>
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/buffer_node.hpp"
#include "../vocaloid/audio_channel.hpp"
#include "effect.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
namespace effect {
	AudioChannel* CreateFadeBuffer(BaseAudioContext *ctx, float active_time, float fade_time) {
		int64_t length1 = ctx->SampleRate() * active_time;
		int64_t length2 = (active_time - 2 * fade_time) * ctx->SampleRate();
		int64_t length = length1 + length2;
		auto buffer = new AudioChannel(1, length);
		buffer->SetSize(length);
		auto p = buffer->Channel(0);

		int64_t fade_length = fade_time * ctx->SampleRate();
		int64_t fade_index1 = fade_length;
		int64_t fade_index2 = length1 - fade_length;

		for (auto i = 0; i < length1; i++) {
			float value = 0;
			if (i < fade_index1) {
				value = sqrtf(float(i) / fade_length);
			}
			else if (i >= fade_index2) {
				value = sqrtf(1.0f - (float(i) - fade_index2) / fade_length);
			}
			else {
				value = 1.0f;
			}
			p->Data()[i] = value;
		}

		for (auto i = length1; i < length; i++) {
			p->Data()[i] = 0;
		}
		return buffer;
	}

	AudioChannel* CreateDelayTimeBuffer(BaseAudioContext *ctx, float active_time, float fade_time, bool shift_up) {
		int64_t length1 = active_time * ctx->SampleRate();
		int64_t length2 = (active_time - 2 * fade_time) * ctx->SampleRate();
		int64_t length = length1 + length2;
		auto buffer = new AudioChannel(1, length);
		buffer->SetSize(length);
		auto p = buffer->Channel(0);

		for (auto i = 0; i < length1; i++) {
			if (shift_up) {
				p->Data()[i] = float(length1 - i) / length;
			}
			else {
				p->Data()[i] = float(i) / length1;
			}
		}

		for (auto i = length1; i < length; i++) {
			p->Data()[i] = 0;
		}
		return buffer;
	}

	class Jungle : public Effect {
	private:
		float previous_pitch_;

		float buffer_time_;
		float fade_time_;
	public:

		BufferNode *mod1_;
		BufferNode *mod2_;
		BufferNode *mod3_;
		BufferNode *mod4_;
		GainNode *mod1_gain_;
		GainNode *mod2_gain_;
		GainNode *mod3_gain_;
		GainNode *mod4_gain_;

		GainNode *mod_gain1_;
		GainNode *mod_gain2_;

		BufferNode *fade1_;
		BufferNode *fade2_;
		DelayNode *delay1_;
		DelayNode *delay2_;
		GainNode *mix1_;
		GainNode *mix2_;

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;


		explicit Jungle(BaseAudioContext *ctx, float delay_time = 0.1, float fade_time = 0.05, float buffer_time = 0.1) :Effect(ctx) {
			id_ = Effects::JUNGLE;
			previous_pitch_ = -1;
			buffer_time_ = buffer_time;
			fade_time_ = fade_time;

			auto mod1 = new BufferNode(ctx);
			auto mod2 = new BufferNode(ctx);
			auto mod3 = new BufferNode(ctx);
			auto mod4 = new BufferNode(ctx);

			auto shift_down_buffer = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, false);
			auto shift_up_buffer = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, true);

			mod1->SetBuffer(shift_down_buffer);
			mod2->SetBuffer(shift_down_buffer);
			mod3->SetBuffer(shift_up_buffer);
			mod4->SetBuffer(shift_up_buffer);

			mod1->Loop(true);
			mod2->Loop(true);
			mod3->Loop(true);
			mod4->Loop(true);

			auto mod1_gain = new GainNode(ctx);
			auto mod2_gain = new GainNode(ctx);
			auto mod3_gain = new GainNode(ctx, 0);
			auto mod4_gain = new GainNode(ctx, 0);

			ctx->Connect(mod1, mod1_gain);
			ctx->Connect(mod2, mod2_gain);
			ctx->Connect(mod3, mod3_gain);
			ctx->Connect(mod4, mod4_gain);

			auto mod_gain1 = new GainNode(ctx);
			auto mod_gain2 = new GainNode(ctx);
			auto delay1 = new DelayNode(ctx);
			auto delay2 = new DelayNode(ctx);

			ctx->Connect(mod1_gain, mod_gain1);
			ctx->Connect(mod2_gain, mod_gain2);
			ctx->Connect(mod3_gain, mod_gain1);
			ctx->Connect(mod4_gain, mod_gain2);

			ctx->Connect(mod_gain1, delay1->delay_time_);
			ctx->Connect(mod_gain2, delay2->delay_time_);

			auto fade1 = new BufferNode(ctx);
			auto fade2 = new BufferNode(ctx);
			auto fade_buffer = CreateFadeBuffer(ctx, buffer_time, fade_time);
			fade1->SetBuffer(fade_buffer);
			fade2->SetBuffer(fade_buffer);
			fade1->Loop(true);
			fade2->Loop(true);

			auto mix1 = new GainNode(ctx, 0);
			auto mix2 = new GainNode(ctx, 0);
			ctx->Connect(fade1, mix1->gain_);
			ctx->Connect(fade2, mix2->gain_);

			ctx->Connect(input_, delay1);
			ctx->Connect(input_, delay2);
			ctx->Connect(delay1, mix1);
			ctx->Connect(delay2, mix2);
			ctx->Connect(mix1, wet_);
			ctx->Connect(mix2, wet_);

			mod1_ = mod1;
			mod2_ = mod2;
			mod3_ = mod3;
			mod4_ = mod4;
			mod1_gain_ = mod1_gain;
			mod2_gain_ = mod2_gain;
			mod3_gain_ = mod3_gain;
			mod4_gain_ = mod4_gain;
			mod_gain1_ = mod_gain1;
			mod_gain2_ = mod_gain2;
			fade1_ = fade1;
			fade2_ = fade2;
			mix1_ = mix1;
			mix2_ = mix2;
			delay1_ = delay1;
			delay2_ = delay2;

			SetPitchOffset(0.0);
		}

		void Dispose() override {
			if (mod1_) {
				mod1_->Dispose();
				delete mod1_; mod1_ = nullptr;
			}
			if (mod2_) {
				mod2_->Dispose();
				delete mod2_; mod2_ = nullptr;
			}
			if (mod3_) {
				mod3_->Dispose();
				delete mod3_; mod3_ = nullptr;
			}
			if (mod4_) {
				mod4_->Dispose();
				delete mod4_; mod4_ = nullptr;
			}
			if (mod1_gain_) {
				mod1_gain_->Dispose();
				delete mod1_gain_; mod1_gain_ = nullptr;
			}
			if (mod2_gain_) {
				mod2_gain_->Dispose();
				delete mod2_gain_; mod2_gain_ = nullptr;
			}
			if (mod3_gain_) {
				mod3_gain_->Dispose();
				delete mod3_gain_; mod3_gain_ = nullptr;
			}
			if (mod4_gain_) {
				mod4_gain_->Dispose();
				delete mod4_gain_; mod4_gain_ = nullptr;
			}
			if (mod_gain1_) {
				mod_gain1_->Dispose();
				delete mod_gain1_; mod_gain1_ = nullptr;
			}
			if (mod_gain2_) {
				mod_gain2_->Dispose();
				delete mod_gain2_; mod_gain2_ = nullptr;
			}
			if (fade1_) {
				fade1_->Dispose();
				delete fade1_; fade1_ = nullptr;
			}
			if (fade2_) {
				fade2_->Dispose();
				delete fade2_; fade2_ = nullptr;
			}
			if (delay1_) {
				delay1_->Dispose();
				delete delay1_; delay1_ = nullptr;
			}
			if (delay2_) {
				delay2_->Dispose();
				delete delay2_; delay2_ = nullptr;
			}
			if(mix1_){
				mix1_->Dispose();
				delete mix1_; mix1_ = nullptr;
			}
			if (mix2_) {
				mix2_->Dispose();
				delete mix2_; mix2_ = nullptr;
			}
			Effect::Dispose();
		}

		void Start() override {
			auto t = 0.05;
			auto t2 = t + buffer_time_ - fade_time_;
			mod1_->Start(0, 0, t * 1000);
			mod2_->Start(0, 0, t2 * 1000);
			mod3_->Start(0, 0, t * 1000);
			mod4_->Start(0, 0, t2 * 1000);
			fade1_->Start(0, 0, t * 1000);
			fade2_->Start(0, 0, t2 * 1000);
		}

		void Resume() override {
			mod1_->Resume();
			mod2_->Resume();
			mod3_->Resume();
			mod4_->Resume();
			fade1_->Resume();
			fade2_->Resume();
		}

		void SetDelay(float delay_time) {
			mod_gain1_->gain_->SetTargetAtTime(0.5 * delay_time, 0, 0.01 * 1000);
			mod_gain2_->gain_->SetTargetAtTime(0.5 * delay_time, 0, 0.01 * 1000);
		}

		void SetPitchOffset(float mul) {
			mul = Clamp(PITCH_OFFSET_MIN, PITCH_OFFSET_MAX, mul);
			if (mul > 0) {
				mod1_gain_->gain_->value_ = 0;
				mod2_gain_->gain_->value_ = 0;
				mod3_gain_->gain_->value_ = 1.0f;
				mod4_gain_->gain_->value_ = 1.0f;
			}
			else {
				mod1_gain_->gain_->value_ = 1.0f;
				mod2_gain_->gain_->value_ = 1.0f;
				mod3_gain_->gain_->value_ = 0;
				mod4_gain_->gain_->value_ = 0;
			}
			SetDelay(0.1  * abs(mul));
			previous_pitch_ = mul;
		}

		void SetOptions(float *opts, int16_t opt_count) override {
			if (opt_count > 0) {
				SetPitchOffset(opts[0]);
			}
			if (opt_count > 1) {
				CrossFade(opts[1]);
			}
		}
	};

	float Jungle::PITCH_OFFSET_DEFAULT = 0.0;
	float Jungle::PITCH_OFFSET_MIN = -2.0;
	float Jungle::PITCH_OFFSET_MAX = 2.0;
}

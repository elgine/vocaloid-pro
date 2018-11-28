#pragma once
#include <math.h>
#include "gain_node.hpp"
#include "delay_node.hpp"
#include "buffer_node.hpp"
#include "audio_context.hpp"
#include "audio_channel.hpp"
#include "effect.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::effect;

AudioChannel* CreateFadeBuffer(AudioContext *ctx, float active_time, float fade_time) {
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

AudioChannel* CreateDelayTimeBuffer(AudioContext *ctx, float active_time, float fade_time, bool shift_up) {
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

class PitchShifter : public Effect {
private:
	float previous_pitch_;
public:
	struct PitchShifterOptions {
		float pitch_offset;
	};

	AudioChannel *shift_down_buffer_;
	AudioChannel *shift_up_buffer_;
	BufferNode *mod1_;
	BufferNode *mod2_;
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


	PitchShifter(AudioContext *ctx, float delay_time = 0.1, float fade_time = 0.05, float buffer_time = 0.1):Effect(ctx) {
		previous_pitch_ = -1;

		auto mod1 = new BufferNode(ctx);
		auto mod2 = new BufferNode(ctx);
		auto mod3 = new BufferNode(ctx);
		auto mod4 = new BufferNode(ctx);

		shift_down_buffer_ = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, false);
		shift_up_buffer_ = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, true);

		mod1->SetBuffer(shift_down_buffer_);
		mod2->SetBuffer(shift_down_buffer_);
		mod3->SetBuffer(shift_up_buffer_);
		mod4->SetBuffer(shift_up_buffer_);
		mod1->loop_ = true;
		mod2->loop_ = true;
		mod3->loop_ = true;
		mod4->loop_ = true;

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
		fade1->loop_ = true;
		fade2->loop_ = true;

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

		auto t = 0.05;
		auto t2 = t + buffer_time - fade_time;

		mod1->Start(t * 1000);
		mod2->Start(t2 * 1000);
		mod3->Start(t * 1000);
		mod4->Start(t2 * 1000);
		fade1->Start(t * 1000);
		fade2->Start(t2 * 1000);

		mod1_ = mod1;
		mod2_ = mod2;
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

		//SetDelay(delay_time);
		SetPitchOffset(0.0);
	}

	void SetDelay(float delay_time) {
		mod_gain1_->gain_->SetTargetAtTime(0.5 * delay_time, 0, 0.01 * 1000);
		mod_gain2_->gain_->SetTargetAtTime(0.5 * delay_time, 0, 0.01 * 1000);
	}

	void SetPitchOffset(float mul) {
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

	void SetOptions(PitchShifterOptions options) {
		SetPitchOffset(options.pitch_offset);
	}
};
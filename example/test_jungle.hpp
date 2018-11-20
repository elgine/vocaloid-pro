#include <math.h>
#include "./jungle.hpp"
#include "../vocaloid/player_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto buffer_time = 0.1;
	auto fade_time = 0.05;
	auto ctx = new AudioContext();
	auto player = new PlayerNode(ctx);
	auto mod1 = new BufferNode(ctx);
	auto mod2 = new BufferNode(ctx);
	auto mod3 = new BufferNode(ctx);
	auto mod4 = new BufferNode(ctx);

	auto shift_down_buffer_ = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, false);
	auto shift_up_buffer_ = CreateDelayTimeBuffer(ctx, buffer_time, fade_time, true);

	mod1->SetBuffer(shift_down_buffer_);
	mod2->SetBuffer(shift_up_buffer_);
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
	ctx->Connect(mod1_gain, mod_gain1);
	ctx->Connect(mod3_gain, mod_gain1);
	ctx->Connect(mod2_gain, mod_gain2);
	ctx->Connect(mod4_gain, mod_gain2);

	auto delay1 = new DelayNode(ctx);
	auto delay2 = new DelayNode(ctx);
	ctx->Connect(mod_gain1, delay1->delay_time_);
	ctx->Connect(mod_gain2, delay2->delay_time_);


	auto fade1 = new BufferNode(ctx);
	auto fade2 = new BufferNode(ctx);
	auto fade_buffer = CreateFadeBuffer(ctx, buffer_time, fade_time);
	fade1->SetBuffer(fade_buffer);
	fade2->SetBuffer(fade_buffer);
	fade1->loop_ = true;
	fade2->loop_ = true;

	auto mix1 = new GainNode(ctx);
	auto mix2 = new GainNode(ctx);
	mix1->gain_->value_ = 0;
	mix2->gain_->value_ = 0;

	ctx->Connect(fade1, mix1->gain_);
	ctx->Connect(fade2, mix2->gain_);

	auto t = 0.05;
	auto t2 = t + buffer_time - fade_time;

	mod1->Start(t * 1000);
	mod3->Start(t * 1000);

	ctx->Prepare();
	ctx->Start();
	getchar();
	ctx->Close();
}	
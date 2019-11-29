#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/file_writer_node.hpp"
#include "equalizer.hpp"
#include "effects.h"
#include "role.hpp"
#include "role_factory.hpp"
#include "effect.hpp"
#include "effect_factory.hpp"
#include "env.hpp"
#include "env_factory.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace role;
using namespace effect;
using namespace env;

enum VoiceRendererState {
	RENDERER_STOP,
	RENDERER_RENDERING,
	RENDERER_FREE
};

class VoiceRenderer {

private:
	Role *role_;
	Env *env_;
	AudioContext *ctx_;
	FileWriterNode *writer_;
	FileReaderNode *source_reader_;
	Equalizer *equalizer_;
	GainNode *gain_;

	string path_;

	mutex tick_mutex_;
	VoiceRendererState state_;

	atomic<int64_t> timestamp_;
	atomic<int64_t> duration_;
	atomic<int64_t> start_timestamp_;
	atomic<int64_t> end_timestamp_;

	int64_t CalcCorrectPlayTime(int64_t delta) {
		int64_t timestamp = 0;
		auto source_sample_rate = source_reader_->SourceSampleRate();
		auto resample_ratio = float(source_sample_rate) / ctx_->SampleRate();
		// 当前播放片段
		auto seg_index = source_reader_->Index();
		auto seg = source_reader_->Segment(seg_index);
		// 期望的播放位置
		auto pos = source_reader_->Played();
		// 递归，若当前播放片段的开始偏移大于期望，则
		// 证明真实的播放位置应该在当前片段之前...
		while (delta > 0 && seg.start > pos - delta) {
			// 去掉当前片段相应的偏移量
			delta -= pos - seg.start;
			if (seg_index - 1 < 0) {
				seg_index += source_reader_->SegmentCount();
			}
			seg = source_reader_->Segment(--seg_index);
			pos = seg.end;
		}
		pos -= delta;
		timestamp = FramesToMsec(source_sample_rate, pos);
		return timestamp;
	}
public:

	explicit VoiceRenderer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		equalizer_ = new Equalizer(ctx_);
		gain_ = new GainNode(ctx_);
		ctx_->SetOutput(OutputType::RECORDER, sample_rate, channels);
		writer_ = static_cast<FileWriterNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		state_ = VoiceRendererState::RENDERER_FREE;
		role_ = nullptr;
		ctx_->Connect(equalizer_->output_, gain_);
		ctx_->Connect(gain_, writer_);
		ctx_->on_tick_->On([&](int) {
			bool need_update = true;
			auto processed_player = 0;
			auto processed = 0;
			unique_lock<mutex> lck(tick_mutex_);
			{
				unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
				processed = source_reader_->Processed();
				processed_player = writer_->Processed();
				if (role_) {
					processed_player = (processed_player - MsecToFrames(ctx_->SampleRate(), role_->Delay() * 1000)) / role_->TimeScale();
				}
				if (processed_player < 0) {
					need_update = false;
					processed_player = 0;
				}
				if (need_update)
					timestamp_ = CalcCorrectPlayTime(processed - processed_player);
			}
		});
	}

	int SetEnv(Envs id) {
		if (env_ == nullptr || (env_ && env_->Id() != id)) {
			Env* new_env = EnvFactory(id, ctx_);
			if (env_ != nullptr) {
				env_->Dispose();
				delete env_;
				env_ = nullptr;
			}
			env_ = new_env;
		}
		return SUCCEED;
	}

	int SetEnvOptions(double* options, int option_count) {
		static float* options_float = new float[500];
		for (auto i = 0; i < option_count; i++) {
			options_float[i] = options[i];
		}
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (env_) {
				env_->SetOptions(options_float, option_count);
			}
		}
	}

	int SetRole(Roles id) {
		if (role_ == nullptr || (role_ && role_->Id() != id)) {
			Role* new_role = RoleFactory(id, ctx_);
			if (role_ != nullptr) {
				role_->Dispose();
				delete role_;
				role_ = nullptr;
			}
			role_ = new_role;
		}
		return SUCCEED;
	}

	void SetRoleOptions(double* options, int option_count) {
		static float* options_float = new float[500];
		for (auto i = 0; i < option_count; i++) {
			options_float[i] = options[i];
		}
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (role_) {
				role_->SetOptions(options_float, option_count);
			}
		}
	}

	void SetEqualizerOptions(double* options) {
		if (options == nullptr)return;
		equalizer_->SetOptions((float*)(options), 11);
	}

	void SetGain(double gain) {
		gain_->gain_->value_ = gain;
	}

	void SetDest(const char* path) {
		writer_->SetPath(path);
	}

	int Open(const char* path) {
		Stop();
		auto ret = source_reader_->Open(path);
		if (ret < 0)return ret;
		path_ = path;
		return ret;
	}

	int RenderAll() {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->Start();
		}
		return ret;
	}

	int RenderSegments(int *segments, int segment_count) {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->StartWithSegments(segments, segment_count);
		}
		return ret;
	}

	int Start() {
		Stop();
		auto ret = SUCCEED;
		AudioNode* from = nullptr;
		if (role_) {
			ctx_->Connect(source_reader_, role_->Input());
			from = role_->Output();
		}
		else {
			from = source_reader_;
		}

		// Chain effects

		if (env_) {
			ctx_->Connect(from, env_->Input());
			ctx_->Connect(env_->Output(), equalizer_->Input());
		}
		else {
			ctx_->Connect(from, equalizer_->Input());
		}

		if (source_reader_->SegmentCount() == 0)
			RenderAll();
		if (role_) {
			role_->Start();
		}
		ret = ctx_->Prepare();
		if (ret < 0)return ret;
		state_ = VoiceRendererState::RENDERER_RENDERING;
		timestamp_ = 0;
		duration_ = source_reader_->Duration();
		start_timestamp_ = FramesToMsec(source_reader_->SourceSampleRate(), source_reader_->FirstSegment().start);
		end_timestamp_ = FramesToMsec(source_reader_->SourceSampleRate(), source_reader_->LastSegment().end);
		source_reader_->Seek(start_timestamp_);
		ctx_->Start();
		return ret;
	}

	void Resume() {
		if (state_ != VoiceRendererState::RENDERER_STOP)return;
		state_ = VoiceRendererState::RENDERER_RENDERING;
		ctx_->Start();
	}

	VoiceRendererState State() {
		return state_;
	}

	string Path() {
		return path_;
	}

	void Close() {
		Stop();
		ctx_->Close();
		Clear();
		state_ = VoiceRendererState::RENDERER_FREE;
	}

	void Dispose() {
		Close();
		if (role_) {
			role_->Dispose();
			delete role_;
			role_ = nullptr;
		}
	}

	void Clear() {
		ctx_->Clear();
	}

	void Stop() {
		if (state_ != VoiceRendererState::RENDERER_RENDERING)return;
		state_ = VoiceRendererState::RENDERER_STOP;
		ctx_->Stop();
	}

	void DisposeAll() {
		Stop();
		if (source_reader_ != nullptr) {
			source_reader_->Dispose();
			delete source_reader_;
			source_reader_ = nullptr;
		}
		if (equalizer_) {
			equalizer_->Dispose();
			delete equalizer_;
			equalizer_ = nullptr;
		}
		if (gain_) {
			gain_->Dispose();
			delete gain_;
			gain_ = nullptr;
		}
		if (writer_ != nullptr) {
			writer_->Dispose();
			delete writer_;
			writer_ = nullptr;
		}
		if (role_ != nullptr) {
			role_->Dispose();
			delete role_;
			role_ = nullptr;
		}
		if (ctx_ != nullptr) {
			ctx_->Dispose();
			delete ctx_;
			ctx_ = nullptr;
		}
	}

	bool IsEnd() {
		return timestamp_ >= end_timestamp_;
	}

	float Progress() {
		return float(timestamp_) / end_timestamp_;
	}
};
#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/file_writer_node.hpp"
#include "effects.h"
#include "effect.hpp"
#include "factory.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;

enum VoiceRendererState {
	RENDERER_STOP,
	RENDERER_RENDERING,
	RENDERER_FREE
};

class VoiceRenderer {

private:
	Effect *effect_;
	AudioContext *ctx_;
	FileWriterNode *writer_;
	FileReaderNode *source_reader_;
	string path_;

	mutex tick_mutex_;
	VoiceRendererState state_;

	atomic<int64_t> timestamp_;
	atomic<int64_t> duration_;

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
		ctx_->SetOutput(OutputType::RECORDER, sample_rate, channels);
		writer_ = static_cast<FileWriterNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		state_ = VoiceRendererState::RENDERER_FREE;
		effect_ = nullptr;
		ctx_->on_tick_->On([&](int) {
			bool need_update = true;
			auto processed_player = 0;
			auto processed = 0;
			unique_lock<mutex> lck(tick_mutex_);
			{
				unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
				processed = source_reader_->Processed();
				processed_player = writer_->Processed();
				if (effect_) {
					processed_player = (processed_player - MsecToFrames(ctx_->SampleRate(), effect_->Delay() * 1000)) / effect_->TimeScale();
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

	int SetEffect(Effects id) {
		if (effect_ == nullptr || (effect_ && effect_->Id() != id)) {
			Effect* new_effect = EffectFactory(id, ctx_);
			if (new_effect == nullptr)return NO_SUCH_EFFECT;
			if (effect_ != nullptr) {
				effect_->Dispose();
				delete effect_;
				effect_ = nullptr;
			}
			effect_ = new_effect;
		}
		return SUCCEED;
	}

	void SetDest(const char* path) {
		writer_->SetPath(path);
	}

	int Open(const char* path) {
		Stop();
		auto ret = source_reader_->Open(path);
		if (ret < 0)return ret;
		path_ = path;
		duration_ = source_reader_->Duration();
		return ret;
	}

	void SetOptions(double* options, int option_count) {
		static float* options_float = new float[500];
		for (auto i = 0; i < option_count; i++) {
			options_float[i] = options[i];
		}
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (effect_) {
				effect_->SetOptions(options_float, option_count);
			}
		}
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
		ctx_->stop_eof_ = true; 
		auto ret = SUCCEED;
		if (effect_) {
			ctx_->Connect(source_reader_, effect_->Input());
			ctx_->Connect(effect_->Output(), writer_);
		}
		else {
			ctx_->Connect(source_reader_, writer_);
		}
		if (source_reader_->SegmentCount() == 0)
			RenderAll();
		if (effect_) {
			effect_->Start();
		}
		ret = ctx_->Prepare();
		if (ret < 0)return ret;
		state_ = VoiceRendererState::RENDERER_RENDERING;
		timestamp_ = 0;
		source_reader_->Seek(0);
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
		ctx_->Dispose();
	}

	void Clear() {
		ctx_->Clear();
	}

	void Stop() {
		if (state_ != VoiceRendererState::RENDERER_RENDERING)return;
		state_ = VoiceRendererState::RENDERER_STOP;
		ctx_->Stop();
	}

	bool IsEnd() {
		unique_lock<mutex> lck(tick_mutex_);
		return timestamp_ >= duration_;
	}

	float Progress() {
		unique_lock<mutex> lck(tick_mutex_);
		return float(timestamp_) / duration_;
	}
};
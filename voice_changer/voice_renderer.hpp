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

	VoiceRendererState state_;

	int64_t timestamp_;
	int64_t duration_;
public:

	explicit VoiceRenderer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::RECORDER, sample_rate, channels);
		writer_ = static_cast<FileWriterNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		source_reader_->watched_ = true;
		state_ = VoiceRendererState::RENDERER_FREE;
		effect_ = nullptr;
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
		return ret;
	}

	void SetOptions(float* options, int option_count) {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (effect_) {
				effect_->SetOptions(options, option_count);
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
		return ctx_->SourceEof();
	}
};
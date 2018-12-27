#pragma once
#include <atomic>
#include <thread>
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "effect.hpp"
#include "effects.h"
#include "events.h"
#include "factory.hpp"
#include "status.h"
using namespace std;
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;

enum PlayerState {
	PLAYER_PLAYING,
	PLAYER_STOPPED,
	PLAYER_DISPOSED
};

class Player: public Emitter{
private:
	FileReaderNode *source_;
	AudioContext *ctx_;
	Effect *effect_;

	atomic<PlayerState> state_;
	
	int64_t **segments_;
	int segment_count_;

	condition_variable tick_condition_;
	mutex tick_mutex_;
	thread* tick_thread_;

	void OnTick() {
		int64_t samples, timestamp;
		while (true) {
			{
				unique_lock<mutex> lck(tick_mutex_);
				if (state_ != PlayerState::PLAYER_DISPOSED)break;
				while (state_ == PlayerState::PLAYER_STOPPED)
					tick_condition_.wait(lck);
			}
			timestamp = CalculateTimestamp(samples);
			
			Emit(TICK, &timestamp);
			this_thread::sleep_for(chrono::milliseconds(20));
		}
	}

	int64_t CalculateTimestamp(int64_t &samples) {
		int64_t timestamp = 0;
		ctx_->Lock();
		samples = ctx_->Destination()->Processed();
		timestamp = samples / ctx_->SampleRate();
		ctx_->Unlock();
		return timestamp;
	}

public:

	explicit Player(int32_t sample_rate = 44100, int16_t channels = 2): Emitter() {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		source_ = new FileReaderNode(ctx_);
		effect_ = nullptr;
		segments_ = nullptr;
		tick_thread_ = nullptr;
	}

	void SetFormat(int32_t sample_rate, int16_t channels) {
		ctx_->Destination()->SetFormat(sample_rate, channels);
	}

	void SetSegments(int64_t** segments, int segment_count) {
		if (segments_ != nullptr) {
			for (auto i = 0; i < segment_count_; i++) {
				delete[] segments_[i];
				segments_[i] = nullptr;
			}
			delete[] segments_;
			segments_ = nullptr;
		}

		segments_ = new int64_t*[segment_count];
		for (auto i = 0; i < segment_count; i++) {
			segments_[i] = new int64_t[2] {segments[i][0], segments[i][1]};
		}
		segment_count_ = segment_count;
	}

	int SetSource(const char* source) {
		return source_->SetPath(source);
	}

	int SetEffect(int effect_id) {
		if (effect_ != nullptr) {
			effect_->Dispose();
			delete effect_;
			effect_ = nullptr;
		}
		effect_ = EffectFactory(Effects(effect_id), ctx_);
		if (effect_ == nullptr) {
			return NO_SUCH_EFFECT;
		}
	}

	void Play(bool restart) {
		if (restart) {
			Stop();
			source_->Reset();
			ctx_->Destination()->Reset();
			if (effect_ != nullptr) {
				ctx_->Connect(source_, effect_->Input());
				ctx_->Connect(effect_->Output(), ctx_->Destination());
			}
			else {
				ctx_->Connect(source_, ctx_->Destination());
			}
			source_->Start(0);
			ctx_->Prepare();
		}
		else {
			source_->Start(0);
		}
		ctx_->Start();
		state_ = PlayerState::PLAYER_PLAYING;
		tick_condition_.notify_one();
		if(tick_thread_ == nullptr)tick_thread_ = new thread(&Player::OnTick, this);
	}

	void Seek(int64_t time) {
		ctx_->Lock();
		ctx_->Clear();
		source_->Seek(time);
		ctx_->Unlock();
	}

	void Stop() {
		state_ = PlayerState::PLAYER_STOPPED;
		ctx_->Lock();
		ctx_->Stop();
		ctx_->Unlock();	
	}

	void Dispose() {
		state_ = PlayerState::PLAYER_DISPOSED;
		tick_condition_.notify_one();
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}
};
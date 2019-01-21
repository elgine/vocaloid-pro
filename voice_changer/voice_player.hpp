#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../utility/signal.hpp"
#include "effect.hpp"
#include "factory.hpp"
#include "status.h"
#include <sstream>
using namespace std;
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;

enum VoicePlayerState {
	PLAYER_PLAYING,
	PLAYER_STOP
};

struct PlayerTickData {
	int timestamp;
	int duration;
};

typedef void(*OnPlayerTick)(PlayerTickData);
typedef void(*OnPlayerEnd)(int);

class VoicePlayer {

private:
	Signal<PlayerTickData> *on_tick_;
	Signal<int> *on_end_;

	AudioContext *ctx_;
	PlayerNode *player_;
	FileReaderNode *source_reader_;
	Effect* effect_;

	string path_;

	int64_t timestamp_;
	int64_t duration_;
	int64_t last_processed_player_;

	thread *tick_thread_;
	mutex tick_mutex_;
	atomic<VoicePlayerState> state_;

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

	void Tick() {
		int timestamp = 0, duration = 0, last_timestamp = 0;
		while (1) {
			if (!Playing())break;
			{
				unique_lock<mutex> lck(tick_mutex_);
				if (state_ == VoicePlayerState::PLAYER_STOP)break;
				timestamp = timestamp_;
				duration = duration_;
			}
			if (last_timestamp != timestamp) {
				on_tick_->Emit({ timestamp, duration });
				last_timestamp = timestamp;
			}

			if (timestamp >= duration) {
				on_end_->Emit(0);
				break;
			}
			this_thread::sleep_for(chrono::milliseconds(33));
		}
		ctx_->Stop();
		{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_STOP;
		}
	}

	void Join() {
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}

public:

	explicit VoicePlayer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		ctx_->stop_eof_ = true;
		player_ = static_cast<PlayerNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		source_reader_->watched_ = true;
		tick_thread_ = nullptr;
		state_ = VoicePlayerState::PLAYER_STOP;
		effect_ = nullptr;
		on_tick_ = new Signal<PlayerTickData>();
		on_end_ = new Signal<int>();
		timestamp_ = 0;
		duration_ = 0;
		ctx_->on_tick_->On([&](int) {
			bool need_update = true;
			auto processed_player = 0;
			auto processed = 0;
			unique_lock<mutex> lck(tick_mutex_);
			{
				unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
				processed = source_reader_->Processed();
				processed_player = player_->Processed();
				if (effect_) {
					processed_player = (processed_player) / effect_->TimeScale() - MsecToFrames(ctx_->SampleRate(), effect_->Delay() * 1000);
				}
				if (processed_player < 0) {
					need_update = false;
					processed_player = 0;
				}
				if(need_update)
					timestamp_ = CalcCorrectPlayTime(processed - processed_player);
			}
		});
	}

	void SubscribeTick(OnPlayerTick h) {
		on_tick_->On(h);
	}

	void SubscribeEnd(OnPlayerEnd h) {
		on_end_->On(h);
	}

	void Loop(bool v) {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			source_reader_->Loop(v);
		}
	}

	void SetFormat(int32_t sample_rate = 44100, int16_t channels = 2) {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			player_->SetFormat(sample_rate, channels);
		}
	}

	int SetOptions(float* options, int option_count) {
		int ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (effect_) {
				effect_->SetOptions(options, option_count); 
				source_reader_->Clear();
				player_->Clear();
			}
			else
				ret = NO_SUCH_EFFECT;
		}
		return ret;
	}

	int Open(const char* path) {
		auto ret = Stop();
		if (ret < 0)return ret;
		ret = source_reader_->Open(path);
		if (ret < 0)return ret;
		path_ = path;
		PlayAll();
		source_reader_->Clear();
		player_->Clear();
		duration_ = SourceDuration();
		return ret;
	}

	int SourceDuration() {
		return source_reader_->Duration();
	}

	io::AudioFormat* SourceFormat() {
		return source_reader_->Format();
	}

	int SetEffect(Effects id) {
		if (effect_ == nullptr || (effect_ && effect_->Id() != id)) {
			auto playing = Playing();
			Stop();
			Effect* new_effect = EffectFactory(id, ctx_);
			if (effect_ != nullptr) {
				effect_->Dispose();
				delete effect_;
				effect_ = nullptr;
			}
			effect_ = new_effect;
			if (playing) {
				Start(true); 
				Seek(timestamp_);
			}
		}
		return SUCCEED;
	}

	int PlayAll() {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->Start();
		}
		return ret;
	}

	int PlaySegments(int *segments, int segment_count) {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->StartWithSegments(segments, segment_count);
		}
		return ret;
	}

	bool Playing() {
		unique_lock<mutex> lck(tick_mutex_);
		return state_ == VoicePlayerState::PLAYER_PLAYING;
	}


	int Start(int restart = true) {
		if (Playing())return SUCCEED;
		Stop();
		auto ret = SUCCEED;
		if (effect_) {
			ctx_->Disconnect(source_reader_, player_);
			ctx_->Connect(source_reader_, effect_->Input());
			ctx_->Connect(effect_->Output(), player_);
		}
		else {
			ctx_->Connect(source_reader_, player_);
		}
		source_reader_->Resume();
		if (effect_) {
			effect_->Start();
		}
		if (restart) {
			ret = ctx_->Prepare();
			if (ret < 0)return ret;
		}
		ctx_->Start();
		state_ = VoicePlayerState::PLAYER_PLAYING;
		tick_thread_ = new thread(&VoicePlayer::Tick, this);
		return ret;
	}

	int Resume() {
		if (Playing())return SUCCEED;
		source_reader_->Resume();
		if (effect_) {
			effect_->Resume();
		}
		ctx_->Start();
		return SUCCEED;
	}

	int Stop() {
		if (Playing()) {
			// TODO: if can remove
			// auto ret = ctx_->Stop();
			{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_STOP;
			}
			Join();
			return SUCCEED;
		}else
			return SUCCEED;
	}

	int Seek(int64_t timestamp) {
		int ret = SUCCEED;
		{
			unique_lock<mutex> render(ctx_->audio_render_thread_mutex_);
			source_reader_->Clear();
			player_->Clear();
			ret = source_reader_->Seek(timestamp);
			source_reader_->Resume();
		}
		if(ret >= 0){
			unique_lock<mutex> lck(tick_mutex_);
			timestamp_ = timestamp;
		}
		return ret;
	}

	string Path() {
		return path_;
	}
};
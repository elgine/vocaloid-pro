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

typedef void(*OnPlayerTick)(int);
typedef void(*OnPlayerEnd)(int);

class VoicePlayer {

private:
	Signal<int> *on_tick_;
	Signal<int> *on_end_;

	AudioContext *ctx_;
	PlayerNode *player_;
	FileReaderNode *source_reader_;
	Effect* effect_;

	/*thread *tick_thread_;
	mutex tick_mutex_;*/
	atomic<VoicePlayerState> state_;
	string path_;

	int64_t timestamp_;

	/*atomic<int64_t> processed_, 
					processed_dest_,
					last_processed_dest_;*/

	//void Tick() {
	//	while (1) {
	//		if (!Playing())break;
	//		{
	//			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
	//			/*processed_ = source_reader_->Processed();
	//			processed_dest_ = player_->Processed();*/
	//		}
	//		// on_tick_->Emit((int)CalcCorrectPlayTime(processed, processed_dest));
	//		{
	//			unique_lock<mutex> lck(tick_mutex_);
	//			/*if (ctx_->State() == AudioContextState::STOPPED && 
	//				(processed <= processed_dest)) {
	//				{
	//					unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
	//					on_tick_->Emit(source_reader_->Duration());
	//				}
	//				on_end_->Emit(0);
	//				break;
	//			}*/
	//		}
	//		//last_processed_dest_ = processed_dest_;
	//		this_thread::sleep_for(chrono::milliseconds(22));
	//	}
	//	{
	//		unique_lock<mutex> lck(tick_mutex_);
	//		state_ = VoicePlayerState::PLAYER_STOP;
	//	}
	//}

	//int64_t CalcCorrectPlayTime(int64_t processed, int64_t processed_dest) {
	//	int64_t timestamp = 0;
	//	{
	//		unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
	//		auto source_sample_rate = source_reader_->SourceSampleRate();
	//		auto resample_ratio = float(source_sample_rate) / ctx_->SampleRate();
	//		processed *= resample_ratio;
	//		processed_dest *= resample_ratio;
	//		// 计算出“已处理”和“已播放”的差值
	//		auto delta = processed - processed_dest;
	//		// 当前播放片段
	//		auto seg_index = source_reader_->Index();
	//		auto seg = source_reader_->Segment(seg_index);
	//		// 期望的播放位置
	//		auto pos = source_reader_->Played();
	//		// 递归，若当前播放片段的开始偏移大于期望，则
	//		// 证明真实的播放位置应该在当前片段之前...
	//		while (delta > 0 && seg.start > pos - delta) {
	//			// 去掉当前片段相应的偏移量
	//			delta -= pos - seg.start;
	//			if (seg_index - 1 < 0) {
	//				delta = 0;
	//				break;
	//			}
	//			seg = source_reader_->Segment(--seg_index);
	//			pos = seg.end;
	//		}
	//		pos -= delta;
	//		timestamp = FramesToMsec(source_sample_rate, pos);
	//	}
	//	return timestamp;
	//}

	/*void Join() {
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}*/

	void UpdateTimestamp() {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);

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
		effect_ = nullptr;
		//tick_thread_ = nullptr;
		state_ = VoicePlayerState::PLAYER_STOP;
		on_tick_ = new Signal<int>();
		on_end_ = new Signal<int>();
		ctx_->on_tick_->On([&](int) {
			UpdateTimestamp();
			on_tick_->Emit(timestamp_);
		});
		ctx_->on_end_->On([&](int v) {
			on_end_->Emit(v);
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
			source_reader_->loop_ = v;
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
		//unique_lock<mutex> lck(tick_mutex_);
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
		/*state_ = VoicePlayerState::PLAYER_PLAYING;
		tick_thread_ = new thread(&VoicePlayer::Tick, this);*/
		return ret;
	}

	int Resume() {
		if (Playing())return SUCCEED;
		source_reader_->Resume();
		if (effect_) {
			effect_->Resume();
		}
		ctx_->Start();
		/*{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_PLAYING;
		}
		tick_thread_ = new thread(&VoicePlayer::Tick, this);*/
		return SUCCEED;
	}

	int Stop() {
		if (Playing()) {
			auto ret = ctx_->Stop();
			/*{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_STOP;
			}
			Join();*/
			return ret;
		}else
			return SUCCEED;
	}

	int Seek(int64_t timestamp) {
		int ret = SUCCEED;
		{
			unique_lock<mutex> render(ctx_->audio_render_thread_mutex_);
			if (source_reader_) {
				ret = source_reader_->Seek(timestamp);
			}
			else {
				ret = HAVE_NOT_DEFINED_SOURCE;
			}
		}
		return ret;
	}

	string Path() {
		return path_;
	}
};
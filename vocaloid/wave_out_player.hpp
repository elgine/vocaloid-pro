#pragma once
#ifdef _WIN32 || _WIN64
#include "player.h"
#include "status.h"
#include "../utility/buffer.hpp"
#include <windows.h>
#include <atomic>
#pragma comment(lib, "winmm.lib")
namespace vocaloid {
	namespace io {
		//Simple audio player for playing pcm data
		class WaveOutPlayer: public Player {
		private:
			static const int BUFFER_SIZE;

			WAVEFORMATEX format_;
			HWAVEOUT wave_out_;
			Buffer<char> *buf_;
			atomic<bool> playing_;
			atomic<bool> feeding_;
			atomic<int64_t> processed_;
			thread *playback_thread_;
			mutex playback_mutex_;
			condition_variable can_play_;
			
			WAVEHDR header_[2];

			// Play index
			int play_index_;


			bool CanPlay() {
				return buf_->Size() >= BUFFER_SIZE;
			}

			void NotifyToPlay() {
				unique_lock<mutex> lck(playback_mutex_);
				feeding_ = true;
				can_play_.notify_one();
			}

			static void CALLBACK WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD instance, DWORD dwParam1, DWORD dwParam2){
				WaveOutPlayer* p_this = (WaveOutPlayer*)instance;
				switch (uMsg) {
				case WOM_DONE:
				case WOM_OPEN:
					p_this->NotifyToPlay();
					break;
				}
				return;
			}

			void Play(int64_t buf_size) {
				// Playback
				header_[play_index_].dwBufferLength = buf_size;
				buf_->Pop(header_[play_index_].lpData, buf_size);
				waveOutWrite(wave_out_, &header_[play_index_], sizeof(WAVEHDR));
				processed_ += buf_size / format_.nBlockAlign;
				play_index_ = !play_index_;
				feeding_ = false;
			}

			void PlayLoop() {
				int ret = 0;
				while (true) {
					{
						unique_lock<mutex> lck(playback_mutex_);
						if (!playing_)break;
						while (playing_ && (!feeding_ || !CanPlay()))can_play_.wait(lck);
						// Playback
						Play(BUFFER_SIZE);
					}
					this_thread::sleep_for(chrono::milliseconds(1));
				}
			}

		public:
			WaveOutPlayer() {
				wave_out_ = nullptr;
				buf_ = new Buffer<char>();
				playing_ = false;
				feeding_ = false;
				playback_thread_ = nullptr;
				processed_ = 0;
				header_[0].dwFlags = 0;
				header_[1].dwFlags = 0;
				header_[0].lpData = (char*)malloc(BUFFER_SIZE);
				header_[1].lpData = (char*)malloc(BUFFER_SIZE);
				header_[0].dwBufferLength = BUFFER_SIZE;
				header_[1].dwBufferLength = BUFFER_SIZE;
			}

			~WaveOutPlayer() {
				Dispose();
			}

			int Open(int32_t nSamplesPerSec, int16_t wBitsPerSample, int16_t nChannels) override {
				format_.wFormatTag = WAVE_FORMAT_PCM;
				format_.nChannels = nChannels;
				format_.nSamplesPerSec = nSamplesPerSec;
				format_.wBitsPerSample = wBitsPerSample;
				format_.cbSize = 0;
				format_.nBlockAlign = (WORD)(format_.wBitsPerSample * format_.nChannels / 8);
				format_.nAvgBytesPerSec = format_.nChannels * format_.nSamplesPerSec * format_.wBitsPerSample / 8;

				// Support format?
				auto ret = waveOutOpen(0, WAVE_MAPPER, &format_, 0, 0, WAVE_FORMAT_QUERY);
				if (ret != MMSYSERR_NOERROR){
					return ret;
				}
				ret = waveOutOpen(&wave_out_, WAVE_MAPPER, &format_, (DWORD)WaveOutProc, (DWORD)this, CALLBACK_FUNCTION);
				if (ret != MMSYSERR_NOERROR) {
					return ret;
				}
				waveOutPrepareHeader(wave_out_, &header_[0], sizeof(WAVEHDR));
				waveOutPrepareHeader(wave_out_, &header_[1], sizeof(WAVEHDR));
				if (!(header_[0].dwFlags & WHDR_PREPARED) || !(header_[1].dwFlags & WHDR_PREPARED)) {
					return UNKNOWN_EXCEPTION;
				}
				play_index_ = 0;
				return ret;
			}

			int Start() override {
				if (Playing())return 0;
				Stop();
				playing_ = true;
				playback_thread_ = new thread(&WaveOutPlayer::PlayLoop, this);
				return 0;
			}

			int Stop() override {
				{
					unique_lock<mutex> lck(playback_mutex_);
					playing_ = false;
				}
				can_play_.notify_one();
				if (playback_thread_ && playback_thread_->joinable()) {
					playback_thread_->join();
					delete playback_thread_;
					playback_thread_ = nullptr;
				}
				return 0;
			}

			int64_t Played() override {
				unique_lock<mutex> lck(playback_mutex_);
				return processed_;
			}

			void Clear() override {
				unique_lock<mutex> lck(playback_mutex_);
				processed_ = 0;
				buf_->SetSize(0);
			}

			bool Playing() {
				unique_lock<mutex> lck(playback_mutex_);
				return playing_;
			}

			void Dispose() override {
				Stop();
				buf_->Dispose();
				waveOutClose(wave_out_);
				wave_out_ = nullptr;
			}

			int Push(const char* buf, size_t size) override {
				if (!Playing())Start();
				{
					unique_lock<mutex> lck(playback_mutex_);
					buf_->Add((char*)buf, size);
					can_play_.notify_all();
				}
				return size;
			}

			int Flush() override {
				return 0;
			}
		};

		const int WaveOutPlayer::BUFFER_SIZE = 16384 * 8;
	}
}
#endif
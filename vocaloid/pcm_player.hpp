#pragma once
#ifdef _WIN32 || _WIN64
#include "player.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")
namespace vocaloid {
	namespace io {
		// 1024 * 16 * 8
		#define MAX_BUFFER_SIZE (176000)
		//Simple audio player for playing pcm data
		class PCMPlayer: public Player {
		private:
			//Use event to control processing data
			HANDLE play_event_;
			HWAVEOUT wave_out_;
			// Double caching
			WAVEHDR wave_header_[2];
			// Buffer caching pcm data
			char* caching_buf_;
			// Buffer offset
			int buf_offset_;
			// Play index
			int play_index_;
			// Has played
			bool has_began_;

			int Play(const char* buf, int size) {
				WaitForSingleObject(play_event_, INFINITE);
				wave_header_[play_index_].dwBufferLength = size;
				memcpy(wave_header_[play_index_].lpData, buf, size);
				if (waveOutWrite(wave_out_, &wave_header_[play_index_], sizeof(WAVEHDR))) {
					SetEvent(play_event_);
					return -1;
				}
				play_index_ = !play_index_;
				return 0;
			}
		public:
			PCMPlayer() {
				wave_out_ = nullptr;
				wave_header_[0].dwFlags = 0;
				wave_header_[1].dwFlags = 0;
				wave_header_[0].lpData = (CHAR*)malloc(MAX_BUFFER_SIZE);
				wave_header_[1].lpData = (CHAR*)malloc(MAX_BUFFER_SIZE);
				wave_header_[0].dwBufferLength = MAX_BUFFER_SIZE;
				wave_header_[1].dwBufferLength = MAX_BUFFER_SIZE;

				caching_buf_ = (CHAR*)malloc(MAX_BUFFER_SIZE);
				play_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			}

			~PCMPlayer() {
				Dispose();
			}

			int Open(int32_t nSamplesPerSec, int16_t wBitsPerSample, int16_t nChannels) override {
				WAVEFORMATEX wfx;
				if (!caching_buf_ || !play_event_ || !wave_header_[0].lpData || !wave_header_[1].lpData) {
					return -1;
				}
				wfx.wFormatTag = WAVE_FORMAT_PCM;
				wfx.nChannels = nChannels;
				wfx.nSamplesPerSec = nSamplesPerSec;
				wfx.wBitsPerSample = wBitsPerSample;
				wfx.cbSize = 0;
				wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample * wfx.nChannels / 8);
				wfx.nAvgBytesPerSec = wfx.nChannels * wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;
				if (waveOutOpen(&wave_out_, WAVE_MAPPER, &wfx, (DWORD_PTR)play_event_, 0, CALLBACK_EVENT)) {
					return UNKNOWN_EXCEPTION;
				}
				waveOutPrepareHeader(wave_out_, &wave_header_[0], sizeof(WAVEHDR));
				waveOutPrepareHeader(wave_out_, &wave_header_[1], sizeof(WAVEHDR));
				if (!(wave_header_[0].dwFlags & WHDR_PREPARED) || !(wave_header_[1].dwFlags & WHDR_PREPARED)) {
					return UNKNOWN_EXCEPTION;
				}
				buf_offset_ = 0;
				play_index_ = 0;
				has_began_ = false;
				return 0;
			}

			int64_t Played() override {
				MMTIME time = {TIME_SAMPLES};
				waveOutGetPosition(wave_out_, &time, sizeof(time));
				return time.u.sample;
			}

			void Clear() override {
				if (wave_out_ == nullptr)return;
				Stop();
				buf_offset_ = 0;
				waveOutReset(wave_out_);
				Resume();
			}

			void Resume() override {
				waveOutRestart(wave_out_);
			}

			void Stop() override {
				waveOutPause(wave_out_);
			}

			void Dispose() override {
				if (wave_header_[0].lpData != nullptr) {
					free(wave_header_[0].lpData);
					wave_header_[0].lpData = nullptr;
				}
				if (wave_header_[1].lpData != nullptr) {
					free(wave_header_[1].lpData);
					wave_header_[1].lpData = nullptr;
				}
				if (caching_buf_ != nullptr) {
					free(caching_buf_);
					caching_buf_ = nullptr;
				}
				CloseHandle(play_event_);
				waveOutUnprepareHeader(wave_out_, &wave_header_[0], sizeof(WAVEHDR));
				waveOutUnprepareHeader(wave_out_, &wave_header_[1], sizeof(WAVEHDR));
				waveOutClose(wave_out_);
				wave_out_ = nullptr;
			}

			int Push(const char* buf, size_t size) override {
			again:
				if (buf_offset_ + size < MAX_BUFFER_SIZE) {
					memcpy(caching_buf_ + buf_offset_, buf, size);
					buf_offset_ += size;
				}
				else {
					memcpy(caching_buf_ + buf_offset_, buf, MAX_BUFFER_SIZE - buf_offset_);
					if (!has_began_) {
						if (0 == play_index_) {
							memcpy(wave_header_[0].lpData, caching_buf_, MAX_BUFFER_SIZE);
							play_index_ = 1;
						}
						else {
							ResetEvent(play_event_);
							memcpy(wave_header_[1].lpData, caching_buf_, MAX_BUFFER_SIZE);
							waveOutWrite(wave_out_, &wave_header_[0], sizeof(WAVEHDR));
							waveOutWrite(wave_out_, &wave_header_[1], sizeof(WAVEHDR));
							has_began_ = true;
							play_index_ = 0;
						}
					}
					else if (Play(caching_buf_, MAX_BUFFER_SIZE) < 0) {
						return -1;
					}
					size -= MAX_BUFFER_SIZE - buf_offset_;
					buf += MAX_BUFFER_SIZE - buf_offset_;
					buf_offset_ = 0;
					if (size > 0) goto again;
				}
				return 0;
			}

			int Flush() override {
				if (buf_offset_ > 0) {
					auto ret = Play(caching_buf_, buf_offset_);
					buf_offset_ = 0;
					return ret;
				}
				return 0;
			}
		};
	}
}
#endif
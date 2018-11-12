#pragma once
#include "player.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")
namespace vocaloid {
	namespace io {
		#define MAX_BUFFER_SIZE (1024 * 16 * 8)
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
				free(wave_header_[0].lpData);
				free(wave_header_[1].lpData);
				free(caching_buf_);
				CloseHandle(play_event_);
			}

			int Open(int32_t nSamplesPerSec, int16_t wBitsPerSample, int16_t nChannels) {
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
					return -1;
				}
				waveOutPrepareHeader(wave_out_, &wave_header_[0], sizeof(WAVEHDR));
				waveOutPrepareHeader(wave_out_, &wave_header_[1], sizeof(WAVEHDR));
				if (!(wave_header_[0].dwFlags & WHDR_PREPARED) || !(wave_header_[1].dwFlags & WHDR_PREPARED)) {
					return -1;
				}
				buf_offset_ = 0;
				play_index_ = 0;
				has_began_ = false;
				return 0;
			}

			void Close() {
				waveOutUnprepareHeader(wave_out_, &wave_header_[0], sizeof(WAVEHDR));
				waveOutUnprepareHeader(wave_out_, &wave_header_[1], sizeof(WAVEHDR));
				waveOutClose(wave_out_);
				wave_out_ = nullptr;
			}

			int Push(const char* buf, size_t size) {
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

			int Flush() {
				if (buf_offset_ > 0 && Play(caching_buf_, buf_offset_) < 0) {
					return -1;
				}
				return 0;
			}
		};
	}
}
#pragma once
#ifdef _WIN32 || _WIN64
#include <MMDeviceAPI.h>
#include <Audioclient.h>
#include "player.h"

#define REFTIMES_PER_SEC       (10000000)
#define REFTIMES_PER_MILLISEC  (10000)

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

namespace vocaloid {
	namespace io {	
		class WASAPIPlayer : public Player {
		private:
			IMMDevice *device_;
			IAudioClient *client_;
			bool inited_;
		public:

			explicit WASAPIPlayer() {
				device_ = nullptr;
				client_ = nullptr;
				inited_ = false;
			}

			int Open(int32_t sample_rate, int16_t bits, int16_t channels) {
				auto ret = 0;
				if (!inited_) {
					IMMDeviceEnumerator *enumerator = nullptr;
					ret = CoCreateInstance(
						CLSID_MMDeviceEnumerator, NULL,
						CLSCTX_ALL, IID_IMMDeviceEnumerator,
						(void**)&enumerator);
					if (FAILED(ret)) {
						return ret;
					}
					ret = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device_);
					if (FAILED(ret)) {
						delete enumerator;
						enumerator = nullptr;
						return ret;
					}

					ret = device_->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client_);
					if (FAILED(ret)) {
						goto failed;
					}

					WAVEFORMATEX *f = nullptr;
					client_->GetMixFormat(&f);

					auto origin_sample_rate = f->nSamplesPerSec;
					auto origin_bits = f->wBitsPerSample;
					auto origin_channels = f->nChannels;
					// Try to modify waveformat
					f->nSamplesPerSec = sample_rate;
					f->wBitsPerSample = bits;
					f->nChannels = channels;
					f->nBlockAlign = f->wBitsPerSample * f->nChannels / 8;
					f->nAvgBytesPerSec = f->nSamplesPerSec * f->nBlockAlign;

					ret = client_->Initialize(
						AUDCLNT_SHAREMODE_SHARED,
						0, REFTIMES_PER_SEC, 0, f, nullptr);
					if (FAILED(ret)) {
						// Fallback
						f->nSamplesPerSec = origin_sample_rate;
						f->wBitsPerSample = origin_bits;
						f->nChannels = origin_channels;
						f->nBlockAlign = f->wBitsPerSample * f->nChannels / 8;
						f->nAvgBytesPerSec = f->nSamplesPerSec * f->nBlockAlign;
						ret = client_->Initialize(
							AUDCLNT_SHAREMODE_SHARED,
							0, REFTIMES_PER_SEC, 0, f, nullptr);
						if (FAILED(ret))goto failed;
					}
					inited_ = true;
				}
				return ret;

			failed:
				if (device_) {
					delete device_;
					device_ = nullptr;
				}
				if (client_) {
					delete client_;
					client_ = nullptr;
				}
			}

			int Push(const char* buf, size_t size) {
				
			}

			int Flush() {
			
			}

			int64_t Played() {
			
			}

			void Clear() {
			
			}

			void Stop() {
			
			}

			void Dispose() {
			
			}

			void Resume() {
				
			}
		};
	}
}

#endif
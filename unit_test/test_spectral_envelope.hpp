#include "../vocaloid/lpc.hpp"
#include "../vocaloid/fft.hpp"
#include "../vocaloid/wav.hpp"
#include "../vocaloid/audio_channel.hpp"
using namespace vocaloid::dsp;
using namespace vocaloid::io;
using namespace vocaloid::node;

// Compute Real Cepstrum Of Signal
void RealCepstrum(FFT *fft, int n, float *signal, float *real_cepstrum){
	// Perform DFT
	fft->Forward(signal, n);
	printf("====================== Fft signal\n");
	for (auto i = 0; i < n; i++) {
		printf("%.4f + %.4fi\n", fft->real_[i], fft->imag_[i]);
	}
	// Calculate Log Of Absolute Value
	for (auto i = 0; i < n; i++){
		real_cepstrum[i] = logf(FFT::CalculateMagnitude(fft->real_[i], fft->imag_[i]));
	}
	printf("====================== Log magn\n");
	for (auto i = 0; i < n; i++) {
		printf("%.4f\n", real_cepstrum[i]);
	}
	// Perform Inverse FFT
	fft->Inverse(real_cepstrum, n, real_cepstrum);
}

void SpectralEnvelope(FFT *fft, float *real_cepstrum, float *w, int n, float *envelope) {
	for (auto i = 0; i < n; i++) {
		envelope[i] = real_cepstrum[i] * w[i];
	}
	fft->Forward(envelope, n);
	for (auto i = 0; i < n; i++) {
		envelope[i] = exp(2 * fft->real_[i]);
	}
	//memcpy(envelope, fft->real_, n * sizeof(float));
}

void LowpassWindow(int n, int mcep, float *win) {
	for (auto i = 0; i < n; i++) {
		if (i >= mcep) {
			win[i] = 0.0f;
		}
		else if (i == mcep) {
			win[i] = 0.5f;
		}
		else
			win[i] = 1.0f;
	}
}

void Run(){
	
	auto fft = new FFT();	
	auto reader = new WAVReader();
	reader->Open("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	auto format = reader->Format();
	auto s_time = 100;
	auto sample_size = 64;
	auto cepstrum = new float[sample_size];
	fft->Initialize(sample_size);
	reader->Seek(s_time);

	auto buffer_size = sample_size * format.bits / 8 * format.channels;
	auto buffer = new char[buffer_size];
	auto channel_data = new AudioChannel();
	channel_data->SetSize(sample_size);
	// Print source signal
	reader->ReadData(buffer, buffer_size);
	channel_data->FromByteArray(buffer, buffer_size, format.bits, format.channels);
	printf("====================== Source signal\n");
	for (auto i = 0; i < sample_size; i++) {
		printf("%.4f\n", channel_data->Channel(0)->Data()[i]);
	}
	RealCepstrum(fft, sample_size, channel_data->Channel(0)->Data(), cepstrum);

	printf("====================== Real cepstrum\n");
	for (auto i = 0; i < sample_size;i++) {
		printf("%.4f\n", cepstrum[i]);
	}

	// Generate low-pass filter
	auto w = new float[sample_size];
	auto envelope = new float[sample_size];
	LowpassWindow(sample_size, 40, w);
	//GenerateWin(WINDOW_TYPE::HANNING, sample_size, w);
	SpectralEnvelope(fft, cepstrum, w, sample_size, envelope);

	printf("====================== Spectral envelope\n");
	for (auto i = 0; i < sample_size; i++) {
		printf("%.4f\n", envelope[i]);
	}
	getchar();
}
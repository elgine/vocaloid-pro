#pragma once
#include "stdafx.h"
#include "fft.hpp"
using namespace std;
namespace vocaloid {

	namespace dsp {
		enum WAVEFORM_TYPE {
			SINE,
			TRIANGLE,
			SAWTOOTH,
			SQUARE,
			CUSTOM
		};

		template<typename T>
		void GenSine(T *wave, int64_t n) {
			for (auto i = 0; i < n; i++) {
				wave[i] = sin(2 * M_PI * i / n);
			}
		}

		template<typename T>
		void GenSquare(T *wave, int64_t n) {
			auto half = n / 2;
			for (auto i = 0; i < half; i++)wave[i] = 1;
			for (auto i = half; i < n; i++)wave[i] = -1;
		}

		template<typename T>
		void GenTriangle(T *wave, int64_t n) {
			auto half = n / 2;
			auto step = half / 2;
			for (auto i = 0; i < step; i++) {
				wave[i] = float(i) / step;
			}
			for (auto i = step; i < step + half; i++) {
				wave[i] = 2 - float(i) / step;
			}
			for (auto i = step + half; i < n; i++) {
				wave[i] = float(i - step - half) / step - 1;
			}
		}

		template<typename T>
		void GenSawTooth(T* wave, int64_t n) {
			auto half = n / 2;
			for (auto i = 0; i < half; i++) {
				wave[i] = float(i) / half;
			}
			for (auto i = half; i < n; i++) {
				wave[i] = float(i - half) / half - 1;
			}
		}

		template<typename T>
		void GenWaveform(WAVEFORM_TYPE type, int64_t n, T *wave) {
			switch (type) {
			case WAVEFORM_TYPE::SAWTOOTH:
				GenSawTooth(wave, n);
				break;
			case WAVEFORM_TYPE::TRIANGLE:
				GenTriangle(wave, n);
				break;
			case WAVEFORM_TYPE::SQUARE:
				GenSquare(wave, n);
				break;
			default:
				GenSine(wave, n);
				break;
			}
		}


		void GenSineCoefficients(float *real, float *imag, int64_t len) {
			real[0] = 0;
			imag[0] = 0;
			for (int i = 1; i < len; i++) {
				real[i] = 0;
				if (i == 1) {
					imag[i] = 1;
				}
				else {
					imag[i] = 0;
				}
			}
		}
		
		//http://mathworld.wolfram.com/FourierSeriesTriangleWave.html
		void GenTriangleCoefficients(float *real, float *imag, int64_t len) {
			real[0] = 0;
			imag[0] = 0;
			for (int i = 1; i < len; i++) {
				real[i] = 0;
				if (i & 1) {
					imag[i] = 8 / powf(i * (float)M_PI, 2.0f) * ((((i - 1) >> 1) & 1) ? -1 : 1);
				}else {
					imag[i] = 0;
				}
			}
		}
		
		void GenSawtoothCofficients(float *real, float *imag, int64_t len) {
			real[0] = 0;
			imag[0] = 0;
			for (int i = 1; i < len; i++) {
				real[i] = 0;
				imag[i] = powf(-1, i) * 2.0f / (i * (float)M_PI);
			}
		}
		
		// Generate waveform through predefined waveform-cofficient model.
		void GenWaveform(float *real, float *imag, int64_t len) {
			FFT(real, imag, len, -1, false);
		}
		
		// Generate custom waveform through data provided by user.
		void GenWaveformFourierSeries(WAVEFORM_TYPE wave_type, int64_t len, float *output) {
			if ((int)wave_type < (int)WAVEFORM_TYPE::SINE || (int)wave_type >(int)WAVEFORM_TYPE::CUSTOM) {
				throw "Should provide wave_type or real, imag array to generate waveform";
			}
			float *imag = nullptr;
			AllocArray(len, &imag);
			switch (wave_type) {
			case WAVEFORM_TYPE::SINE:GenSineCoefficients(output, imag, len); break;
			case WAVEFORM_TYPE::TRIANGLE:GenTriangleCoefficients(output, imag, len); break;
			case WAVEFORM_TYPE::SAWTOOTH:GenSawtoothCofficients(output, imag, len); break;
			default:GenSineCoefficients(output, imag, len); break;
			}
			GenWaveform(output, imag, len);
		}
	}
}
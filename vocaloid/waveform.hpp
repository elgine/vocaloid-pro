#pragma once
#include "stdafx.h"
#include "fft.hpp"
using namespace std;
// Generate a waveform between [0, 2pi]
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
	}
}
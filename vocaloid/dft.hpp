#pragma once
#include "maths.hpp"

namespace vocaloid {
	namespace dsp {

		int64_t DFT(float *in, int64_t size, float *real, float *imag) {
			int64_t freq_size = size * 0.5 + 1;
			for (auto k = 0; k < freq_size; k++) {
				real[k] = 0.0f;
				imag[k] = 0.0f;
				for (auto i = 0; i < size; i++) {
					real[k] += in[i] * cos(2 * M_PI * i / size);
					imag[k] += -in[i] * sin(2 * M_PI * i / size);
				}
			}
			return freq_size;
		}

		int64_t IDFT(float *real, float *imag, int64_t size, float *out) {
			for (auto k = 0; k < size; k++) {
				real[k] *= 2 / size;
				imag[k] *= 2 / size;
			}
			real[0] /= size;
			imag[size / 2] /= size;

			int64_t out_size = (size - 1) * 2;
			for (auto i = 0; i < out_size; i++){
				out[i] = 0.0f;
				for (int k = 0; k < size; k++){
					out[i] += real[k] * cos(2 * M_PI*k*i / out_size);
					out[i] += -imag[k] * sin(2 * M_PI*k*i / out_size);
				}
			}
			return out_size;
		}
	}
}
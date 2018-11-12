#pragma once
#include "stdafx.h"
#include <random>
// Generate white Gaussian noise samples
namespace vocaloid {

	void Randn(int64_t len, vector<float> &out) {
		random_device rd;
		mt19937 gen(rd());
		for (int i = 0; i < len; i++) {
			normal_distribution<float> normal(0, 1);
			out[i] = normal(gen);
		}
	}

	float CalculateSignalPower(vector<float> signal, int64_t len) {
		float sum = 0;
		for (int i = 0; i < len; i++) {
			sum += powf(abs(signal[i]), 2);
		}
		return sum / len;
	}

	// Generate white Gaussian noise samples
	// @param {int64_t} len - sample count
	// @param {float} p - noise power
	// @param {vector<float>&} out - output samples
	void WGN(int64_t len, float p, vector<float> &out) {
		float noise_power_sqrt = sqrtf(p);
		Randn(len, out);
		for (int i = 0; i < len; i++) {
			out[i] *= noise_power_sqrt;
		}
	}

	// Add white Gaussian noise samples to input signal
	// @param {vector<float>} signal - input samples
	// @param {int64_t} len - sample count
	// @param {float} snr - the ratio of noise
	// @param {vector<float>&} output - output samples
	void AWGN(vector<float> signal, int64_t len, float snr, vector<float> &output) {
		float signal_power = CalculateSignalPower(signal, len);
		float noise_power = signal_power / snr;
		vector<float> noise(len);
		WGN(len, noise_power, noise);
		for (int i = 0; i < len; i++) {
			output[i] = signal[i] + noise[i];
		}
	}
}
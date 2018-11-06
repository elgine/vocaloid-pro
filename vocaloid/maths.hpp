#pragma once
#include "stdafx.h"
namespace vocaloid {

	template<typename T>
	T Clamp(T min, T max, T v) {
		if (v < min)return min;
		else if (v > max)return max;
		return v;
	}

	template<typename T>
	T NextPow2(T v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	template<typename T>
	T LinearInterpolateStep(T y, T y_n, float percent) {
		return (y_n - y) * percent + y;
	}

	template<typename T>
	T ExponentialInterpolateStep(T y, T y_n, float percent) {
		if (y == 0 || percent == 0)return 0;
		return y * pow(double(y_n / y), percent);
	}

	template<typename T>
	void TDMA(vector<T> &output, const int n, const vector<T> A, const vector<T> B, vector<T> C, vector<T> D) {
		int i;
		float tmp;
		C[0] = C[0] / B[0];
		D[0] = D[0] / B[0];
		for (i = 1; i < n; i++) {
			tmp = (B[i] - A[i] * C[i - 1]);
			C[i] = C[i] / tmp;
			D[i] = (D[i] - A[i] * D[i - 1]) / tmp;
		}
		output[n - 1] = D[n - 1];
		for (i = n - 2; i >= 0; i--) {
			output[i] = D[i] - C[i] * output[i + 1];
		}
	}

	template<typename T>
	void CalD(const vector<T> y, int n, vector<T> &output) {
		vector<T> A(n, 1), C(n, 1), B(n, 4), D(n);
		for (int i = 0; i < n; i++) {
			if (i == 0)D[i] = 3 * (y[i + 1] - y[i]);
			else if (i == n - 1)D[i] = 3 * (y[i] - y[i - 1]);
			else D[i] = 3 * (y[i + 1] - y[i - 1]);
		}
		TDMA(output, n, A, B, C, D);
	}

	template<typename T>
	T Cubic(T yi, T yi1, T Di, T Di1, float percent) {
		return yi + Di * percent + (3 * (yi1 - yi) - 2 * Di - Di1) * powf(percent, 2) + (2.0f * (yi - yi1) + Di + Di1) * powf(percent, 3);
	}

	enum INTERPOLATOR_TYPE {
		NONE,
		LINEAR,
		EXPONENTIAL,
		CUBIC
	};


	// Linear interpolator
	template<typename T>
	void LinearInterpolate(const vector<T> input, uint64_t input_len, vector<T> &output, uint64_t output_len) {
		float ratio = (float)output_len / (input_len - 1);
		int next_offset = 0, offset = 0;
		for (int i = 0; i < input_len - 1; i++) {
			offset = (int)round(ratio * i);
			next_offset = (int)round(ratio * (i + 1));
			output[offset] = input[i];
			int gutter = next_offset - offset;
			for (int j = offset + 1; j < next_offset; j++) {
				float percent = (float)(j - offset) / gutter;
				output[j] = LinearInterpolateStep(input[i], input[i + 1], percent);
			}
		}
	}

	template<typename T>
	void ExponentialInterpolate(const vector<T> input, uint64_t input_len, vector<T> &output, uint64_t output_len) {
		float ratio = (float)output_len / (input_len - 1);
		int next_offset = 0, offset = 0;
		for (int i = 0; i < input_len - 1; i++) {
			offset = (int)round(ratio * i);
			next_offset = (int)round(ratio * (i + 1));
			output[offset] = input[i];
			int gutter = next_offset - offset;
			for (int j = offset + 1; j < next_offset; j++) {
				float percent = (float)(j - offset) / gutter;
				output[j] = ExponentialInterpolateStep(input[i], input[i + 1], percent);
			}
		}
	}

	// Cubic spline interpolator
	template<typename T>
	void CubicInterpolate(const vector<T> input, uint64_t input_len, vector<T> &output, uint64_t output_len) {
		vector<float> D(input_len);
		CalD(input, input_len, D);
		float ratio = (float)output_len / (input_len - 1);
		int next_offset = 0, offset = 0;
		for (int i = 0; i < input_len - 1; i++) {
			offset = (int)round(ratio * i);
			next_offset = (int)round(ratio * (i + 1));
			output[offset] = input[i];
			int gutter = next_offset - offset;
			for (int j = offset + 1; j < next_offset; j++) {
				float percent = (float)(j - offset) / gutter;
				output[j] = Cubic(input[i], input[i + 1], D[i], D[i + 1], percent);
			}
		}
	}

	template<typename T>
	void Interpolate(INTERPOLATOR_TYPE type, const vector<T> input, uint64_t input_len, vector<T> &output, uint64_t output_len) {
		switch (type) {
		case INTERPOLATOR_TYPE::CUBIC:
			CubicInterpolate(input, input_len, output, output_len);
			break;
		case INTERPOLATOR_TYPE::EXPONENTIAL:
			ExponentialInterpolate(input, input_len, output, output_len);
			break;
		default:
			LinearInterpolate(input, input_len, output, output_len);
			break;
		}
	}

	template<typename T>
	uint64_t Resample(vector<T> input, uint64_t input_len, INTERPOLATOR_TYPE interpolator, float ratio, vector<T>& output) {
		auto output_len = uint64_t(input_len * ratio);
		if (ratio == 1.0f)output.assign(input.begin(), input.begin() + input_len);
		else {
			if (output.size() != output_len)
				output.resize(output_len);
			if (ratio < 1) {
				for (int i = 0; i < output_len; i++) {
					auto index = round(i / ratio);
					output[i] = input[index >= input_len ? input_len - 1 : index];
				}
			}
			else {
				Interpolate(interpolator, input, input_len, output, output_len);
			}
		}
		return output_len;
	}
}
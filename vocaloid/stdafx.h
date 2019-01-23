#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <thread>
#include <mutex>

using namespace std;

#define LN2 0.69314718055994528623
#define M_PI 3.1415926
#define M_PI_2 M_PI * 2
#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_FRAME_SIZE 4096
#define MAX_FFT_SIZE 32768
#define MAX_FRAME_SIZE MAX_FFT_SIZE * 10
#define MINUS_SLEEP_UNIT 10
#define BITS_PER_SEC 16
#define EOF -1

template<typename T>
void AllocArray(int64_t len, T** arr) {
	//(*arr) = new T[len]{0};
	*arr = (T*)malloc(sizeof(T) * len);
	if (*arr != nullptr) {
		memset(*arr, 0, sizeof(T) * len);
	}
}

template<typename T>
void DeleteArray(T **arr) {
	if (*arr != nullptr) {
		free(*arr);
		*arr = nullptr;
		//delete[](*arr);
		(*arr) = nullptr;
	}
}
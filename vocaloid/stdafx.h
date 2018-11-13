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

#define M_PI 3.1415926
#define M_PI_2 M_PI * 2
#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_FRAME_SIZE 4096
#define MAX_FFT_SIZE 16384
#define MINUS_SLEEP_UNIT 5
#define BITS_PER_SEC 16

template<typename T>
void AllocArray(uint64_t len, T** arr) {
	*arr = new T[len]{};
}

template<typename T>
void DeleteArray(T **arr) {
	if (*arr != nullptr) {
		delete[](*arr);
		(*arr) = nullptr;
	}
}
#pragma once
#include "ane.h"
#include <string>
using namespace std;

FREObjectType GetType(FREObject obj) {
	FREObjectType type;
	if (FRE_OK != FREGetObjectType(obj, &type))return FREObjectType::FRE_TYPE_NULL;
	return type;
}

bool IsArray(FREObject obj) {
	return GetType(obj) == FREObjectType::FRE_TYPE_ARRAY;
}

bool IsNumber(FREObject obj) {
	return GetType(obj) == FREObjectType::FRE_TYPE_NUMBER;
}

bool IsString(FREObject obj) {
	return GetType(obj) == FREObjectType::FRE_TYPE_STRING;
}

int ArrayLen(FREObject obj) {
	if (!IsArray(obj))return -1;
	uint32_t len = 0;
	if (FRE_OK != FREGetArrayLength(obj, &len))return -1;
	return len;
}

const char* ToString(FREObject obj) {
	if (!IsString(obj))return nullptr;
	const uint8_t *str = 0;
	uint32_t len = 0;
	if (FRE_OK != FREGetObjectAsUTF8(obj, &len, &str))return nullptr;
	return (const char*)(str);
}

int ToInt(FREObject obj) {
	if (!IsNumber(obj))return 0;
	int v = 0;
	if (FRE_OK != FREGetObjectAsInt32(obj, &v))return 0;
	return v;
}

double ToDouble(FREObject obj) {
	if (!IsNumber(obj))return 0;
	double v = 0;
	if (FRE_OK != FREGetObjectAsDouble(obj, &v))return 0;
	return v;
}

int* ToIntArray(FREObject obj) {
	if (!IsArray(obj))return nullptr;
	auto len = ArrayLen(obj);
	if (len <= 0)return nullptr;
	auto data = new int[len];
	FREObject item = nullptr;
	for (auto i = 0; i < len; i++) {
		if (FRE_OK != FREGetArrayElementAt(obj, i, &item)) {
			goto fail;
		}
		if (IsNumber(obj)) {
			data[i] = ToInt(obj);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
}

double* ToDoubleArray(FREObject obj) {
	if (!IsArray(obj))return nullptr;
	auto len = ArrayLen(obj);
	if (len <= 0)return nullptr;
	auto data = new double[len];
	FREObject item = nullptr;
	for (auto i = 0; i < len; i++) {
		if (FRE_OK != FREGetArrayElementAt(obj, i, &item)) {
			goto fail;
		}
		if (IsNumber(obj)) {
			data[i] = ToDouble(obj);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
}


const char** ToStringArray(FREObject obj) {
	if (!IsArray(obj))return nullptr;
	auto len = ArrayLen(obj);
	if (len <= 0)return nullptr;
	auto data = new const char*[len];
	FREObject item = nullptr;
	for (auto i = 0; i < len; i++) {
		if (FRE_OK != FREGetArrayElementAt(obj, i, &item)) {
			goto fail;
		}
		if (IsString(obj)) {
			data[i] = ToString(obj);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
}

FREObject FromInt(int v) {
	FREObject obj = nullptr;
	FRENewObjectFromInt32(v, &obj);
	return obj;
}

FREObject FromString(const char* str) {
	FREObject obj = nullptr;
	FRENewObjectFromUTF8(strlen(str), (const uint8_t*)str, &obj);
	return obj;
}

FREObject FromDouble(double v) {
	FREObject obj = nullptr;
	FRENewObjectFromDouble(v, &obj);
	return obj;
}
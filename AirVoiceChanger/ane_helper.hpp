#pragma once
#include "ane.h"
#include <string>
#include "../utility/string_wstring.hpp"
#include "../utility/utf8_string.hpp"
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

bool IsBool(FREObject obj) {
	return GetType(obj) == FREObjectType::FRE_TYPE_BOOLEAN;
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
	//return WStringToString(StringToWString((const char*)str));
	return UTF8ToGBK((const char*)str);
}

bool ToBool(FREObject obj) {
	if (!IsBool(obj))return false;
	uint32_t v;
	if (FRE_OK != FREGetObjectAsBool(obj, &v))return false;
	return bool(v);
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
		if (IsNumber(item)) {
			data[i] = ToInt(item);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
	return nullptr;
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
		if (IsNumber(item)) {
			data[i] = ToDouble(item);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
	return nullptr;
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
		if (IsString(item)) {
			data[i] = ToString(item);
		}
		else {
			goto fail;
		}
	}
	return data;
fail:
	delete[] data;
	data = nullptr;
	return nullptr;
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
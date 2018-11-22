#pragma once
#include "stdafx.h"

enum Effect {
	MALE,
	FEMALE,
	ALIEN,
	ROBOT,
	ECHO
};


void SetParam();

void Prepare();

void Start();

void Stop();

void Close();
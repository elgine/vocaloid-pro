#pragma once
#include "stdafx.h"

enum Character {
	CHILD,
	MALE,
	FEMALE,
	ROBOT1,
	ROBOT2,
	ALIEN,
	ASTRONAUT,
	TRANSFORMER
};

enum Environment {
	TELEPHONE,
	CAVE,
	BROAD
};

void SetParam();

void Prepare();

void Start();

void Stop();

void Close();
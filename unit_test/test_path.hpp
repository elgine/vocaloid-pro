#pragma once
#include "stdafx.h"
#include <path.hpp>

TEST(TestPathUtil, Normal) {
	string path = "\\Desktop\\output.wav";
	ASSERT_EQ(GetExtension(path), ".wav");
	ASSERT_EQ(GetSubDirectory(path), "\\Desktop");
	ASSERT_EQ(GetFileName(path), "output.wav");
	ASSERT_EQ(CombinePath("C:\\Users\\Elgine", path), "C:\\Users\\Elgine\\Desktop\\output.wav");
}
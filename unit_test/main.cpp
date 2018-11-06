#include "stdafx.h"
#include "test_path.hpp"
#include "test_tone.hpp"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	getchar();
	return ret;
}
#pragma once
#include <map>
using namespace std;
namespace effect {
	enum Effects {
		NONE,
		// Character
		ALIEN,
		ROBOT1,
		ROBOT2,
		ASTRONAUT,
		UNCLE,
		FEMALE,
		CHILD,
		MALE,
		OLD_MALE,
		OLD_FEMALE,
		TRANSFORMER,
		BALROG,
		// Environment
		CAVE,
		BROAD_ROOM,
		UNDER_WATER,
		HALL,
		// Tools
		MUFFLER,
		TELEPHONE,
		RADIO,
		MEGAPHONE,
		PORTABLE_RADIO
	};

	static const int CHARACTER_COUNT = 12;
	Effects *CHARACTER = new Effects[CHARACTER_COUNT]{
		Effects::ALIEN,
		Effects::ROBOT1,
		Effects::ROBOT2,
		Effects::ASTRONAUT,
		Effects::UNCLE,
		Effects::FEMALE,
		Effects::CHILD,
		Effects::MALE,
		Effects::OLD_MALE,
		Effects::OLD_FEMALE,
		Effects::TRANSFORMER,
		Effects::BALROG
	};

	static const int ENVIRONMENT_COUNT = 4;
	Effects *ENVIRONMENT = new Effects[ENVIRONMENT_COUNT]{
		Effects::CAVE,
		Effects::BROAD_ROOM,
		Effects::UNDER_WATER,
		Effects::HALL
	};

	static const int TOOL_COUNT = 5;
	Effects *TOOL = new Effects[5]{
		Effects::MUFFLER,
		Effects::TELEPHONE,
		Effects::RADIO,
		Effects::MEGAPHONE,
		Effects::PORTABLE_RADIO
	};
}
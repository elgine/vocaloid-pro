#pragma once
#include "env.hpp"
#include "envs.h"
#include "broad_room.hpp"
#include "cave.hpp"
#include "hall.hpp"
#include "under_water.hpp"
namespace env {
	Env* EnvFactory(Envs id, BaseAudioContext *ctx) {
		Env* env = nullptr;
		switch (id) {
		case Envs::BROAD_ROOM:
			env = new BroadRoom(ctx);
			break;
		case Envs::CAVE:
			env = new Cave(ctx);
			break;
		case Envs::HALL:
			env = new Hall(ctx);
			break;
		case Envs::UNDER_WATER:
			env = new UnderWater(ctx);
			break;
		}
		return env;
	}
}
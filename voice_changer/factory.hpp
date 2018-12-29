#pragma once
#include "effect.hpp"
#include "effects.h"
#include "alien.hpp"
#include "robot1.hpp"
#include "robot2.hpp"
#include "astronaut.hpp"
#include "big_guy.hpp"
#include "female.hpp"
#include "child.hpp"
#include "male.hpp"
#include "old_male.hpp"
#include "old_female.hpp"
#include "transformer.hpp"
#include "balrog.hpp"
#include "cave.hpp"
#include "broad_room.hpp"
#include "under_water.hpp"
#include "hall.hpp"
#include "muffler.hpp"
#include "telephone.hpp"
#include "radio.hpp"
#include "megaphone.hpp"
namespace effect {

	Effect* EffectFactory(Effects id, AudioContext *ctx) {
		switch (id) {
		case Effects::ALIEN:return new Alien(ctx);
		case Effects::ROBOT1:return new Robot1(ctx);
		case Effects::ROBOT2:return new Robot2(ctx);
		case Effects::ASTRONAUT:return new Astronaut(ctx);
		case Effects::BIG_GUY:return new BigGuy(ctx);
		case Effects::FEMALE:return new Female(ctx);
		case Effects::CHILD:return new Child(ctx);
		case Effects::MALE:return new Male(ctx);
		case Effects::OLD_MALE:return new OldMale(ctx);
		case Effects::OLD_FEMALE:return new OldFemale(ctx);
		case Effects::TRANSFORMER:return new Transformer(ctx);
		case Effects::BALROG:return new Balrog(ctx);
		case Effects::CAVE:return new Cave(ctx);
		case Effects::BROAD_ROOM:return new BroadRoom(ctx);
		case Effects::UNDER_WATER:return new UnderWater(ctx);
		case Effects::HALL:return new Hall(ctx);
		case Effects::MUFFLER:return new Muffler(ctx);
		case Effects::TELEPHONE:return new Telephone(ctx);
		case Effects::RADIO:return new Radio(ctx);
		case Effects::MEGAPHONE:return new Megaphone(ctx);
		}
		return nullptr;
	}
}
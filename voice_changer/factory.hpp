#pragma once
#include "effect.hpp"
#include "effects.h"
#include "alien.hpp"
#include "robot1.hpp"
#include "robot2.hpp"
#include "astronaut.hpp"
#include "uncle.hpp"
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
#include "portable_radio.hpp"
namespace effect {

	Effect* EffectFactory(Effects id, BaseAudioContext *ctx) {
		Effect *effect = nullptr;
		switch (id) {
		case Effects::ALIEN:effect = new Alien(ctx); break;
		case Effects::ROBOT1:effect = new Robot1(ctx); break;
		case Effects::ROBOT2:effect = new Robot2(ctx); break;
		case Effects::ASTRONAUT:effect = new Astronaut(ctx); break;
		case Effects::UNCLE:effect = new Uncle(ctx); break;
		case Effects::FEMALE:effect = new Female(ctx); break;
		case Effects::CHILD:effect = new Child(ctx); break;
		case Effects::MALE:effect = new Male(ctx); break;
		case Effects::OLD_MALE:effect = new OldMale(ctx); break;
		case Effects::OLD_FEMALE:effect = new OldFemale(ctx); break;
		case Effects::TRANSFORMER:effect = new Transformer(ctx); break;
		case Effects::BALROG:effect = new Balrog(ctx); break;
		case Effects::CAVE:effect = new Cave(ctx); break;
		case Effects::BROAD_ROOM:effect = new BroadRoom(ctx); break;
		case Effects::UNDER_WATER:effect = new UnderWater(ctx); break;
		case Effects::HALL:effect = new Hall(ctx); break;
		case Effects::MUFFLER:effect = new Muffler(ctx); break;
		case Effects::TELEPHONE:effect = new Telephone(ctx); break;
		case Effects::RADIO:effect = new Radio(ctx); break;
		case Effects::MEGAPHONE:effect = new Megaphone(ctx); break;
		case Effects::PORTABLE_RADIO:effect = new PortableRadio(ctx); break;
		}
		return effect;
	}
}
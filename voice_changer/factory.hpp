#pragma once
#include "effect.hpp"
#include "effects.h"
#include "alien.hpp"
namespace effect {

	Effect* EffectFactory(Effects id, AudioContext *ctx) {
		switch (id) {
		case Effects::ALIEN:return new Alien(ctx);
		}
	}
}
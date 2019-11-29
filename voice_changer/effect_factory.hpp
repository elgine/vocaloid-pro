#pragma once
#include "effect.hpp"
#include "effects.h"
#include "auto_wah.hpp"
#include "chorus.hpp"
#include "equalizer.hpp"
#include "equalizer_3_band.hpp"
#include "flanger.hpp"
#include "jungle.hpp"
#include "muffler.hpp"
#include "telephone.hpp"
#include "radio.hpp"
#include "megaphone.hpp"
#include "ring_modulator.hpp"
#include "tremolo.hpp"
#include "vibrato.hpp"
namespace effect {

	Effect* EffectFactory(Effects id, BaseAudioContext *ctx) {
		Effect *effect = nullptr;
		switch (id) {
		case Effects::AUTO_WAH:effect = new AutoWah(ctx); break;
		case Effects::CHORUS: effect = new Chorus(ctx); break;
		case Effects::EQUALIZER:effect = new Equalizer(ctx); break;
		case Effects::EQUALIZER_3_BAND:effect = new Equalizer3Band(ctx); break;
		case Effects::DISTORTION:effect = new Distortion(ctx); break;
		case Effects::FLANGER:effect = new Flanger(ctx); break;
		case Effects::JUNGLE:effect = new Jungle(ctx); break;
		case Effects::MUFFLER:effect = new Muffler(ctx); break;
		case Effects::TELEPHONE:effect = new Telephone(ctx); break;
		case Effects::RADIO:effect = new Radio(ctx); break;
		case Effects::MEGAPHONE:effect = new Megaphone(ctx); break;
		case Effects::RING_MODULATOR:effect = new RingModulator(ctx); break;
		case Effects::TREMOLO:effect = new Tremolo(ctx); break;
		case Effects::VIBRATO:effect = new Vibrato(ctx); break;
		}
		return effect;
	}
}
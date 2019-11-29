#pragma once
#include "effect.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/diode_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
namespace effect {
	class RingModulator : public Effect {
	private:
		OscillatorNode *v_in_;
		GainNode* v_in_gain_;
		GainNode* v_in_inverter1_;
		GainNode* v_in_inverter2_;
		DiodeNode* v_in_diode1_;
		DiodeNode* v_in_diode2_;
		GainNode* v_in_inverter3_;
		GainNode* vc_inverter1_;
		DiodeNode* vc_diode3_;
		DiodeNode* vc_diode4_;
		DynamicsCompressorNode* compressor_;
		GainNode* out_gain_;
	public:

		static float RING_MODULATOR_SPEED_DEFAULT;
		static float RING_MODULATOR_SPEED_MIN;
		static float RING_MODULATOR_SPEED_MAX;

		static float RING_MODULATOR_DISTORTION_DEFAULT;
		static float RING_MODULATOR_DISTORTION_MIN;
		static float RING_MODULATOR_DISTORTION_MAX;

		explicit RingModulator(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::RING_MODULATOR;
			v_in_ = new OscillatorNode(ctx);
			v_in_gain_ = new GainNode(ctx);
			v_in_gain_->gain_->value_ = 0.5;
			v_in_inverter1_ = new GainNode(ctx);
			v_in_inverter1_->gain_->value_ = -1;
			v_in_inverter2_ = new GainNode(ctx);
			v_in_inverter2_->gain_->value_ = -1;
			v_in_diode1_ = new DiodeNode(ctx);
			v_in_diode2_ = new DiodeNode(ctx);
			v_in_inverter3_ = new GainNode(ctx);
			v_in_inverter3_->gain_->value_ = -1;
			vc_inverter1_ = new GainNode(ctx);
			vc_inverter1_->gain_->value_ = -1;
			vc_diode3_ = new DiodeNode(ctx);
			vc_diode4_ = new DiodeNode(ctx);
			out_gain_ = new GainNode(ctx);
			out_gain_->gain_->value_ = 3;
			compressor_ = new DynamicsCompressorNode(ctx);
			compressor_->threshold_ = -24;
			compressor_->ratio_ = 16;

			ctx->Connect(input_, vc_inverter1_);
			ctx->Connect(input_, vc_diode4_);
			ctx->Connect(vc_inverter1_, vc_diode3_);
			ctx->Connect(v_in_, v_in_gain_);
			ctx->Connect(v_in_gain_, v_in_inverter1_);
			ctx->Connect(v_in_gain_, vc_inverter1_);
			ctx->Connect(v_in_gain_, vc_diode4_);

			ctx->Connect(v_in_inverter1_, v_in_inverter2_);
			ctx->Connect(v_in_inverter1_, v_in_diode2_);
			ctx->Connect(v_in_inverter2_, v_in_diode1_);
			ctx->Connect(v_in_diode1_, v_in_inverter3_);
			ctx->Connect(v_in_diode2_, v_in_inverter3_);

			ctx->Connect(v_in_inverter3_, compressor_);
			ctx->Connect(vc_diode3_, compressor_);
			ctx->Connect(vc_diode4_, compressor_);

			ctx->Connect(compressor_, out_gain_);
			ctx->Connect(out_gain_, wet_);

			SetSpeed(RING_MODULATOR_SPEED_DEFAULT);
			SetDistortion(RING_MODULATOR_DISTORTION_DEFAULT);
		}

		void SetSpeed(float v) {
			v_in_->SetFrequency(Clamp(RING_MODULATOR_SPEED_MIN, RING_MODULATOR_SPEED_MAX, v));
		}

		void SetDistortion(float v) {
			v = Clamp(RING_MODULATOR_DISTORTION_MIN, RING_MODULATOR_DISTORTION_MAX, v);
			v_in_diode1_->h_ = v;
			v_in_diode2_->h_ = v;
			vc_diode3_->h_ = v;
			vc_diode4_->h_ = v;
			v_in_diode1_->UpdateCurve();
			v_in_diode2_->UpdateCurve();
			vc_diode3_->UpdateCurve();
			vc_diode4_->UpdateCurve();
		}

		void SetOptions(float *opts, int16_t opt_count) override {
			if (opt_count > 0) {
				SetSpeed(opts[0]);
			}
			if (opt_count > 1) {
				SetDistortion(opts[1]);
			}
			if (opt_count > 2) {
				CrossFade(opts[2]);
			}
		}

		void Start() override {
			v_in_->Start();
		}

		void Dispose() {
			if (v_in_) {
				v_in_->Dispose();
				delete v_in_;
				v_in_ = nullptr;
			}
			if (v_in_gain_) {
				v_in_gain_->Dispose();
				delete v_in_gain_;
				v_in_gain_ = nullptr;
			}
			if (v_in_inverter1_) {
				v_in_inverter1_->Dispose();
				delete v_in_inverter1_;
				v_in_inverter1_ = nullptr;
			}
			if (v_in_inverter2_) {
				v_in_inverter2_->Dispose();
				delete v_in_inverter2_;
				v_in_inverter2_ = nullptr;
			}
			if (v_in_diode1_) {
				v_in_diode1_->Dispose();
				delete v_in_diode1_;
				v_in_diode1_ = nullptr;
			}
			if (v_in_diode2_) {
				v_in_diode2_->Dispose();
				delete v_in_diode2_;
				v_in_diode2_ = nullptr;
			}
			if (v_in_inverter2_) {
				v_in_inverter2_->Dispose();
				delete v_in_inverter2_;
				v_in_inverter2_ = nullptr;
			}
			if (v_in_inverter3_) {
				v_in_inverter3_->Dispose();
				delete v_in_inverter3_;
				v_in_inverter3_ = nullptr;
			}
			if (vc_inverter1_) {
				vc_inverter1_->Dispose();
				delete vc_inverter1_;
				vc_inverter1_ = nullptr;
			}
			if (vc_diode3_) {
				vc_diode3_->Dispose();
				delete vc_diode3_;
				vc_diode3_ = nullptr;
			}
			if (vc_diode4_) {
				vc_diode4_->Dispose();
				delete vc_diode4_;
				vc_diode4_ = nullptr;
			}
			if (compressor_) {
				compressor_->Dispose();
				delete compressor_;
				compressor_ = nullptr;
			}
			if (out_gain_) {
				out_gain_->Dispose();
				delete out_gain_;
				out_gain_ = nullptr;
			}
			Effect::Dispose();
		}
	};

	float RingModulator::RING_MODULATOR_SPEED_DEFAULT = 30;
	float RingModulator::RING_MODULATOR_SPEED_MIN = 0;
	float RingModulator::RING_MODULATOR_SPEED_MAX = 2000;

	float RingModulator::RING_MODULATOR_DISTORTION_DEFAULT = 1;
	float RingModulator::RING_MODULATOR_DISTORTION_MIN = 0.2;
	float RingModulator::RING_MODULATOR_DISTORTION_MAX = 50;
}
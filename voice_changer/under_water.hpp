#pragma once
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/auto_wah.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "effect.hpp"
#include "effects.h"
#include "extract_resource.hpp"
#include "resource.h"
using namespace vocaloid::composite;
namespace effect {
	
	class UnderWater : public Effect {

	private:
		BiquadNode *lowpass_;
		DynamicsCompressorNode *compressor_;
		FileReaderNode *underwater_;
		vocaloid::composite::AutoWah *wahwah_;
		GainNode *input_gain_;
		GainNode *underwater_gain_;
	public:
		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float ENVELOPE_FOllOWER_FILTER_FREQ_DEFAULT;
		static float ENVELOPE_FOllOWER_FILTER_FREQ_MIN;
		static float ENVELOPE_FOllOWER_FILTER_FREQ_MAX;

		static float FILTER_DEPTH_DEFAULT;
		static float FILTER_DEPTH_MIN;
		static float FILTER_DEPTH_MAX;

		static float FILTER_Q_DEFAULT;
		static float FILTER_Q_MIN;
		static float FILTER_Q_MAX;

		static float INPUT_GAIN_DEFAULT;
		static float INPUT_GAIN_MIN;
		static float INPUT_GAIN_MAX;

		static float UNDER_WATER_BACKGROUND_GAIN_DEFAULT;
		static float UNDER_WATER_BACKGROUND_GAIN_MIN;
		static float UNDER_WATER_BACKGROUND_GAIN_MAX;

		explicit UnderWater(AudioContext *ctx) : Effect(ctx) {
			id_ = Effects::UNDER_WATER;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = LOWPASS_FREQ_DEFAULT;
			compressor_ = new DynamicsCompressorNode(ctx);

			underwater_ = new FileReaderNode(ctx);
			underwater_->Open(ExtractResource(IDR_UNDERWATER, L"mp3").data());
			underwater_->loop_ = true;
			underwater_gain_ = new GainNode(ctx, UNDER_WATER_BACKGROUND_GAIN_DEFAULT);

			wahwah_ = new vocaloid::composite::AutoWah(ctx);
			wahwah_->SetOptions({
				ENVELOPE_FOllOWER_FILTER_FREQ_DEFAULT,
				FILTER_DEPTH_DEFAULT,
				FILTER_Q_DEFAULT
			});

			input_gain_ = new GainNode(ctx, INPUT_GAIN_DEFAULT);

			ctx->Connect(input_gain_, wahwah_->input_);
			ctx->Connect(wahwah_->output_, lowpass_);
			ctx->Connect(lowpass_, compressor_);
			ctx->Connect(compressor_, gain_);
			ctx->Connect(underwater_, underwater_gain_);
			ctx->Connect(underwater_gain_, compressor_);
		}

		void SetLowpassFrequency(float v) {
			lowpass_->frequency_->value_ = Clamp(LOWPASS_FREQ_MIN, LOWPASS_FREQ_MAX, v);
		}

		void SetEnvelopeFollowerFilterFrequency(float v) {
			wahwah_->SetEnvelopeFollowerFilterFreq(Clamp(ENVELOPE_FOllOWER_FILTER_FREQ_MIN, ENVELOPE_FOllOWER_FILTER_FREQ_MAX, v));
		}

		void SetFilterDepth(float v) {
			wahwah_->SetFilterDepth(Clamp(FILTER_DEPTH_MIN, FILTER_DEPTH_MAX, v));
		}

		void SetFilterQ(float v) {
			wahwah_->SetFilterQ(Clamp(FILTER_Q_MIN, FILTER_Q_MAX, v));
		}

		void SetInputGain(float v) {
			input_gain_->gain_->value_ = Clamp(INPUT_GAIN_MIN, INPUT_GAIN_MAX, v);
		}

		void SetUnderWaterBackgroundGain(float v) {
			underwater_gain_->gain_->value_ = Clamp(UNDER_WATER_BACKGROUND_GAIN_MIN, UNDER_WATER_BACKGROUND_GAIN_MAX, v);
		}

		AudioNode *Input() {
			return input_gain_;
		}

		void Start() override {
			wahwah_->Start();
			underwater_->Start(0);
		}

		void Dispose() override {
			lowpass_->Dispose();
			delete lowpass_; lowpass_ = nullptr;
			compressor_->Dispose();
			delete compressor_; compressor_ = nullptr;
			underwater_->Dispose();
			delete underwater_; underwater_ = nullptr;
			wahwah_->Dispose();
			delete wahwah_; wahwah_ = nullptr;
			input_gain_->Dispose();
			delete input_gain_; input_gain_ = nullptr;
			underwater_gain_->Dispose();
			delete underwater_gain_; underwater_gain_ = nullptr;
			Effect::Dispose();
		}
	};

	float UnderWater::LOWPASS_FREQ_DEFAULT = 500;
	float UnderWater::LOWPASS_FREQ_MIN = 50;
	float UnderWater::LOWPASS_FREQ_MAX = 2500;

	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_DEFAULT = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_DEFAULT;
	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_MIN = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MIN;
	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_MAX = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MAX;

	float UnderWater::FILTER_DEPTH_DEFAULT = AutoWah::FILTER_DEPTH_DEFAULT;
	float UnderWater::FILTER_DEPTH_MIN = AutoWah::FILTER_DEPTH_MIN;
	float UnderWater::FILTER_DEPTH_MAX = AutoWah::FILTER_DEPTH_MAX;

	float UnderWater::FILTER_Q_DEFAULT = AutoWah::FILTER_Q_DEFAULT;
	float UnderWater::FILTER_Q_MIN = AutoWah::FILTER_Q_MIN;
	float UnderWater::FILTER_Q_MAX = AutoWah::FILTER_Q_MAX;

	float UnderWater::INPUT_GAIN_DEFAULT = 0.5f;
	float UnderWater::INPUT_GAIN_MIN = 0.0f;
	float UnderWater::INPUT_GAIN_MAX = 1.0f;

	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_DEFAULT = 0.3f;
	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_MIN = 0.0f;
	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_MAX = 1.0f;
}
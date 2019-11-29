#pragma once
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "auto_wah.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "env.hpp"
#include "extract_resource.hpp"
#include "resource.h"
using namespace vocaloid;
using namespace effect;
namespace env {
	
	class UnderWater : public Env {

	private:
		BiquadNode *lowpass_;
		DynamicsCompressorNode *compressor_;
		FileReaderNode *underwater_;
		AutoWah *wahwah_;
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

		static float UNDER_WATER_BACKGROUND_GAIN_DEFAULT;
		static float UNDER_WATER_BACKGROUND_GAIN_MIN;
		static float UNDER_WATER_BACKGROUND_GAIN_MAX;

		explicit UnderWater(BaseAudioContext *ctx) : Env(ctx) {
			id_ = Envs::UNDER_WATER;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = LOWPASS_FREQ_DEFAULT;
			compressor_ = new DynamicsCompressorNode(ctx);

			underwater_ = new FileReaderNode(ctx);
			underwater_->Open(ExtractResource(IDR_UNDERWATER, L"MP3").data());
			underwater_->Loop(true);
			underwater_gain_ = new GainNode(ctx, UNDER_WATER_BACKGROUND_GAIN_DEFAULT);

			wahwah_ = new AutoWah(ctx);

			ctx->Connect(input_, wahwah_->input_);
			ctx->Connect(wahwah_->output_, lowpass_);
			ctx->Connect(lowpass_, compressor_);
			ctx->Connect(compressor_, wet_);
			ctx->Connect(underwater_, underwater_gain_);
			ctx->Connect(underwater_gain_, compressor_);

			SetLowpassFrequency(LOWPASS_FREQ_DEFAULT);
			SetEnvelopeFollowerFilterFrequency(ENVELOPE_FOllOWER_FILTER_FREQ_DEFAULT);
			SetFilterDepth(FILTER_DEPTH_DEFAULT);
			SetFilterQ(FILTER_Q_DEFAULT);
			SetUnderWaterBackgroundGain(UNDER_WATER_BACKGROUND_GAIN_DEFAULT);
		}

		void Dispose() override {
			if (lowpass_) {
				lowpass_->Dispose();
				delete lowpass_; lowpass_ = nullptr;
			}
			if (compressor_) {
				compressor_->Dispose();
				delete compressor_; compressor_ = nullptr;
			}
			if (underwater_) {
				underwater_->Dispose();
				delete underwater_; underwater_ = nullptr;
			}
			if (wahwah_) {
				wahwah_->Dispose();
				delete wahwah_; wahwah_ = nullptr;
			}
			if (underwater_gain_) {
				underwater_gain_->Dispose();
				delete underwater_gain_; underwater_gain_ = nullptr;
			}
			Env::Dispose();
		}

		void SetOptions(float *options, int16_t option_count) override {
			if (option_count > 0) {
				SetLowpassFrequency(options[0]);
			}
			if (option_count > 1) {
				SetEnvelopeFollowerFilterFrequency(options[1]);
			}
			if (option_count > 2) {
				SetFilterDepth(options[2]);
			}
			if (option_count > 3) {
				SetFilterQ(options[3]);
			}
			if (option_count > 4) {
				SetUnderWaterBackgroundGain(options[4]);
			}
			if (option_count > 5) {
				CrossFade(options[5]);
			}
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

		void SetUnderWaterBackgroundGain(float v) {
			underwater_gain_->gain_->value_ = Clamp(UNDER_WATER_BACKGROUND_GAIN_MIN, UNDER_WATER_BACKGROUND_GAIN_MAX, v);
		}

		void Start() override {
			wahwah_->Start();
			underwater_->Start(0);
		}

		void Resume() override {
			wahwah_->Resume();
			underwater_->Resume();
		}
	};

	float UnderWater::LOWPASS_FREQ_DEFAULT = 500;
	float UnderWater::LOWPASS_FREQ_MIN = FLT_MIN;
	float UnderWater::LOWPASS_FREQ_MAX = FLT_MAX;

	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_DEFAULT = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_DEFAULT;
	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_MIN = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MIN;
	float UnderWater::ENVELOPE_FOllOWER_FILTER_FREQ_MAX = AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MAX;

	float UnderWater::FILTER_DEPTH_DEFAULT = AutoWah::FILTER_DEPTH_DEFAULT;
	float UnderWater::FILTER_DEPTH_MIN = AutoWah::FILTER_DEPTH_MIN;
	float UnderWater::FILTER_DEPTH_MAX = AutoWah::FILTER_DEPTH_MAX;

	float UnderWater::FILTER_Q_DEFAULT = AutoWah::FILTER_Q_DEFAULT;
	float UnderWater::FILTER_Q_MIN = AutoWah::FILTER_Q_MIN;
	float UnderWater::FILTER_Q_MAX = AutoWah::FILTER_Q_MAX;

	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_DEFAULT = 0.3f;
	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_MIN = 0.0f;
	float UnderWater::UNDER_WATER_BACKGROUND_GAIN_MAX = 4.0f;
}
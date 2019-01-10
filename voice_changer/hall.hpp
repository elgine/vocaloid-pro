#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../utility/buffer.hpp"
#include "effect.hpp"
#include "effects.h"
#include "extract_resource.hpp"
#include "resource.h"
namespace effect {

	class Hall : public Effect {
	private:
		ConvolutionNode *convolution_;
	public:
		explicit Hall(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::HALL;
			convolution_ = new ConvolutionNode(ctx);
			auto channel_data = new AudioChannel();
			auto buffer = new vocaloid::Buffer<char>();
			auto format = new vocaloid::io::AudioFormat();
			ReadFileBuffer(ExtractResource(IDR_CHURCH, L"wav").data(), format, buffer);
			channel_data->FromBuffer(buffer, format->bits, format->channels);
			convolution_->kernel_ = channel_data;
			ctx->Connect(convolution_, gain_);
		}

		void Dispose() override {
			if (convolution_) {
				convolution_->Dispose();
				delete convolution_;
				convolution_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode *Input() {
			return convolution_;
		}
	};
}
#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../utility/buffer.hpp"
#include "effect.hpp"
#include "effects.h"
namespace effect {

	class Muffler : public Effect {
	private:
		ConvolutionNode *convolution_;
	public:
		explicit Muffler(AudioContext *ctx) :Effect(ctx) {
			id_ = Effects::MUFFLER;
			convolution_ = new ConvolutionNode(ctx);
			auto channel_data = new AudioChannel();
			auto buffer = new vocaloid::Buffer<char>();
			auto format = new vocaloid::io::AudioFormat();
			ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\muffler.wav", format, buffer);
			channel_data->FromBuffer(buffer, format->bits, format->channels);
			convolution_->kernel_ = channel_data;
			ctx->Connect(convolution_, gain_);
		}

		AudioNode *Input() {
			return convolution_;
		}

		void Dispose() override {
			convolution_->Dispose();
			delete convolution_;
			convolution_ = nullptr;

			Effect::Dispose();
		}
	};
}
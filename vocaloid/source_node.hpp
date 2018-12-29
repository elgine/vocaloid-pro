#pragma once
#include "input_node.hpp"
#include "timeline.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public InputNode, public Timeline{
		protected:			
			bool ready_to_play_;
			int64_t delay_;

			int64_t delay_pos_;
			int64_t delay_bytes_;

			int64_t played_bytes_;
			int64_t play_pos_;
		public:

			explicit SourceNode(BaseAudioContext *ctx) :InputNode(ctx) {
				loop_ = false;
				ready_to_play_ = false;
				delay_ = 0;
				delay_pos_ = 0;
				delay_bytes_ = 0;
				played_bytes_ = 0;
				play_pos_ = 0;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				InputNode::Initialize(sample_rate, frame_size);
				delay_bytes_ = MsecToBytes(sample_rate, delay_);
			}

			void SetChannels(int16_t c) final {}

			virtual void GetBuffer(int64_t offset, int64_t len) {
				
			}

			void Seek(int64_t timestamp) {
				
			}

			virtual void SeekInternal(int64_t timestamp) {
				
			}

			int64_t ProcessFrame() override {
				auto byte_count = 0;
				if (!ready_to_play_){
					if (delay_pos_ + frame_size_ < delay_bytes_) {
						return 0;
					}
					else {
						ready_to_play_ = true;
						delay_pos_ += frame_size_;
						byte_count = delay_pos_ - delay_bytes_;
						play_pos_ += byte_count;
					}
				}

				if (SegmentCount() <= 0)return EOF;

				int64_t played_timestamp = 0, byte_left = 0, count = 0;
				int prev_segment_index = 0, cur_segment_index = 0;
				TimeSegment last_segment = Timeline::LastSegment(), first_segment = Timeline::FirstSegment();

				if (Timeline::IsEnd() && played_timestamp > last_segment.end && !loop_)return EOF;

				while (byte_count < frame_size_) {
					played_timestamp = BytesToMsec(sample_rate_, play_pos_);
					if (Timeline::IsEnd() && played_timestamp > last_segment.end) {
						if (loop_) {
							Timeline::Clear();
							play_pos_ = play_pos_ - Timeline::LastSegment().end + first_segment.start;
							played_timestamp = BytesToMsec(sample_rate_, play_pos_);
						}
						else {
							break;
						}
					}
					cur_segment_index = Timeline::Seek(played_timestamp);
					if(prev_segment_index != cur_segment_index)
						SeekInternal(played_timestamp);
					
					byte_left = MsecToBytes(sample_rate_, (Timeline::Segment(cur_segment_index).end - played_timestamp));
					count = min(frame_size_, byte_left);

					GetBuffer(byte_count, count);
					byte_count += count;
					play_pos_ += count;

					SetSegmentIndex(cur_segment_index);
					prev_segment_index = cur_segment_index;
				}
				return byte_count;
			}

			int Start(int64_t **segments, int segment_count, int64_t delay = 0) {
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				return SetSegments(segments, segment_count);
			}

			int Start(int64_t start = 0, int64_t duration = 0, int64_t delay = 0) {
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				return SetSegment(start, duration <= 0?Duration():duration);
			}

			void Clear() override {
				played_bytes_ = 0;
				play_pos_ = 0;
				delay_pos_ = 0;
			}

			virtual int64_t Duration() { return 0; }
		};
	}
}
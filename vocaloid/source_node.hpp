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
				return SUCCEED;
			}

			void SetChannels(int16_t c) final {}

			virtual int64_t GetBuffer(int64_t offset, int64_t len) {
				return 0;
			}

			void Seek(int64_t timestamp) {
				auto seg_index = Timeline::Seek(timestamp);
				if (seg_index > -1) {
					play_pos_ = timestamp * 0.001f * sample_rate_;
					auto seg = Timeline::Segment(seg_index);
					if (timestamp < seg.start || timestamp >= seg.end) {
						Timeline::SetSegmentIndex(seg_index);
					}
					SeekInternal(timestamp);
				}
			}

			virtual int64_t SeekInternal(int64_t timestamp) { return timestamp; }

			int64_t ProcessFrame() override {
				auto byte_count = 0;
				if (!ready_to_play_){
					if (delay_pos_ + frame_size_ < delay_bytes_) {
						return 0;
					}
					else {
						if (SegmentCount() <= 0)return EOF;
						ready_to_play_ = true;
						byte_count = delay_pos_ - delay_bytes_;
						Seek(Timeline::FirstSegment().start + BytesToMsec(sample_rate_, byte_count));
					}
				}

				if (SegmentCount() <= 0)return EOF;

				int64_t played_timestamp = 0, byte_left = 0, count = 0;
				
				int32_t source_sample_rate = SourceSampleRate();
				int64_t prev_segment_index = Timeline::Index(), cur_segment_index = Timeline::Index();
				TimeSegment last_segment = Timeline::LastSegment(), first_segment = Timeline::FirstSegment(), cur_segment = Segment(cur_segment_index);

				while (byte_count < frame_size_) {
					played_timestamp = BytesToMsec(source_sample_rate, play_pos_);
					if (Timeline::IsEnd() && played_timestamp >= last_segment.end) {
						if (loop_) {
							Timeline::SetSegmentIndex(0);
							play_pos_ = play_pos_ + MsecToBytes(source_sample_rate, -last_segment.end + first_segment.start);
							played_timestamp = BytesToMsec(source_sample_rate, play_pos_);
							cur_segment_index = 0;
							SeekInternal(played_timestamp);
						}
						else {
							break;
						}
					}
					else {
						cur_segment_index = Timeline::Seek(played_timestamp);
						if (prev_segment_index != cur_segment_index || cur_segment_index == -1) {
							play_pos_ -= MsecToBytes(source_sample_rate, cur_segment.end);
							cur_segment_index = Timeline::Next();
							if (cur_segment_index < 0)return EOF;
							cur_segment = Timeline::Segment(cur_segment_index);
							play_pos_ += MsecToBytes(source_sample_rate, cur_segment.start);
							played_timestamp = BytesToMsec(source_sample_rate, play_pos_);
							// Post update
							played_timestamp = SeekInternal(played_timestamp);
							play_pos_ = MsecToBytes(source_sample_rate, played_timestamp);
						}
					}
					cur_segment = Timeline::Segment(cur_segment_index);
					prev_segment_index = cur_segment_index;
					
					byte_left = MsecToBytes(source_sample_rate, (cur_segment.end - played_timestamp));
					count = min(frame_size_, byte_left);

					count = GetBuffer(byte_count, count);
					if(count <= 0)break;
					byte_count += count;
					play_pos_ += count;
				}
				return byte_count;
			}

			int StartWithSegments(int64_t **segments, int segment_count, int64_t delay = 0) {
				try {
					auto duration = Duration();
					int64_t last = 0;
					for (auto i = 0; i < segment_count; i++) {
						segments[i][0] = Clamp(int64_t(0), duration, segments[i][0]);
						segments[i][1] = Clamp(int64_t(0), duration, segments[i][1]);
						if (segments[i][0] < last) {
							segments[i][0] = last;
						}
						if (segments[i][1] < segments[i][0]) {
							segments[i][1] = segments[i][0] + 1;
						}
						last = segments[i][1];
					}
				}
				catch (exception e) {
					return INVALIDATE_SEGMENT_DATA;
				}
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				return SetSegments(segments, segment_count);
			}

			int Start(int64_t start = 0, int64_t duration = 0, int64_t delay = 0) {
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				start = Clamp(int64_t(0), Duration(), start);
				return SetSegment(start, (duration <= 0 || (start + duration) > Duration())?(Duration() - start):duration);
			}

			void Clear() override {
				played_bytes_ = 0;
				play_pos_ = 0;
				delay_pos_ = 0;
			}

			virtual int32_t SourceSampleRate() { return sample_rate_; }

			virtual int64_t Duration() { return 0; }
		};
	}
}
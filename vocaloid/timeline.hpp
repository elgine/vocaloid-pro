#pragma once
#include <stdint.h>
#include <vector>
using namespace std;

namespace vocaloid {
	namespace node {
		struct TimeSegment {
			int64_t start;
			int64_t end;
		};

		class Timeline {
		private:
			vector<TimeSegment> segments_;
			int64_t cur_index_;
		public:

			explicit Timeline() {}

			int SetSegment(int64_t start, int64_t end) {
				if (start >= end)return INVALIDATE_SEGMENT_DATA;
				Dispose();
				segments_.push_back({ start, end });
				return SUCCEED;
			}

			int SetSegments(int64_t **segments, int segment_count) {
				if (segment_count <= 0)return INVALIDATE_SEGMENT_DATA;
				Dispose();
				int64_t last_timestamp = 0, start = 0, end = 0;
				for (auto i = 0; i < segment_count; i++) {
					start = segments[i][0];
					end = segments[i][1];
					if (start < last_timestamp) {
						start = last_timestamp;
					}
					if (end < start) {
						end = start + 1;
					}
					segments_.push_back({ start, end });
					last_timestamp = end;
				}
				return SUCCEED;
			}

			int Seek(int64_t timestamp) {
				int64_t index = -1;
				if (timestamp < segments_[cur_index_].start || timestamp >= segments_[cur_index_].end) {
					int64_t start = 0, end = segments_.size() - 1, middle = 0;
					if (end <= 0)return -1;
					while (start <= end) {
						middle = (start + end) / 2;
						if (timestamp >= segments_[middle].start && timestamp < segments_[middle].end) {
							index = middle;
							break;
						}
						else if (timestamp >= segments_[middle].end) {
							start = middle + 1;
						}
						else {
							end = middle - 1;
						}
					}
				}
				else {
					index = cur_index_;
				}
				return index;
			}

			TimeSegment Segment(int index) {
				return segments_[index];
			}

			TimeSegment LastSegment() {
				return segments_[segments_.size() - 1];
			}

			TimeSegment FirstSegment() {
				return segments_[0];
			}

			int SegmentCount() {
				return segments_.size();
			}

			void SetSegmentIndex(int64_t index) {
				if (index < 0 || index >= segments_.size())return;
				cur_index_ = index;
			}

			int Next(bool loop = false) {
				if (!loop && cur_index_ == segments_.size() - 1)return -1;
				cur_index_++;
				return cur_index_;
			}

			bool IsEnd() {
				return cur_index_ >= segments_.size() - 1;
			}

			int Prev() {
				if (cur_index_ - 1 < 0)return -1;
				cur_index_--;
				return cur_index_;
			}

			void Clear() {
				cur_index_ = 0;
			}

			void Dispose() {
				cur_index_ = 0;
				segments_.clear();
			}

			int64_t Index() {
				return cur_index_;
			}
		};
	}
}
#pragma once
#include <stdint.h>
#include <vector>
#include <math.h>
#include "maths.hpp"
using namespace std;
namespace vocaloid {

	struct AudioTimelineValue {
		int64_t timestamp;
		float value;
		INTERPOLATOR_TYPE interpolator;
	};

	class AudioTimeline {
	protected:
		vector<AudioTimelineValue> value_list;
	public:
		float value_;

		explicit AudioTimeline(float value = 0.0f) {
			value_ = value;
		}

		int64_t FindIndex(int64_t timestamp, bool &accurate) {
			long long start = 0, last = (long long)value_list.size() - 1, middle = 0;
			long long delta = 0;
			accurate = false;
			if (last < 0)return 0;
			while (start <= last) {
				middle = (last + start) / 2;
				delta = (long long)timestamp - (long long)value_list[middle].timestamp;
				if (abs(delta) < 0.000001) {
					accurate = true;
					break;
				}
				else if (delta > 0) {
					start = middle + 1;
				}
				else {
					last = middle - 1;
				}
			}
			if (delta > 0) {
				middle++;
			}
			return middle;
		}

		void SetTargetAtTime(float v, int64_t start_time, int64_t time_constant) {
			SetValueAtTime(v, start_time);
			float end = v == 0.0 ? 1.0 : 0.0;
			ExponentialRampToValueAtTime(end, start_time + time_constant);
		}

		void SetValueAtTime(float v, int64_t end_time) {
			bool accurate = false;
			auto index = FindIndex(end_time, accurate);
			value_list.insert(value_list.begin() + index, { end_time, v, INTERPOLATOR_TYPE::NONE });
		}

		void ExponentialRampToValueAtTime(float v, int64_t end_time) {
			bool accurate = false;
			auto index = FindIndex(end_time, accurate);
			value_list.insert(value_list.begin() + index, { end_time, v, INTERPOLATOR_TYPE::EXPONENTIAL });
		}

		void LinearRampToValueAtTime(float v, int64_t end_time) {
			bool accurate = false;
			auto index = FindIndex(end_time, accurate);
			value_list.insert(value_list.begin() + index, { end_time, v, INTERPOLATOR_TYPE::LINEAR });
		}

		float GetValueAtTime(int64_t time) {
			bool accurate = false;
			int64_t index = FindIndex(time, accurate);
			if (index - 1 < 0)return value_;
			if (index >= (int64_t)value_list.size()) {
				return value_list[value_list.size() - 1].value;
			}
			float v = value_;
			if (accurate) {
				return value_list[index].value;
			}
			else {
				int64_t pre = index - 1;
				auto preV = value_list[pre].value;
				auto curV = value_list[index].value;
				auto duration = value_list[index].timestamp - value_list[pre].timestamp;
				auto interpolator = value_list[index].interpolator;
				if (interpolator == INTERPOLATOR_TYPE::LINEAR)
					v = LinearInterpolateStep(preV, curV, float(time - value_list[pre].timestamp) / duration);
				else if (interpolator == INTERPOLATOR_TYPE::EXPONENTIAL)
					v = ExponentialInterpolateStep(preV, curV, float(time - value_list[pre].timestamp) / duration);
			}
			return v;
		}

		bool TimelineEmpty() {
			return value_list.empty();
		}
	};
}
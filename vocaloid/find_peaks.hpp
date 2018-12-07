#include "stdafx.h"
#include <stdlib.h>
#include <iostream>

namespace vocaloid {
	namespace dsp {

		void FindPeaks(double *src, double src_lenth, double distance, int *indMax, int &indMax_len, int *indMin, int &indMin_len) {
			int *sign = (int*)malloc(src_lenth * sizeof(int));
			int max_index = 0,
				min_index = 0;
			indMax_len = 0;
			indMin_len = 0;

			for (int i = 1; i<src_lenth; i++) {
				double diff = src[i] - src[i - 1];
				if (diff>0)
					sign[i - 1] = 1;
				else if (diff<0)
					sign[i - 1] = -1;
				else
					sign[i - 1] = 0;
			}

			for (int j = 1; j<src_lenth - 1; j++) {
				double diff = sign[j] - sign[j - 1];
				if (diff<0)      indMax[max_index++] = j;
				else if (diff>0)indMin[min_index++] = j;
			}

			int *flag_max_index = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
			int *idelete = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
			int *temp_max_index = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
			int bigger = 0;
			double tempvalue = 0;
			int i, j, k;

			for (int i = 0; i < max_index; i++) {
				flag_max_index[i] = 0;
				idelete[i] = 0;
			}

			for (i = 0; i < max_index; i++) {
				tempvalue = -1;
				for (j = 0; j < max_index; j++) {
					if (!flag_max_index[j]) {
						if (src[indMax[j]] > tempvalue) {
							bigger = j;
							tempvalue = src[indMax[j]];
						}
					}
				}
				flag_max_index[bigger] = 1;
				if (!idelete[bigger]) {
					for (k = 0; k < max_index; k++) {
						idelete[k] |= (indMax[k] - distance <= indMax[bigger] & indMax[bigger] <= indMax[k] + distance);
					}
					idelete[bigger] = 0;
				}
			}
			for (i = 0, j = 0; i < max_index; i++) {
				if (!idelete[i])
					temp_max_index[j++] = indMax[i];
			}

			for (i = 0; i < max_index; i++) {
				if (i < j)
					indMax[i] = temp_max_index[i];
				else
					indMax[i] = 0;
			}

			max_index = j;


			for (int i = 0; i < min_index; i++) {
				flag_max_index[i] = 0;
				idelete[i] = 0;
			}

			for (i = 0; i < min_index; i++) {
				tempvalue = 1;
				for (j = 0; j < min_index; j++) {
					if (!flag_max_index[j]) {
						if (src[indMin[j]] < tempvalue) {
							bigger = j;
							tempvalue = src[indMin[j]];
						}
					}
				}
				flag_max_index[bigger] = 1;
				if (!idelete[bigger]) {
					for (k = 0; k < min_index; k++) {
						idelete[k] |= (indMin[k] - distance <= indMin[bigger] & indMin[bigger] <= indMin[k] + distance);
					}
					idelete[bigger] = 0;
				}
			}

			for (i = 0, j = 0; i < min_index; i++) {
				if (!idelete[i])
					temp_max_index[j++] = indMin[i];
			}

			for (i = 0; i < min_index; i++) {
				if (i < j)
					indMin[i] = temp_max_index[i];
				else
					indMin[i] = 0;
			}

			min_index = j;

			indMax_len = max_index;
			indMin_len = min_index;

			free(sign);
			free(flag_max_index);
			free(temp_max_index);
			free(idelete);
		}
	}
}
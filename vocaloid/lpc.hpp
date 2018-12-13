namespace vocaloid {
	namespace dsp {
		
		// @param r -- auto-correlation factor
		// @param p -- order
		// @param a -- lpc coefficient
		// @return err
		float LPC(const float *r, int p, float *a){
			int i, j;
			float err;

			if (0 == r[0]){
				for (i = 0; i < p; i++) a[i] = 0;
				return 0;
			}
			a[0] = 1.0;
			err = r[0];
			for (i = 0; i < p; i++){
				float lambda = 0.0;
				for (j = 0; j <= i; j++)
					lambda -= a[j] * r[i + 1 - j];
				lambda /= err;
				// Update LPC coefficients and total error
				for (j = 0; j <= (i + 1) / 2; j++){
					float temp = a[i + 1 - j] + lambda * a[j];
					a[j] = a[j] + lambda * a[i + 1 - j];
					a[i + 1 - j] = temp;
				}
				err *= (1.0 - lambda*lambda);
			}
			return err;
		}

		// @param x -- signal predicted
		// @param n -- signal len
		// @param r -- factor
		// @param k -- iteration
		void AutoCorrelation(const float *x, int n, float *r, int k){
			float d;
			int i, p;
			for (p = 0; p <= k; p++){
				for (i = 0, d = 0.0; i < n - p; i++)
					d += x[i] * x[i + p];
				r[p] = d;
			}
		}
	}
}
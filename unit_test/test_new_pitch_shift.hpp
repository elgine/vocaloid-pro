#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/window.hpp"
#include "../vocaloid/lpc.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;


#define MAX_FRAME_LENGTH 8192 * 16

void smbFft(float *fftBuffer, long fftFrameSize, long sign)
/*
FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
and returns the cosine and sine parts in an interleaved manner, ie.
fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
must be a power of 2. It expects a complex input signal (see footnote 2),
ie. when working with 'common' audio signals our input signal has to be
passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	long i, bitm, j, le, le2, k;

	for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
		for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j) {
			p1 = fftBuffer + i; p2 = fftBuffer + j;
			temp = *p1; *(p1++) = *p2;
			*(p2++) = temp; temp = *p1;
			*p1 = *p2; *p2 = temp;
		}
	}
	for (k = 0, le = 2; k < (long)(log(fftFrameSize) / log(2.) + .5); k++) {
		le <<= 1;
		le2 = le >> 1;
		ur = 1.0;
		ui = 0.0;
		arg = M_PI / (le2 >> 1);
		wr = cos(arg);
		wi = sign*sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer + j; p1i = p1r + 1;
			p2r = p1r + le2; p2i = p2r + 1;
			for (i = j; i < 2 * fftFrameSize; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr; *p2i = *p1i - ti;
				*p1r += tr; *p1i += ti;
				p1r += le; p1i += le;
				p2r += le; p2i += le;
			}
			tr = ur*wr - ui*wi;
			ui = ur*wi + ui*wr;
			ur = tr;
		}
	}
}

void SimpleSpectralEnvelope(float* magn, int64_t size, int64_t peak_count, float *envelope) {
	static float tmp[MAX_FRAME_LENGTH];
	static int peak_bins[MAX_FRAME_LENGTH];
	static float peak_value[MAX_FRAME_LENGTH];
	static bool init = false;
	if (!init) {
		memset(peak_value, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(tmp, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(peak_bins, 0, MAX_FRAME_LENGTH * sizeof(int));
		init = true;
	}
	memcpy(tmp, magn, size * sizeof(float));
	memset(envelope, 0, size * sizeof(float));
	memset(peak_value, 0, peak_count * sizeof(float));
	memset(peak_bins, 0, peak_count * sizeof(int));
	peak_bins[0] = 0.0f;
	peak_bins[1] = size - 1;

	auto peaks_found = 2;
	auto zero_width = (int)(size * 0.5f / peak_count);
	while (peaks_found < peak_count) {
		double max_v = 0;
		int max_at = 0;
		for (auto j = 0; j < size; j++) {
			if (tmp[j] > max_v) {
				max_v = tmp[j];
				max_at = j;
			}
		}
		if (max_v == 0)break;

		peak_bins[peaks_found - 1] = max_at;
		peaks_found++;

		auto zero_start = max(max_at - zero_width, 0);
		auto zero_end = min(max_at + zero_width + 1, size);
		for (auto index = zero_start; index < zero_end; index++) {
			tmp[index] = 0.0f;
		}
	}

	sort(peak_bins, peak_bins + peak_count);
	
	for (auto i = 0; i < peak_count; i++) {
		peak_value[i] = magn[peak_bins[i]];
	}

	double delta = 0;
	for (auto i = 1; i < peak_count; i++) {
		int this_bin_index = peak_bins[i];
		int prior_bin_index = peak_bins[i - 1];
		int dy = this_bin_index - prior_bin_index;
		if (dy == 0) {
			delta = 0;
		}
		else {
			delta = (peak_value[i] - peak_value[i - 1]) / dy;
		}

		double m = peak_value[i - 1];
		for (auto j = prior_bin_index; j < this_bin_index; j++) {
			envelope[j] = m;
			m += delta;
		}
	}
}

void LPCSpectralEnvelope(float *source, int64_t size, float *envelope) {
	static float tmp[MAX_FRAME_LENGTH];
	static float a[MAX_FRAME_LENGTH];
	static float fft[MAX_FRAME_LENGTH];
	static bool init = false;
	if (!init) {
		memset(tmp, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(a, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(fft, 0, 2 * MAX_FRAME_LENGTH * sizeof(float));
		init = true;
	}
	auto order = 12;
	AutoCorrelation(source, size, tmp, order);
	auto err = LPC(tmp, order, a);
	for (auto i = 0; i < size; i++) {
		fft[i * 2] = a[i];
	}
	smbFft(fft, size, -1);
	for (auto i = 0; i < size; i++) {
		envelope[i] = log(err * order) - log(sqrt(pow(fft[i * 2], 2) + pow(fft[i * 2 + 1], 2))/size);
	}
}

bool CepstrumSpectralEnvelope(float* magn, float f0, int64_t size, float *envelope) {
	static float tmp[MAX_FRAME_LENGTH];
	static float win[MAX_FRAME_LENGTH];
	static bool init = false;
	if (!init) {
		memset(tmp, 0, MAX_FRAME_LENGTH * sizeof(float));
		init = true;
	}
	
	bool valid = true;
	if (f0 <= 0)return false;
	for (auto i = 0; i < size; i++) {
		if (magn[i] == 0) {
			valid = false;
			break;
		}
		tmp[i * 2] = log(magn[i]);
		tmp[i * 2 + 1] = 0.0f;
	}
	if (!valid)return valid;
	smbFft(tmp, size, 1);
	auto cut_off = 50;
	GenerateWin(WINDOW_TYPE::HANNING, size, win);
	for (auto i = 0; i < size; i++) {
		tmp[2 * i] /= size;
		if (i > cut_off) {
			tmp[2 * i] = 0;
		}
		else if (i == cut_off) {
			tmp[2 * i] /= 2;
		}
	}
	smbFft(tmp, size, -1);
	for (auto i = 0; i < size; i++) {
		envelope[i] = tmp[i * 2];
	}
	return valid;
}

void smbPitchShift(float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, float *indata, float *outdata)
/*
Routine smbPitchShift(). See top of file for explanation
Purpose: doing pitch shifting while maintaining duration using the Short
Time Fourier Transform.
Author: (c)1999-2015 Stephan M. Bernsee <s.bernsee [AT] zynaptiq [DOT] com>
*/
{

	static float gInFIFO[MAX_FRAME_LENGTH];
	static float gOutFIFO[MAX_FRAME_LENGTH];
	static float gFFTworksp[2 * MAX_FRAME_LENGTH];
	static float gLastPhase[MAX_FRAME_LENGTH / 2 + 1];
	static float gSumPhase[MAX_FRAME_LENGTH / 2 + 1];
	static float gOutputAccum[2 * MAX_FRAME_LENGTH];
	static float gAnaFreq[MAX_FRAME_LENGTH];
	static float gAnaMagn[MAX_FRAME_LENGTH];
	static float gSynFreq[MAX_FRAME_LENGTH];
	static float gSynMagn[MAX_FRAME_LENGTH];
	static float envelope1[MAX_FRAME_LENGTH];
	static float envelope2[MAX_FRAME_LENGTH];
	static long gRover = false, gInit = false;
	double magn, phase, tmp, window, real, imag;
	double freqPerBin, expct;
	long i, k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;

	/* set up some handy variables */
	fftFrameSize2 = fftFrameSize / 2;
	stepSize = fftFrameSize / osamp;
	freqPerBin = sampleRate / (double)fftFrameSize;
	expct = 2.* M_PI *(double)stepSize / (double)fftFrameSize;
	inFifoLatency = fftFrameSize - stepSize;
	if (gRover == false) gRover = inFifoLatency;

	/* initialize our static arrays */
	if (gInit == false) {
		memset(gInFIFO, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(gOutFIFO, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(gFFTworksp, 0, 2 * MAX_FRAME_LENGTH * sizeof(float));
		memset(gLastPhase, 0, (MAX_FRAME_LENGTH / 2 + 1) * sizeof(float));
		memset(gSumPhase, 0, (MAX_FRAME_LENGTH / 2 + 1) * sizeof(float));
		memset(gOutputAccum, 0, 2 * MAX_FRAME_LENGTH * sizeof(float));
		memset(gAnaFreq, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(gAnaMagn, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(envelope1, 0, MAX_FRAME_LENGTH * sizeof(float));
		memset(envelope2, 0, MAX_FRAME_LENGTH * sizeof(float));
		gInit = true;
	}

	/* main processing loop */
	for (i = 0; i < numSampsToProcess; i++) {

		/* As long as we have not yet collected enough data just read in */
		gInFIFO[gRover] = indata[i];
		outdata[i] = gOutFIFO[gRover - inFifoLatency];
		gRover++;

		/* now we have enough data for processing */
		if (gRover >= fftFrameSize) {
			gRover = inFifoLatency;

			/* do windowing and re,im interleave */
			for (k = 0; k < fftFrameSize; k++) {
				window = -.5*cos(2.*M_PI*(double)k / (double)fftFrameSize) + .5;
				gFFTworksp[2 * k] = gInFIFO[k] * window;
				gFFTworksp[2 * k + 1] = 0.;
			}


			/* ***************** ANALYSIS ******************* */
			/* do transform */
			smbFft(gFFTworksp, fftFrameSize, -1);

			/* this is the analysis step */
			for (k = 0; k <= fftFrameSize2; k++) {

				/* de-interlace FFT buffer */
				real = gFFTworksp[2 * k];
				imag = gFFTworksp[2 * k + 1];

				/* compute magnitude and phase */
				magn = 2 * sqrt(real*real + imag*imag);
				phase = atan2(imag, real);

				/* compute phase difference */
				tmp = phase - gLastPhase[k];
				gLastPhase[k] = phase;

				/* subtract expected phase difference */
				tmp -= (double)k*expct;

				/* map delta phase into +/- Pi interval */
				qpd = tmp / M_PI;
				if (qpd >= 0) qpd += qpd & 1;
				else qpd -= qpd & 1;
				tmp -= M_PI*(double)qpd;

				/* get deviation from bin frequency from the +/- Pi interval */
				tmp = osamp*tmp / (2.*M_PI);

				/* compute the k-th partials' true frequency */
				tmp = (double)k*freqPerBin + tmp*freqPerBin;

				/* store magnitude and true frequency in analysis arrays */
				gAnaMagn[k] = magn;
				gAnaFreq[k] = tmp;
			}

			bool valid = true;
			//SimpleSpectralEnvelope(gAnaMagn, fftFrameSize2, 100, envelope1);
			valid = CepstrumSpectralEnvelope(gAnaMagn, gAnaFreq[0], fftFrameSize2, envelope1);

			/* ***************** PROCESSING ******************* */
			/* this does the actual pitch shifting */
			memset(gSynMagn, 0, fftFrameSize * sizeof(float));
			memset(gSynFreq, 0, fftFrameSize * sizeof(float));
			for (k = 0; k <= fftFrameSize2; k++) {
				index = k*pitchShift;
				if (index <= fftFrameSize2) {
					//gSynMagn[index] += gAnaMagn[k];
					if (valid && envelope1[k] != 0 && envelope1[index] != 0) {
						gSynMagn[index] = gAnaMagn[k] / envelope1[k] * envelope1[index];
					}
					gSynFreq[index] = gAnaFreq[k] * pitchShift;
				}
			}

			//CepstrumSpectralEnvelope(gSynMagn, fftFrameSize2 + 1, envelope2);

			/* ***************** SYNTHESIS ******************* */
			/* this is the synthesis step */
			for (k = 0; k <= fftFrameSize2; k++) {
				/* get magnitude and true frequency from synthesis arrays */
				magn = gSynMagn[k];
				tmp = gSynFreq[k];

				/* subtract bin mid frequency */
				tmp -= (double)k*freqPerBin;

				/* get bin deviation from freq deviation */
				tmp /= freqPerBin;

				/* take osamp into account */
				tmp = 2.*M_PI*tmp / osamp;

				/* add the overlap phase advance back in */
				tmp += (double)k*expct;

				/* accumulate delta phase to get bin phase */
				gSumPhase[k] += tmp;
				phase = gSumPhase[k];

				/* get real and imag part and re-interleave */
				gFFTworksp[2 * k] = magn*cos(phase);
				gFFTworksp[2 * k + 1] = magn*sin(phase);
			}

			/* zero negative frequencies */
			for (k = fftFrameSize + 2; k < 2 * fftFrameSize; k++) gFFTworksp[k] = 0.;

			/* do inverse transform */
			smbFft(gFFTworksp, fftFrameSize, 1);

			/* do windowing and add to output accumulator */
			for (k = 0; k < fftFrameSize; k++) {
				window = -.5*cos(2.*M_PI*(double)k / (double)fftFrameSize) + .5;
				gOutputAccum[k] += 2.*window*gFFTworksp[2 * k] / (fftFrameSize2*osamp);
			}
			for (k = 0; k < stepSize; k++) gOutFIFO[k] = gOutputAccum[k];

			/* shift accumulator */
			memmove(gOutputAccum, gOutputAccum + stepSize, fftFrameSize * sizeof(float));

			/* move input FIFO */
			for (k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k + stepSize];
		}
	}
}

class NewPitchShifterNode: public AudioNode {

public:

	float pitch_;
	long osamp_;

	explicit NewPitchShifterNode(AudioContext *ctx) :AudioNode(ctx) {
		pitch_ = 1.0f;
		osamp_ = 4;
		channels_ = 1;
	}

	int64_t ProcessFrame() {
		for (auto i = 0; i < channels_; i++) {
			smbPitchShift(pitch_, frame_size_, 1024, osamp_, sample_rate_, summing_buffer_->Channel(i)->Data(), result_buffer_->Channel(i)->Data());
		}
		return frame_size_;
	}
};

void Run() {
	
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto pitch_shifter = new NewPitchShifterNode(context);
	pitch_shifter->pitch_ = 0.67;
	auto source = new FileReaderNode(context);
	auto filter = new BiquadNode(context);
	filter->frequency_->value_ = 4000;
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	context->On(AudioContext::ALL_INPUT_NOT_LOOP_FINISHED, [](void*){
		printf("Finished");
	});

	context->Connect(source, pitch_shifter);
	context->Connect(pitch_shifter, filter);
	context->Connect(filter, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}
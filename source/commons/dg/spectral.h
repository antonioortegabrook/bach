/**
	@file	notation_spectral.h 
	@brief	Spectral tools for notation objects
	
	by Daniele Ghisi
*/

#ifndef _SPECTRAL_H_
#define _SPECTRAL_H_

#include "notation.h" // header with all the structures for the notation objects
#include "kiss_fft.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS
BEGIN_CHECK_LINKAGE
#endif

kiss_fft_scalar cpx_euclidean_dist(long num_entries, kiss_fft_cpx *vec1, kiss_fft_cpx *vec2);
kiss_fft_cpx cpx_product(kiss_fft_cpx num1, kiss_fft_cpx num2);
kiss_fft_cpx cpx_conjugate(kiss_fft_cpx num1);

kiss_fft_cpx polar_to_cpx(double radius, double angle);
kiss_fft_scalar get_cpx_ampli(kiss_fft_cpx num);
kiss_fft_scalar get_cpx_phase(kiss_fft_cpx num);

void bach_fft(int nfft, char is_inverse_fft, const kiss_fft_cpx *fin, double *ampli, double *phase); // uses kiss_fft
void bach_acf(int nfft, kiss_fft_cfg cfg, kiss_fft_cfg icfg, const kiss_fft_cpx *fin, kiss_fft_cpx *fout); // UNUSED
void bach_fft_cartesian_complex(int nfft, char is_inverse_fft, const kiss_fft_cpx *fin, kiss_fft_cpx *fout); // like bach_fft but returns frequency bin values in a+ib form

void bach_fm_acf(int nfft, const kiss_fft_cpx *fin, double sample_rate, double *freq_mapped_acf, char rectify, char filter_last_peak); // frequency-mapped autocorrelation function

void normalize(long num_samples, double *wave, double max_amplitude_to_set);
void windowize(long nfft, e_win_types window_type, const kiss_fft_cpx *fin, kiss_fft_cpx *fout);
void find_peaks_for_frame(int num_bins, double *fft_amplis, double *fft_phases, long max_num_peaks, double min_peaks_ampli, double min_relative_peaks_ampli,
					long *num_found_peaks, double **peak_bins, double **peak_amplis, double **peak_phases, long **peak_bin_widths, char reduce_to_half_spectrum);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
END_CHECK_LINKAGE
#endif

#endif // _SPECTRAL_H_
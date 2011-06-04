/**
 * \file scorer.h 
 * \brief object to score spectrum vs. spectrum or spectrum vs. scorer
 */

/*
 * AUTHOR: Chris Park
 * CREATE DATE: 9 Oct 2006
 * $Revision: 1.22 $
 *****************************************************************************/
#ifndef SCORER_H 
#define SCORER_H

#include <stdio.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include <string>
#include "objects.h"
#include "Spectrum.h"
#include "peptide.h"
#include "Ion.h"

/**
 * \returns An (empty) scorer object.
 */
SCORER_T* allocate_scorer(void);

/**
 * Instantiates a new scorer object from a filename. 
 * \returns a new scorer object
 */
SCORER_T* new_scorer(
  SCORER_TYPE_T type ///< the type of scorer -in
  );

/**
 * Frees an allocated scorer object.
 */
void free_scorer(
  SCORER_T* scorer ///< the ion collection to free - in
);

/**
 * Score a spectrum vs. an ion series
 */
FLOAT_T score_spectrum_v_ion_series(
  SCORER_T* scorer,        ///< the scorer object -in
  Spectrum* spectrum,      ///< the spectrum to score -in
  IonSeries* ion_series ///< the ion series to score against the spectrum -in
);

/**
 * Frees the single_ion_constraints array
 */
void free_single_ion_constraints(
  IonConstraint** ion_constraints
);

/**
 * Creates the an array of ion constraints for GMTK models.
 * TODO do we need one for paired and single? Do we want an iterator?
 */
IonConstraint** single_ion_constraints(
    void
);

/**
 * Score a spectrum vs. another spectrum
 */
FLOAT_T score_spectrum_v_spectrum(
  SCORER_T* scorer,           ///< the scorer object -in
  Spectrum* first_spectrum,   ///< the first spectrum to score -in
  Spectrum* second_spectrum   ///<  the second spectrum to score -in
);

/*************************************
 * Score for LOGP_*
 ************************************/

/**
 * Compute a p-value for a given score w.r.t. an exponential with the given parameters.
 *\returns the -log(p_value) of the exponential distribution
 */
FLOAT_T score_logp_exp_sp(
  FLOAT_T sp_score, ///< The sp score for the scoring peptide -in
  FLOAT_T mean      ///< The overall mean of the sp scored peptides -in
  );

/**
 * Compute a p-value for a given score w.r.t. an exponential with the given parameters.
 *\returns the -log(p_value) of the exponential distribution with Bonferroni correction
 */
FLOAT_T score_logp_bonf_exp_sp(
  FLOAT_T sp_score, ///< The sp score for the scoring peptide -in
  FLOAT_T mean,      ///< The overall mean of the sp scored peptides -in
  int num_peptide  ///< The number of peptides scored for sp
  );

/**
 * Apply a Bonferroni correction to a given p-value.
 * \returns the corrected -log(p_value)
 */
FLOAT_T bonferroni_correction(
  FLOAT_T p_value, ///< The uncorrected p-value.
  int num_tests ///< The number of tests performed.
  );

/**
 * Compute a p-value for a given score w.r.t. a Weibull with given parameters.
 *\returns the p_value
 */
FLOAT_T compute_weibull_pvalue(
  FLOAT_T score, ///< The score for the scoring peptide -in
  FLOAT_T eta,   ///< The eta parameter of the Weibull -in
  FLOAT_T beta,  ///< The beta parameter of the Weibull -in
  FLOAT_T shift  ///< The shift parameter of the Weibull -in
  );

/**
 * Compute a p-value for a given score w.r.t. a Weibull with given parameters.
 *\returns the -log(p_value)
 */
double score_logp_bonf_weibull(
  FLOAT_T score, ///< The score for the scoring peptide -in
  FLOAT_T eta,  ///< The eta parameter of the Weibull
  FLOAT_T beta, ///< The beta parameter of the Weibull
  FLOAT_T shift, ///< The shift parameter of the Weibull
  int num_peptides ///< The number of peptides
  );


/**
 * Compute a p-value for a given score w.r.t. an EVD with the given parameters.
 *\returns the -log(p_value) of the EVD distribution 
 */
FLOAT_T score_logp_evd_xcorr(
  FLOAT_T xcorr_score, ///< The xcorr score for the scoring peptide -in
  FLOAT_T mu, ///<  EVD parameter Xcorr(characteristic value of extreme value distribution) -in
  FLOAT_T l_value ///< EVD parameter Xcorr(decay constant of extreme value distribution) -in
  );

/**
 * Compute a p-value for a given score w.r.t. an EVD with the given parameters.
 *\returns the -log(p_value) of the EVD distribution with Bonferroni correction
 */
FLOAT_T score_logp_bonf_evd_xcorr(
  FLOAT_T xcorr_score, ///< The xcorr score for the scoring peptide -in
  FLOAT_T mu, ///<  EVD parameter Xcorr(characteristic value of extreme value distribution) -in
  FLOAT_T l_value, ///< EVD parameter Xcorr(decay constant of extreme value distribution) -in
  int num_peptide  ///< The number of peptides scored for sp -in
  );

/*******************************
 * get, set methods for scorer
 *******************************/

/**
 *\returns the score type of the scorer
 */
SCORER_TYPE_T get_scorer_type(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer type
 */
void set_scorer_type(
  SCORER_T* scorer, ///< the scorer object -out                     
  SCORER_TYPE_T type ///< The type of scorer -in
  );

/**
 *\returns the beta value of the scorer
 */
FLOAT_T get_scorer_sp_beta(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer beta value
 */
void set_scorer_sp_beta(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_beta ///< used for Sp: the beta variable -in
  );

/**
 *\returns the gamma value of the scorer
 */
FLOAT_T get_scorer_sp_gamma(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer gamma value
 */
void set_scorer_sp_gamma(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_gamma ///< used for Sp: the gamma variable -in
  );


/**
 *\returns the min_mz value of the scorer
 */
FLOAT_T get_scorer_sp_min_mz(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer min_mz value
 */
void set_scorer_sp_min_mz(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_min_mz ///< used for Sp: the min_mz variable -in
  );


/**
 *\returns the max_mz value of the scorer
 */
FLOAT_T get_scorer_sp_max_mz(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer max_mz value
 */
void set_scorer_sp_max_mz(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_max_mz ///< used for Sp: the max_mz variable -in
  );

/**
 *\returns the max bin index of the scorer array(s).
 */
int get_scorer_max_bin(SCORER_T* scorer);

/**
 *\returns the sp_array_resolution value of the scorer
 */
FLOAT_T get_scorer_sp_array_resolution(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer sp_array_resolution value
 */
void set_scorer_sp_array_resolution(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_array_resolution ///< used for Sp: the sp_array_resolution variable -in
  );

/**
 *\returns the sp_sum_resolution value of the scorer
 */
FLOAT_T get_scorer_sp_sum_resolution(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer sp_sum_resolution value
 */
void set_scorer_sp_sum_resolution(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_sum_resolution ///< used for Sp: the sp_sum_resolution variable -in
  );

/**
 *\returns the equalize_resolution value of the scorer
 */
FLOAT_T get_scorer_sp_equalize_resolution(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *sets the scorer equalize_resolution value
 */
void set_scorer_sp_equalize_resolution(
  SCORER_T* scorer, ///< the scorer object -out                     
  FLOAT_T sp_equalize_resolution ///< used for Sp: the equalize_resolution variable -in
  );

/**
 *\returns the fraction of b,y ions matched for scoring SP, the values is valid for the last ion series scored with this scorer object
 */
FLOAT_T get_scorer_sp_b_y_ion_fraction_matched(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *\returns the number of possible matched b,y ions for scoring SP
 */
int get_scorer_sp_b_y_ion_matched(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 *\returns the number of matched b,y ions for scoring SP
 */
int get_scorer_sp_b_y_ion_possible(
  SCORER_T* scorer ///< the scorer object -in
  );

/**
 * Generate the processed peaks for the spectrum and return via the
 * intensities array.  It's implemented here so that
 * create_intensity_array_observed() can remain private and so that
 * the scorer->observed array can be accessed directly.
 * .
 */
void get_processed_peaks(
  Spectrum* spectrum, 
  int charge,
  SCORER_TYPE_T score_type,
  FLOAT_T** intensities, ///< pointer to array of intensities
  int* mz_bins);

/**
 * create the intensity arrays for both observed and theoretical spectrum
 * SCORER must have been created for XCORR type
 * \returns TRUE if successful, else FLASE
 */
BOOLEAN_T create_intensity_array_xcorr(
  Spectrum* spectrum,    ///< the spectrum to score(observed) -in
  SCORER_T* scorer,        ///< the scorer object -in/out
  int charge               ///< the peptide charge -in 
  );

/**
 * Uses an iterative cross correlation
 *
 *\return the final cross correlation score between the observed and the
 *theoretical spectra
 */
FLOAT_T cross_correlation(
  SCORER_T* scorer,  ///< the scorer object that contains observed spectrum -in
  FLOAT_T* theoretical ///< the theoretical spectrum to score against the observed spectrum -in
  );

FLOAT_T* get_intensity_array_observed(SCORER_T* scorer);

BOOLEAN_T create_intensity_array_observed(
  SCORER_T* scorer,        ///< the scorer object -in/out
  Spectrum* spectrum,    ///< the spectrum to score(observed) -in
  int charge               ///< the peptide charge -in 
  );

/**
 * adds the intensity at add_idx
 * if, there already exist a peak at the index, only overwrite if
 * intensity is larger than the existing peak.
 */
void add_intensity(
  FLOAT_T* intensity_array, ///< the intensity array to add intensity at index add_idx -out
  int add_idx,            ///< the idex to add the intensity -in
  FLOAT_T intensity         ///< the intensity to add -in
  );




/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */
#endif

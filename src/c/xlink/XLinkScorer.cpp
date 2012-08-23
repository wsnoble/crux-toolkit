#include "XLinkScorer.h"
#include "Scorer.h"
#include "Ion.h"
#include "IonSeries.h"

#include <iostream>

using namespace std;

void XLinkScorer::init(
  Spectrum* spectrum,
  int charge,
  bool compute_sp
  ) {

  scorer_xcorr_ = new Scorer(XCORR);
  scorer_sp_ = new Scorer(SP);
  spectrum_ = spectrum;
  charge_ = charge;
  compute_sp_ = compute_sp;

  if ((spectrum_ != NULL) && (charge_ > 0)) {

    ion_constraint_xcorr_ = 
      IonConstraint::newIonConstraintSmart(XCORR, charge_);

    ion_constraint_sp_ =
      IonConstraint::newIonConstraintSmart(SP, charge_);

    ion_series_xcorr_ = 
      new IonSeries(ion_constraint_xcorr_, charge_);

    ion_series_sp_ =
      new IonSeries(ion_constraint_sp_, charge_);
  } else {

    ion_constraint_xcorr_ = NULL;
    ion_constraint_sp_ = NULL;
    ion_series_xcorr_ = NULL;
    ion_series_sp_ = NULL;
  }
}

XLinkScorer::XLinkScorer() {

  init(NULL, 0, get_boolean_parameter("compute-sp"));
}

XLinkScorer::XLinkScorer(Spectrum* spectrum, int charge) {

  init(spectrum, charge, get_boolean_parameter("compute-sp"));
}

XLinkScorer::XLinkScorer(
  Spectrum* spectrum, 
  int charge, 
  bool compute_sp
  ) {

  init(spectrum, charge, compute_sp);
}

XLinkScorer::~XLinkScorer() {
  //carp(CARP_INFO, "XLinkScorer::~XLinkScorer()");
  delete ion_series_xcorr_;
  delete ion_series_sp_;
  delete ion_constraint_xcorr_;
  delete ion_constraint_sp_;
  delete scorer_xcorr_;
  delete scorer_sp_;
  
}

FLOAT_T XLinkScorer::scoreCandidate(XLinkMatch* candidate) {

  candidate->predictIons(ion_series_xcorr_, charge_);
  FLOAT_T xcorr = scorer_xcorr_->scoreSpectrumVIonSeries(spectrum_, ion_series_xcorr_);
  candidate->setScore(XCORR, xcorr);

  if (compute_sp_) {

    candidate->predictIons(ion_series_sp_, charge_);
    FLOAT_T sp = scorer_sp_->scoreSpectrumVIonSeries(spectrum_, ion_series_sp_);
    
    candidate->setScore(SP, sp);
    candidate->setBYIonInfo(scorer_sp_);

  }
  return xcorr;

}
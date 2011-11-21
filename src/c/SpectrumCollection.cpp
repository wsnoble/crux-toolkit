/**
 * \file SpectrumCollection.cpp
 * AUTHOR: Barbara Frewen
 * CREATE DATE: 11 April 2011
 * \brief Abstract class for accessing spectra from a file.
 */
#include "SpectrumCollection.h" 
#include "ProteinIndex.h" 
#include "Peak.h"
#include "utils.h"
#ifndef WIN32
#include "unistd.h"
#endif
#include "parameter.h"
#include <cerrno>
#include <cstring>
#include "carp.h"
#include "WinCrux.h"

/**
 * Instantiates a new spectrum_collection object from a filename. 
 * Resolves any relative paths.  Confirms that file exists.
 */
SpectrumCollection::SpectrumCollection (
  const char* filename ///< The spectrum collection filename. 
  ) 
: filename_(filename), is_parsed_(false), num_charged_spectra_(0) 
{
  #if DARWIN
  char path_buffer[PATH_MAX];
  char* absolute_path_file =  realpath(filename, path_buffer);
  #else
  char* absolute_path_file =  realpath(filename, NULL);
  #endif
  if (absolute_path_file == NULL){
    carp(CARP_FATAL, "Error from spectrum file '%s'. (%s)",
         filename, strerror(errno)); 
  }
  
  if(access(absolute_path_file, F_OK)){
    // FIXEME can't refer to memory after freeing it! free(absolute_path_file);
    carp(CARP_FATAL, "File %s could not be opened\n", absolute_path_file);
  }
  filename_ = absolute_path_file;

  #ifndef DARWIN
  free(absolute_path_file);
  #endif
}

/**
 * Copy constructor.  Creates copies of each spectrum for the new
 * collection.
 */
SpectrumCollection::SpectrumCollection(
  SpectrumCollection& old_collection
  ) : filename_(old_collection.filename_),
      is_parsed_(old_collection.is_parsed_),
      num_charged_spectra_(old_collection.num_charged_spectra_)
{
  // copy spectra
  for (SpectrumIterator spectrum_iterator = old_collection.begin();
    spectrum_iterator != old_collection.end();
    ++spectrum_iterator) {

    Spectrum* old_spectrum = *spectrum_iterator;
    Spectrum* new_spectrum = new Spectrum(*old_spectrum);
    this->addSpectrumToEnd(new_spectrum);
  }
} 

/**
 * Default destructor
 */
SpectrumCollection::~SpectrumCollection() {

  for (SpectrumIterator spectrum_iterator = this->begin();
    spectrum_iterator != this->end();
    ++spectrum_iterator) {
    delete *spectrum_iterator;    
  }
  spectra_.clear();
}  

/**
 * \returns the begining of the spectra vector
 */
SpectrumIterator SpectrumCollection::begin() {
  return spectra_.begin();
}

/**
 * \returns the end of the spectra vector
 */
SpectrumIterator SpectrumCollection::end() {
  return spectra_.end();
}

/**
 * Adds a spectrum to the spectrum_collection.
 * adds the spectrum to the end of the spectra array
 * should only be used when the adding in increasing scan num order
 * when adding in random order should use add_spectrum
 * spectrum must be heap allocated
 */
void SpectrumCollection::addSpectrumToEnd(
  Spectrum* spectrum ///< spectrum to add to spectrum_collection -in
  )
{
  // set spectrum
  spectra_.push_back(spectrum);
  num_charged_spectra_ += spectrum->getNumZStates();
}

/**
 * Adds a spectrum to the spectrum_collection.
 * adds the spectrum in correct order into the spectra array
 * spectrum must be heap allocated
 */
void SpectrumCollection::addSpectrum(
  Spectrum* spectrum ///< spectrum to add to spectrum_collection -in
  )
{
  unsigned int add_index = 0;

  // find correct location
  // TODO -- replace with binary search if necessary.
  for(; add_index < spectra_.size(); ++add_index){
    if(spectra_[add_index]->getFirstScan() >
       spectrum->getFirstScan()){
      break;
    }
  }

  spectra_.insert(spectra_.begin()+add_index, spectrum);

  num_charged_spectra_ += spectrum->getNumZStates();
}


// FIXME maybe a faster way? can't perform binary search since we must know the array index
/**
 * Removes a spectrum from the spectrum_collection.
 */
void SpectrumCollection::removeSpectrum(
  Spectrum* spectrum ///< spectrum to be removed from spectrum_collection -in
  )
{
  int scan_num = spectrum->getFirstScan();
  unsigned int spectrum_index = 0;
  
  // find where the spectrum is located in the spectrum array
  for(; spectrum_index < spectra_.size(); ++spectrum_index){
    if(scan_num == spectra_[spectrum_index]->getFirstScan() ){
      break;
    }
  }
  
  num_charged_spectra_ -= spectrum->getNumZStates();

  delete spectra_[spectrum_index];
  spectra_[spectrum_index] = NULL;
  spectra_.erase(spectra_.begin() + spectrum_index);

} 



/**
 * \returns A pointer to the name of the file from which the spectra
 * were parsed.
 */
const char* SpectrumCollection::getFilename()
{  
  return filename_.c_str();
}

/**
 * \returns The current number of spectrum in the
 * spectrum_collection.  Zero if the file has not yet been parsed.
 */
int SpectrumCollection::getNumSpectra()
{
  return spectra_.size();
}

/**
 * \returns The current number of spectra assuming differnt
 * charge(i.e. one spectrum with two charge states are counted as two
 * spectra) in the spectrum_collection.  Zero if the file has not been
 * parsed.
 */
int SpectrumCollection::getNumChargedSpectra()
{
  return num_charged_spectra_;
}


/**
 * \returns True if the spectrum_collection file has been parsed.
 */
bool SpectrumCollection::getIsParsed()
{
  return is_parsed_;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */

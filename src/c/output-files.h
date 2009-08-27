/**
 * \file output-files.h
 */
/*
 * FILE: output-files.h
 * AUTHOR: Barbara Frewen
 * CREATE DATE: Aug 24, 2009
 * PROJECT: crux
 * DESCRIPTION: A class description for handling all the various
 * output files, excluding parameter and log files.  The filenames,
 * locations and overwrite status would be taken from parameter.c.
 */
#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include "carp.h"
#include "parameter.h"

class OutputFiles{

 public:
  OutputFiles(const char* program_name);///< name of the crux function
                                      ///(e.g. search)
  ~OutputFiles();

 private:
  BOOLEAN_T createFiles(FILE*** file_array_ptr,
                        const char* output_dir,
                        const char* fileroot,
                        const char* command_name,
                        const char* extension,
                        BOOLEAN_T overwrite);

  int num_files_; // in each array
  FILE** psm_file_array_; ///< array of .csm files
  FILE** tab_file_array_;
  FILE** sqt_file_array_;

};










#endif //OUTPUT_FILES_H

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */































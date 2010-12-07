/**
 * \file crux-main.cpp
 * AUTHOR: Barbara Frewen
 * CREATE DATE: November 24, 2008
 * \brief The starting point for the main crux program.
 *
 * Usage is "crux [command] [options] [arguments]" where command
 * is one of the primary crux commands.
 **/

#include "crux-main.h"
#include "crux-utils.h" // Need to get definition of NUM_FEATURES.

const char* usage_str = "Usage: crux <command> [options] <argument>\n"
"\n"
"Crux supports the following commands:\n"
"  create-index        Create an index for all peptides in a fasta file.\n"
"  search-for-matches  Search a collection of spectra against a sequence\n"
"                      database, returning a collection of peptide-spectrum\n"
"                      matches (PSMs) scored by XCorr.\n"
"  sequest-search      Similar to search-for-matches but use Sp as a \n"
"                      preliminary score followed by XCorr.\n"
"  compute-q-values    Assign a q-value, which is a statistical confidence\n"
"                      measure that accounts for multiple testing, to each\n"
"                      PSM in a given set.\n" 
"  percolator          Analyze a collection of PSMs to target and decoy\n"
"                      sequences using the percolator algorithm.\n"
"  q-ranker            Analyze a collection of PSMs using the Q-ranker\n"
"                      algorithm.\n"
"  print-processed-spectra\n"
"                      Write a new ms2 file with all of the same spectra\n"
"                      with only the peaks used for computing xcorr.\n"
"  search-for-xlinks   Search a collection of spectra against a sequence\n"
"                      database, returning a collection of matches\n"
"                      corresponding to linear and cross-linked peptides\n"
"                      scored by XCorr.\n"
"  version             Print the Crux version number to standard output,\n"
"                      then exit.\n"
"\n"
"Options and arguments are specific to each command. Type 'crux <command>'\n"
"for details.\n"
; 

/**
 * The starting point for crux.  Prints a general usage statement when
 * given no arguments.  Runs one of the crux commands, including
 * printing the current version number.
 */
int main(int argc, char** argv){


  CruxApplicationList applications("crux");

  applications.add(new CreateIndex());
  applications.add(new MatchSearch());
  applications.add(new SequestSearch());
  applications.add(new ComputeQValues());
  applications.add(new Percolator());
  applications.add(new QRanker());
  applications.add(new PrintProcessedSpectra());
  applications.add(new SearchForXLinks());

  applications.add(new ExtractColumns());
  applications.add(new ExtractRows());


  applications.add(new Version());

  int ret = applications.main(argc, argv);

  exit(ret);

  /*

  // check the syntax for crux <operation>
  if( argc < 2 ){
    carp(CARP_FATAL, usage_str);
  }

  // determine the operation
  char* op_string = argv[1];
  COMMAND_T command = string_to_command_type(op_string);

  // call the appropriate function 
  // passing the command line minus the first token ('crux')
  switch(command){
  case INDEX_COMMAND:
    create_index_main(argc-1, argv+1);
    break;

  case SEARCH_COMMAND:
    search_main(argc-1, argv+1);
    break;

  case SEQUEST_COMMAND:
    sequest_search_main(argc-1, argv+1);
    break;

  case PROCESS_SPEC_COMMAND:
    print_processed_spectra_main(argc-1, argv+1);
    break;
  
  case XLINK_SEARCH_COMMAND:
    xlink_search_main(argc-1, argv+1);
    break;

  case QVALUE_COMMAND:
  case QRANKER_COMMAND:
  case PERCOLATOR_COMMAND:
    analyze_matches_main(command, argc-1, argv+1);
    break;

  case VERSION_COMMAND:
    printf("Crux version %s\n", VERSION);
    break;    

  case INVALID_COMMAND:
  case NUMBER_COMMAND_TYPES:
    carp(CARP_FATAL, "Invalid command '%s'\n%s", op_string, usage_str);
    break;
  }
  
  exit ();
  */
}// end main
















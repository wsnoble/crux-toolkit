/**
 * \file CruxApplication.cpp
 * AUTHOR: Sean McIlwain
 * CREATE DATE: 6 December 2010
 * \brief Abstract Object for a CruxApplication
 *****************************************************************************/
#include "CruxApplication.h"

#include "io/carp.h"
#include "parameter.h"
#include "util/ArgParser.h"
#include "util/FileUtils.h"
#include "util/Params.h"
#include "util/WinCrux.h"

#include <iostream>

using namespace std;

/**
 * Frees an allocated CruxApplication
 */
CruxApplication::~CruxApplication() {
}

/**
 * \returns the arguments of the application
 */
vector<string> CruxApplication::getArgs() const {
  return vector<string>();
}

/**
 * \returns the options of the application
 */
vector<string> CruxApplication::getOptions() const {
  return vector<string>();
}

/**
 * \returns the outputs of the application as name -> description
 */
map<string, string> CruxApplication::getOutputs() const {
  return map<string, string>();
}

/**
 * \returns the file stem of the application, default getName.
 */
string CruxApplication::getFileStem() const {
  return getName();
}

/**
 * \returns the enum of the application, default MISC_COMMAND
 */
COMMAND_T CruxApplication::getCommand() const {
  return MISC_COMMAND;
}


bool CruxApplication::needsOutputDirectory() const {
  return false;
}

void CruxApplication::initialize(int argc, char** argv) {
  initializeParams(getName(), getArgs(), getOptions(), argc, argv);
  processParams();
  Params::Finalize();

  set_verbosity_level(Params::GetInt("verbosity"));

  carp(CARP_INFO, "Beginning %s.", getName().c_str());

  // Set seed for random number generation 
  if (Params::GetString("seed") == "time") {
    time_t seconds; // use current time to seed
    time(&seconds); // Get value from sys clock and set seconds variable.
    mysrandom((unsigned)seconds); // Convert seconds to a unsigned int
  } else {
    mysrandom(StringUtils::FromString<unsigned>(Params::GetString("seed")));
  }
  
  // Start the timer.
  wall_clock();

  // Create output directory if appliation needs it.
  if (needsOutputDirectory()) {
    // Create output directory 
    string output_folder = Params::GetString("output-dir");
    if (create_output_directory(output_folder, Params::GetBool("overwrite")) == -1) {
      carp(CARP_FATAL, "Unable to create output directory %s.", output_folder.c_str());
    }

    // Open the log file to record carp messages 
    open_log_file(getFileStem() + ".log.txt");
  
    // Store the host name, start date and time, and command line.
    carp(CARP_INFO, "CPU: %s", hostname());
    carp(CARP_INFO, date_and_time());
    log_command_line(argc, argv);

    // Write the parameter file
    string paramFile = make_file_path(getFileStem() + ".params.txt");
    ofstream* file = FileUtils::GetWriteStream(paramFile, Params::GetBool("overwrite"));
    if (file == NULL) {
      throw runtime_error("Could not open " + paramFile + " for writing");
    }
    Params::Write(file);
    delete file;
  }
}

/**
 * Should this application be kept from the usage statement?
 */
bool CruxApplication::hidden() const {
  return false;
}

/**
 * Read in all parameters from command line and parameter file
 */
void CruxApplication::initializeParams(
  const string& appName,
  const vector<string>& appArgs,
  const vector<string>& appOptions,
  int argc,
  char** argv
) {
  initialize_parameters();
  set_verbosity_level(Params::GetInt("verbosity"));

  // Parse command line
  ArgParser argParser;
  try {
    argParser.Parse(argc, argv, appArgs);

    // Read parameter file if specified
    string parameter_file = argParser.GetOption("parameter-file");
    if (!parameter_file.empty()) {
      parse_parameter_file(parameter_file.c_str());
      read_mods_from_file(parameter_file.c_str());
    }
    // Process command line options
    const map<string, string>& options = argParser.GetOptions();
    for (map<string, string>::const_iterator i = options.begin(); i != options.end(); i++) {
      Params::Set(i->first, i->second);
    }
    // Process command line arguments
    const map< string, vector<string> >& args = argParser.GetArgs();
    for (map< string, vector<string> >::const_iterator i = args.begin(); i != args.end(); i++) {
      for (vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); j++) {
        Params::AddArgValue(i->first, *j);
      }
    }
  } catch (const runtime_error& e) {
    carp(CARP_FATAL, "%s\n\n%s\n", e.what(),
         getUsage(appName, appArgs, appOptions).c_str());
  }
}

/**
 * Process parameters after they have been set up, but before they have been
 * finalized
 */
void CruxApplication::processParams() {
}

string CruxApplication::getUsage(
  const string& appName,
  const vector<string>& args,
  const vector<string>& options
) {
  vector<string> argDisplay;
  for (vector<string>::const_iterator i = args.begin(); i != args.end(); i++) {
    argDisplay.push_back(StringUtils::EndsWith(*i, "+") ?
      "<" + i->substr(0, i->length() - 1) + ">+" :"<" + *i + ">");
  }

  stringstream usage;
  usage << "USAGE:" << endl
        << endl
        << "  crux " << appName << " [options]";
  for (vector<string>::const_iterator i = argDisplay.begin(); i != argDisplay.end(); i++) {
    usage << ' ' << *i;
  }
  usage << endl << endl
        << "REQUIRED ARGUMENTS:";
  for (vector<string>::const_iterator i = argDisplay.begin(); i != argDisplay.end(); i++) {
    stringstream line;
    string argName = i->substr(1, i->length() - (StringUtils::EndsWith(*i, "+") ? 3 : 2));
    line << *i << ' ' << Params::GetUsage(argName);
    usage << endl << endl << StringUtils::LineFormat(line.str(), 80, 2);
  }
  usage << endl << endl
        << "OPTIONAL ARGUMENTS:" << endl;
  for (vector<string>::const_iterator i = options.begin(); i != options.end(); i++) {
    usage << endl
          << "  [--" << *i << " <" << Params::GetType(*i)
          << ">]" << endl
          << StringUtils::LineFormat(Params::ProcessHtmlDocTags(Params::GetUsage(*i)), 80, 5);
  }
  usage << endl << endl
        << "Additional parameters are documented in the online documentation.";
  return usage.str();
}

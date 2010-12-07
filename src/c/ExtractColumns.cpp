#include "ExtractColumns.h"

#include "DelimitedFile.h"

using namespace std;

ExtractColumns::ExtractColumns() {

}

ExtractColumns::~ExtractColumns() {
}


int ExtractColumns::main(int argc, char** argv) {

   /* Define optional command line arguments */
  const char* option_list[] = {
    "verbosity"
  };
  int num_options = sizeof(option_list) / sizeof(char*);

  /* Define required command line arguments */
  const char* argument_list[] = {"tsv file", "column names"};
  int num_arguments = sizeof(argument_list) / sizeof(char*);

  // Verbosity level for set-up/command line reading 
  set_verbosity_level(CARP_WARNING);

  // Initialize parameter.c and set default values
  initialize_parameters();

  // Define optional and required arguments 
  select_cmd_line_options(option_list, num_options);
  select_cmd_line_arguments(argument_list, num_arguments);

  // Parse the command line, including optional params file
  // Includes syntax, type, and bounds checking, dies on error 
  const char* cmd_name = this->getName().c_str();
  char* full_cmd = cat_string("crux ", cmd_name);
  parse_cmd_line_into_params_hash(argc, argv, cmd_name);
  free(full_cmd);

  const char* delimited_filename = get_string_parameter_pointer("tsv file");

  string column_names_string = string(get_string_parameter_pointer("column names"));

  DelimitedFileReader delimited_file(delimited_filename, true);
  
  vector<string> column_name_list;
  DelimitedFile::tokenize(column_names_string, column_name_list, ',');

  vector<int> column_indices;
  for (unsigned int i=0;i<column_name_list.size();i++) {
    int col_idx = delimited_file.findColumn(column_name_list[i]);
    if (col_idx != -1) {
      column_indices.push_back(col_idx);
    } else {
      carp(CARP_ERROR,"column not found:%s\n\n%s", 
        column_name_list[i].c_str(),
        delimited_file.getAvailableColumnsString().c_str());
      return(-1);
    }
  }

  cout << column_name_list[0];
  for (unsigned int col_idx = 1;col_idx<column_name_list.size();col_idx++) {
    cout <<"\t" << column_name_list[col_idx];
  }
  cout<<endl;

  while(delimited_file.hasNext()) {
    int col_idx = column_indices[0];
    cout << delimited_file.getString(col_idx);
    for (unsigned int col_idx_idx = 1;col_idx_idx < column_indices.size();col_idx_idx++) {
      col_idx = column_indices[col_idx_idx];
      cout << "\t" << delimited_file.getString(col_idx);
    }
    cout <<endl;
    delimited_file.next();
  }

  return 0;

}

string ExtractColumns::getName() {
  return "extract-columns";
}

string ExtractColumns::getDescription() {

  return "prints a delimited file using only the columns specified from the original delimited file";

}

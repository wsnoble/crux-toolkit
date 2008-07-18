#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "check-peak.h"
#include "../mass.h"
#include "../objects.h"
#include "../spectrum.h"
#include "../peak.h"
#include "../peptide.h"
#include "../peptide_src.h"
#include "../peptide_constraint.h"
#include "../protein.h"
#include "../database.h"
// also from parameter.c
void force_set_aa_mod_list(AA_MOD_T** amod_list, int num_mods);

/********************************************
 * to check peptide.c & peptide_constraint.c
 *
 ********************************************/


PEPTIDE_T *peptide1, *peptide2, *peptide3, *peptide4;
PROTEIN_T *protein, *protein1, *protein2, *protein3; 
PEPTIDE_CONSTRAINT_T* constraint;
PEPTIDE_SRC_T *association1, *association2, *association3;
DATABASE_T* database;
char* protseq1 = "MRVLKFGGTSVANAERFLRVADILESNARQGQVATVLSAPAKITNHLVAMIEKTISGQDALPNISDAERIFAELLTGLAAAQPGFPLAQLKTFVDQEFAQIKHVLHGISLLGQCPDSINAALICRGEKMSIAIMAGVLEARGHNVTVIDPVEKLLAVGHYLESTVDIAESTRRIAASRIPADHMVLMAGFTAGNEKGELVVLGRNGSDYSAAVLAACLRADCCEIWTDVDGVYTCDPRQVPDARLLKSMSYQEAMELSYFGAKVLHPRTITPIAQFQIPCLIKNTGNPQAPGTLIGASRDEDELPVKGISNLNNMAMFSVSGPGMKGMVGMAARVFAAMSRARISVVLITQSSSEYSISFCVPQSDCVRAERAMQEEFYLELKEGLLEPLAVTERLAIISVVGDGMRTLRGISAKFFAALARANINIVAIAQGSSERSISVVVNNDDATTGVRVTHQMLFNTDQVIEVFVIGVGGVGGALLEQLKRQQSW";

// set setup!!

START_TEST (test_create){
  database = new_database("test", FALSE);

  //test on link_list implementaion of peptide_src
  set_peptide_src_implementation(TRUE);
  
  //peptide4 = allocate_peptide();
  char* seq = NULL;
  
  protein = new_protein("test protein", protseq1, 490,
                        "this is a my test protein", 44, 4, database); //offset and protein_idx are random
  
  protein2 = new_protein("test protein", protseq1, 490,
                         "this is a my test protein", 44, 4, database);

  //create peptides
  peptide1 = new_peptide( 6, 684.75, protein, 239, TRYPTIC); //QVPDAR
  peptide2 = new_peptide( 6, 656.69, protein, 6, PARTIALLY_TRYPTIC);//FGGTSV
  peptide3 = new_peptide( 9, 1341.32, protein, 221, NOT_TRYPTIC); //DCCEIWTDV
  
  
  //check peptide
  fail_unless(strncmp((seq= get_peptide_sequence(peptide1)), "QVPDAR", 6) == 0,
              "peptide sequence no set");
  free(seq);

  //test
  //printf("peptide mass %.2f\n", get_peptide_peptide_mass(peptide1));
  fail_unless( compare_float(get_peptide_peptide_mass(peptide1), 684.75) ==0,
               "failed mass #1");
  //debug
  //printf("The peptide1 mass: %.2f\n", calc_peptide_mass(peptide1, AVERAGE));

  fail_unless(684.74 < calc_peptide_mass(peptide1, AVERAGE) &&
              calc_peptide_mass(peptide1, AVERAGE) < 684.76, "failed mass #2");
  
  fail_unless( compare_float(get_peptide_neutral_mass(peptide1), 684.75) == 0,
               "failed mass #3");
  //printf("peptide charged mass(charge2): %f\n", get_peptide_charged_mass(peptide1, 2));
  //printf("peptide mz(charge2): %f\n", get_peptide_mz(peptide1, 2));


  /*************peptide constraint**************/
  constraint = new_peptide_constraint(PARTIALLY_TRYPTIC, 660, 2000, 7, 20, 1, AVERAGE);
  fail_unless(!peptide_constraint_is_satisfied(constraint, peptide1), "constraint fail1");
  fail_unless(!peptide_constraint_is_satisfied(constraint, peptide2), "constraint fail2");
  fail_unless(peptide_constraint_is_satisfied(constraint, peptide3), "constraint fail3");
  
  //get, set for peptide_constraint (TRYPTIC ,400, 3000, 4, 26)
  set_peptide_constraint_peptide_type(constraint, TRYPTIC);
  fail_unless(get_peptide_constraint_peptide_type(constraint) == TRYPTIC, "constraint peptide type not set correctly");

  set_peptide_constraint_min_mass(constraint, 400);
  fail_unless( compare_float(get_peptide_constraint_min_mass(constraint), 400) == 0 , "constraint min mass not set correctly");
  
  set_peptide_constraint_max_mass(constraint, 3000);
  fail_unless( compare_float(get_peptide_constraint_max_mass(constraint), 3000) == 0 , "constraint max mass not set correctly");
 
  set_peptide_constraint_min_length(constraint, 4);
  fail_unless( get_peptide_constraint_min_length(constraint) == 4, "constraint min length no right");

  set_peptide_constraint_max_length(constraint, 26);
  fail_unless( get_peptide_constraint_max_length(constraint) == 26, "constraint max length no right");

  set_peptide_constraint_mass_type(constraint, MONO);
  fail_unless(get_peptide_constraint_mass_type(constraint) == MONO, "constraint mass type not set correctly");
  
  //check new peptide constraint
  fail_unless(peptide_constraint_is_satisfied(constraint, peptide1), "constraint fail, peptide1");
  fail_unless(peptide_constraint_is_satisfied(constraint, peptide2), "constraint fail, peptide2");
  fail_unless(peptide_constraint_is_satisfied(constraint, peptide3), "constraint fail, peptide3");


  //check set, get for peptides again
  set_peptide_length(peptide1, 10);
  fail_unless(get_peptide_length(peptide1) == 10, "peptide length no right");
  
  set_peptide_peptide_mass(peptide1, 546.5958*2 );
  fail_unless( compare_float(get_peptide_peptide_mass(peptide1), 546.5958*2) == 0 , "peptide mass not set correctly");

  //print_peptide(peptide1, stdout);
  

  //check copy peptide ADD for association
  //copy_peptide(peptide1, peptide4);
  peptide4 = copy_peptide(peptide1);
  //print_peptide(peptide4, stdout);


  //check peptide_src
  association1 = new_peptide_src(TRYPTIC, protein, 4);
  association2 = new_peptide_src(NOT_TRYPTIC, protein2, 8); //try to add this to end
  association3 = allocate_peptide_src();
  copy_peptide_src(association1, association3);
  
  //try adding peptide_src to a peptide
  add_peptide_peptide_src(peptide4, association1);
  add_peptide_peptide_src(peptide4, association2);
  add_peptide_peptide_src(peptide4, association3);
 
  fail_unless(get_peptide_src_peptide_type(association3) == TRYPTIC, "failed to copy | set peptide type");
  fail_unless(get_peptide_src_parent_protein(association3) == protein, "failed to copy | set parent protein");
  fail_unless(get_peptide_src_start_idx(association3) == 4, "failed to copy | set start idx");
 
  //try printing peptide in various forms..to ensure nothing blows up
  //print_peptide(peptide4, stdout);
  //print_peptide_in_format(peptide4, TRUE, TRUE, stdout);
  //serialize_peptide(peptide4, stdout);
 
  free_database(database);
  free_peptide_constraint(constraint);
  free_peptide(peptide1);
  free_peptide(peptide2);
  free_peptide(peptide3);
  free_peptide(peptide4);
  free_protein(protein);
  free_protein(protein2);
}
END_TEST

void pep_setup(){
  protein1 = new_protein( "Protein1", protseq1, strlen(protseq1), 
                          NULL, 0, 0, NULL);//description, offset, idx, dbase
  peptide1 = new_peptide( 10, 1087.20, protein1, 20, TRYPTIC);//VADILESNAR
  peptide2 = new_peptide( 16, 1736.02, protein1, 1, TRYPTIC);//MRVLKFGGTSVANAER
  peptide3 = new_peptide( 4, 547.57, protein1, 487, TRYPTIC);//QQSW

}

void pep_teardown(){
  free_peptide(peptide1);
  free_peptide(peptide2);
  free_peptide(peptide3);
  free_protein(protein1);
}

START_TEST (test_ndist){// start index
  // get_peptide_n_distance at 0, mid, near end
  int ndist = get_peptide_n_distance(peptide1); 
  fail_unless( ndist == 461, 
               "Incorrect distance from peptide n-terminus to " \
               "protein n-ternimus.  Got %d, should be 461", ndist);

  ndist = get_peptide_n_distance(peptide2); 
  fail_unless( ndist == 474, 
               "Incorrect distance from peptide n-terminus to " \
               "protein n-ternimus.  Got %d, should be 474", ndist);


  ndist = get_peptide_n_distance(peptide3); 
  fail_unless( ndist == 0, 
               "Incorrect distance from peptide n-terminus to " \
               "protein n-ternimus.  Got %d, should be 0", ndist);

  // test for multiple protein sources, first is least, first not least
  add_peptide_peptide_src( peptide1,
                           new_peptide_src(NOT_TRYPTIC, protein1, 25) );
  ndist = get_peptide_n_distance(peptide1); 
  fail_unless( ndist == 456, 
               "Incorrect distance from peptide n-terminus to " \
               "protein n-ternimus.  Got %d, should be 456", ndist);
  add_peptide_peptide_src( peptide1,
                           new_peptide_src(NOT_TRYPTIC, protein1, 100) );

  ndist = get_peptide_n_distance(peptide1); 
  fail_unless( ndist == 381, 
               "Incorrect distance from peptide n-terminus to " \
               "protein n-ternimus.  Got %d, should be 381", ndist);
}
END_TEST

START_TEST (test_cdist){// start index
  // get peptide_cdist at 0, mid, near end
  int cdist = get_peptide_c_distance(peptide1); 
  //fail_unless( cdist == 461, 
  fail_unless( cdist == 19, 
               "Incorrect distance from peptide c-terminus to " \
               "protein c-ternimus.  Got %d, should be 19", cdist);
  cdist = get_peptide_c_distance(peptide2); 
  fail_unless( cdist == 0, 
               "Incorrect distance from peptide c-terminus to " \
               "protein c-ternimus.  Got %d, should be 0", cdist);
  cdist = get_peptide_c_distance(peptide3); 
  fail_unless( cdist == 486, 
               "Incorrect distance from peptide c-terminus to " \
               "protein c-ternimus.  Got %d, should be 486", cdist);

  // test for multiple protein sources, first is least, first not least
  add_peptide_peptide_src( peptide1,
                           new_peptide_src(NOT_TRYPTIC, protein1, 5) );
  cdist = get_peptide_c_distance(peptide1); 
  fail_unless( cdist == 4, 
               "Incorrect distance from peptide c-terminus to " \
               "protein c-ternimus.  Got %d, should be 4", cdist);
  add_peptide_peptide_src( peptide1,
                           new_peptide_src(NOT_TRYPTIC, protein1, 30) );
  cdist = get_peptide_c_distance(peptide1); 
  fail_unless( cdist == 4, 
               "Incorrect distance from peptide c-terminus to " \
               "protein c-ternimus.  Got %d, should be 4", cdist);
}
END_TEST

START_TEST(test_mod_on_unmodified){
  // check is_modified
  // get modified seq from peptide w/out modificationso
  MODIFIED_AA_T* mod_seq = get_peptide_modified_aa_sequence(peptide1);
  
  fail_unless( mod_seq != NULL,
               "An unmodified peptide should not return NULL mod seq");
  char* seq = get_peptide_sequence(peptide1);
  char* converted = modified_aa_string_to_string(mod_seq);
  fail_unless( strcmp(seq, converted) == 0,
               "The modified seq returned should be the same as seq.");

}
END_TEST

START_TEST(test_with_mod){
  double initial_mass = get_peptide_neutral_mass(peptide3);
  // set up the mod
  AA_MOD_T* amod = new_aa_mod(0);
  aa_mod_set_mass_change(amod, 100);
  AA_MOD_T* amod_list[1];
  amod_list[0] = amod;
  // initialize mods in parameter.c
  force_set_aa_mod_list(amod_list, 1);

  PEPTIDE_MOD_T* pep_mod = new_peptide_mod();
  peptide_mod_add_aa_mod(pep_mod, 0, 1);

  // set up the mod seq
  char* pep_seq = get_peptide_sequence(peptide3);
  MODIFIED_AA_T* mod_seq = convert_to_mod_aa_seq(pep_seq);
  modify_aa(&mod_seq[2], amod);
  fail_unless( mod_seq[2] > pep_seq[2] - 'A',
               "Third aa should no longer be unmodified.");
  char* a = modified_aa_to_string(mod_seq[1]);
  a = modified_aa_to_string(mod_seq[2]);
  fail_unless( strcmp(a, "S*") == 0,  "aa should be S* but is %s", a);

  char* mod_seq_str = modified_aa_string_to_string(mod_seq);

  // set the modification
  set_peptide_mod(peptide3, mod_seq, pep_mod);
  // check is_modified
  // get modified seq
  MODIFIED_AA_T* returned_seq = get_peptide_modified_aa_sequence(peptide3);
  char* returned_str = modified_aa_string_to_string(returned_seq);
  fail_unless( strcmp(returned_str, mod_seq_str) == 0,
   "Peptide3 should have returned modified seq %s, but instead returned %s",
               mod_seq_str, returned_str);
  // check the mass
  fail_unless( initial_mass = get_peptide_neutral_mass(peptide3),
               "Modified peptide should have initial mass + 100.");
}
END_TEST


Suite* peptide_suite(void){
  Suite *s = suite_create("Peptide");
  TCase *tc_core = tcase_create("Core");
  suite_add_tcase(s, tc_core);
  tcase_add_test(tc_core, test_create);
  //tcase_add_checked_fixture(tc_core, setup, teardown);

  TCase *tc_with_setup = tcase_create("With startup");
  suite_add_tcase(s, tc_with_setup);
  tcase_add_test(tc_with_setup, test_ndist);
  tcase_add_test(tc_with_setup, test_cdist);
  tcase_add_test(tc_with_setup, test_mod_on_unmodified);
  tcase_add_test(tc_with_setup, test_with_mod);
  tcase_add_checked_fixture(tc_with_setup, pep_setup, pep_teardown);

  return s;
}

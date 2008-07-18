/*************************************************************************//**
 * \file peptide.c
 * $Revision: 1.72.2.10 $
 * \brief: Object for representing a single peptide.
 ****************************************************************************/
#include "peptide.h"

/*
  TABLE OF CONTENTS
  Global variables
  Private data types
  Private functions
  Public functions
    Allocators/deallocators
    Getters and Setters
      mass-related
      source-related
      sequence-related
      getters requiring calculation

    Comparisons for sorting // MOVEME
    Printing / parsing      // MOVEME
      text
      binary

    Iterators
      residue iterator
      source iterator

      
 */


/**
 * static global variable
 * determines if the peptide src are created by link lists or array
 * if TRUE, peptides are implented with link list peptide src, else array
 */
static BOOLEAN_T PEPTIDE_SRC_USE_LINK_LIST;

/* Private data types */

/**
 * \struct peptide
 * \brief A subsequence of a protein.
 */
struct peptide {
  unsigned char length; ///< The length of the peptide
  float peptide_mass;   ///< The peptide's mass.
  PEPTIDE_SRC_T* peptide_src; ///< a linklist of peptide_src   
  // comment me to fix files
  MODIFIED_AA_T* modified_seq; ///< peptide sequence with modifications
};


/**
 * \struct residue_iterator
 * \brief Object to iterate over the residues in a peptide, starting at the
 * first residue of the peptide, and proceeding in order.
 */
struct residue_iterator {
  PEPTIDE_T*  peptide; ///< The peptide whose residues to iterate over.
  char*   sequence;    ///< The peptide sequence
  int     residue_idx; ///< The index of the current peak
};

/**
 * \struct peptide_src_iterator
 * \brief Object to iterate over the peptide_srcs linklist in a peptide
 */
struct peptide_src_iterator{
  PEPTIDE_T*  peptide; ///< The peptide whose peptide_srcs to iterate over.
  PEPTIDE_SRC_T* current; ///< the current peptide_srcs
};

/* Private functions */

/* Public functions--Allocators/Deallocators */

/**
 * \returns An (empty) peptide object.
 */
PEPTIDE_T* allocate_peptide(void){
  PEPTIDE_T* peptide = (PEPTIDE_T*)mycalloc(1, sizeof(PEPTIDE_T));
  peptide->peptide_src = NULL;
  return peptide;
}

/**
 *\returns the protein struct size, value of sizeof function
 */
int get_peptide_sizeof(){
  return sizeof(PEPTIDE_T);
}

// FIXME association part might be need to change
/**
 * \returns A new peptide object, populated with the user specified parameters.
 */
PEPTIDE_T* new_peptide(
  unsigned char length,     ///< The length of the peptide -in
  float peptide_mass,       ///< The neutral mass of the peptide -in
  PROTEIN_T* parent_protein, ///< the parent_protein of this peptide -in
  int start_idx, ///< the start index of this peptide in the protein sequence -in
  PEPTIDE_TYPE_T peptide_type ///<  The type of peptides(TRYPTIC, C_TRYPTIC, N_TRYPTIC, NOT_TRYPTIC, ANY_TRYPTIC) -in
  )
{
  PEPTIDE_T* peptide = allocate_peptide();
  set_peptide_length(peptide, length);
  set_peptide_peptide_mass(peptide, peptide_mass);
  peptide->peptide_src = 
    new_peptide_src(peptide_type, parent_protein, start_idx );
  // comment me to fix files
  peptide->modified_seq = NULL;
  
  return peptide;
}
  
/**
 * \brief Allocates a new peptide giving it the values of the source
 * peptide.
 * \returns A newly allocated peptide identical to the source.
 */
PEPTIDE_T* copy_peptide(
  PEPTIDE_T* src ///< source peptide -in
){
 
  if( src == NULL ){
    return NULL;
  }
  PEPTIDE_T* new_peptide = allocate_peptide();
  new_peptide->length = src->length;
  new_peptide->peptide_mass = src->peptide_mass;
  new_peptide->peptide_src = allocate_peptide_src();
  copy_peptide_src(src->peptide_src, new_peptide->peptide_src);

  if( src->modified_seq == NULL ){
    // get the peptide sequence and convert to MODIFIED_AA_T*
    /*    
    char* sequence = get_peptide_sequence(src);
    MODIFIED_AA_T* mod_seq = convert_to_mod_aa_seq(sequence);
    free(sequence);
    new_peptide->modified_seq = mod_seq;
    */
    new_peptide->modified_seq = NULL;
  }else{
    new_peptide->modified_seq = copy_mod_aa_seq(src->modified_seq);
  }
  //PEPTIDE_SRC_T* new_association;

  //  set_peptide_length(dest, get_peptide_length(src));
  //  set_peptide_peptide_mass(dest, get_peptide_peptide_mass(src));

  // copy all of the peptide_src in the peptide
  //  new_association = allocate_peptide_src();
  //  copy_peptide_src(src->peptide_src, new_association);
  //  set_peptide_peptide_src(dest, new_association);

  return new_peptide;
}

/**
 * Merge two identical peptides, copy all peptide_src into one of the peptide
 * peptide_dest, peptide_bye must have at least one peptide src
 * frees the peptide_bye, once the peptide_src are re-linked to the peptide_dest
 * Assumes that both peptides use linklist implemenation for peptide_src
 * \returns TRUE if merge is successful else FALSE
 */
BOOLEAN_T merge_peptides(
  PEPTIDE_T* peptide_dest, ///< the peptide to merge into  -out
  PEPTIDE_T* peptide_bye ///< the peptide to be merged  -in
  )
{
  PEPTIDE_SRC_T* current_src = peptide_dest->peptide_src;
  PEPTIDE_SRC_T* next_src = get_peptide_src_next_association(current_src);
  
  // does all peptides have at least one peptide_src?
  if(current_src == NULL || peptide_bye->peptide_src == NULL){
    carp(CARP_ERROR, "failed to merge two peptides");
    return FALSE;
  }

  // find the end of the peptide src link list..
  while(next_src != NULL){
    current_src = next_src;
    next_src =  get_peptide_src_next_association(current_src);
  }
  set_peptide_src_next_association(current_src, peptide_bye->peptide_src);
  free(peptide_bye);
  return TRUE;
}

/**
 * Frees an allocated peptide object.
 * Depending on peptide_src implementation determines how to free srcs
 * This decision is made by global variable PEPTIDE_SRC_USE_LINK_LIST
 */
void free_peptide(
  PEPTIDE_T* peptide ///< peptide to free -in
  )
{

  if( peptide == NULL ){
    return;
  }
  // check which implementation peptide_src uses
  if(!PEPTIDE_SRC_USE_LINK_LIST){
    // array implementation
    free(peptide->peptide_src);    
  }
  else{
    // link list implementation
    free_peptide_src(peptide->peptide_src);
  }

  if(peptide->modified_seq){
    free(peptide->modified_seq);;
  }
  free(peptide);
}

/* Public functions--Getters and Setters */

/**
 * sets the peptide src implementation in the peptide object
 * This should be set only once and not be altered
 */
void set_peptide_src_implementation(
  BOOLEAN_T use_link_list ///< does the peptide use link list peptide src
  )
{  
  PEPTIDE_SRC_USE_LINK_LIST = use_link_list; 
}
                            
/* mass-related getters and setters */

/**
 * sets the peptide mass
 */
void set_peptide_peptide_mass( 
  PEPTIDE_T* peptide,  ///< the peptide to set -out
  float peptide_mass  ///< the mass of the peptide - in
  )
{
  peptide->peptide_mass = peptide_mass;
}

// TODO: this a little absurd.  why not just one function?
/**
 * \returns the peptide mass
 */
inline float get_peptide_peptide_mass( 
  PEPTIDE_T* peptide  ///< the peptide to query the mass -in
  )
{
  return peptide->peptide_mass;
}


/** 
 * \returns The neutral (uncharged) mass of the peptide.
 */
float get_peptide_neutral_mass(
  PEPTIDE_T* peptide ///< the query peptide -in
  )
{
  return get_peptide_peptide_mass(peptide);
}

/** 
 * \returns The mass of the peptide if it had charge "charge"
 */
float get_peptide_charged_mass(
 PEPTIDE_T* peptide, ///< the query peptide -in
 int charge ///< charge of peptide -in
 )
{
  return get_peptide_mz(peptide, charge) * charge;
}

/** 
 * \returns The m/z of the peptide if it had charge "charge"
 */
float get_peptide_mz(
    PEPTIDE_T* peptide, ///< the query peptide -in
    int charge ///< the charge of peptide -in
    )
{
  return ((get_peptide_peptide_mass(peptide) + MASS_H * charge)/ charge);
}

/* source-related getters and setters */

/**
 * sets the peptide_src field in the peptide
 * this method should be ONLY used when the peptide has no existing list of peptide_src
 * use add_peptide_peptide_src method to add to existing list
 * must pass on a heap allocated peptide_src object
 * does not copy in the object, just the pointer to the object.
 */
void set_peptide_peptide_src(
  PEPTIDE_T* peptide,  ///< the peptide to set -out                                             
  PEPTIDE_SRC_T* new_association ///< new peptide_src -in
  )
{
  peptide->peptide_src = new_association;
}

/**
 * this method adds the new_association to the end of the existing peptide's 
 * linklist of peptide_srcs
 * if no prior existing list, adds it at the front
 * must pass on a heap allocated peptide_src object
 * does not copy in the object, just the pointer to the object.
 */
void add_peptide_peptide_src(
  PEPTIDE_T* peptide,  ///< the peptide to set -out
  PEPTIDE_SRC_T* new_association ///< new peptide_src -in
  )
{
  PEPTIDE_SRC_T* add_association = peptide->peptide_src;
  PEPTIDE_SRC_ITERATOR_T* iterator = NULL;
  
  // is the peptide src list empty?
  if(add_association == NULL){
    peptide->peptide_src = new_association;
    return;
  }

  // create peptide src iterator
  iterator = new_peptide_src_iterator(peptide);

  // find the last peptide_src object in the list
  while(peptide_src_iterator_has_next(iterator)){
    add_association = peptide_src_iterator_next(iterator);
  }
  
  set_peptide_src_next_association(add_association, new_association);
  free_peptide_src_iterator(iterator);
}

/**
 * this method adds the peptide src array to an EMPTY peptide
 * only used in index.c, when the peptide src count for  peptide is known
 * Any existing peptide_src will lose it's reference
 */
void add_peptide_peptide_src_array(
  PEPTIDE_T* peptide,  ///< the peptide to set -out
  PEPTIDE_SRC_T* peptide_src_array ///< new peptide_src -in
  )
{
  // should be empty peptide src list
  peptide->peptide_src = peptide_src_array;

}

// TODO: why do we need both of these?
/**
 * returns a pointer to the peptide_protein_association field of the peptide
 */
PEPTIDE_SRC_T* get_peptide_peptide_src(
  PEPTIDE_T* peptide  ///< the peptide to query the peptide_peptide_src -in
  )
{
  return peptide->peptide_src;
}

/**
 * get the peptide->first peptide_src->parent protein->database
 */
DATABASE_T* get_peptide_first_src_database(
  PEPTIDE_T* peptide ///< working peptide -in
  )
{
  return get_protein_database(get_peptide_src_parent_protein(peptide->peptide_src));
}

// set by peptide_src?
/**
 * returns a pointer to the peptide's first parent protein field of the peptide
 */
PROTEIN_T* get_peptide_parent_protein(
  PEPTIDE_T* peptide  ///< the peptide to query the parent_protein -in
  )
{
  return get_peptide_src_parent_protein(peptide->peptide_src);
}

/**
 * sets the sequence length of the peptide
 * length maximum of 255
 */
void set_peptide_length( 
  PEPTIDE_T* peptide,  ///< the peptide to set the length -out
  unsigned char length  ///< the length of sequence -in
  )
{
  peptide->length = length;
}

/* sequence-related getters and setters */
/**
 *\returns the sequence length of the peptide
 */
unsigned char get_peptide_length( 
  PEPTIDE_T* peptide  ///< the peptide to query the length -in
  )
{
  return peptide->length;
}

/**
 * \brief Get a string representation of the peptide sequence
 * Sequence is taken from the first peptide_src to gain sequence, thus
 * must have at least one peptide src. 
 * \returns The newly-allocated sequence of peptide
 */
char* get_peptide_sequence(
 PEPTIDE_T* peptide ///< peptide to query sequence -in
 )
{
  if(peptide == NULL){
    carp(CARP_ERROR, "Cannot get sequence from NULL peptide.");
    return NULL;
  }
  if(peptide->peptide_src == NULL){
    //    die("ERROR: no peptide_src to retrieve peptide sequence\n");
    carp(CARP_ERROR, "Cannot get sequence from peptide with no peptide src.");
    return NULL;
  }

  //PROTEIN_T* protein = get_peptide_src_parent_protein(peptide->peptide_src));
  char* parent_sequence = 
    get_protein_sequence_pointer(get_peptide_src_parent_protein(peptide->peptide_src));
  //get_protein_sequence_pointer(protein);
  int start_idx = get_peptide_src_start_idx(peptide->peptide_src);

  char* copy_sequence = copy_string_part(&parent_sequence[start_idx-1],
                                         peptide->length);
 
  return copy_sequence; 
}

/**
 * \brief Get a pointer to the peptide sequence that is NOT null
 * terminated.
 * USE WITH CAUTION.  Pointer is to the parent protein sequence and
 * thus is not null-terminated until the end of the protein.  Parent
 * protein is taken from the first protein source.
 * 
 * \returns A pointer to an existing peptide sequence.
 */
char* get_peptide_sequence_pointer(
  PEPTIDE_T* peptide ///< peptide to query sequence -in
  )
{
  if(peptide->peptide_src == NULL){
    die("ERROR: no peptide_src to retrieve peptide sequence pointer\n");
  }
  char* parent_sequence = 
    get_protein_sequence_pointer(get_peptide_src_parent_protein(peptide->peptide_src));
  int start_idx = get_peptide_src_start_idx(peptide->peptide_src);

  char* pointer_peptide_sequence = &parent_sequence[start_idx-1];
  
  return pointer_peptide_sequence;
}

/**
 * \returns the sequence of peptide with each flanking AA
 *  template "*.peptide_sequence.*", where "*" are flanking amino acids
 * "*", left empty if no flanking sequence
 * goes to the first peptide_src to gain sequence, thus must have at least one peptide src
 * returns a char* to a heap allocated copy of the sequence
 * user must free the memory
 */
char* get_peptide_sequence_sqt(
 PEPTIDE_T* peptide ///< peptide to query sequence -in
 )
{
  if(peptide->peptide_src == NULL){
    die("ERROR: no peptide_src to retrieve peptide sequence\n");
  }
  
  // yeah return!!
  return get_peptide_sequence_from_peptide_src_sqt(peptide, peptide->peptide_src);
}

/**
 * \brief Formats the sequence of the peptide from a particular
 * peptide_src.
 *
 * Is called by get_peptide_sequence_sqt()
 * Format is "X.peptide_sequence.X", where "X" is a flanking amino acid.
 * "X", is printed as "-" if there is no flanking sequence.
 * Goes to the first peptide_src to gain sequence, thus must have at
 * least one peptide src 
 *
 * \returns A newly allocated string with the sqt-formated peptide sequence.
 */
char* get_peptide_sequence_from_peptide_src_sqt(
 PEPTIDE_T* peptide, ///< peptide to query sequence -in
 PEPTIDE_SRC_T* peptide_src ///< peptide_src -in 
 )
{
  char* copy_sequence = NULL;
  PROTEIN_T* protein = get_peptide_src_parent_protein(peptide_src);
  // get peptide start idx of protein in prarent protein
  int start_idx = get_peptide_src_start_idx(peptide_src);
  // parent protein length
  int protein_length = get_protein_length(protein);
  // parent protein length
  char* parent_sequence = 
    get_protein_sequence_pointer(protein);

  // allocate peptide memory
  copy_sequence = (char*)mycalloc(peptide->length+5, sizeof(char));

  // Default template is "*.peptide.*", where "*" are flanking amino acids
  copy_sequence[0] = '-';
  copy_sequence[1] = '.';
  copy_sequence[peptide->length+2] = '.';
  copy_sequence[peptide->length+3] = '-';
  copy_sequence[peptide->length+4] = '\0';

  // copy over the peptide sequences
  strncpy(&copy_sequence[2], &parent_sequence[start_idx-1], peptide->length);
  
  // is there an AA before?
  if(start_idx != 1){
    copy_sequence[0] = parent_sequence[start_idx-2];
  }
  // is there an AA after?
  if((start_idx + peptide->length - 1) < protein_length){
    copy_sequence[peptide->length+3] = parent_sequence[start_idx+peptide->length-1];
  }
  
  // yeah return!!
  return copy_sequence; 
}

/**
 * \brief Add a modification to a peptide.
 *
 * Adds the modified sequence to the peptide and changes the peptide
 * mass based on the mass change in the peptide_mod.
 * \returns void
 */
void set_peptide_mod(PEPTIDE_T* peptide,     ///< peptide to be modified
                     MODIFIED_AA_T* mod_seq, ///< modified seq to add
                     PEPTIDE_MOD_T* pep_mod  ///< mod that made the seq
){
  if( peptide == NULL || mod_seq == NULL || pep_mod == NULL ){
    carp(CARP_ERROR, "Cannot modify peptide.  Peptide, mod, or seq is NULL.");
    return;
  }

  // comment me to fix files
  // check that peptide doesn't already have a mod?
  peptide->modified_seq = mod_seq;// should this be a copy instead??
  // change mass
  peptide->peptide_mass += peptide_mod_get_mass_change(pep_mod);

}

/**
 * \brief Get the modified peptide sequence
 *
 * If the peptide has no modifications, create a sequence of
 * MODIFIED_AA_T's in which none of them are actually modified.
 * \returns A newly allocated copy of the sequence of MODIFIED_AA_Ts.
 */
MODIFIED_AA_T* get_peptide_modified_aa_sequence(PEPTIDE_T* peptide){
  if( peptide == NULL ){
    carp(CARP_ERROR, "Cannot get modified sequence from NULL peptide.");
    return NULL;
  }

  MODIFIED_AA_T* seq_copy = NULL;
  
  // comment me to fix files
  if( peptide->modified_seq != NULL ){
    seq_copy = copy_mod_aa_seq(peptide->modified_seq);
  }else{// create one from char seq
    char* seq = get_peptide_sequence(peptide);
    seq_copy = convert_to_mod_aa_seq(seq);
    free(seq);
  }
  
  return seq_copy;
}

/**
 * \brief Get the modified aa sequence in string form.
 *
 * If the peptide has no modifications, returns same string as
 * get_peptide_sequence.  If modified, adds the mod symbols to the string.
 * \returns A newly allocated string of the peptide sequence including
 * any modifications.
 */
char* get_peptide_modified_sequence(
 PEPTIDE_T* peptide
 ){
  char* seq_string = NULL;
  if( peptide->modified_seq == NULL ){
    seq_string = get_peptide_sequence(peptide);
  }else{
    seq_string = modified_aa_string_to_string(peptide->modified_seq);
  }
  
  return seq_string;
}


/* getters requiring calculation */

/**
 * \returns The mass of the given peptide as determined by the aa sequence.
 */
float calc_sequence_mass(
  char* peptide, ///< the query peptide -in
  MASS_TYPE_T mass_type ///< isotopic mass type (AVERAGE, MONO) -in
  )
{
  float peptide_mass = 0;
  int idx = 0;
  char amino;
  while(peptide[idx] != '\0'){
    amino = peptide[idx++];
    peptide_mass += get_mass_amino_acid(amino, mass_type);
  }
  if(mass_type == AVERAGE){
    return peptide_mass + MASS_H2O_AVERAGE;
  }
  return peptide_mass + MASS_H2O_MONO;
}

/**
 * This appears to be the same as calc_sequence_mass??
 * \returns The mass of the given peptide.
 */
float calc_peptide_mass(
  PEPTIDE_T* peptide, ///< the query peptide -in
  MASS_TYPE_T mass_type ///< isotopic mass type (AVERAGE, MONO) -in
  )
{
  float peptide_mass = 0;
  RESIDUE_ITERATOR_T * residue_iterator = new_residue_iterator(peptide);
  
  while(residue_iterator_has_next(residue_iterator)){
    peptide_mass += get_mass_amino_acid(residue_iterator_next(residue_iterator), mass_type);
  }
  free_residue_iterator(residue_iterator);

  if(mass_type == AVERAGE){
    return peptide_mass + MASS_H2O_AVERAGE;
  }
  return peptide_mass + MASS_H2O_MONO;
}

static float krokhin_index['Z'-'A'] = {
  0.8, 0.0, -0.8, -0.5, 0.0, 10.5, -0.9, -1.3, 8.4, 0.0, 
  -1.9, 9.6, 5.8, -1.2, 0.0, 0.2, -0.9, -1.3, -0.8, 0.4,
  0.0, 5.0, 11.0, 0.0, 4.0};

/*
 * Calculates the peptide hydrophobicity, as in Krokhin (2004).
 */
float calc_krokhin_hydrophobicity(
  PEPTIDE_T* peptide ///< the query peptide -in
)
{
  float krokhin = 0.0;
  RESIDUE_ITERATOR_T * residue_iterator = new_residue_iterator(peptide);
  while(residue_iterator_has_next(residue_iterator)){
    char c = residue_iterator_next(residue_iterator)-'A';
    krokhin += krokhin_index[(int)c];
  }
  free_residue_iterator(residue_iterator);

  return krokhin;
}

/**
 * Examines the peptide sequence and counts how many tryptic missed
 * cleavage sites exist. 
 *\returns the number of missed cleavage sites in the peptide
 */
int get_peptide_missed_cleavage_sites(
  PEPTIDE_T* peptide  ///< the peptide to query -in
  )
{
  int missed_count = 0;
  int aa_idx = 0;
  char* sequence = get_peptide_sequence_pointer(peptide);

  // count the missed cleavage sites
  for(; aa_idx < peptide->length-1; ++aa_idx){
    if(sequence[aa_idx] == 'K' ||
       sequence[aa_idx] == 'R'){
      
      // skip one that are followed by a P
      if(sequence[aa_idx+1] == 'P'){
        continue;
      }
      else{
        ++missed_count;
      }      
    } 
  }
  
  return missed_count;
}

/**
 * \brief Find the distance from the c-terminus of the source protein
 * to the c-terminus of the peptide (seq[0]).  
 * In the case of multiple source proteins, return the smallest
 * distance.
 * \returns The distance from the protein c-terminus.
 */
int get_peptide_c_distance(PEPTIDE_T* peptide){

  int min_index = MAX_PROTEIN_SEQ_LENGTH;
  PEPTIDE_SRC_T* cur_src = peptide->peptide_src;
  while( cur_src != NULL ){
    int index = get_peptide_src_start_idx(cur_src);
    if( index < min_index ){
      min_index = index;
    }
    cur_src = get_peptide_src_next_association(cur_src);
  }
  return min_index - 1;
}

/**
 * \brief Find the distance from the n-terminus of the source protein
 * to the n-terminus of the peptide.
 * In the case of multiple source proteins, return the smallest
 * distance.
 * \returns The distance from the protein c-terminus.
 */
int get_peptide_n_distance(PEPTIDE_T* peptide){

  int min_index = MAX_PROTEIN_SEQ_LENGTH;
  int peptide_length = get_peptide_length(peptide);
  PEPTIDE_SRC_T* cur_src = peptide->peptide_src;

  while( cur_src != NULL ){
    // get protein length
    int protein_length = get_protein_length( 
                           get_peptide_src_parent_protein(cur_src));
    // get index of end
    int start_index = get_peptide_src_start_idx(cur_src);

    int cidx = protein_length - (start_index + peptide_length - 1);
    if( cidx < min_index){
      min_index = cidx;
    }
    cur_src = get_peptide_src_next_association(cur_src);
  }
  return min_index;


}

/**
 * Creates a heap allocated hash_value for the peptide that should
 * uniquely identify the peptide
 *\returns the string of "<first src protein idx><start idx><length>"
 */
char* get_peptide_hash_value( 
  PEPTIDE_T*  peptide ///< The peptide whose residues to iterate over.
  )
{
  char* hash_value = NULL;
  int peptide_length_space = get_number_digits(peptide->length);
  unsigned int protein_idx = get_protein_protein_idx(get_peptide_src_parent_protein(peptide->peptide_src));
  int protein_idx_space = get_number_digits(protein_idx);
  int peptide_start_idx = get_peptide_src_start_idx(peptide->peptide_src);
  int peptide_start_idx_space = get_number_digits(peptide_start_idx);
  int status;
  int space = peptide_length_space + protein_idx_space + peptide_start_idx_space + 1;

  // allocate space for three integers
  hash_value = (char*)mycalloc(space,sizeof(char));
  
  // copy over the itegers
  status = snprintf(hash_value, 
                    space,
                    "%d%d%d", 
                   protein_idx, 
                   peptide_start_idx, 
                   peptide->length);
  
  if(status != (space-1)){
    carp(CARP_ERROR, "failed to create peptide hash value");
  }
  
  return hash_value;
}

/**
 * 
 *\returns a randomly shuffled sequence but preserves the tryptic property
 */
char* generate_shuffled_sequence(
  PEPTIDE_T* peptide, 
  ///< The peptide sequence to shuffle -in                                
  PEPTIDE_TYPE_T peptide_type 
  ///< The peptide type to enfore on the shuffled sequence
  )
{
  char* sequence = get_peptide_sequence(peptide);
  int length = peptide->length;
  int start_idx = 0;
  int end_idx = length - 1;
  int switch_idx = 0;
  char temp_char = 0;

  // set shuffle bound
  // TODO consider changing bounds depending on trypticity
  // But for now, leave the extreme N- and C-term AAs the same
  if (peptide_type == peptide_type){
    ++start_idx;
    --end_idx;
  }
  /* if(peptide_type == TRYPTIC){
    ++start_idx;
    --end_idx;
  }
  else if(peptide_type == N_TRYPTIC){
    ++start_idx;
  }
  else if(peptide_type == C_TRYPTIC){
    --end_idx;
  }*/
  
  // shuffle from left ot right, using the Knuth algorithm for shuffling.
  while(start_idx < end_idx){
    switch_idx = get_random_number_interval(start_idx, end_idx);
    temp_char = sequence[start_idx];
    sequence[start_idx] = sequence[switch_idx];
    sequence[switch_idx] = temp_char;
    ++start_idx;
  }

  return sequence;
}
/* Comparisons for sorting */

/**
 * Compare peptide sequence
 * \returns TRUE if peptide sequence is identical else FALSE
 */
BOOLEAN_T compare_peptide_sequence(
  PEPTIDE_T* peptide_one,  ///< the peptide sequence to compare  -out
  PEPTIDE_T* peptide_two  ///< the peptide sequence to compare  -out
  )
{
  // is Mass and Length identical
  if(compare_float(peptide_one->peptide_mass, peptide_two->peptide_mass) != 0 ||
     peptide_one->length != peptide_two->length){
    return FALSE;
  }
  else{
    int current_idx = 0;
    char* start_one = get_peptide_src_sequence_pointer(peptide_one->peptide_src);
    char* start_two = get_peptide_src_sequence_pointer(peptide_two->peptide_src);
    
    while(current_idx < peptide_one->length){
      if(start_one[current_idx] != start_two[current_idx]){
        return FALSE;
      }
      ++current_idx;
    } 
  }
  return TRUE;
}


/**
 * compares two peptides with the lexical sort type
 * for qsort
 * /returns 1 if peptide_one has lower priority, 0 if equal, -1 if greater priority
 */
int compare_peptide_lexical_qsort(
  PEPTIDE_T** peptide_one, ///< peptide to compare one -in
  PEPTIDE_T** peptide_two ///< peptide to compare two -in
  )
{
  // convert the protein to heavy if needed
  /* uncomment if needed, to use light heavy protein
  protein_to_heavy(get_peptide_parent_protein(peptide_one));
  protein_to_heavy(get_peptide_parent_protein(peptide_two));
  */
  PEPTIDE_T* peptide_1 = *peptide_one;
  PEPTIDE_T* peptide_2 = *peptide_two;
  char* peptide_one_sequence = get_peptide_sequence_pointer(peptide_1);
  char* peptide_two_sequence = get_peptide_sequence_pointer(peptide_2);
  int peptide_one_length = peptide_1->length;
  int peptide_two_length = peptide_2->length;
  int current_idx = 0;
  
  // check if all alphabetically identical
  while(current_idx < peptide_one_length &&
        current_idx < peptide_two_length){
    if(peptide_one_sequence[current_idx] > peptide_two_sequence[current_idx]){        
      return 1;
    }
    else if(peptide_one_sequence[current_idx] < peptide_two_sequence[current_idx]){
      return -1;
    }
    ++current_idx;
  }
  
  // alphabetically identical, check if same length
  if(peptide_one_length > peptide_two_length){
    return 1;
  }
  else if(peptide_one_length < peptide_two_length){
    return -1;
  }
  else{
    return 0;
  }
}

/**
 * compares two peptides with the mass sort type
 * if peptide mass is identical sort by lexicographical order
 * used for qsort function
 * /returns 1 if peptide_one has lower priority, 0 if equal, -1 if greater priority
 */
int compare_peptide_mass_qsort(
  PEPTIDE_T** peptide_one, ///< peptide to compare one -in
  PEPTIDE_T** peptide_two ///< peptide to compare two -in
  )
{
  // determine mass order
  int result = compare_float((*peptide_one)->peptide_mass, 
                             (*peptide_two)->peptide_mass);

  // if identical mass, sort in lexical order
  if(result == 0){
    return compare_peptide_lexical_qsort(peptide_one, peptide_two);
  }
  else{// if not identical
    return result;
  }
}

/**
 * compares two peptides with the length sort type
 * /returns 1 if peptide_one has lower priority, 0 if equal, -1 if greater priority
 */
int compare_peptide_length_qsort(
  PEPTIDE_T** peptide_one, ///< peptide to compare one -in
  PEPTIDE_T** peptide_two ///< peptide to compare two -in
  )
{
  int peptide_one_length = (*peptide_one)->length;
  int peptide_two_length = (*peptide_two)->length;
  
  // alphabetically identical, check if same length
  if(peptide_one_length > peptide_two_length){
    return 1;
  }
  else if(peptide_one_length < peptide_two_length){
    return -1;
  }
  else{
    return 0;
  }
}

/**
 * Compare peptide mass
 * \returns 0 if peptide mass is identical else 1 if peptide_one is larger, -1 if peptide_two is larger
 */
int compare_peptide_mass(
  PEPTIDE_T* peptide_one,  ///< the peptide mass to compare  -out
  PEPTIDE_T* peptide_two  ///< the peptide mass to compare  -out
  )
{
  return compare_float(peptide_one->peptide_mass, peptide_two->peptide_mass);
}

/* Public functions--Printing / parsing */

/* text writing */

/**
 * \brief Prints a peptide object in text to file.
 *
 * Used by crux-generate-peptides. Prints the peptide once for all
 * peptide_src objects associated with it.  Optional fields are
 * determined by arguments. Tab-delimited format is:
 * mass protein-id peptide-start peptide-length <peptide-trypticity>
 * <peptide-sequence>
 * Peptide start begins with 1.
 */
void print_peptide_in_format(
  PEPTIDE_T* peptide,  ///< the query peptide -in
  BOOLEAN_T flag_out, ///< print peptide sequence? -in
  BOOLEAN_T trypticity_opt, ///< print trypticity of peptide? -in
  FILE* file  ///< the out put stream -out
  )
{
  PROTEIN_T* parent = NULL;
  PEPTIDE_SRC_T* next_src = peptide->peptide_src;
  char* id = NULL;
  int start_idx = 0;
  char* sequence = NULL;

  // print mass of the peptide
    fprintf(file, "%.2f", peptide->peptide_mass);

  // obtain peptide sequence
  if(flag_out){
    parent = get_peptide_src_parent_protein(next_src);        
    if( peptide->modified_seq== NULL ){
      sequence = get_peptide_sequence(peptide);
    }else{
      sequence = modified_aa_string_to_string(peptide->modified_seq);
    }
  }

  // iterate over all peptide src
  while(next_src != NULL){
    parent = get_peptide_src_parent_protein(next_src);    
    id = get_protein_id_pointer(parent);
    start_idx = get_peptide_src_start_idx(next_src);
    
    fprintf(file, "\t%s\t%d\t%d", id, start_idx, peptide->length);
  
    // print trypticity of peptide??
    if(trypticity_opt){
      // TODO: change this to switch statement with only one get() call
      if(get_peptide_src_peptide_type(next_src) == TRYPTIC){
        fprintf(file, "\t%s", "TRYPTIC");
      }
      else if(get_peptide_src_peptide_type(next_src) == PARTIALLY_TRYPTIC){
        fprintf(file, "\t%s", "PARTIALLY_TRYPTIC");
      }
      else if(get_peptide_src_peptide_type(next_src) == N_TRYPTIC){
        fprintf(file, "\t%s", "N_TRYPTIC");
      }
      else if(get_peptide_src_peptide_type(next_src) == C_TRYPTIC){
        fprintf(file, "\t%s", "C_TRYPTIC");
      }
      else if(get_peptide_src_peptide_type(next_src) == NOT_TRYPTIC){
        fprintf(file, "\t%s", "NOT_TRYPTIC");
      }
      else if(get_peptide_src_peptide_type(next_src) == ANY_TRYPTIC){
        fprintf(file, "\t%s", "ANY_TRYPTIC");
      }
    }

    // print peptide sequence?
    if(flag_out){
      fprintf(file, "\t%s\n", sequence);
    }
    else{
      fprintf(file, "\n");
    }
    next_src = get_peptide_src_next_association(next_src);    
  }

  // free sequence if allocated
  if(flag_out){
    free(sequence);
  }
}

// TODO: this should be merged with other print, flags for optional
// fields and filtering should be taken from parameter.c and format
// adjusted accordingly
/**
 * Prints a peptide object to file.
 * ONLY prints peptide_src that match the peptide_src
 * mass \\t protein-id \\t peptide-start \\t peptide-length <\\t peptide-sequence> \n
 *      \\t protein-id \\t peptide-start \\t peptide-length <\\t peptide-sequence> \n
 * prints in correct format for generate_peptide
 */
void print_filtered_peptide_in_format(
  PEPTIDE_T* peptide,  ///< the query peptide -in
  BOOLEAN_T flag_out, ///< print peptide sequence? -in
  FILE* file,  ///< the out put stream -out
  PEPTIDE_TYPE_T peptide_type ///< the peptide_type of src to print -in
  )
{
  PROTEIN_T* parent = NULL;
  PEPTIDE_SRC_T* next_src = peptide->peptide_src;
  char* id = NULL;
  int start_idx = 0;
  char* sequence = NULL;
  // BOOLEAN_T light = FALSE;

  // print mass of the peptide
  fprintf(file, "%.2f", peptide->peptide_mass);

  // obtain peptide sequence
  if(flag_out){
    parent = get_peptide_src_parent_protein(next_src);
    
    // covnert to heavy protein
    /*
    FIXME, IF use light heavy put back
    if(get_protein_is_light(parent)){
      protein_to_heavy(parent);
      light = TRUE;
    }
    */
    sequence = get_peptide_sequence(peptide);
  }

  // iterate over all peptide src
  while(next_src != NULL){
    if(peptide_type == ANY_TRYPTIC ||
       peptide_type == get_peptide_src_peptide_type(next_src) ||
       (peptide_type == PARTIALLY_TRYPTIC && 
        (get_peptide_src_peptide_type(next_src) == N_TRYPTIC ||
         get_peptide_src_peptide_type(next_src) == C_TRYPTIC)) ){
      
      // if(!light){
      parent = get_peptide_src_parent_protein(next_src);
        
      // covnert to heavy protein
      /*
      FIXME, IF use light heavy put back
      if(get_protein_is_light(parent)){
        protein_to_heavy(parent);
        light = TRUE;
      }
      */
        // }
      
      id = get_protein_id_pointer(parent);
      start_idx = get_peptide_src_start_idx(next_src);
      
      fprintf(file, "\t%s\t%d\t%d", id, start_idx, peptide->length);
      
      // print peptide sequence?
      if(flag_out){
        fprintf(file, "\t%s\n", sequence);
      }
      else{
        fprintf(file, "\n");
      }
    
      /** 
       * uncomment this code if you want to restore a protein to 
       * light after converted to heavy
      // convert back to light
      if(light){
        protein_to_light(parent);
        light = FALSE;
      }
      */
    }
    next_src = get_peptide_src_next_association(next_src);
  }

  // free sequence if allocated
  if(flag_out){
    free(sequence);
  }
}


/* binary read/writing (serialization) */

/**
 * Serialize a peptide in binary to a FILE 
 *
 * The peptide serialization format looks like this:
 *
 * <PEPTIDE_T: peptide struct><int: number of peptide_src>[<int:
 * protein index><PEPTIDE_TYPE_T: peptide_type><int: peptide start
 * index>]+<int: modified_seq length>[<MODIFIED_AA_T>]+ 
 * The peptide src information (in square brackets) repeats for the
 * number times indicated by the number between the struct and the
 * first peptide src entry.  The protein index is the index of the
 * parent protein in the database DATABASE_T. The number of
 * MODIFIED_AA_T's is given by the int preceeding it.
 *
 * \returns TRUE if serialization is successful, else FALSE
 */
BOOLEAN_T serialize_peptide(
  PEPTIDE_T* peptide, ///< the peptide to serialize -in
  FILE* file ///< the output file to serlize -out
  )
{

  PEPTIDE_SRC_ITERATOR_T* iterator = 
    new_peptide_src_iterator(peptide);
  PEPTIDE_SRC_T* peptide_src = NULL;
  long num_src_location;
  long original_location;
  int num_src = 0;
  
  carp(CARP_DETAILED_DEBUG, "Serializing peptide %s", 
       get_peptide_sequence(peptide));
  // write the peptide struct
  fwrite(peptide, sizeof(PEPTIDE_T), 1, file);
  
  // store number of src location in file
  num_src_location = ftell(file);

  // write dummie peptide src count
  fwrite(&num_src, sizeof(int), 1, file);

  // there must be at least one peptide src
  if(!peptide_src_iterator_has_next(iterator)){
    carp(CARP_WARNING, "no peptide src");
    return FALSE;
  }

  // for each peptide src, serialize
  while(peptide_src_iterator_has_next(iterator)){
    peptide_src = peptide_src_iterator_next(iterator);

    // serialize the peptide src
    serialize_peptide_src(peptide_src, file);
    
    ++num_src;
  }
  free_peptide_src_iterator(iterator);
  
  original_location = ftell(file);
  fseek(file, num_src_location, SEEK_SET);
  
  // over write the dummie peptide_src count with real value
  fwrite(&num_src, sizeof(int), 1, file);
  
  // return to original poistion
  fseek(file, original_location, SEEK_SET);

  // write the number of MODIFIED_AA_T's to serialize
  int mod_seq_length = peptide->length;
  if( peptide->modified_seq == NULL ){
    mod_seq_length = 0;
  }
  fwrite(&mod_seq_length, sizeof(int), 1, file);

  // write modified seq
  fwrite(peptide->modified_seq, sizeof(MODIFIED_AA_T), mod_seq_length, file);

  return TRUE;
}


/**
 * Parse the binary serialized peptide use for match_analysis
 * Assumes that the file* is set at the start of the peptide_src count field
 *\returns the Peptide if successful parse the peptide form the serialized file, else NULL
 */
PEPTIDE_T* parse_peptide(
  FILE* file, ///< the serialized peptide file -in
  DATABASE_T* database, ///< the database containing the peptides -in
  BOOLEAN_T use_array  ///< should I use array peptide_src or link list -in  
  )
{  
  PROTEIN_T* parent_protein = NULL;
  PEPTIDE_SRC_T* peptide_src = NULL;
  PEPTIDE_SRC_T* current_peptide_src = NULL;
  int num_peptide_src = -1;
  unsigned int protein_idx = 0;
  int src_index =  0;
  PEPTIDE_TYPE_T peptide_type = -1;
  int start_index = -1;
  
  // the peptide to parse into
  PEPTIDE_T* peptide = allocate_peptide();
  
  // read peptide struct
  if(fread(peptide, get_peptide_sizeof(), 1, file) != 1){
    // there is no peptide
    free(peptide);
    return NULL;
  }
  
  // get total number of peptide_src for this peptide
  // peptide must have at least one peptide src
  fread(&num_peptide_src, sizeof(int), 1, file);
  //  if(fread(&num_peptide_src, sizeof(int), 1, file) != 1 ||
  //  num_peptide_src < 1){
  if( num_peptide_src < 1){
    carp(CARP_ERROR, 
         "Index file corrupted, peptide must have at least one peptide src");
    free(peptide);
    return NULL;
  }
  
  // which implemenation of peptide_src to use? Array or linklist?
  if(use_array){
    // allocate an array of empty peptide_src to be parsed information into
    // we want to allocate the number of peptide src the current protein needs
    peptide_src = new_peptide_src_array(num_peptide_src);
  }
  else{// link list
    peptide_src = new_peptide_src_linklist(num_peptide_src);
  }
  
  // set peptide src array to peptide
  add_peptide_peptide_src_array(peptide, peptide_src);
  
  // set the current peptide src to parse information into
  current_peptide_src = peptide_src;

  // parse and fill all peptide src information into peptide
  // TODO this should be moved into a parse peptide_src routine
  for(; src_index < num_peptide_src; ++src_index){
    // **read peptide src fields**//
    
    // get protein index
    if(fread(&protein_idx, (sizeof(int)), 1, file) != 1){
      carp(CARP_ERROR, "index file corrupted, incorrect protein index");
      free(peptide);
      return NULL;
    }
    carp(CARP_DETAILED_DEBUG, "protein idx read is %i", protein_idx);
    
    // read peptide type of peptide src
    if(fread(&peptide_type, sizeof(PEPTIDE_TYPE_T), 1, file) != 1){
      carp(CARP_ERROR, "index file corrupted, failed to read peptide src");
      free(peptide_src);
      free(peptide);
      return NULL;
    }

    // read start index of peptide in parent protein of thsi peptide src
    if(fread(&start_index, sizeof(int), 1, file) != 1){
      carp(CARP_ERROR, "index file corrupted, failed to read peptide src");
      free(peptide_src);
      free(peptide);
      return NULL;
    }
    
    /** set all fields in peptide src that have been read **/

    // get the peptide src parent protein
    parent_protein = 
      get_database_protein_at_idx(database, protein_idx);
    
    // set parent protein of the peptide src
    set_peptide_src_parent_protein(current_peptide_src, parent_protein);

    // set peptide type of peptide src
    set_peptide_src_peptide_type(current_peptide_src, peptide_type);

    // set start index of peptide src
    set_peptide_src_start_idx(current_peptide_src, start_index);
    
    // set current_peptide_src to the next empty peptide src
    current_peptide_src= get_peptide_src_next_association(current_peptide_src);
  }
  
  // read the length of the modified aa sequence
  int mod_seq_len = -1;
  fread(&mod_seq_len, sizeof(int), 1, file);
  if( mod_seq_len < 0 ){
    carp(CARP_ERROR, "Did not read the correct length of modified sequence");
    peptide->modified_seq = NULL;
  }

  // read in modified sequence
  fread(peptide->modified_seq, sizeof(MODIFIED_AA_T), mod_seq_len, file);
  
  return peptide;
}

/* Public functions--Iterators */

/**
 * Instantiates a new residue_iterator from a peptide.
 * \returns a RESIDUE_ITERATOR_T object.
 */
RESIDUE_ITERATOR_T* new_residue_iterator(
  PEPTIDE_T* peptide ///< peptide sequence to iterate -in
  )
{
  RESIDUE_ITERATOR_T* residue_iterator =
    (RESIDUE_ITERATOR_T*)mycalloc(1, sizeof(RESIDUE_ITERATOR_T));
  
  residue_iterator->peptide =  peptide;
  residue_iterator->residue_idx = 0;
  residue_iterator->sequence = get_peptide_sequence(peptide);
  return residue_iterator;
}        

/**
 * Frees an allocated residue_iterator object.
 */
void free_residue_iterator(
  RESIDUE_ITERATOR_T* residue_iterator ///< free this object -in
  )
{
  free(residue_iterator->sequence);
  free(residue_iterator);
}

/**
 * The basic iterator functions.
 * \returns TRUE if there are additional residues to iterate over, FALSE if not.
 */
BOOLEAN_T residue_iterator_has_next(
  RESIDUE_ITERATOR_T* residue_iterator ///< the query iterator -in
  )
{
  return (residue_iterator->residue_idx < residue_iterator->peptide->length);
}

/**
 * \returns The next residue (a character) in the peptide.
 */
char residue_iterator_next(
  RESIDUE_ITERATOR_T* residue_iterator  ///< the query iterator -in
  )
{
  ++residue_iterator->residue_idx;
  return residue_iterator->sequence[residue_iterator->residue_idx - 1];
}


/**
 * Protein peptide association Iterator
 */

/**
 * Instantiates a new peptide_src_iterator from a peptide.
 * \returns a PEPTIDE_SRC_T object.
 */
PEPTIDE_SRC_ITERATOR_T* new_peptide_src_iterator(
  PEPTIDE_T* peptide ///< peptide's fields to iterate -in
  )
{
  PEPTIDE_SRC_ITERATOR_T* association_iterator =
    (PEPTIDE_SRC_ITERATOR_T*)mycalloc(1,sizeof(PEPTIDE_SRC_ITERATOR_T));
  association_iterator->peptide = peptide;
  association_iterator->current = peptide->peptide_src;
  return association_iterator;
}

/**
 * Frees an allocated peptide_src_iterator object.
 */
void free_peptide_src_iterator(
  PEPTIDE_SRC_ITERATOR_T* peptide_src_iterator ///< free this object -in
  )
{
  free(peptide_src_iterator);

}

/**
 * The basic iterator functions.
 * \returns TRUE if there are additional peptide_srcs to iterate over, FALSE if not.
 */
BOOLEAN_T peptide_src_iterator_has_next(
  PEPTIDE_SRC_ITERATOR_T* peptide_src_iterator///< the query iterator -in
  )
{
  return !(peptide_src_iterator->current == NULL);
}

/**
 * \returns The next peptide_srcs in the peptide.
 */
PEPTIDE_SRC_T* peptide_src_iterator_next(
  PEPTIDE_SRC_ITERATOR_T* peptide_src_iterator///< the query iterator -in
  )
{
  PEPTIDE_SRC_T* previous = peptide_src_iterator->current;
  if(peptide_src_iterator->current != NULL){
    peptide_src_iterator->current = 
      get_peptide_src_next_association(peptide_src_iterator->current);
  }
  else{
    free(peptide_src_iterator);
    die("ERROR: no more peptide_srcs to iterate\n");
  }
  return previous;
}
 

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */


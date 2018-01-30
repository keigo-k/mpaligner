/*
mpaligner is program to align string and string.
Copyright (C) 2010, 2011, 2012 Keigo Kubo

mpaligner is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

mpaligner is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with mpaligner.  If not, see <http://www.gnu.org/licenses/>.

Imprement main function of mnimum pattern alignment algorithm
date:   2012/3/15
author: Keigo Kubo
*/

#ifndef _INCLUDE_MPAlign_
#define _INCLUDE_MPAlign_

#include "HASH_func.h"
#define INV_LOG_TEN 0.434294482
// 1.0e-9 is measure for calculation error
#define CMP_VITERBI_HYP(vh1, vh2)                                              \
  (vh1.score > (vh2.score - 1.0e-9) &&                                         \
   ((vh1.score - 1.0e-9) > vh2.score ||                                        \
    (vh1.num_of_del + vh1.num_of_ins) < (vh2.num_of_del + vh2.num_of_ins) ||   \
    ((vh1.num_of_del + vh1.num_of_ins) == (vh2.num_of_del + vh2.num_of_ins) && \
     vh1.num_of_trans < vh2.num_of_trans)))                                    \
      ? (1)                                                                    \
      : (0)

typedef struct Pair_Data {
  char **x_part_strs;
  char **y_part_strs;
  LEN x_size;
  LEN y_size;
  struct Pair_Data *next;
} PAIR_DATA;

typedef struct Viterbi_Hyp {
  LEN backX;
  LEN backY;
  LEN back_num_of_del_and_ins;
  unsigned char rank;
  LEN num_of_trans;
  LEN num_of_del;
  LEN num_of_ins;
  LEN num_of_consecutive_del;
  LEN max_num_of_consecutive_del;
  Parameter score;
} VITERBI_HYP;

typedef struct Viterbi_Hyp_List {
  char rank_size;
  VITERBI_HYP *viterbi_hyp;
} VITERBI_HYP_LIST;

typedef struct Align_Hyp {
  COP **cop;
  Parameter score;
} ALIGN_HYP;

// IO condition
extern char *input_file;
extern char *output_file;
extern char *previous_knowledge_file;
extern char *input_align_model;
extern char *output_align_model;
extern char *output_used_mapping;
extern char printScore;

// alignment condition
extern unsigned char noConsecutiveDelAndIns;
extern unsigned char noEqMap;
extern unsigned char substringCheck;
extern unsigned char alignment_type;
extern unsigned char doubtful_context_size;
extern unsigned char doubtful_condition;
extern unsigned char training_n_best;
extern unsigned char first_n_best;
extern unsigned char second_n_best;
extern unsigned char output_n_best;
extern unsigned char training_type;
extern unsigned char kind_of_cityblock;

extern char del; // allow deletion
extern char ins; // allow insertion
extern float del_penalty;
extern float ins_penalty;
extern short restrictX; // restrict length of part string X
extern short restrictY; // restrict length of part string Y
extern LEN maxX;        // max length of X
extern LEN maxY;        // max length of Y
extern LEN maxLEN;      // maxX+maxY or maxX or maxY

extern char unknown_char;  // separate char of input file
extern char separate_char; // separate char of output file
extern char join_char;     // join char
extern char del_ins_char;  // deletion and insertion char
extern char escape_char;

extern const char inter_unknown;  // separate char of input file
extern const char inter_separate; // separate char of output file
extern const char inter_join;     // join char
extern const char inter_del_ins;  // deletion and insertion char
extern char tmp_del_ins_char[2];  // string version of deletion and insertion
                                  // char
extern char tmp_separate_char;    // This is needed semi-supervised learning

extern float threshold_fb_eot; // threshold for the end of the forward-backward
                               // training
extern float threshold_v_eot;  // threshold for the end of the n-best viterbi
                               // training

// other condition
extern unsigned int hash_size;
extern unsigned int sqrt_hash_size;

typedef struct total_info {
  PAIR_DATA *pair_data;
  char *delimiters_x; // array for storing mulitiple kind of delimiter of x
  char *delimiters_y;

  Parameter total_update; // total of update value by training
  Parameter total_change; // total of change value by training
} TOTAL_INFO;

void readInputFile(TOTAL_INFO *info);
void readPreviousKnowledge();
void readAlignFromFile();
void training(TOTAL_INFO *info);
void writeMappingToFile(char used_mapping);

void requireAlignments(TOTAL_INFO *info);

#endif

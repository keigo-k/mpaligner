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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "toolOptions.h"
#include "mpAlign.h"

// IO condition
char *input_file = NULL;
char *output_file = NULL;
char *previous_knowledge_file = NULL;
char *input_align_model = NULL;
char *output_align_model = NULL;
char *output_used_mapping = NULL;
char printScore = 0;

// alignment condition
unsigned char noConsecutiveDelAndIns = 0;
unsigned char noEqMap = 0;
unsigned char alignment_type = 1;
unsigned char substringCheck = 0;
unsigned char doubtful_context_size = 1;
unsigned char doubtful_condition = 1;
unsigned char first_n_best = 1;
unsigned char second_n_best = 1;
unsigned char output_n_best = 0;
unsigned char training_type = 5;
unsigned char training_n_best = 2;
unsigned char kind_of_cityblock = 0;

char del; // allow deletion
char ins; // allow insertion
float del_penalty = 0.5;
float ins_penalty = 1.0;
short restrictX = 0; // restrict length of part string X
short restrictY = 0; // restrict length of part string Y
LEN maxX = 0;        // max length of X
LEN maxY = 0;        // max length of Y
LEN maxLEN = 0;      // maxX+maxY or maxX or maxY

char unknown_char = ' ';  // separate char of input file
char separate_char = '|'; // separate char of output file
char join_char = ':';     // join char
char del_ins_char = '_';  // deletion char
char escape_char = '\\';

const char inter_unknown = 0x1c;           // separate char of input file
const char inter_separate = 0x1d;          // separate char of output file
const char inter_join = 0x1e;              // join char
const char inter_del_ins = 0x1f;           // deletion char
char tmp_del_ins_char[2] = { 0x1f, '\0' }; // string version of deletion char
char tmp_separate_char = 0x0b;

float threshold_fb_eot =
    0.1; // threshold for the end of the forward-backward training
float threshold_v_eot =
    0.1; // threshold for the end of the n-best viterbi training

// other condition
unsigned int hash_size = 99940009;
unsigned int sqrt_hash_size = 0;

void usage() {

  fprintf(stderr, "mpaligner is tool to align string and string.\n"
                  "Copyright (C) 2010, 2011, 2012 Keigo Kubo\n"
                  "version 0.97\n\n");

  fprintf(
      stderr,
      "usage:"
      "./mpaligner -i <string> [-o <string>] [-p <string>] [-ai <string>]\n"
      "           [-ao <string>] [-output_used_mapping <string>] [-s]\n"
      "           [-hs <int>] [-del] [-ins] [-rx <int>] [-ry <int>]\n"
      "           [-uc <char>] [-sc <char>] [-jc <char>] [-dic <char>]\n"
      "           [-training_type 0|1|2|3|4|5] [-kind_of_cityblock 0|1|2]\n"
      "           [-t <int or float>] [-tfb <int or float>] [-tv <int or "
      "float>]\n"
      "           [-substringCheck] [-noConsecutiveDelAndIns] [-noEqMap]\n"
      "           [-alignment_type 0|1|2] [-doubtful_context_size <int>]\n"
      "           [-doubtful_condition 0|1] [-t_nbest <int>] [-f_nbest <int>]\n"
      "           [-s_nbest <int>] [-output_nbest <int>] [-nbest <int>] "
      "[-h]\n");

  fprintf(
      stderr,
      "options:\n"
      "  -i <string>\n    input alignments file.\n\n"
      "  -o <string>\n    output alignments file. (default <input file "
      "name>.align)\n\n"
      "  -p <string>\n    input previous knowledge file.\n\n"
      "  -ai <string>\n    input align model file.\n\n"
      "  -ao <string>\n    output align model file.\n\n"
      "  -output_used_mapping <string>\n    output used mappings file.\n\n"
      "  -s\n    print score in output file. (default don't print)\n\n"
      "  -hs <int>\n    hash size. (default 99,940,009)\n\n"
      "  -del\n    allow to deletion (del). (default don't allow)\n\n"
      "  -ins\n    allow to insertion (ins). (default don't allow)\n\n"
      "  -dp <float>\n    penalty value of del. (default 0.5)\n\n"
      "  -ip <float>\n    penalty value of ins. (default 1.0)\n\n"
      "  -rx <int>\n    restrict length of part string X.\n"
      "    (default 0: 0 is alignment without restrict.)\n\n"
      "  -ry <int>\n    restrict length of part string Y.\n"
      "    (default 0: 0 is alignment without restrict.)\n\n"
      "  -uc <char>\n    unknown char. (default ' ')\n\n"
      "  -sc <char>\n    separate char. (default '|')\n\n"
      "  -jc <char>\n    join char. (default ':')\n\n"
      "  -dic <char>\n    del and ins char. (default '_')\n\n"
      "  -training_type 0|1|2|3|4|5\n    training type. (defalut 5)\n"
      "    0 is the forward-backward training that does not introduce city "
      "block distance.\n"
      "    1 is the forward-backward training and the n-best viterbi training, "
      "not introducing city block distance respectively.\n"
      "    2 is the forward-backward training that introduces city block "
      "distance.\n"
      "    3 is the forward-backward training and the n-best viterbi training, "
      "introducing city block distance respectively.\n"
      "    4 is the forward-backward training that prohibits del and ins and "
      "the n-best viterbi training within del and ins inference, introducing "
      "city block distance respectively. The viterbi training within del and "
      "ins inference infers del and ins by using peri-mappings and runs one "
      "iteration.\n"
      "    5 is the forward-backward training that prohibits del and ins, the "
      "n-best viterbi training within del and ins inference and the n-best "
      "viterbi training, introducing city block distance respectively.\n\n"
      "  -t <int or float>\n    Threshold for the end of the forward-backward "
      "training and the n-best viterbi training. (defalut 0.1)\n"
      "    If it's more than 1, it's the number of iteration of the training.\n"
      "    If it's less than 1, it's threshold in change values of a\n"
      "    parameter by training. If a total of change values of a parameter\n"
      "    is less than it, the training is end.\n\n"
      "  -kind_of_cityblock 0|1|2\n    Kind of city block distance.(defalut "
      "0)\n"
      "    0 uses the number of all characters as city block distance.\n"
      "    1 uses the number of y's characters as city block distance.\n"
      "    2 uses the number of x's characters as city block distance.\n\n"
      "  -tfb <int or float>\n    Threshold for the end of the "
      "forward-backward training. (defalut 0.1)\n\n"
      "  -tv <int or float>\n    Threshold for the end of the n-best viterbi "
      "training. (defalut 0.1)\n\n"
      "  -substringCheck\n    Also check substring in validation of pair "
      "data.\n\n"
      "  -noConsecutiveDelAndIns\n    Don't allow to exist a consecutive del "
      "and ins\n\n"
      "  -noEqMap\n    Not map a x's char size == y's char size mapping.\n\n"
      "  -alignment_type 0|1|2\n    alignment type. (defalut 1)\n"
      "    0 is the alignment without join of doubtful mappings.\n"
      "    1 is the alignment with join of doubtful mappings.\n"
      "    2 is a detection of pair data that include doubtful mappings.\n\n"
      "  -doubtful_context_size <int>\n   context size to judge doubtful "
      "mappings. (defalut 1)\n"
      "    This tool regards a mappings that the number of context of front "
      "or\n"
      "    back is small as doubtful mappings. So set the small context "
      "size.\n\n"
      "  -doubtful_condition 0|1\n    condition to judge doubtful mappings. "
      "(defalut 1)\n"
      "    0 regards a mappings that the number of context of front and back\n"
      "    is small as doubtful mappings. if doubtful_context_size=1 and\n"
      "    a mapping format is <[context_size]mapping[context_size]>, \n"
      "    \"<[9]mapping[1]><[1]mapping[5]>\" is doubtful mappings.\n\n"
      "    1 is regards a mappings that the number of context of front or\n"
      "    back is small as doubtful mappings. If doubtful_context_size=1,\n"
      "    \"<[9]mapping[4]><[1]mapping[5]>\" or "
      "\"<[9]mapping[1]><[3]mapping[5]>\"\n"
      "    is doubtful mappings.\n\n"
      "  -t_nbset <int>\n    N-best of the n-best viterbi training or a "
      "modified n-best viterbi training. (defalut 2)\n\n"
      "  -f_nbest <int>\n    Internal n-best of first alignment. (defalut 1)\n"
      "    Alignment type 0's n-best and alignment type 2's n-best follows\n"
      "    this option. N-best of first alignment in alignment type 1 also "
      "follows\n"
      "    this option.\n\n"
      "  -s_nbest <int>\n    Internal n-best of second alignment in alignment "
      "type 1. (defalut 1)\n\n"
      "  -output_nbest <int>\n    N-best output. (defalut -f_nbest or "
      "-s_nbest)\n"
      "  -nbest <int>\n    A simple option of n-best.\n"
      "    If alignment_type is 0 or 2, this option is equal to -f_nbest.\n"
      "    If alignment_type is 1, this option is equal to -s_nbest.\n");
}

int main(int argc, char **argv) {
  int len = 0;
  char help = 0;
  unsigned char n_best = 0;
  float threshold_eot = 0;
  time_t time1, time2;
  time(&time1);

  TOTAL_INFO info = { NULL, // pair data
                      NULL, // array for delimiters of x
                      NULL, // array for delimiters of y
                      0,    // total of update value by training
                      0 };  // total of change value by training

  set_option("-i", &input_file);
  set_option("-o", &output_file);
  set_option("-p", &previous_knowledge_file);
  set_option("-ai", &input_align_model);
  set_option("-ao", &output_align_model);
  set_option("-output_used_mapping", &output_used_mapping);
  set_option("-s", &printScore);
  set_option("-hs", &hash_size);
  set_option("-del", &del);
  set_option("-ins", &ins);
  set_option("-dp", &del_penalty);
  set_option("-ip", &ins_penalty);
  set_option("-rx", &restrictX);
  set_option("-ry", &restrictY);
  set_option("-uc", &unknown_char);
  set_option("-sc", &separate_char);
  set_option("-jc", &join_char);
  set_option("-dic", &del_ins_char);
  set_option("-t", &threshold_eot);
  set_option("-tfb", &threshold_fb_eot);
  set_option("-tv", &threshold_v_eot);
  set_option("-substringCheck", &substringCheck);
  set_option("-noConsecutiveDelAndIns", &noConsecutiveDelAndIns);
  set_option("-noEqMap", &noEqMap);
  set_option("-alignment_type", &alignment_type);
  set_option("-training_type", &training_type);
  set_option("-kind_of_cityblock", &kind_of_cityblock);
  set_option("-doubtful_context_size", &doubtful_context_size);
  set_option("-doubtful_condition", &doubtful_condition);
  set_option("-t_nbest", &training_n_best);
  set_option("-f_nbest", &first_n_best);
  set_option("-s_nbest", &second_n_best);
  set_option("-output_nbest", &output_n_best);
  set_option("-nbest", &n_best);
  set_option("-h", &help);

  parse_argv(argv);

  if (help) {
    usage();
    exit(EXIT_FAILURE);
  }

  if (input_file == NULL) {
    fprintf(stderr,
            "Please set -i option with input file name or set -h option.\n\n");
    exit(EXIT_FAILURE);
  }

  if (n_best != 0) {
    if (alignment_type != 1) {
      first_n_best = n_best;
    } else {
      second_n_best = n_best;
    }
  }

  if (alignment_type != 1 &&
      (output_n_best == 0 || output_n_best > first_n_best)) {
    output_n_best = first_n_best;
  } else if (alignment_type == 1 &&
             (output_n_best == 0 || output_n_best > second_n_best)) {
    output_n_best = second_n_best;
  }

  if (threshold_eot != 0) {
    threshold_fb_eot = threshold_eot;
    threshold_v_eot = threshold_eot;
  }

  if (training_type > 5) {
    fprintf(stderr, "Please set the number that ranges from 0 to 5 in "
                    "-training_type option.\n\n");
    exit(EXIT_FAILURE);
  } else if (training_type == 4 && del == 0 && ins == 0) {
    fprintf(stderr, "The n-best viterbi training within del and ins inference "
                    "needs to allow del or ins.\n");
    exit(EXIT_FAILURE);
  }

  if (alignment_type > 2) {
    fprintf(stderr, "Please set the number that ranges from 0 to 2 in "
                    "-alignment_type option.\n\n");
    exit(EXIT_FAILURE);
  }

  if (doubtful_condition > 1) {
    fprintf(stderr, "Please set the number that ranges from 0 to 1 in "
                    "-doubtful_condition option.\n\n");
    exit(EXIT_FAILURE);
  }

  sqrt_hash_size = (unsigned int)sqrt((double)hash_size);
  COP_init(sqrt_hash_size, sqrt_hash_size);

  if (input_align_model == NULL) {
    if (substringCheck == 1) {
      fprintf(stderr,
              "The substringCheck option is for validation of pair data.\n");
      fprintf(stderr, "In validation of pair data, the input_align_model "
                      "option should be set.\n");
      exit(EXIT_FAILURE);
    }

    // Training with input file
    readInputFile(&info);

    // For getting memory for Viterbi training within del and ins inference
    // and back trace of Viterbi algorithm
    if (del == 0 && ins == 0) {
      maxLEN = (maxX > maxY) ? maxX : maxY;
    } else if (del == 1 && ins == 0) {
      maxLEN = maxX;
    } else if (del == 0 && ins == 1) {
      maxLEN = maxY;
    } else {
      maxLEN = maxX + maxY;
    }

    training(&info);

    if (output_align_model != NULL) {
      writeMappingToFile(0); // 0 means to write all mappings
    }
    /*
    else{
            len=strlen(input_file);
            if((output_align_model=(char *)malloc(len+6))==NULL){
                    fprintf(stderr,"Don't get memory in malloc.\nYou must need
    more memory.\n");
                    exit(EXIT_FAILURE);
            }

            strcpy(output_align_model, input_file);
            strcat(output_align_model,".model");
            writeMappingToFile(0);
            free(output_align_model);
    }
    */
  } else {
    readInputFile(&info);

    fprintf(stderr, "Read align model.\n");
    readAlignFromFile();

    // For getting memory for back trace of Viterbi algorithm
    if (del == 0 && ins == 0) {
      maxLEN = (maxX > maxY) ? maxX : maxY;
    } else if (del == 1 && ins == 0) {
      maxLEN = maxX;
    } else if (del == 0 && ins == 1) {
      maxLEN = maxY;
    } else {
      maxLEN = maxX + maxY;
    }
  }

  // Align input file
  requireAlignments(&info);

  if (output_used_mapping != NULL) {
    writeMappingToFile(1); // 1 means to only write used mappings
  }

  COP_destroy();
  time(&time2);
  fprintf(stderr, "finish: %f (sec)\n", difftime(time2, time1));
  return 1;
}


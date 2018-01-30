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

#include "toolOptions.h"

void *options[35][3] = { { "-i", "string", NULL },
                         { "-o", "string", NULL },
                         { "-p", "string", NULL },
                         { "-ai", "string", NULL },
                         { "-ao", "string", NULL },
                         { "-output_used_mapping", "string", NULL },
                         { "-s", "boolean", NULL },
                         { "-hs", "unsigned int", NULL },
                         { "-del", "boolean", NULL },
                         { "-ins", "boolean", NULL },
                         { "-dp", "float", NULL },
                         { "-ip", "float", NULL },
                         { "-rx", "short", NULL },
                         { "-ry", "short", NULL },
                         { "-uc", "char", NULL },
                         { "-sc", "char", NULL },
                         { "-jc", "char", NULL },
                         { "-dic", "char", NULL },
                         { "-t", "float", NULL },
                         { "-tfb", "float", NULL },
                         { "-tv", "float", NULL },
                         { "-substringCheck", "boolean", NULL },
                         { "-noConsecutiveDelAndIns", "boolean", NULL },
                         { "-noEqMap", "boolean", NULL },
                         { "-alignment_type", "unsigned char", NULL },
                         { "-doubtful_context_size", "unsigned char", NULL },
                         { "-doubtful_condition", "unsigned char", NULL },
                         { "-t_nbest", "unsigned char", NULL },
                         { "-f_nbest", "unsigned char", NULL },
                         { "-s_nbest", "unsigned char", NULL },
                         { "-output_nbest", "unsigned char", NULL },
                         { "-nbest", "unsigned char", NULL },
                         { "-h", "boolean", NULL },
                         { "-training_type", "unsigned char", NULL },
                         { "-kind_of_cityblock", "unsigned char", NULL } };

int set_option(char *option, void *address) {
  int i;
  for (i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
    if (STRCMP((char *)options[i][0], option)) {
      options[i][2] = address;
      return 1;
    }
  }
  return 0;
}

void parse_argv(char **argv) {
  int i, error = 0;
  while (*(++argv)) {
    for (i = 0; i < sizeof(options) / sizeof(options[0]) || !(error = 1); i++) {
      if (STRCMP((char *)options[i][0], *argv)) {

        if (STRCMP((char *)options[i][1], "int")) {
          if (*(++argv) != NULL) {
            *((int *)options[i][2]) = atoi(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "unsigned int")) {
          if (*(++argv) != NULL) {
            *((unsigned int *)options[i][2]) = atoi(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "short")) {
          if (*(++argv) != NULL) {
            *((short *)options[i][2]) = atof(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "unsigned short")) {
          if (*(++argv) != NULL) {
            *((unsigned short *)options[i][2]) = atof(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "float")) {
          if (*(++argv) != NULL) {
            *((float *)options[i][2]) = atof(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "double")) {
          if (*(++argv) != NULL) {
            *((double *)options[i][2]) = atof(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "string")) {
          if (*(++argv) != NULL) {
            *((char **)options[i][2]) = *argv;
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "char")) {
          if (*(++argv) != NULL) {
            if (strlen(*argv) == 1) {
              *((char *)options[i][2]) = **argv;
            } else {
              fprintf(stderr, "ERROR:%s option is set char type. "
                              "\"%s\" is string type.\n",
                      (char *)options[i][0], *argv);
              exit(EXIT_FAILURE);
            }
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "unsigned char")) {
          if (*(++argv) != NULL) {
            *((unsigned char *)options[i][2]) = atoi(*argv);
          } else {
            fprintf(stderr, "ERROR:Don't set argument after %s option.\n",
                    (char *)options[i][0]);
            exit(EXIT_FAILURE);
          }
        } else if (STRCMP((char *)options[i][1], "boolean")) {
          *((char *)options[i][2]) = 1;
        } else {
          fprintf(stderr, "ERROR:Unknown option type:%s\n",
                  (char *)options[i][1]);
          exit(EXIT_FAILURE);
        }

        break;
      }
    }
    if (error == 1) {
      fprintf(stderr, "ERROR:Unknown option:%s\n", *argv);
      exit(EXIT_FAILURE);
    }
  }
}


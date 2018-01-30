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

#ifndef _INCLUDE_HASH_func_
#define _INCLUDE_HASH_func_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef short LEN;
typedef double Parameter;
#define LOW_LOG_VALUE -1.0e+22

typedef struct cop {
  char *x;
  char *y;
  LEN x_len;
  LEN y_len;

  char refer_flag;

  Parameter para;
  Parameter update_para;
  struct cop *next;
} COP;

COP ***COP_TABLE;
unsigned int COP_NUM;

void COP_init(unsigned int row, unsigned int col);
void COP_destroy();

Parameter COP_refer_para(char *target_x, LEN target_x_len, char *target_y,
                         LEN target_y_len);

COP *COP_get(char *target_x, LEN target_x_len, char *target_y,
             LEN target_y_len);

COP *COP_refer(char *target_x, LEN target_x_len, char *target_y,
               LEN target_y_len);

COP *COP_get_with_memory(char *target_x, LEN target_x_len, char *target_y,
                         LEN target_y_len);

typedef struct num_of_context_type {
  unsigned short left;
  unsigned short right;
  COP *cop;
  // COP *inside_left_cop;
  // COP *inside_right_cop;
  struct num_of_context_type *next;
} NumOfContextType;

NumOfContextType **NumOfContextType_TABLE;
void NumOfContextType_init(unsigned int row);
void NumOfContextType_destroy();
NumOfContextType *NumOfContextType_get(COP *cop);
NumOfContextType *NumOfContextType_refer(COP *cop);

typedef struct bigram {
  unsigned short refer;
  char split;
  COP *front;
  COP *back;
  struct bigram *next;
} Bigram;

Bigram ***Bigram_TABLE;
void Bigram_init(unsigned int row, unsigned int col);
void Bigram_destroy();
Bigram *Bigram_get(COP *front, COP *back);
Bigram *Bigram_refer(COP *front, COP *back);

#endif


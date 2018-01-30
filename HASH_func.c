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

#include "HASH_func.h"
#include "mpAlign.h"

static unsigned int COP_ROW_HASH_SIZE;
static unsigned int COP_COL_HASH_SIZE;

void COP_init(unsigned int row, unsigned int col) {
  unsigned int i, j;

  COP_NUM = 0;
  COP_ROW_HASH_SIZE = row;
  COP_COL_HASH_SIZE = col;
  if ((COP_TABLE = (COP ***)malloc(row * sizeof(COP **) +
                                   row * col * sizeof(COP *))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < row; i++) {
    COP_TABLE[i] = (COP **)(COP_TABLE + row) + i * col;
    for (j = 0; j < col; j++) {
      COP_TABLE[i][j] = NULL;
    }
  }
}

void COP_destroy() {
  COP *p;
  unsigned int i, j;
  for (i = 0; i < COP_ROW_HASH_SIZE; i++) {
    for (j = 0; j < COP_COL_HASH_SIZE; j++) {
      for (p = COP_TABLE[i][j]; p != NULL; p = COP_TABLE[i][j]) {
        COP_TABLE[i][j] = p->next;
        free(p);
      }
    }
  }
  free(COP_TABLE);
}

static COP **COP_hash_len(char *target_x, LEN target_x_len, char *target_y,
                          LEN target_y_len) {
  COP **p;
  unsigned int value = 0;
  char c;
  while (target_x_len--) {
    while ((c = *target_x++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      value += (c * c) * (255 << target_x_len);
    }
    value /= 7;
  }
  p = COP_TABLE[value % COP_ROW_HASH_SIZE];

  value = 0;
  while (target_y_len--) {
    while ((c = *target_y++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      value += (c * c) * (255 << target_y_len);
    }
    value /= 7;
  }

  return &(p[value % COP_COL_HASH_SIZE]);
}

static int COP_keyequal_len(char *x, char *y, char *target_x, LEN target_x_len,
                            char *target_y, LEN target_y_len) {
  char c;
  while (target_x_len--) {
    while ((c = *target_x++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      if (*x++ != c) {
        return 0;
      }
    }
    if (*x != inter_unknown && *x != inter_join && *x != inter_separate &&
        *x != '\0') {
      return 0;
    }
    x++;
  }

  while (target_y_len--) {
    while ((c = *target_y++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      if (*y++ != c) {
        return 0;
      }
    }
    if (*y != inter_unknown && *y != inter_join && *y != inter_separate &&
        *y != '\0') {
      return 0;
    }
    y++;
  }

  return 1;
}

Parameter COP_refer_para(char *target_x, LEN target_x_len, char *target_y,
                         LEN target_y_len) {
  COP **pp = COP_hash_len(target_x, target_x_len, target_y, target_y_len);
  COP *p = *pp;

  // search co-occurrence parameter node
  for (; p != NULL; p = p->next) {
    if (p->x_len == target_x_len && p->y_len == target_y_len &&
        COP_keyequal_len(p->x, p->y, target_x, target_x_len, target_y,
                         target_y_len)) {
      return p->para;
    }
  }

  return 0;
}

COP *COP_get(char *target_x, LEN target_x_len, char *target_y,
             LEN target_y_len) {
  COP **pp = COP_hash_len(target_x, target_x_len, target_y, target_y_len);
  COP *p = *pp;

  // search co-occurrence parameter node
  for (; p != NULL; p = p->next) {
    if (p->x_len == target_x_len && p->y_len == target_y_len &&
        COP_keyequal_len(p->x, p->y, target_x, target_x_len, target_y,
                         target_y_len)) {
      return p;
    }
  }

  // generate new co-occurrence parameter node
  if ((p = (COP *)malloc(sizeof(COP))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  p->next = *pp;
  *pp = p;

  p->x = target_x;
  p->x_len = target_x_len;

  p->y = target_y;
  p->y_len = target_y_len;

  p->refer_flag = 0;
  p->para = 0;
  p->update_para = LOW_LOG_VALUE;

  COP_NUM++;
  return p;
}

COP *COP_refer(char *target_x, LEN target_x_len, char *target_y,
               LEN target_y_len) {
  COP **pp = COP_hash_len(target_x, target_x_len, target_y, target_y_len);
  COP *p = *pp;

  // search co-occurrence parameter node
  for (; p != NULL; p = p->next) {
    if (p->x_len == target_x_len && p->y_len == target_y_len &&
        COP_keyequal_len(p->x, p->y, target_x, target_x_len, target_y,
                         target_y_len)) {
      return p;
    }
  }
  return NULL;
}

COP *COP_get_with_memory(char *target_x, LEN target_x_len, char *target_y,
                         LEN target_y_len) {
  char c;
  char *tmp_target;
  LEN i, byte_x = 0, byte_y = 0, tmp_len;
  COP **pp = COP_hash_len(target_x, target_x_len, target_y, target_y_len);
  COP *p = *pp;

  // search co-occurrence parameter node
  for (; p != NULL; p = p->next) {
    if (p->x_len == target_x_len && p->y_len == target_y_len &&
        COP_keyequal_len(p->x, p->y, target_x, target_x_len, target_y,
                         target_y_len)) {
      return p;
    }
  }

  // generate new co-occurrence parameter node
  tmp_len = target_x_len;
  tmp_target = target_x;
  while (tmp_len--) {
    while ((c = *tmp_target++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      byte_x++;
    }
    byte_x++;
  }

  tmp_len = target_y_len;
  tmp_target = target_y;
  while (tmp_len--) {
    while ((c = *tmp_target++) != inter_unknown && c != inter_join &&
           c != inter_separate && c != '\0') {
      byte_y++;
    }
    byte_y++;
  }

  if ((p = (COP *)malloc(sizeof(COP) + byte_x + byte_y)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  p->next = *pp;
  *pp = p;

  p->x_len = target_x_len;
  p->y_len = target_y_len;

  p->x = (char *)(p + 1);
  for (i = 0; i < byte_x; i++) {
    p->x[i] = target_x[i];
  }

  p->y = &(p->x[byte_x]);
  for (i = 0; i < byte_y; i++) {
    p->y[i] = target_y[i];
  }

  p->refer_flag = 0;
  p->para = 0;
  p->update_para = LOW_LOG_VALUE;

  COP_NUM++;
  return p;
}

//////////////////////////////////////////////////////////////////////////////
// Hash for Num of context type
//////////////////////////////////////////////////////////////////////////////

static unsigned int NumOfContextType_ROW_HASH_SIZE;

void NumOfContextType_init(unsigned int row) {
  unsigned int i;

  NumOfContextType_ROW_HASH_SIZE = row;
  if ((NumOfContextType_TABLE = (NumOfContextType **)malloc(
           row * sizeof(NumOfContextType *))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < row; i++) {
    NumOfContextType_TABLE[i] = NULL;
  }
}

void NumOfContextType_destroy() {
  NumOfContextType *p;
  unsigned int i;
  for (i = 0; i < NumOfContextType_ROW_HASH_SIZE; i++) {
    for (p = NumOfContextType_TABLE[i]; p != NULL;
         p = NumOfContextType_TABLE[i]) {
      NumOfContextType_TABLE[i] = p->next;
      free(p);
    }
  }
  free(NumOfContextType_TABLE);
}

static unsigned int NumOfContextType_hash(COP *cop) {
  unsigned long int i = (unsigned long int)cop;
  i += (i >> 13);
  i += (i << 13);

  return i % NumOfContextType_ROW_HASH_SIZE;
}

NumOfContextType *NumOfContextType_get(COP *cop) {
  NumOfContextType *p;
  unsigned int i = NumOfContextType_hash(cop);

  for (p = NumOfContextType_TABLE[i]; p != NULL; p = p->next) {
    if (p->cop == cop) {
      return p;
    }
  }

  if ((p = (NumOfContextType *)malloc(sizeof(NumOfContextType))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  p->next = NumOfContextType_TABLE[i];
  NumOfContextType_TABLE[i] = p;

  p->left = 0;
  p->right = 0;
  p->cop = cop;
  // p->inside_left_cop=NULL;
  // p->inside_right_cop=NULL;
  return p;
}

NumOfContextType *NumOfContextType_refer(COP *cop) {
  NumOfContextType *p;
  unsigned int i = NumOfContextType_hash(cop);

  for (p = NumOfContextType_TABLE[i]; p != NULL; p = p->next) {
    if (p->cop == cop) {
      return p;
    }
  }

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
//// Hash for Bigram
////////////////////////////////////////////////////////////////////////////////

static unsigned int Bigram_ROW_HASH_SIZE;
static unsigned int Bigram_COL_HASH_SIZE;

void Bigram_init(unsigned int row, unsigned int col) {
  unsigned int i, j;

  Bigram_ROW_HASH_SIZE = row;
  Bigram_COL_HASH_SIZE = col;
  if ((Bigram_TABLE = (Bigram ***)malloc(
           row * sizeof(Bigram **) + row * col * sizeof(Bigram *))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < row; i++) {
    Bigram_TABLE[i] = (Bigram **)(Bigram_TABLE + row) + i * col;
    for (j = 0; j < col; j++) {
      Bigram_TABLE[i][j] = NULL;
    }
  }
}

void Bigram_destroy() {
  Bigram *p;
  unsigned int i, j;
  for (i = 0; i < Bigram_ROW_HASH_SIZE; i++) {
    for (j = 0; j < Bigram_COL_HASH_SIZE; j++) {
      for (p = Bigram_TABLE[i][j]; p != NULL; p = Bigram_TABLE[i][j]) {
        Bigram_TABLE[i][j] = p->next;
        free(p);
      }
    }
  }
  free(Bigram_TABLE);
}

static Bigram **Bigram_hash(COP *front, COP *back) {
  Bigram **p;
  unsigned long int i = (unsigned long int)front;
  i += (i >> 13);
  i += (i << 13);

  p = Bigram_TABLE[i % Bigram_ROW_HASH_SIZE];

  i = (unsigned long int)back;
  i += (i >> 13);
  i += (i << 13);

  return &(p[i % Bigram_COL_HASH_SIZE]);
}

Bigram *Bigram_get(COP *front, COP *back) {
  Bigram **pp = Bigram_hash(front, back);
  Bigram *p = *pp;

  for (; p != NULL; p = p->next) {
    if (p->front == front && p->back == back) {
      return p;
    }
  }

  if ((p = (Bigram *)malloc(sizeof(Bigram))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  p->next = *pp;
  *pp = p;

  p->refer = 0;
  p->split = 0;
  p->front = front;
  p->back = back;
  return p;
}

Bigram *Bigram_refer(COP *front, COP *back) {
  Bigram **pp = Bigram_hash(front, back);
  Bigram *p = *pp;

  for (; p != NULL; p = p->next) {
    if (p->front == front && p->back == back) {
      return p;
    }
  }

  return NULL;
}


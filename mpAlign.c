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

#include "mpAlign.h"

char start_and_end_symbol[2] = { 0x0b, '\0' }; // Start symbol and end symbol
                                               // are 0x1e.
NumOfContextType *start_and_end_noct =
    NULL; // NumOfContextType for start and end

static Parameter addlog10(Parameter x, Parameter y) {
  if (x < y) {
    return y + log(1 + pow(10, x - y)) * INV_LOG_TEN;
  } else {
    return x + log(1 + pow(10, y - x)) * INV_LOG_TEN;
  }
}

static LEN strlenByDelimiter(char *s) {
  LEN len = 0;
  while (*s != inter_unknown && *s != inter_separate && *s != inter_join &&
         *s != '\0') {
    len++;
    s++;
  }
  return len;
}

static void printJoinedString(FILE *fp, char join, char *s, LEN len) {
  char tmp;
  char *delimiter = s;

  /*
  if(strcmp(s,del_ins_char)==0){
          fprintf(fp,"%s",s);
          return;
  }
  */

  while (*delimiter != inter_unknown && *delimiter != inter_join &&
         *delimiter != inter_separate && *delimiter != '\0') {
    if (*delimiter == inter_del_ins) {
      *delimiter = del_ins_char;
    }
    delimiter++;
  }

  tmp = *delimiter;
  *delimiter = '\0';
  while (--len > 0) {
    fprintf(fp, "%s%c", s, join);

    while (*s != '\0') {
      if (*s == escape_char) {
        s++;
      } else if (*s == del_ins_char) {
        *s = inter_del_ins;
      }
      s++;
    }

    *s = tmp;
    s++;
    delimiter = s;

    while (*delimiter != inter_unknown && *delimiter != inter_join &&
           *delimiter != inter_separate && *delimiter != '\0') {
      if (*delimiter == inter_del_ins) {
        *delimiter = del_ins_char;
      }
      delimiter++;
    }

    tmp = *delimiter;
    *delimiter = '\0';
  }

  fprintf(fp, "%s", s);
  while (*s != '\0') {
    if (*s == escape_char) {
      s++;
    } else if (*s == del_ins_char) {
      *s = inter_del_ins;
    }
    s++;
  }

  *s = tmp;
}

static void printPairData(FILE *fp, char *s) {
  char *tmp = s;

  while (*tmp != '\0') {
    if (*tmp == inter_unknown) {
      *tmp = unknown_char;
    } else if (*tmp == inter_join) {
      *tmp = join_char;
    } else if (*tmp == inter_separate) {
      *tmp = separate_char;
    } else if (*tmp == inter_del_ins) {
      *tmp = del_ins_char;
    }
    tmp++;
  }

  fprintf(fp, "%s", s);

  tmp = s;
  while (*tmp != '\0') {
    if (*tmp == escape_char) {
      tmp++;
    } else if (*tmp == unknown_char) {
      *tmp = inter_unknown;
    } else if (*tmp == join_char) {
      *tmp = inter_join;
    } else if (*tmp == separate_char) {
      *tmp = inter_separate;
    } else if (*tmp == del_ins_char) {
      *tmp = inter_del_ins;
    }
    tmp++;
  }
}

void readInputFile(TOTAL_INFO *info) {
  FILE *fp;
  size_t size = 256, byte_num = 0;
  int c, i = 1, state = 0;
  char *s;
  char num_of_separate_char = 0, escape_flag = 0;
  LEN x_size = 0, y_size = 0;
  PAIR_DATA top, *pair_data;

  // temporary substitution.
  info->pair_data = &top;
  pair_data = &top;

  fprintf(stderr, "Reading the input file: %s\n", input_file);

  if ((fp = fopen(input_file, "r")) == NULL) {
    fprintf(stderr, "The input FILE open error: %s\n", input_file);
    exit(EXIT_FAILURE);
  }

  if ((s = (char *)malloc(size)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    if (byte_num >= size) {
      size += size;
      if ((s = (char *)realloc((void *)s, size)) == NULL) {
        fprintf(stderr,
                "Don't get memory in realloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }
    }
    c = getc(fp);
    byte_num++;
    switch (c) {
    case '\t':
      if (byte_num == 1 || s[byte_num - 2] == '\0' ||
          s[byte_num - 2] == inter_unknown || s[byte_num - 2] == inter_join) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        exit(EXIT_FAILURE);
      } else if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "ERROR:A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';
      if (s[byte_num - 2] != inter_separate) {
        x_size++;
      }

      state++;
      break;
    case '\n':
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "ERROR:A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      } else if (byte_num > 1 && s[byte_num - 2] == '\r') {
        s[byte_num - 2] = '\0';
        byte_num--;
      } else {
        s[byte_num - 1] = '\0';
      }

      if (byte_num > 1 && state && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_unknown && s[byte_num - 2] != inter_join &&
          num_of_separate_char == 0) {

        if (s[byte_num - 2] != inter_separate) {
          y_size++;
        }

        if ((pair_data->next = (PAIR_DATA *)malloc(
                 sizeof(PAIR_DATA) + (x_size + y_size) * sizeof(char *) +
                 byte_num * sizeof(char))) == NULL) {
          fprintf(stderr,
                  "Don't get memory in malloc.\nYou must need more memory.\n");
          exit(EXIT_FAILURE);
        }

        pair_data = pair_data->next;
        pair_data->next = NULL;

        pair_data->x_part_strs = (char **)(pair_data + 1);
        pair_data->y_part_strs = pair_data->x_part_strs + x_size;
        pair_data->x_part_strs[0] = (char *)(pair_data->y_part_strs + y_size);

        memcpy((void *)pair_data->x_part_strs[0], (void *)s, byte_num);
        for (c = 1; c < x_size; c++) {
          pair_data->x_part_strs[c] =
              pair_data->x_part_strs[c - 1] +
              strlenByDelimiter(pair_data->x_part_strs[c - 1]) + 1;
        }

        pair_data->y_part_strs[0] =
            pair_data->x_part_strs[c - 1] +
            strlenByDelimiter(pair_data->x_part_strs[c - 1]) + 1;
        if (*(pair_data->y_part_strs[0] - 1) == inter_separate) {
          pair_data->y_part_strs[0] += 1;
        }

        for (c = 1; c < y_size; c++) {
          pair_data->y_part_strs[c] =
              pair_data->y_part_strs[c - 1] +
              strlenByDelimiter(pair_data->y_part_strs[c - 1]) + 1;
        }

        pair_data->x_size = x_size;
        pair_data->y_size = y_size;
        if (maxX < x_size) {
          maxX = x_size;
        }
        if (maxY < y_size) {
          maxY = y_size;
        }
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          exit(EXIT_FAILURE);
        }
      }

      x_size = 0;
      y_size = 0;
      state = 0;
      byte_num = 0;
      i++;
      break;
    case EOF:
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "ERROR:A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';
      if (byte_num > 1 && state && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_unknown && s[byte_num - 2] != inter_join &&
          num_of_separate_char == 0) {

        if (s[byte_num - 2] != inter_separate) {
          y_size++;
        }

        if ((pair_data->next = (PAIR_DATA *)malloc(
                 sizeof(PAIR_DATA) + (x_size + y_size) * sizeof(char *) +
                 byte_num * sizeof(char))) == NULL) {
          fprintf(stderr,
                  "Don't get memory in malloc.\nYou must need more memory.\n");
          exit(EXIT_FAILURE);
        }

        pair_data = pair_data->next;
        pair_data->next = NULL;

        pair_data->x_part_strs = (char **)(pair_data + 1);
        pair_data->y_part_strs = pair_data->x_part_strs + x_size;
        pair_data->x_part_strs[0] = (char *)(pair_data->y_part_strs + y_size);

        memcpy((void *)pair_data->x_part_strs[0], (void *)s, byte_num);
        for (c = 1; c < x_size; c++) {
          pair_data->x_part_strs[c] =
              pair_data->x_part_strs[c - 1] +
              strlenByDelimiter(pair_data->x_part_strs[c - 1]) + 1;
        }

        pair_data->y_part_strs[0] =
            pair_data->x_part_strs[c - 1] +
            strlenByDelimiter(pair_data->x_part_strs[c - 1]) + 1;

        if (*(pair_data->y_part_strs[0] - 1) == inter_separate) {
          pair_data->y_part_strs[0] += 1;
        }

        for (c = 1; c < y_size; c++) {
          pair_data->y_part_strs[c] =
              pair_data->y_part_strs[c - 1] +
              strlenByDelimiter(pair_data->y_part_strs[c - 1]) + 1;
        }

        pair_data->x_size = x_size;
        pair_data->y_size = y_size;
        if (maxX < x_size) {
          maxX = x_size;
        }
        if (maxY < y_size) {
          maxY = y_size;
        }
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          exit(EXIT_FAILURE);
        }
      }
      goto END_INPUT;
    default:
      if (escape_flag == 0 && unknown_char == c) {
        if (byte_num == 1 || s[byte_num - 2] == '\0' ||
            s[byte_num - 2] == inter_unknown ||
            s[byte_num - 2] == inter_separate ||
            s[byte_num - 2] == inter_join) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Ahead or consecutive special char.\n");
          exit(EXIT_FAILURE);
        }
        s[byte_num - 1] = inter_unknown;
        if (state) {
          y_size++;
        } else {
          x_size++;
        }
      } else if (escape_flag == 0 && separate_char == c) {
        if (s[byte_num - 2] == inter_unknown ||
            s[byte_num - 2] == inter_separate ||
            s[byte_num - 2] == inter_join) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Ahead or consecutive special char.\n");
          exit(EXIT_FAILURE);
        }

        if (state) {
          num_of_separate_char--;
          if (s[byte_num - 2] == '\0') {
            byte_num--;
          } else {
            s[byte_num - 1] = inter_separate;
            y_size++;
          }
        } else {
          num_of_separate_char++;
          if (byte_num == 1) {
            byte_num--;
          } else {
            s[byte_num - 1] = inter_separate;
            x_size++;
          }
        }
      } else if (escape_flag == 0 && join_char == c) {
        if (byte_num == 1 || s[byte_num - 2] == '\0' ||
            s[byte_num - 2] == inter_unknown ||
            s[byte_num - 2] == inter_separate ||
            s[byte_num - 2] == inter_join) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Ahead or consecutive special char.\n");
          exit(EXIT_FAILURE);
        } else if (s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Unknown char, separate char, \"\\t\" or "
                          "\"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }

        s[byte_num - 1] = inter_join;
        if (state) {
          y_size++;
        } else {
          x_size++;
        }
      } else if (escape_flag == 0 && del_ins_char == c) {
        if (byte_num > 1 && s[byte_num - 2] != '\0' &&
            s[byte_num - 2] != inter_unknown &&
            s[byte_num - 2] != inter_separate) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Unknown char, separate char, \"\\t\" or "
                          "\"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }
        s[byte_num - 1] = inter_del_ins;
      } else if (escape_char == c) {
        escape_flag ^= 1;

        s[byte_num - 1] = c;
      } else {
        if (byte_num > 1 && s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:Unknown char, separate char, \"\\t\" or "
                          "\"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        } else if (unknown_char == c || separate_char == c || join_char == c ||
                   del_ins_char == c || escape_char == c) {
          escape_flag = 0;
        } else if (escape_flag == 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "ERROR:A character behind an escape char \"%c\" is "
                          "an unknown char, a separate char, a join char, a "
                          "deletion char or an escape char.\n",
                  escape_char);
          exit(EXIT_FAILURE);
        }

        s[byte_num - 1] = c;
      }
    }
  }

END_INPUT:
  fclose(fp);
  free(s);
  info->pair_data = top.next;

  if (restrictX == 0) {
    fprintf(stderr, "Set restrictX = %d\n", maxX);
    restrictX = (short)maxX;
  }

  if (restrictY == 0) {
    fprintf(stderr, "Set restrictY = %d\n", maxY);
    restrictY = (short)maxY;
  }

  if ((info->delimiters_x = (char *)malloc(maxX + 1)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  if ((info->delimiters_y = (char *)malloc(maxY + 1)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }
}

void readAlignFromFile() {
  FILE *fp;
  size_t size = 256, byte_num = 0;
  int c, i = 1, state = 0;
  LEN x_len = 0, y_len = 0;
  char *s, *m, *e;
  char escape_flag = 0;
  COP *p;

  fprintf(stderr, "Reading the align model file: %s\n", input_align_model);

  if ((fp = fopen(input_align_model, "r")) == NULL) {
    fprintf(stderr, "The align model FILE open error: %s\n", input_align_model);
    exit(EXIT_FAILURE);
  }

  if ((s = (char *)malloc(size)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    if (byte_num > size) {
      size += size;
      if ((s = (char *)realloc((void *)s, size)) == NULL) {
        fprintf(stderr,
                "Don't get memory in realloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }
    }
    c = getc(fp);
    byte_num++;
    switch (c) {
    case '\t':
      if (byte_num == 1 || s[byte_num - 2] == '\0' ||
          s[byte_num - 2] == inter_join) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                input_align_model, i);
        exit(EXIT_FAILURE);
      } else if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "A character behind an escape char \"%c\" is only an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';

      if (state == 1) {
        y_len++;
        e = (char *)&(s[byte_num - 1]) + 1;
      } else if (state == 0) {
        x_len++;
        m = (char *)&(s[byte_num - 1]) + 1;
      }

      state++;
      break;
    case '\n':
      i++;
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      } else if (byte_num > 1 && s[byte_num - 2] == '\r') {
        s[byte_num - 2] = '\0';
        byte_num--;
      } else {
        s[byte_num - 1] = '\0';
      }

      if (byte_num > 1 && state == 2 && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_join) {
        /*
        printJoinedString(stderr, join_char, s,  x_len);
        fprintf(stderr,"\t");
        printJoinedString(stderr, join_char, m,  y_len);
        fprintf(stderr,"\t%s\n",e);
        */

        p = COP_get_with_memory(s, x_len, m, y_len);

        p->para = (Parameter)atof(e);
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          exit(EXIT_FAILURE);
        }
      }
      x_len = 0;
      y_len = 0;
      state = 0;
      byte_num = 0;
      break;
    case EOF:
      i++;
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';
      if (byte_num > 1 && state == 2 && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_join) {
        p = COP_get_with_memory(s, x_len, m, y_len);

        p->para = (Parameter)atof(e);
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          exit(EXIT_FAILURE);
        }
      }
      goto END_readAlignFromFile;
    default:
      if (escape_flag == 0 && join_char == c) {
        if (byte_num == 1 || s[byte_num - 2] == inter_join) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          fprintf(stderr, "Ahead or consecutive join char.\n");
          exit(EXIT_FAILURE);
        } else if (s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          fprintf(stderr, "\"\\t\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }

        s[byte_num - 1] = inter_join;
        if (state == 1) {
          y_len++;
        } else if (state == 0) {
          x_len++;
        } else {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          exit(EXIT_FAILURE);
        }
      } else if (escape_flag == 0 &&
                 (unknown_char == c || separate_char == c)) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                input_align_model, i);
        fprintf(stderr, "An appearance of unknown char and separate char is "
                        "not allowed in align model file.\n");
        exit(EXIT_FAILURE);
      } else if (escape_flag == 0 && del_ins_char == c) {
        if (byte_num > 1 && s[byte_num - 2] != '\0') {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          fprintf(stderr, "\"\\t\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }
        s[byte_num - 1] = inter_del_ins;
      } else if (escape_char == c) {
        escape_flag ^= 1;

        s[byte_num - 1] = c;
      } else {
        if (byte_num > 1 && s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  input_align_model, i);
          fprintf(stderr, "\"\\t\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        } else if (unknown_char == c || separate_char == c || join_char == c ||
                   del_ins_char == c || escape_char == c) {
          escape_flag = 0;
        } else if (escape_flag == 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i);
          fprintf(stderr, "A character behind an escape char \"%c\" is only an "
                          "unknown char, a separate char, a join char, a "
                          "deletion char or an escape char.\n",
                  escape_char);
          exit(EXIT_FAILURE);
        }

        s[byte_num - 1] = c;
      }
    }
  }

END_readAlignFromFile:
  fclose(fp);
  free(s);

  if (previous_knowledge_file != NULL) {
    readPreviousKnowledge();
  }
}

void readPreviousKnowledge() {
  FILE *fp;
  size_t size = 256, byte_num = 0;
  int c, i = 1, state = 0;
  LEN x_len = 0, y_len = 0;
  char *s, *m, *e;
  char escape_flag = 0;
  COP *p;

  fprintf(stderr, "Reading the previous knowledge: %s\n",
          previous_knowledge_file);

  if ((fp = fopen(previous_knowledge_file, "r")) == NULL) {
    fprintf(stderr, "The previous knowledge FILE open error: %s\n",
            previous_knowledge_file);
    exit(EXIT_FAILURE);
  }

  if ((s = (char *)malloc(size)) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    if (byte_num > size) {
      size += size;
      if ((s = (char *)realloc((void *)s, size)) == NULL) {
        fprintf(stderr,
                "Don't get memory in realloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }
    }
    c = getc(fp);
    byte_num++;
    switch (c) {
    case '\t':
      if (byte_num == 1 || s[byte_num - 2] == '\0' ||
          s[byte_num - 2] == inter_join) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                previous_knowledge_file, i);
        exit(EXIT_FAILURE);
      } else if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i);
        fprintf(stderr, "A character behind an escape char \"%c\" is only  an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      } else if (state == 2) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                previous_knowledge_file, i);
        fprintf(stderr, "Previous knowledge file must have three elements "
                        "every other line. (x, y and previous knowledge "
                        "type).\n");
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';

      if (state == 1) {
        y_len++;
        e = (char *)&(s[byte_num - 1]) + 1;
      } else if (state == 0) {
        x_len++;
        m = (char *)&(s[byte_num - 1]) + 1;
      }

      state++;
      break;
    case '\n':
      i++;
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i - 1);
        fprintf(stderr, "ERROR:A character behind an escape char \"%c\" is an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      } else if (byte_num > 1 && s[byte_num - 2] == '\r') {
        s[byte_num - 2] = '\0';
        byte_num--;
      } else {
        s[byte_num - 1] = '\0';
      }

      if (byte_num > 1 && state == 2 && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_join) {
        if (input_align_model == NULL) {
          // Use COP only appearing in input file
          if ((p = COP_refer(s, x_len, m, y_len)) != NULL) {
            if (!(strcmp(e, "0")) && p->refer_flag != 2) {
              p->refer_flag = -1;
            } else if (!(strcmp(e, "1")) && p->refer_flag != -1) {
              if (p->refer_flag != 2) {
                p->refer_flag = 2;
                COP_NUM--;
              }
            } else {
              fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                      previous_knowledge_file, i - 1);
              fprintf(stderr, "Inappropriate previous knowledge Type or "
                              "duprication of previous knowledge.\n");
              exit(EXIT_FAILURE);
            }
          }
        } else {
          // also get memory for string
          p = COP_get_with_memory(s, x_len, m, y_len);
          if (!(strcmp(e, "0")) && p->refer_flag != 2) {
            p->refer_flag = -1;
          } else if (!(strcmp(e, "1")) && p->refer_flag != -1) {
            p->refer_flag = 2;
          } else {
            fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                    previous_knowledge_file, i - 1);
            fprintf(stderr, "Inappropriate previous knowledge Type or "
                            "duprication of previous knowledge.\n");
            exit(EXIT_FAILURE);
          }
        }
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i - 1);
          exit(EXIT_FAILURE);
        }
      }
      x_len = 0;
      y_len = 0;
      state = 0;
      byte_num = 0;
      break;
    case EOF:
      i++;
      if (escape_flag == 1) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                i - 1);
        fprintf(stderr, "A character behind an escape char \"%c\" is only an "
                        "unknown char, a separate char, a join char, a "
                        "deletion char or an escape char.\n",
                escape_char);
        exit(EXIT_FAILURE);
      }

      s[byte_num - 1] = '\0';
      if (byte_num > 1 && state == 2 && s[byte_num - 2] != '\0' &&
          s[byte_num - 2] != inter_join) {
        if (input_align_model == NULL) {
          if ((p = COP_refer(s, x_len, m, y_len)) != NULL) {
            if (!(strcmp(e, "0")) && p->refer_flag != 2) {
              p->refer_flag = -1;
            } else if (!(strcmp(e, "1")) && p->refer_flag != -1) {
              if (p->refer_flag != 2) {
                p->refer_flag = 2;
                COP_NUM--;
              }
            } else {
              fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                      previous_knowledge_file, i - 1);
              fprintf(stderr, "Inappropriate previous knowledge Type or "
                              "duprication of previous knowledge.\n");
              exit(EXIT_FAILURE);
            }
          }
        } else {
          // also get memory for string
          p = COP_get_with_memory(s, x_len, m, y_len);
          if (!(strcmp(e, "0")) && p->refer_flag != 2) {
            p->refer_flag = -1;
          } else if (!(strcmp(e, "1")) && p->refer_flag != -1) {
            p->refer_flag = 2;
          } else {
            fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                    previous_knowledge_file, i - 1);
            fprintf(stderr, "Inappropriate previous knowledge Type or "
                            "duprication of previous knowledge.\n");
            exit(EXIT_FAILURE);
          }
        }
      } else {
        if (byte_num > 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          exit(EXIT_FAILURE);
        }
      }
      goto END_PreviousKnowledge;
    default:
      if (escape_flag == 0 && join_char == c) {
        if (byte_num == 1 || s[byte_num - 2] == inter_join) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          fprintf(stderr, "Ahead or consecutive join char.\n");
          exit(EXIT_FAILURE);
        } else if (s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          fprintf(stderr, "\"\\t\" or \"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }
        s[byte_num - 1] = inter_join;
        if (state == 1) {
          y_len++;
        } else if (state == 0) {
          x_len++;
        } else {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          exit(EXIT_FAILURE);
        }
      } else if (escape_flag == 0 &&
                 (unknown_char == c || separate_char == c)) {
        fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                previous_knowledge_file, i);
        fprintf(stderr, "An appearance of unknown char and separate char is "
                        "not allowed in previous knowledge file.\n");
        exit(EXIT_FAILURE);
      } else if (escape_flag == 0 && del_ins_char == c) {
        if (byte_num > 1 && s[byte_num - 2] != '\0') {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          fprintf(stderr, "\"\\t\" or \"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        }
        s[byte_num - 1] = inter_del_ins;
      } else if (escape_char == c) {
        escape_flag ^= 1;

        s[byte_num - 1] = c;
      } else {
        if (byte_num > 1 && s[byte_num - 2] == inter_del_ins) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n",
                  previous_knowledge_file, i);
          fprintf(stderr, "\"\\t\" or \"\\n\" around deletion char \"%c\".\n",
                  del_ins_char);
          exit(EXIT_FAILURE);
        } else if (unknown_char == c || separate_char == c || join_char == c ||
                   del_ins_char == c || escape_char == c) {
          escape_flag = 0;
        } else if (escape_flag == 1) {
          fprintf(stderr, "ERROR:The format is wrong:%s:%d line\n", input_file,
                  i - 1);
          fprintf(stderr, "A character behind an escape char \"%c\" is an "
                          "unknown char, a separate char, a join char, a "
                          "deletion char or an escape char.\n",
                  escape_char);
          exit(EXIT_FAILURE);
        }

        s[byte_num - 1] = c;
      }
    }
  }

END_PreviousKnowledge:
  fclose(fp);
  free(s);
}

static void forward(TOTAL_INFO *info, PAIR_DATA *pair_data, double **alpha,
                    LEN x_size, LEN y_size) {
  LEN xl, yl, i, j;
  double tmp;
  char delimiter_x, delimiter_y;
  COP *p;

  for (i = 0; i <= x_size; i++) {
    for (j = 0; j <= y_size; j++) {
      alpha[i][j] = LOW_LOG_VALUE;
    }
  }

  // fprintf(stderr,"x: %s\n",pair_data->x_part_strs[0]);
  // fprintf(stderr,"y: %s\n",pair_data->y_part_strs[0]);

  alpha[0][0] = 0.0;
  for (xl = 0; xl <= x_size; xl++) {
    for (yl = 0; yl <= y_size; yl++) {
      if (alpha[xl][yl] <= LOW_LOG_VALUE) {
        continue;
      }
      if (info->delimiters_x[xl] == inter_separate &&
          info->delimiters_y[yl] == inter_separate) {
        info->delimiters_x[xl] = tmp_separate_char;
        info->delimiters_y[yl] = tmp_separate_char;
      }

      if (del && info->delimiters_x[xl] != inter_separate) {

        for (i = 1; (i <= restrictX) && (xl + i <= x_size); i++) {

          if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_x = info->delimiters_x[xl + i]) == inter_join) {
            continue;
          }

          p = COP_refer(pair_data->x_part_strs[xl], i, tmp_del_ins_char, 1);
          if (p == NULL) {
            alpha[xl + i][yl] = addlog10(alpha[xl + i][yl], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                tmp = alpha[xl][yl] + (i + del_penalty) * p->para;
              } else {
                tmp = alpha[xl][yl] + del_penalty * p->para;
              }
            } else {
              tmp = alpha[xl][yl] + p->para;
            }
            alpha[xl + i][yl] = addlog10(alpha[xl + i][yl], tmp);
          }

          if (delimiter_x == inter_separate) {
            break;
          }
        }
      }

      if (ins && info->delimiters_y[yl] != inter_separate) {
        for (j = 1; (j <= restrictY) && (yl + j <= y_size); j++) {

          if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join) {
            continue;
          }

          p = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[yl], j);
          if (p == NULL) {
            alpha[xl][yl + j] = addlog10(alpha[xl][yl + j], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                tmp = alpha[xl][yl] + (j + ins_penalty) * p->para;
              } else {
                tmp = alpha[xl][yl] + (ins_penalty) * p->para;
              }
            } else {
              tmp = alpha[xl][yl] + p->para;
            }
            alpha[xl][yl + j] = addlog10(alpha[xl][yl + j], tmp);
          }

          if (delimiter_y == inter_separate) {
            break;
          }
        }
      }

      if (info->delimiters_y[yl] == inter_separate) {
        break;
      } else if (info->delimiters_x[xl] == inter_separate) {
        continue;
      }

      for (i = 1; (i <= restrictX) && (xl + i <= x_size); i++) {

        delimiter_x = info->delimiters_x[xl + i];
        if (i > 1 && *(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
          break;
        } else if (delimiter_x == inter_join) {
          continue;
        }

        for (j = 1; (j <= restrictY) && (yl + j <= y_size); j++) {
          if (j > 1 && *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins &&
                     *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join) {
            continue;
          }

          p = COP_refer(pair_data->x_part_strs[xl], i,
                        pair_data->y_part_strs[yl], j);

          if (p == NULL) {
            alpha[xl + i][yl + j] = addlog10(alpha[xl + i][yl + j], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                  tmp = alpha[xl][yl] + (i + del_penalty) * p->para;
                } else {
                  tmp = alpha[xl][yl] + del_penalty * p->para;
                }
              } else if (*(pair_data->x_part_strs[xl + i - 1]) ==
                         inter_del_ins) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                  tmp = alpha[xl][yl] + (j + ins_penalty) * p->para;
                } else {
                  tmp = alpha[xl][yl] + (ins_penalty) * p->para;
                }
              } else {
                if (kind_of_cityblock == 0) {
                  tmp = alpha[xl][yl] + (i + j) * p->para;
                } else if (kind_of_cityblock == 1) {
                  tmp = alpha[xl][yl] + (j) * p->para;
                } else {
                  tmp = alpha[xl][yl] + (i) * p->para;
                }
              }
            } else {
              tmp = alpha[xl][yl] + p->para;
            }
            alpha[xl + i][yl + j] = addlog10(alpha[xl + i][yl + j], tmp);
          }

          if (delimiter_x == inter_separate && delimiter_y == inter_separate) {
            goto END_TRANS_FOR;
          } else if (delimiter_y == inter_separate ||
                     *(pair_data->y_part_strs[yl]) == inter_del_ins) {
            break;
          }
        }
        if (*(pair_data->x_part_strs[xl]) == inter_del_ins) {
          break;
        }
      }
    END_TRANS_FOR:
      ;
    }
  }
}

static void backward(TOTAL_INFO *info, PAIR_DATA *pair_data, double **beta,
                     LEN x_size, LEN y_size) {
  LEN xl, yl, i, j;
  double tmp;
  char delimiter_x, delimiter_y;
  COP *p;

  for (i = 0; i <= x_size; i++) {
    for (j = 0; j <= y_size; j++) {
      beta[i][j] = LOW_LOG_VALUE;
    }
  }

  beta[x_size][y_size] = 0.0;
  for (xl = x_size; xl >= 0; xl--) {
    for (yl = y_size; yl >= 0; yl--) {
      if (beta[xl][yl] <= LOW_LOG_VALUE) {
        continue;
      }
      if (info->delimiters_x[xl] == tmp_separate_char &&
          info->delimiters_y[yl] == tmp_separate_char) {
        info->delimiters_x[xl] = inter_separate;
        info->delimiters_y[yl] = inter_separate;
      }

      if (del && info->delimiters_x[xl] != tmp_separate_char) {
        for (i = 1; (i <= restrictX) && (xl - i >= 0); i++) {

          if (*(pair_data->x_part_strs[xl - i]) == inter_del_ins) {
            break;
          } else if ((delimiter_x = info->delimiters_x[xl - i]) == inter_join) {
            continue;
          }

          p = COP_refer(pair_data->x_part_strs[xl - i], i, tmp_del_ins_char, 1);
          if (p == NULL) {
            beta[xl - i][yl] = addlog10(beta[xl - i][yl], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                tmp = beta[xl][yl] + (i + del_penalty) * p->para;
              } else {
                tmp = beta[xl][yl] + del_penalty * p->para;
              }
            } else {
              tmp = beta[xl][yl] + p->para;
            }
            beta[xl - i][yl] = addlog10(beta[xl - i][yl], tmp);
          }

          if (delimiter_x == tmp_separate_char) {
            break;
          }
        }
      }

      if (ins && info->delimiters_y[yl] != tmp_separate_char) {
        for (j = 1; (j <= restrictY) && (yl - j >= 0); j++) {

          if (*(pair_data->y_part_strs[yl - j]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl - j]) == inter_join) {
            continue;
          }

          p = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[yl - j], j);
          if (p == NULL) {
            beta[xl][yl - j] = addlog10(beta[xl][yl - j], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                tmp = beta[xl][yl] + (j + ins_penalty) * p->para;
              } else {
                tmp = beta[xl][yl] + (ins_penalty) * p->para;
              }
            } else {
              tmp = beta[xl][yl] + p->para;
            }
            beta[xl][yl - j] = addlog10(beta[xl][yl - j], tmp);
          }

          if (delimiter_y == tmp_separate_char) {
            break;
          }
        }
      }

      if (info->delimiters_y[yl] == tmp_separate_char) {
        break;
      } else if (info->delimiters_x[xl] == tmp_separate_char) {
        continue;
      }

      for (i = 1; (i <= restrictX) && (xl - i >= 0); i++) {

        delimiter_x = info->delimiters_x[xl - i];
        if (i > 1 && *(pair_data->x_part_strs[xl - i]) == inter_del_ins) {
          break;
        } else if (delimiter_x == inter_join) {
          continue;
        }

        for (j = 1; (j <= restrictY) && (yl - j >= 0); j++) {

          if (j > 1 && *(pair_data->y_part_strs[yl - j]) == inter_del_ins) {
            break;
          } else if (*(pair_data->x_part_strs[xl - i]) == inter_del_ins &&
                     *(pair_data->y_part_strs[yl - j]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl - j]) == inter_join) {
            continue;
          }

          p = COP_refer(pair_data->x_part_strs[xl - i], i,
                        pair_data->y_part_strs[yl - j], j);

          if (p == NULL) {
            beta[xl - i][yl - j] = addlog10(beta[xl - i][yl - j], 0);
          } else if (p->refer_flag != -1) {
            if (training_type > 1) {
              if (*(pair_data->y_part_strs[yl - j]) == inter_del_ins) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                  tmp = beta[xl][yl] + (i + del_penalty) * p->para;
                } else {
                  tmp = beta[xl][yl] + del_penalty * p->para;
                }
              } else if (*(pair_data->x_part_strs[xl - i]) == inter_del_ins) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                  tmp = beta[xl][yl] + (j + ins_penalty) * p->para;
                } else {
                  tmp = beta[xl][yl] + (ins_penalty) * p->para;
                }
              } else {
                if (kind_of_cityblock == 0) {
                  tmp = beta[xl][yl] + (i + j) * p->para;
                } else if (kind_of_cityblock == 1) {
                  tmp = beta[xl][yl] + (j) * p->para;
                } else {
                  tmp = beta[xl][yl] + (i) * p->para;
                }
              }
            } else {
              tmp = beta[xl][yl] + p->para;
            }
            beta[xl - i][yl - j] = addlog10(beta[xl - i][yl - j], tmp);
          }

          if (delimiter_x == tmp_separate_char &&
              delimiter_y == tmp_separate_char) {
            goto END_TRANS_BACK;
          } else if (delimiter_y == tmp_separate_char ||
                     *(pair_data->y_part_strs[yl - 1]) == inter_del_ins) {
            break;
          }
        }

        if (*(pair_data->x_part_strs[xl - 1]) == inter_del_ins) {
          break;
        }
      }
    END_TRANS_BACK:
      ;
    }
  }
}

static void expectation(TOTAL_INFO *info, PAIR_DATA *pair_data, double **alpha,
                        double **beta, LEN x_size, LEN y_size) {
  LEN xl, yl, i, j;
  COP *p;
  Parameter update, alpha_xy;
  char *s;
  char delimiter_x, delimiter_y;

  s = pair_data->x_part_strs[0];
  info->delimiters_x[0] = inter_separate;
  for (xl = 1; xl <= x_size; xl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_x[xl] = *s;
    s++;
  }

  s = pair_data->y_part_strs[0];
  info->delimiters_y[0] = inter_separate;
  for (yl = 1; yl <= y_size; yl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_y[yl] = *s;
    s++;
  }

  forward(info, pair_data, alpha, x_size, y_size);
  backward(info, pair_data, beta, x_size, y_size);

  alpha_xy = alpha[x_size][y_size];
  for (xl = 0; xl <= x_size; xl++) {
    for (yl = 0; yl <= y_size; yl++) {

      if (alpha[xl][yl] > LOW_LOG_VALUE) {

        if (info->delimiters_x[xl] == inter_separate &&
            info->delimiters_y[yl] == inter_separate) {
          info->delimiters_x[xl] = tmp_separate_char;
          info->delimiters_y[yl] = tmp_separate_char;
        }

        if (del && info->delimiters_x[xl] != inter_separate) {
          for (i = 1; (i <= restrictX) && (xl + i <= x_size); i++) {

            if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
              break;
            } else if (beta[xl + i][yl] <= LOW_LOG_VALUE ||
                       (delimiter_x = info->delimiters_x[xl + i]) ==
                           inter_join) {
              continue;
            }

            p = COP_get(pair_data->x_part_strs[xl], i, tmp_del_ins_char, 1);

            if (p->refer_flag != 2 && p->refer_flag != -1) {
              if (training_type > 1) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                  update = alpha[xl][yl] + beta[xl + i][yl] +
                           (i + del_penalty) * p->para - alpha_xy;
                } else {
                  update = alpha[xl][yl] + beta[xl + i][yl] +
                           del_penalty * p->para - alpha_xy;
                }
              } else {
                update = alpha[xl][yl] + beta[xl + i][yl] + p->para - alpha_xy;
              }

              info->total_update = addlog10(info->total_update, update);
              p->update_para = addlog10(p->update_para, update);
            }

            if (delimiter_x == inter_separate) {
              break;
            }
          }
        }

        if (ins && info->delimiters_y[yl] != inter_separate) {
          for (j = 1; (j <= restrictY) && (yl + j <= y_size); j++) {

            if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
              break;
            } else if (beta[xl][yl + j] <= LOW_LOG_VALUE ||
                       (delimiter_y = info->delimiters_y[yl + j]) ==
                           inter_join) {
              continue;
            }

            p = COP_get(tmp_del_ins_char, 1, pair_data->y_part_strs[yl], j);

            if (p->refer_flag != 2 && p->refer_flag != -1) {
              if (training_type > 1) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                  update = alpha[xl][yl] + beta[xl][yl + j] +
                           (j + ins_penalty) * p->para - alpha_xy;
                } else {
                  update = alpha[xl][yl] + beta[xl][yl + j] +
                           (ins_penalty) * p->para - alpha_xy;
                }
              } else {
                update = alpha[xl][yl] + beta[xl][yl + j] + p->para - alpha_xy;
              }

              info->total_update = addlog10(info->total_update, update);
              p->update_para = addlog10(p->update_para, update);
            }

            if (delimiter_y == inter_separate) {
              break;
            }
          }
        }

        if (info->delimiters_y[yl] == inter_separate) {
          break;
        } else if (info->delimiters_x[xl] == inter_separate) {
          continue;
        }

        for (i = 1; (i <= restrictX) && (xl + i <= x_size); i++) {

          delimiter_x = info->delimiters_x[xl + i];
          if (i > 1 && *(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
            break;
          } else if (delimiter_x == inter_join) {
            continue;
          }

          for (j = 1; (j <= restrictY) && (yl + j <= y_size); j++) {

            if (j > 1 &&
                *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
              break;
            } else if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins &&
                       *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
              break;
            } else if (beta[xl + i][yl + j] <= LOW_LOG_VALUE ||
                       (delimiter_y = info->delimiters_y[yl + j]) ==
                           inter_join) {
              if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
                break;
              }
              continue;
            }

            p = COP_get(pair_data->x_part_strs[xl], i,
                        pair_data->y_part_strs[yl], j);

            if (p->refer_flag != 2 && p->refer_flag != -1) {
              if (training_type > 1) {
                if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
                  if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (i + del_penalty) * p->para - alpha_xy;
                  } else {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             del_penalty * p->para - alpha_xy;
                  }
                } else if (*(pair_data->x_part_strs[xl + i - 1]) ==
                           inter_del_ins) {
                  if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (j + ins_penalty) * p->para - alpha_xy;
                  } else {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (ins_penalty) * p->para - alpha_xy;
                  }
                } else {
                  if (kind_of_cityblock == 0) {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (i + j) * p->para - alpha_xy;
                  } else if (kind_of_cityblock == 1) {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (j) * p->para - alpha_xy;
                  } else {
                    update = alpha[xl][yl] + beta[xl + i][yl + j] +
                             (i) * p->para - alpha_xy;
                  }
                }
              } else {
                update =
                    alpha[xl][yl] + beta[xl + i][yl + j] + p->para - alpha_xy;
              }

              info->total_update = addlog10(info->total_update, update);
              p->update_para = addlog10(p->update_para, update);
            }

            /*
            fprintf(stderr,"\nx-y:%d:%d\tx-y:%d:%d\n",xl,yl,xl+i,yl+j);
            fprintf(stderr,"x cop:");
            printJoinedString(stderr, join_char, p->x, p->x_len);
            fprintf(stderr," l:%d\ny cop:",p->x_len);
            printJoinedString(stderr, join_char, p->y, p->y_len);
            fprintf(stderr," l:%d\n",p->y_len);
            */

            if (delimiter_x == inter_separate &&
                delimiter_y == inter_separate) {
              goto END_TRANS_EXP;
            } else if (delimiter_y == inter_separate ||
                       *(pair_data->y_part_strs[yl]) == inter_del_ins) {
              break;
            }
          }

          if (*(pair_data->x_part_strs[xl]) == inter_del_ins) {
            break;
          }
        }
      END_TRANS_EXP:
        ;
      }
    }
  }
}

static void viterbiWithinDelAndInsInference(TOTAL_INFO *info,
                                            PAIR_DATA *pair_data,
                                            VITERBI_HYP_LIST ***viterbi_table) {
  char delimiter_x, delimiter_y;
  Parameter update, normalize_score;
  LEN xl, yl, i, j, k, d, z, rank_index;
  VITERBI_HYP_LIST *vhl, *nvhl;
  VITERBI_HYP vh = { -1, // backX
                     -1, // backY
                     0, // back_num_of_del_and_ins
                     0, // rank
                     0, // num_of_trans
                     0, // num_of_del
                     0, // num_of_ins
                     0, // num_of_consecutive_del
                     0, // max_num_of_consecutive_del
                     0 }; // score
  COP *cop;
  char *s;

  s = pair_data->x_part_strs[0];
  info->delimiters_x[0] = inter_separate;
  for (xl = 1; xl <= pair_data->x_size; xl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_x[xl] = *s;
    s++;
  }

  s = pair_data->y_part_strs[0];
  info->delimiters_y[0] = inter_separate;
  for (yl = 1; yl <= pair_data->y_size; yl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_y[yl] = *s;
    s++;
  }

  for (xl = 0; xl <= pair_data->x_size; xl++) {
    for (yl = 0; yl <= pair_data->y_size; yl++) {
      for (i = 0; i <= xl + yl; i++) {
        viterbi_table[xl][yl][i].rank_size = 0;
      }
    }
  }

  viterbi_table[0][0][0].rank_size = 1;
  viterbi_table[0][0][0].viterbi_hyp[0] = vh;
  for (xl = 0; xl <= pair_data->x_size; xl++) {
    for (yl = 0; yl <= pair_data->y_size; yl++) {

      if (info->delimiters_x[xl] == inter_separate &&
          info->delimiters_y[yl] == inter_separate) {
        info->delimiters_x[xl] = tmp_separate_char;
        info->delimiters_y[yl] = tmp_separate_char;
      }

      // z is last index for VITERBI_HYP_LIST in viterbi_table[xl][yl]
      z = xl + yl;

      vh.backX = xl;
      vh.backY = yl;

      if (del && info->delimiters_x[xl] != inter_separate) {
        for (i = 1; (i <= restrictX) && (xl + i <= pair_data->x_size); i++) {
          if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_x = info->delimiters_x[xl + i]) == inter_join) {
            continue;
          }

          cop = COP_refer(pair_data->x_part_strs[xl], i, tmp_del_ins_char, 1);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          }

          for (d = 0; d <= z; d++) { // d is number of characters of deletion

            vh.back_num_of_del_and_ins = d;
            vhl = &(viterbi_table[xl][yl][d]);
            for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {
              if (vhl->viterbi_hyp[rank_index].backX == xl ||
                  (0 <= vhl->viterbi_hyp[rank_index].backX &&
                   *(pair_data->x_part_strs[vhl->viterbi_hyp[rank_index]
                                                .backX]) == inter_del_ins)) {
                continue;
              } else if (noConsecutiveDelAndIns &&
                         (vhl->viterbi_hyp[rank_index].backY == yl ||
                          (0 <= vhl->viterbi_hyp[rank_index].backY &&
                           *(pair_data->y_part_strs
                                 [vhl->viterbi_hyp[rank_index].backY]) ==
                               inter_del_ins))) {
                continue;
              }

              vh.rank = rank_index;
              vh.score = vhl->viterbi_hyp[rank_index].score;
              vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;
              vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del + i;
              vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;

              nvhl =
                  &(viterbi_table[xl + i][yl][vh.num_of_del + vh.num_of_ins]);
              if ((k = nvhl->rank_size) == training_n_best) {
                if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  k--;
                  while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                    nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                    k--;
                  }
                  nvhl->viterbi_hyp[k] = vh;
                } else {
                  break;
                }
              } else {
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
                nvhl->rank_size++;
              }
            }
          }

          if (delimiter_x == inter_separate) {
            break;
          }
        }
      }

      if (ins && info->delimiters_y[yl] != inter_separate) {
        for (j = 1; (j <= restrictY) && (yl + j <= pair_data->y_size); j++) {

          if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join) {
            continue;
          }

          cop = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[yl], j);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          }

          for (d = 0; d <= z; d++) { // d is number of characters of deletion

            vh.back_num_of_del_and_ins = d;
            vhl = &(viterbi_table[xl][yl][d]);
            for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {
              if (vhl->viterbi_hyp[rank_index].backY == yl ||
                  (0 <= vhl->viterbi_hyp[rank_index].backY &&
                   *(pair_data->y_part_strs[vhl->viterbi_hyp[rank_index]
                                                .backY]) == inter_del_ins)) {
                continue;
              } else if (noConsecutiveDelAndIns &&
                         (vhl->viterbi_hyp[rank_index].backX == xl ||
                          (0 <= vhl->viterbi_hyp[rank_index].backX &&
                           *(pair_data->x_part_strs
                                 [vhl->viterbi_hyp[rank_index].backX]) ==
                               inter_del_ins))) {
                continue;
              }

              vh.rank = rank_index;
              vh.score = vhl->viterbi_hyp[rank_index].score;
              vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;
              vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
              vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins + j;

              nvhl =
                  &(viterbi_table[xl][yl + j][vh.num_of_del + vh.num_of_ins]);
              if ((k = nvhl->rank_size) == training_n_best) {
                if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  k--;
                  while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                    nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                    k--;
                  }
                  nvhl->viterbi_hyp[k] = vh;
                } else {
                  break;
                }
              } else {
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
                nvhl->rank_size++;
              }
            }
          }

          if (delimiter_y == inter_separate) {
            break;
          }
        }
      }

      if (info->delimiters_y[yl] == inter_separate) {
        break;
      } else if (info->delimiters_x[xl] == inter_separate) {
        continue;
      }

      for (i = 1; (i <= restrictX) && (xl + i <= pair_data->x_size); i++) {

        delimiter_x = info->delimiters_x[xl + i];
        if (i > 1 && *(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
          break;
        } else if (delimiter_x == inter_join) {
          continue;
        }

        for (j = 1; (j <= restrictY) && (yl + j <= pair_data->y_size); j++) {

          if (j > 1 && *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins &&
                     *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join ||
                     (noEqMap && i > 1 && i == j)) {
            continue;
          }

          cop = COP_refer(pair_data->x_part_strs[xl], i,
                          pair_data->y_part_strs[yl], j);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          }

          for (d = 0; d <= z; d++) { // d is number of characters of deletion

            vh.back_num_of_del_and_ins = d;
            vhl = &(viterbi_table[xl][yl][d]);
            for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {

              if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
                if (vhl->viterbi_hyp[rank_index].backX == xl ||
                    (0 <= vhl->viterbi_hyp[rank_index].backX &&
                     *(pair_data->x_part_strs[vhl->viterbi_hyp[rank_index]
                                                  .backX]) == inter_del_ins)) {
                  continue;
                } else if (noConsecutiveDelAndIns &&
                           (vhl->viterbi_hyp[rank_index].backY == yl ||
                            (0 <= vhl->viterbi_hyp[rank_index].backY &&
                             *(pair_data->y_part_strs
                                   [vhl->viterbi_hyp[rank_index].backY]) ==
                                 inter_del_ins))) {
                  continue;
                }

                vh.score = vhl->viterbi_hyp[rank_index].score;
                vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del + i;
                vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;
              } else if (*(pair_data->x_part_strs[xl + i - 1]) ==
                         inter_del_ins) {
                if (vhl->viterbi_hyp[rank_index].backY == yl ||
                    (0 <= vhl->viterbi_hyp[rank_index].backY &&
                     *(pair_data->y_part_strs[vhl->viterbi_hyp[rank_index]
                                                  .backY]) == inter_del_ins)) {
                  continue;
                } else if (noConsecutiveDelAndIns &&
                           (vhl->viterbi_hyp[rank_index].backX == xl ||
                            (0 <= vhl->viterbi_hyp[rank_index].backX &&
                             *(pair_data->x_part_strs
                                   [vhl->viterbi_hyp[rank_index].backX]) ==
                                 inter_del_ins))) {
                  continue;
                }

                vh.score = vhl->viterbi_hyp[rank_index].score;
                vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
                vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins + j;
              } else {
                if (kind_of_cityblock == 0) {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (i + j) * cop->para;
                } else if (kind_of_cityblock == 1) {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (j) * cop->para;
                } else {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (i) * cop->para;
                }
                vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
                vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;
              }

              vh.rank = rank_index;
              vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;

              nvhl = &(viterbi_table[xl + i][yl + j]
                                    [vh.num_of_del + vh.num_of_ins]);
              if ((k = nvhl->rank_size) == training_n_best) {
                if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  k--;
                  while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                    nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                    k--;
                  }
                  nvhl->viterbi_hyp[k] = vh;
                } else {
                  break;
                }
              } else {
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
                nvhl->rank_size++;
              }
            }
          }

          if (delimiter_x == inter_separate && delimiter_y == inter_separate) {
            goto END_TRANS_VITERBI_WITHIN_DEL_AND_INS_INFERENCE;
          } else if (delimiter_y == inter_separate ||
                     *(pair_data->y_part_strs[yl]) == inter_del_ins) {
            break;
          }
        }

        if (*(pair_data->x_part_strs[xl]) == inter_del_ins) {
          break;
        }
      }
    END_TRANS_VITERBI_WITHIN_DEL_AND_INS_INFERENCE:
      ;
    }
  }

  // make n-best
  z = pair_data->x_size + pair_data->y_size;

  normalize_score = LOW_LOG_VALUE;
  k = pair_data->x_size + pair_data->y_size;

  // hyp included in last z is ignored because all character is deletion and
  // insertion.
  viterbi_table[pair_data->x_size][pair_data->y_size][z].rank_size = 0;
  for (i = 0; i < training_n_best; i++) {

    for (d = 0; d < z; d++) {
      vhl = &(viterbi_table[pair_data->x_size][pair_data->y_size][d]);
      if (vhl->rank_size > 0) {
        // d is vhl->viterbi_hyp[0].num_of_del + vhl->viterbi_hyp[0].num_of_ins.
        if (kind_of_cityblock == 0) {
          vhl->viterbi_hyp[0].score +=
              (vhl->viterbi_hyp[0].num_of_del *
                   (1 + del_penalty) // freq of del include the penalty
               +
               vhl->viterbi_hyp[0].num_of_ins *
                   (1 + ins_penalty)) // freq of ins include the penalty
              *
              vhl->viterbi_hyp[0].score /
              (k - d); // average score except del and ins score
        } else if (kind_of_cityblock == 1) {
          vhl->viterbi_hyp[0].score +=
              (vhl->viterbi_hyp[0].num_of_del *
                   (del_penalty) // freq of del include the penalty
               +
               vhl->viterbi_hyp[0].num_of_ins *
                   (1 + ins_penalty)) // freq of ins include the penalty
              *
              vhl->viterbi_hyp[0].score /
              (k - d); // average score except del and ins score
        } else {
          vhl->viterbi_hyp[0].score +=
              (vhl->viterbi_hyp[0].num_of_del *
                   (1 + del_penalty) // freq of del include the penalty
               +
               vhl->viterbi_hyp[0].num_of_ins *
                   (ins_penalty)) // freq of ins include the penalty
              *
              vhl->viterbi_hyp[0].score /
              (k - d); // average score except del and ins score
        }
        break;
      }
    }

    d++;
    for (; d < z; d++) {
      nvhl = &(viterbi_table[pair_data->x_size][pair_data->y_size][d]);
      if (nvhl->rank_size == 0) {
        continue;
      }

      if (kind_of_cityblock == 0) {
        nvhl->viterbi_hyp[0].score +=
            (nvhl->viterbi_hyp[0].num_of_del *
                 (1 + del_penalty) // freq of del include the penalty
             +
             nvhl->viterbi_hyp[0].num_of_ins *
                 (1 + ins_penalty)) // freq of ins include the penalty
            *
            nvhl->viterbi_hyp[0].score /
            (k - d); // average score except del and ins score
      } else if (kind_of_cityblock == 1) {
        nvhl->viterbi_hyp[0].score +=
            (nvhl->viterbi_hyp[0].num_of_del *
                 (del_penalty) // freq of del include the penalty
             +
             nvhl->viterbi_hyp[0].num_of_ins *
                 (1 + ins_penalty)) // freq of ins include the penalty
            *
            nvhl->viterbi_hyp[0].score /
            (k - d); // average score except del and ins score
      } else {
        nvhl->viterbi_hyp[0].score +=
            (nvhl->viterbi_hyp[0].num_of_del *
                 (1 + del_penalty) // freq of del include the penalty
             +
             nvhl->viterbi_hyp[0].num_of_ins *
                 (ins_penalty)) // freq of ins include the penalty
            *
            nvhl->viterbi_hyp[0].score /
            (k - d); // average score except del and ins score
      }

      if (CMP_VITERBI_HYP(nvhl->viterbi_hyp[0], vhl->viterbi_hyp[0])) {
        // nvhl->viterbi_hyp[0] > vhl->viterbi_hyp[0]
        vhl = nvhl;
      }
    }

    if (vhl->rank_size == 0) {
      break;
    }

    // best hyp
    vh = vhl->viterbi_hyp[0];
    for (rank_index = 1; rank_index < vhl->rank_size; rank_index++) {
      vhl->viterbi_hyp[rank_index - 1] = vhl->viterbi_hyp[rank_index];
    }
    vhl->rank_size--;

    normalize_score = addlog10(normalize_score, vh.score);
    vhl = &(viterbi_table[pair_data->x_size][pair_data->y_size][z]);
    vhl->viterbi_hyp[i] = vh;
    vhl->rank_size++;
  }

  vhl = &(viterbi_table[pair_data->x_size][pair_data->y_size][z]);
  for (i = 0; i < vhl->rank_size; i++) {
    vh = vhl->viterbi_hyp[i];
    update = vh.score - normalize_score;
    xl = pair_data->x_size;
    yl = pair_data->y_size;
    while ((xl > 0) || (yl > 0)) {
      if (vh.backX == xl) {
        cop = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[vh.backY],
                        yl - vh.backY);

      } else if (vh.backY == yl) {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        tmp_del_ins_char, 1);

      } else {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        pair_data->y_part_strs[vh.backY], yl - vh.backY);
      }

      if (cop->refer_flag != 2) {
        info->total_update = addlog10(info->total_update, update);
        cop->update_para = addlog10(cop->update_para, update);
        // info->total_update = addlog10(info->total_update, 0);
        // cop->update_para = addlog10(cop->update_para, 0);
      }

      /*
      printJoinedString(stderr, join_char, cop->x, cop->x_len);
      fprintf(stderr, "#");
      printJoinedString(stderr, join_char, cop->y, cop->y_len);
      fprintf(stderr, "#%f\t", cop->para);
      */

      xl = vh.backX;
      yl = vh.backY;
      vh = viterbi_table[xl][yl][vh.back_num_of_del_and_ins]
               .viterbi_hyp[vh.rank];
    }
    // fprintf(stderr, "\n");
  }
}

static void viterbi(TOTAL_INFO *info, PAIR_DATA *pair_data,
                    VITERBI_HYP_LIST **viterbi_table, unsigned char nbest) {
  char delimiter_x, delimiter_y;
  LEN xl, yl, i, j, k, rank_index;
  VITERBI_HYP_LIST *vhl, *nvhl;
  VITERBI_HYP vh = { -1, // backX
                     -1, // backY
                     0, // back_num_of_del_and_ins. This is used as
                        // num_of_characters in this function.
                     0, // rank
                     0, // num_of_trans
                     0, // num_of_del
                     0, // num_of_ins
                     0, // num_of_consecutive_del
                     0, // max_num_of_consecutive_del
                     0 }; // score
  COP *cop;
  char *s;

  s = pair_data->x_part_strs[0];
  info->delimiters_x[0] = inter_separate;
  for (xl = 1; xl <= pair_data->x_size; xl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_x[xl] = *s;
    s++;
  }

  s = pair_data->y_part_strs[0];
  info->delimiters_y[0] = inter_separate;
  for (yl = 1; yl <= pair_data->y_size; yl++) {
    while (*s != inter_unknown && *s != inter_join && *s != inter_separate &&
           *s != '\0') {
      s++;
    }
    info->delimiters_y[yl] = *s;
    s++;
  }

  for (xl = 0; xl <= pair_data->x_size; xl++) {
    for (yl = 0; yl <= pair_data->y_size; yl++) {
      viterbi_table[xl][yl].rank_size = 0;
    }
  }

  viterbi_table[0][0].rank_size = 1;
  viterbi_table[0][0].viterbi_hyp[0] = vh;
  if (substringCheck == 1) {
    for (yl = 1; yl <= pair_data->y_size; yl++) {
      viterbi_table[0][yl].rank_size = 1;
      viterbi_table[0][yl].viterbi_hyp[0] = vh;
    }
  }

  for (xl = 0; xl <= pair_data->x_size; xl++) {
    for (yl = 0; yl <= pair_data->y_size; yl++) {

      if (info->delimiters_x[xl] == inter_separate &&
          info->delimiters_y[yl] == inter_separate) {
        info->delimiters_x[xl] = tmp_separate_char;
        info->delimiters_y[yl] = tmp_separate_char;
      }

      vhl = &(viterbi_table[xl][yl]);
      vh.backX = xl;
      vh.backY = yl;

      if (del && info->delimiters_x[xl] != inter_separate) {
        for (i = 1; (i <= restrictX) && (xl + i <= pair_data->x_size); i++) {

          if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_x = info->delimiters_x[xl + i]) == inter_join) {
            continue;
          }

          cop = COP_refer(pair_data->x_part_strs[xl], i, tmp_del_ins_char, 1);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          } else if (alignment_type == 102 && cop->refer_flag == 0) {
            continue;
          }

          nvhl = &(viterbi_table[xl + i][yl]);
          for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {
            if (vhl->viterbi_hyp[rank_index].backX == xl ||
                (0 <= vhl->viterbi_hyp[rank_index].backX &&
                 *(pair_data->x_part_strs[vhl->viterbi_hyp[rank_index]
                                              .backX]) == inter_del_ins)) {
              continue;
            } else if (vhl->viterbi_hyp[rank_index].backY == yl ||
                       (0 <= vhl->viterbi_hyp[rank_index].backY &&
                        *(pair_data->y_part_strs
                              [vhl->viterbi_hyp[rank_index].backY]) ==
                            inter_del_ins)) {

              if (noConsecutiveDelAndIns) {
                continue;
              } else {
                vh.num_of_consecutive_del =
                    1 + vhl->viterbi_hyp[rank_index].num_of_consecutive_del;
              }

            } else {
              vh.num_of_consecutive_del = 1;
            }

            vh.rank = rank_index;
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                vh.score = vhl->viterbi_hyp[rank_index].score +
                           (i + del_penalty) * cop->para;
              } else {
                vh.score = vhl->viterbi_hyp[rank_index].score +
                           (del_penalty) * cop->para;
              }
            } else {
              vh.score = vhl->viterbi_hyp[rank_index].score + cop->para;
            }
            vh.back_num_of_del_and_ins =
                vhl->viterbi_hyp[rank_index].back_num_of_del_and_ins + i;
            vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;
            vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del + i;
            vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;

            if (vh.num_of_consecutive_del >=
                vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del) {
              vh.max_num_of_consecutive_del = vh.num_of_consecutive_del;
            } else {
              vh.max_num_of_consecutive_del =
                  vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del;
            }

            if ((k = nvhl->rank_size) == nbest) {
              if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                k--;
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
              } else {
                break;
              }
            } else {
              while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                k--;
              }
              nvhl->viterbi_hyp[k] = vh;
              nvhl->rank_size++;
            }
          }

          if (delimiter_x == inter_separate) {
            break;
          }
        }
      }

      if (ins && info->delimiters_y[yl] != inter_separate) {
        for (j = 1; (j <= restrictY) && (yl + j <= pair_data->y_size); j++) {

          if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join) {
            continue;
          }

          cop = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[yl], j);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          } else if (alignment_type == 102 && cop->refer_flag == 0) {
            continue;
          }

          nvhl = &(viterbi_table[xl][yl + j]);
          for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {
            if (vhl->viterbi_hyp[rank_index].backY == yl ||
                (0 <= vhl->viterbi_hyp[rank_index].backY &&
                 *(pair_data->y_part_strs[vhl->viterbi_hyp[rank_index]
                                              .backY]) == inter_del_ins)) {
              continue;
            } else if (noConsecutiveDelAndIns &&
                       (vhl->viterbi_hyp[rank_index].backX == xl ||
                        (0 <= vhl->viterbi_hyp[rank_index].backX &&
                         *(pair_data->x_part_strs
                               [vhl->viterbi_hyp[rank_index].backX]) ==
                             inter_del_ins))) {
              continue;
            }

            vh.rank = rank_index;
            if (training_type > 1) {
              if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                vh.score = vhl->viterbi_hyp[rank_index].score +
                           (j + ins_penalty) * cop->para;
              } else {
                vh.score = vhl->viterbi_hyp[rank_index].score +
                           (ins_penalty) * cop->para;
              }
            } else {
              vh.score = vhl->viterbi_hyp[rank_index].score + cop->para;
            }
            vh.back_num_of_del_and_ins =
                vhl->viterbi_hyp[rank_index].back_num_of_del_and_ins + j;
            vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;
            vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
            vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins + j;
            vh.num_of_consecutive_del = 0;
            vh.max_num_of_consecutive_del =
                vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del;

            if ((k = nvhl->rank_size) == nbest) {
              if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                k--;
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
              } else {
                break;
              }
            } else {
              while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                k--;
              }
              nvhl->viterbi_hyp[k] = vh;
              nvhl->rank_size++;
            }
          }

          if (delimiter_y == inter_separate) {
            break;
          }
        }
      }

      if (info->delimiters_y[yl] == inter_separate) {
        break;
      } else if (info->delimiters_x[xl] == inter_separate) {
        continue;
      }

      for (i = 1; (i <= restrictX) && (xl + i <= pair_data->x_size); i++) {

        delimiter_x = info->delimiters_x[xl + i];
        if (i > 1 && *(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
          break;
        } else if (delimiter_x == inter_join) {
          continue;
        }

        for (j = 1; (j <= restrictY) && (yl + j <= pair_data->y_size); j++) {

          if (j > 1 && *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins &&
                     *(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
            break;
          } else if ((delimiter_y = info->delimiters_y[yl + j]) == inter_join ||
                     (noEqMap && i > 1 && i == j)) {
            continue;
          }

          cop = COP_refer(pair_data->x_part_strs[xl], i,
                          pair_data->y_part_strs[yl], j);
          if (cop == NULL || cop->refer_flag == -1) {
            continue;
          } else if (alignment_type == 102 && cop->refer_flag == 0) {
            continue;
          }

          nvhl = &(viterbi_table[xl + i][yl + j]);
          for (rank_index = 0; rank_index < vhl->rank_size; rank_index++) {

            if (*(pair_data->y_part_strs[yl + j - 1]) == inter_del_ins) {
              if (vhl->viterbi_hyp[rank_index].backX == xl ||
                  (0 <= vhl->viterbi_hyp[rank_index].backX &&
                   *(pair_data->x_part_strs[vhl->viterbi_hyp[rank_index]
                                                .backX]) == inter_del_ins)) {
                continue;
              } else if (vhl->viterbi_hyp[rank_index].backY == yl ||
                         (0 <= vhl->viterbi_hyp[rank_index].backY &&
                          *(pair_data->y_part_strs
                                [vhl->viterbi_hyp[rank_index].backY]) ==
                              inter_del_ins)) {

                if (noConsecutiveDelAndIns) {
                  continue;
                } else {
                  vh.num_of_consecutive_del =
                      1 + vhl->viterbi_hyp[rank_index].num_of_consecutive_del;
                }

              } else {
                vh.num_of_consecutive_del = 1;
              }

              if (training_type > 1) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 2) {
                  vh.score = vhl->viterbi_hyp[rank_index].score +
                             (i + del_penalty) * cop->para;
                } else {
                  vh.score = vhl->viterbi_hyp[rank_index].score +
                             (del_penalty) * cop->para;
                }
              } else {
                vh.score = vhl->viterbi_hyp[rank_index].score + cop->para;
              }
              vh.back_num_of_del_and_ins =
                  vhl->viterbi_hyp[rank_index].back_num_of_del_and_ins + i;
              vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del + i;
              vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;

              if (vh.num_of_consecutive_del >=
                  vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del) {
                vh.max_num_of_consecutive_del = vh.num_of_consecutive_del;
              } else {
                vh.max_num_of_consecutive_del =
                    vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del;
              }
            } else if (*(pair_data->x_part_strs[xl + i - 1]) == inter_del_ins) {
              if (vhl->viterbi_hyp[rank_index].backY == yl ||
                  (0 <= vhl->viterbi_hyp[rank_index].backY &&
                   *(pair_data->y_part_strs[vhl->viterbi_hyp[rank_index]
                                                .backY]) == inter_del_ins)) {
                continue;
              } else if (noConsecutiveDelAndIns &&
                         (vhl->viterbi_hyp[rank_index].backX == xl ||
                          (0 <= vhl->viterbi_hyp[rank_index].backX &&
                           *(pair_data->x_part_strs
                                 [vhl->viterbi_hyp[rank_index].backX]) ==
                               inter_del_ins))) {
                continue;
              }

              if (training_type > 1) {
                if (kind_of_cityblock == 0 || kind_of_cityblock == 1) {
                  vh.score = vhl->viterbi_hyp[rank_index].score +
                             (j + ins_penalty) * cop->para;
                } else {
                  vh.score = vhl->viterbi_hyp[rank_index].score +
                             (ins_penalty) * cop->para;
                }
              } else {
                vh.score = vhl->viterbi_hyp[rank_index].score + cop->para;
              }
              vh.back_num_of_del_and_ins =
                  vhl->viterbi_hyp[rank_index].back_num_of_del_and_ins + j;
              vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
              vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins + j;
              vh.num_of_consecutive_del = 0;
              vh.max_num_of_consecutive_del =
                  vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del;
            } else {
              if (training_type > 1) {
                if (kind_of_cityblock == 0) {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (i + j) * cop->para;
                } else if (kind_of_cityblock == 1) {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (j) * cop->para;
                } else {
                  vh.score =
                      vhl->viterbi_hyp[rank_index].score + (i) * cop->para;
                }
              } else {
                vh.score = vhl->viterbi_hyp[rank_index].score + cop->para;
              }
              vh.back_num_of_del_and_ins =
                  vhl->viterbi_hyp[rank_index].back_num_of_del_and_ins + i + j;
              vh.num_of_del = vhl->viterbi_hyp[rank_index].num_of_del;
              vh.num_of_ins = vhl->viterbi_hyp[rank_index].num_of_ins;
              vh.num_of_consecutive_del = 0;
              vh.max_num_of_consecutive_del =
                  vhl->viterbi_hyp[rank_index].max_num_of_consecutive_del;
            }

            vh.rank = rank_index;
            vh.num_of_trans = vhl->viterbi_hyp[rank_index].num_of_trans + 1;

            if ((k = nvhl->rank_size) == nbest) {
              if (CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                k--;
                while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                  nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                  k--;
                }
                nvhl->viterbi_hyp[k] = vh;
              } else {
                break;
              }
            } else {
              while (k && CMP_VITERBI_HYP(vh, nvhl->viterbi_hyp[k - 1])) {
                nvhl->viterbi_hyp[k] = nvhl->viterbi_hyp[k - 1];
                k--;
              }
              nvhl->viterbi_hyp[k] = vh;
              nvhl->rank_size++;
            }
          }

          if (delimiter_x == inter_separate && delimiter_y == inter_separate) {
            goto END_TRANS_VITERBI;
          } else if (delimiter_y == inter_separate ||
                     *(pair_data->y_part_strs[yl]) == inter_del_ins) {
            break;
          }
        }

        if (*(pair_data->x_part_strs[xl]) == inter_del_ins) {
          break;
        }
      }
    END_TRANS_VITERBI:
      ;
    }
  }
}

static void backtrackForTraining(TOTAL_INFO *info, PAIR_DATA *pair_data,
                                 VITERBI_HYP_LIST **viterbi_table) {
  LEN xl, yl, i;
  COP *cop;
  Parameter update, normalize_score = LOW_LOG_VALUE;
  VITERBI_HYP_LIST *vhl;
  VITERBI_HYP vh;

  vhl = &(viterbi_table[pair_data->x_size][pair_data->y_size]);
  for (i = 0; i < vhl->rank_size; i++) {
    normalize_score = addlog10(normalize_score, vhl->viterbi_hyp[i].score);
  }

  for (i = 0; i < vhl->rank_size; i++) {
    vh = vhl->viterbi_hyp[i];
    update = vh.score - normalize_score;
    xl = pair_data->x_size;
    yl = pair_data->y_size;
    while ((xl > 0) || (yl > 0)) {
      if (vh.backX == xl) {
        cop = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[vh.backY],
                        yl - vh.backY);

      } else if (vh.backY == yl) {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        tmp_del_ins_char, 1);

      } else {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        pair_data->y_part_strs[vh.backY], yl - vh.backY);
      }

      if (cop->refer_flag != 2) {
        info->total_update = addlog10(info->total_update, update);
        cop->update_para = addlog10(cop->update_para, update);
        // info->total_update = addlog10(info->total_update, 0);
        // cop->update_para = addlog10(cop->update_para, 0);
      }

      xl = vh.backX;
      yl = vh.backY;
      vh = viterbi_table[xl][yl].viterbi_hyp[vh.rank];
    }
  }
}

static void printAlphaBeta(double **a_or_b, LEN x_len, LEN y_len) {
  int i, j;
  for (i = 0; i <= x_len; i++) {
    for (j = 0; j <= y_len; j++) {
      fprintf(stderr, "%4f ", a_or_b[i][j]);
    }
    fprintf(stderr, "\n");
  }
}

static void maximization(TOTAL_INFO *info) {
  unsigned int i, j;
  COP *p;

  for (i = 0; i < sqrt_hash_size; i++) {
    for (j = 0; j < sqrt_hash_size; j++) {
      for (p = COP_TABLE[i][j]; p != NULL; p = p->next) {
        if (p->refer_flag != 2 && p->refer_flag != -1) {
          p->update_para -= info->total_update;
          if (p->para > p->update_para) {
            info->total_change += pow(10, p->para) - pow(10, p->update_para);
          } else {
            info->total_change += pow(10, p->update_para) - pow(10, p->para);
          }
          p->para = p->update_para;
          p->update_para = LOW_LOG_VALUE;
        }
      }
    }
  }
}

static void initialization(TOTAL_INFO *info, double **alpha, double **beta) {
  unsigned int i, j;
  COP *p;
  LEN xl, yl, x_size, y_size;
  PAIR_DATA *pair_data;

  for (pair_data = info->pair_data; pair_data != NULL;
       pair_data = pair_data->next) {
    expectation(info, pair_data, alpha, beta, pair_data->x_size,
                pair_data->y_size);
  }

  if (previous_knowledge_file != NULL) {
    readPreviousKnowledge();
  }

  for (i = 0; i < sqrt_hash_size; i++) {
    for (j = 0; j < sqrt_hash_size; j++) {
      for (p = COP_TABLE[i][j]; p != NULL; p = p->next) {
        if (p->refer_flag == 2) {
          p->para = 0;
          p->update_para = LOW_LOG_VALUE;
        } else if (p->refer_flag == -1) {
          p->para = LOW_LOG_VALUE;
          p->update_para = LOW_LOG_VALUE;
        } else {
          p->para = log(1.0 / COP_NUM) * INV_LOG_TEN;
          p->update_para = LOW_LOG_VALUE;
        }
      }
    }
  }
}

void training(TOTAL_INFO *info) {
  char tmp_del, tmp_ins;
  int i, j, k, r, z;
  PAIR_DATA *pair_data;
  double **alpha;
  double **beta;
  VITERBI_HYP_LIST **viterbi_table, ***viterbi_table_for_del_and_ins_inference;

  // get memory for Alpha and Beta
  if ((alpha = (double **)malloc(
           2 * ((maxX + 1) *
                (sizeof(double *) + (maxY + 1) * sizeof(double))))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i <= maxX; i++) {
    alpha[i] = ((double *)(alpha + maxX + 1)) + i * (maxY + 1);
  }

  beta = (double **)((double *)(alpha + maxX + 1) + i * (maxY + 1));
  for (i = 0; i <= maxX; i++) {
    beta[i] = ((double *)(beta + maxX + 1)) + i * (maxY + 1);
  }

  fprintf(stderr, "Initialization\n");
  initialization(info, alpha, beta);

  if (training_type >= 4) {
    // 4 and 5 are the forward-backward training that prohibits a deletion.
    tmp_del = del;
    tmp_ins = ins;
    del = 0;
    ins = 0;
  }

  fprintf(stderr, "Start the forward-backward training.\n");
  i = 0;
  while (1) {
    i++;
    fprintf(stderr, "Iteration:%d\n", i);
    info->total_update = LOW_LOG_VALUE;
    j = 1;
    for (pair_data = info->pair_data; pair_data != NULL;
         pair_data = pair_data->next) {
      if ((j % 1000) == 0) {
        fprintf(stderr, "Expectation:%d\r", j);
      }

      expectation(info, pair_data, alpha, beta, pair_data->x_size,
                  pair_data->y_size);

      j++;

      /* debug
      fprintf(stderr,"x size:%d       y size:%d\n",pair_data->x_size,
      pair_data->y_size);
      fprintf(stderr,"COP size:%d\nx: ",COP_NUM);
      printJoinedString(stderr, ' ', pair_data->x_part_strs[0],
      pair_data->x_size);

      fprintf(stderr,"y: ");
      printJoinedString(stderr, ' ', pair_data->y_part_strs[0],
      pair_data->y_size);

      fprintf(stderr,"alpha: \n");
      printAlphaBeta(alpha, pair_data->x_size, pair_data->y_size);

      fprintf(stderr,"beta: \n");
      printAlphaBeta(beta, pair_data->x_size, pair_data->y_size);
      */
    }
    fprintf(stderr, "Expectation:%d\n", j - 1);
    fprintf(stderr, "Maximization\n");
    info->total_change = 0;
    maximization(info);
    fprintf(stderr, "change prob:%f\n\n", info->total_change);
    if ((1 <= threshold_fb_eot && threshold_fb_eot <= i) ||
        (threshold_fb_eot < 1 && info->total_change <= threshold_fb_eot)) {
      break;
    }
  }
  free(alpha);

  if (training_type >=
      4) { // n-best viterbi training within del and ins inference
    fprintf(
        stderr,
        "Start the n-best viterbi training within del and ins inference.\n");

    // Viterbi training within del and ins inference infers a deletion by using
    // peri-mappings.
    // So we allow del, ins or both.
    del = tmp_del;
    ins = tmp_ins;

    // get memory for Viterbi training within del and ins inference
    // calc the num of VITERBI_HYP_LIST (assign to i)
    // sum of (am+an) for numerator of arithmetic progression
    i = 0;
    j = 1;
    while (j <= maxX + 1) {
      i += (j + maxY + j);
      j++;
    }

    // mul n/2 for arithmetic progression
    i = (i * (maxY + 1)) / 2;

    if ((viterbi_table_for_del_and_ins_inference = (VITERBI_HYP_LIST ***)malloc(
             (maxX + 1) * (sizeof(VITERBI_HYP_LIST **) +
                           (maxY + 1) * (sizeof(VITERBI_HYP_LIST *))) +
             i * (sizeof(VITERBI_HYP_LIST) +
                  training_n_best * sizeof(VITERBI_HYP)))) == NULL) {
      fprintf(stderr,
              "Don't get memory in malloc.\nYou must need more memory.\n");
      exit(EXIT_FAILURE);
    }

    // set address for Viterbi training within del and ins inference
    k = 0;
    for (i = 0; i <= maxX; i++) {
      viterbi_table_for_del_and_ins_inference[i] =
          (VITERBI_HYP_LIST **)((VITERBI_HYP *)((VITERBI_HYP_LIST *)((VITERBI_HYP_LIST **)(viterbi_table_for_del_and_ins_inference +
                                                                                           maxX +
                                                                                           1) +
                                                                     i * (maxY +
                                                                          1)) +
                                                k) +
                                k * training_n_best);

      k += (maxY + 1) * (i + maxY + i + 2) / 2;
      r = 0;
      for (j = 0; j <= maxY; j++) {
        viterbi_table_for_del_and_ins_inference[i][j] =
            (VITERBI_HYP_LIST *)((VITERBI_HYP *)((VITERBI_HYP_LIST *)(viterbi_table_for_del_and_ins_inference
                                                                          [i] +
                                                                      maxY +
                                                                      1) +
                                                 r) +
                                 r * training_n_best);

        r += i + j + 1;
        for (z = 0; z <= i + j; z++) {
          viterbi_table_for_del_and_ins_inference[i][j][z].viterbi_hyp =
              (VITERBI_HYP *)(viterbi_table_for_del_and_ins_inference[i][j] +
                              i + j + 1) +
              z * training_n_best;
        }
      }
    }

    // Viterbi training within del and ins inference runs one iteration.
    info->total_update = LOW_LOG_VALUE;
    j = 1;
    for (pair_data = info->pair_data; pair_data != NULL;
         pair_data = pair_data->next) {
      if ((j % 1000) == 0) {
        fprintf(stderr,
                "The n-best viterbi training within del and ins inference:%d\r",
                j);
      }

      viterbiWithinDelAndInsInference(info, pair_data,
                                      viterbi_table_for_del_and_ins_inference);

      j++;
    }

    fprintf(stderr,
            "The n-best viterbi training within del and ins inference:%d\n",
            j - 1);
    fprintf(stderr, "Maximization\n");
    info->total_change = 0;
    maximization(info);
    fprintf(stderr, "change prob:%f\n\n", info->total_change);

    free(viterbi_table_for_del_and_ins_inference);
  }

  if (training_type == 1 || training_type == 3 ||
      training_type == 5) { // n-best viterbi training
    fprintf(stderr, "Start the n-best viterbi training.\n");

    if ((viterbi_table = (VITERBI_HYP_LIST **)malloc(
             (maxX + 1) *
             (sizeof(VITERBI_HYP_LIST *) +
              (maxY + 1) * (sizeof(VITERBI_HYP_LIST) +
                            training_n_best * sizeof(VITERBI_HYP))))) == NULL) {
      fprintf(stderr,
              "Don't get memory in malloc.\nYou must need more memory.\n");
      exit(EXIT_FAILURE);
    }

    for (i = 0; i <= maxX; i++) {
      viterbi_table[i] =
          (VITERBI_HYP_LIST *)((VITERBI_HYP *)(((VITERBI_HYP_LIST *)(viterbi_table +
                                                                     maxX +
                                                                     1)) +
                                               i * (maxY + 1)) +
                               i * (maxY + 1) * training_n_best);
      for (j = 0; j <= maxY; j++) {
        viterbi_table[i][j].viterbi_hyp =
            (VITERBI_HYP *)(viterbi_table[i] + maxY + 1) + j * training_n_best;
      }
    }

    i = 0;
    while (1) {
      i++;
      fprintf(stderr, "Iteration:%d\n", i);
      info->total_update = LOW_LOG_VALUE;
      j = 1;
      for (pair_data = info->pair_data; pair_data != NULL;
           pair_data = pair_data->next) {
        if ((j % 1000) == 0) {
          fprintf(stderr, "The n-best viterbi training:%d\r", j);
        }

        viterbi(info, pair_data, viterbi_table, training_n_best);
        backtrackForTraining(info, pair_data, viterbi_table);

        j++;
      }

      fprintf(stderr, "The n-best viterbi training:%d\r", j - 1);
      fprintf(stderr, "Maximization\n");
      info->total_change = 0;
      maximization(info);
      fprintf(stderr, "change prob:%f\n\n", info->total_change);
      if ((1 <= threshold_v_eot && threshold_v_eot <= i) ||
          (threshold_v_eot < 1 && info->total_change <= threshold_v_eot)) {
        break;
      }
    }

    free(viterbi_table);
  }
}

void writeMappingToFile(char used_mapping) {
  FILE *fp;
  unsigned int i, j;
  COP *p;

  if (used_mapping == 0) {
    fprintf(stderr, "Writting the align model: %s\n", output_align_model);
    if ((fp = fopen(output_align_model, "w")) == NULL) {
      fprintf(stderr, "The align model FILE open error: %s\n",
              output_align_model);
      exit(EXIT_FAILURE);
    }
  } else { // write used mapping
    fprintf(stderr, "Writting the align model for used mappings: %s\n",
            output_used_mapping);
    if ((fp = fopen(output_used_mapping, "w")) == NULL) {
      fprintf(stderr, "The align model FILE open error: %s\n",
              output_used_mapping);
      exit(EXIT_FAILURE);
    }
  }

  for (i = 0; i < sqrt_hash_size; i++) {
    for (j = 0; j < sqrt_hash_size; j++) {
      for (p = COP_TABLE[i][j]; p != NULL; p = p->next) {
        if (used_mapping == 0 || p->refer_flag > 1) {
          printJoinedString(fp, join_char, p->x, p->x_len);

          fprintf(fp, "\t");
          printJoinedString(fp, join_char, p->y, p->y_len);

          fprintf(fp, "\t%f\n", p->para);
        }
      }
    }
  }

  fclose(fp);
}

static char backtrackForAlignment(TOTAL_INFO *info, PAIR_DATA *pair_data,
                                  VITERBI_HYP_LIST **viterbi_table,
                                  ALIGN_HYP *align_hyps,
                                  VITERBI_HYP_LIST **end_vhl) {
  LEN xl, yl, tmp_y_size, tmp_xl, tmp_yl, tmp1, tmp2, i;
  VITERBI_HYP_LIST *nvhl;
  VITERBI_HYP vh;
  COP *cop;
  NumOfContextType *tmp_noct, *new_noct, *old_noct = start_and_end_noct;
  Bigram *bigram, *tmp_bigram;
  char warn = 0;

  tmp_y_size = pair_data->y_size;
  if (substringCheck == 1) {
    for (nvhl = &(viterbi_table[pair_data->x_size][tmp_y_size]);
         nvhl->rank_size == 0 && tmp_y_size > 1;
         nvhl = &(viterbi_table[pair_data->x_size][tmp_y_size])) {
      tmp_y_size--;
    }

    for (xl = 1; xl < nvhl->rank_size; xl++) {
      for (yl = xl; yl > 0; yl--) {
        if (nvhl->viterbi_hyp[yl - 1].back_num_of_del_and_ins <
            nvhl->viterbi_hyp[yl].back_num_of_del_and_ins) {
          vh = nvhl->viterbi_hyp[yl - 1];
          nvhl->viterbi_hyp[yl - 1] = nvhl->viterbi_hyp[yl];
          nvhl->viterbi_hyp[yl] = vh;
        } else {
          break;
        }
      }
    }
  } else {
    nvhl = &(viterbi_table[pair_data->x_size][tmp_y_size]);
  }
  *end_vhl = nvhl;

  for (i = 0; i < nvhl->rank_size; i++) {
    vh = nvhl->viterbi_hyp[i];
    xl = pair_data->x_size;
    yl = tmp_y_size;
    align_hyps[i].score = vh.score;
    while ((vh.backX >= 0) || (vh.backY >= 0)) {
      if (vh.backX == xl) {
        cop = COP_refer(tmp_del_ins_char, 1, pair_data->y_part_strs[vh.backY],
                        yl - vh.backY);

      } else if (vh.backY == yl) {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        tmp_del_ins_char, 1);

      } else {
        cop = COP_refer(pair_data->x_part_strs[vh.backX], xl - vh.backX,
                        pair_data->y_part_strs[vh.backY], yl - vh.backY);
      }
      if (alignment_type == 0 || alignment_type == 102) {
        if (cop->refer_flag < 2) {
          cop->refer_flag = 2;
        }
        align_hyps[i].cop[vh.num_of_trans - 1] = cop;
      } else if (alignment_type == 1 || alignment_type == 2) {

        new_noct = NumOfContextType_get(cop);
        bigram = Bigram_get(new_noct->cop, old_noct->cop);
        if (bigram->refer == 0) {
          new_noct->right++;
          old_noct->left++;
        }

        if (old_noct == start_and_end_noct) {
          bigram->split = 1;
        } else if (info->delimiters_x[xl] == tmp_separate_char &&
                   info->delimiters_y[yl] == tmp_separate_char) {

          if (new_noct->cop->refer_flag < 2) {
            for (tmp_xl = xl - 1; tmp_xl > 0; tmp_xl--) {
              if (info->delimiters_x[tmp_xl] == inter_unknown) {
                goto END_BIGRAM_SPLIT;
              } else if (info->delimiters_x[tmp_xl] == tmp_separate_char) {
                break;
              }
            }

            for (tmp_yl = yl - 1; tmp_yl > 0; tmp_yl--) {
              if (info->delimiters_y[tmp_yl] == inter_unknown) {
                goto END_BIGRAM_SPLIT;
              } else if (info->delimiters_y[tmp_yl] == tmp_separate_char) {
                break;
              }
            }
          }

          if (old_noct->cop->refer_flag < 2) {
            for (tmp_xl = xl + 1; tmp_xl < pair_data->x_size; tmp_xl++) {
              if (info->delimiters_x[tmp_xl] == inter_unknown) {
                goto END_BIGRAM_SPLIT;
              } else if (info->delimiters_x[tmp_xl] == tmp_separate_char) {
                break;
              }
            }

            for (tmp_yl = yl + 1; tmp_yl < tmp_y_size; tmp_yl++) {
              if (info->delimiters_y[tmp_yl] == inter_unknown) {
                goto END_BIGRAM_SPLIT;
              } else if (info->delimiters_y[tmp_yl] == tmp_separate_char) {
                break;
              }
            }
          }

          bigram->split = 1;
        }
      END_BIGRAM_SPLIT:

        bigram->refer++;
        old_noct = new_noct;
      } else { // alignment_type==100 || alignment_type==101. These mean an
               // internal state.
        if (alignment_type == 101) {
          align_hyps[i].cop[vh.num_of_trans - 1] = cop;
        }

        new_noct = NumOfContextType_get(cop);
        bigram = Bigram_get(new_noct->cop, old_noct->cop);
        if (doubtful_condition == 0) { // both
          if (bigram->split == 0 &&
              !(info->delimiters_x[xl] == tmp_separate_char &&
                info->delimiters_y[yl] == tmp_separate_char) &&
              new_noct->right <= doubtful_context_size &&
              old_noct->left <= doubtful_context_size &&
              new_noct->cop->refer_flag < 2 // This cop is not previous
                                            // knowledge.
              && old_noct->cop->refer_flag < 2) {
            warn = 1;                    // detect doubtful mappings
            if (alignment_type == 100) { // join doubtful mappings
              if (bigram->refer == 0) {
                for (tmp1 = 1; tmp1 <= old_noct->cop->x_len; tmp1++) {
                  for (tmp2 = 1; tmp2 <= old_noct->cop->y_len; tmp2++) {
                    if (tmp1 == old_noct->cop->x_len &&
                        tmp2 == old_noct->cop->y_len) {
                      break;
                    }
                    cop = COP_refer(old_noct->cop->x, tmp1, old_noct->cop->y,
                                    tmp2);

                    if (cop != NULL && cop->refer_flag != -1) {
                      tmp_noct = NumOfContextType_refer(cop);
                      if (tmp_noct != NULL) {
                        tmp_bigram = Bigram_refer(new_noct->cop, tmp_noct->cop);
                        if (tmp_bigram != NULL && tmp_bigram->split == 1) {
                          bigram->split = 1;
                          goto END_NEW_BIGRAM_SPLIT_CHECK_IN_BOTH;
                        }
                      }
                    }
                  }
                }
              END_NEW_BIGRAM_SPLIT_CHECK_IN_BOTH:

                bigram->refer++;
              }

              if (bigram->split == 0
                      // Both mappings are deletion or insertion char in input
                      // data, they are not joined
                  &&
                  (((vh.backX == pair_data->x_size ||
                     *(pair_data->x_part_strs[vh.backX]) == inter_del_ins) &&
                    tmp_xl == xl) ||
                   ((xl == pair_data->x_size ||
                     *(pair_data->x_part_strs[xl]) == inter_del_ins) &&
                    vh.backX == xl) ||
                   (*(pair_data->x_part_strs[vh.backX]) != inter_del_ins &&
                    (xl == pair_data->x_size ||
                     *(pair_data->x_part_strs[xl]) != inter_del_ins))) &&
                  (((vh.backY == tmp_y_size ||
                     *(pair_data->y_part_strs[vh.backY]) == inter_del_ins) &&
                    tmp_yl == yl) ||
                   ((yl == tmp_y_size ||
                     *(pair_data->y_part_strs[yl]) == inter_del_ins) &&
                    vh.backY == yl) ||
                   (*(pair_data->y_part_strs[vh.backY]) != inter_del_ins &&
                    (yl == tmp_y_size ||
                     *(pair_data->y_part_strs[yl]) != inter_del_ins)))) {
                if ((tmp_xl - vh.backX) == 0) {
                  cop = COP_refer(tmp_del_ins_char, 1,
                                  pair_data->y_part_strs[vh.backY],
                                  tmp_yl - vh.backY);
                } else if ((tmp_yl - vh.backY) == 0) {
                  cop = COP_refer(pair_data->x_part_strs[vh.backX],
                                  tmp_xl - vh.backX, tmp_del_ins_char, 1);

                } else {
                  cop = COP_refer(
                      pair_data->x_part_strs[vh.backX], tmp_xl - vh.backX,
                      pair_data->y_part_strs[vh.backY], tmp_yl - vh.backY);
                }

                if (cop != NULL && cop->refer_flag != -1) {
                  tmp_noct = NumOfContextType_get(cop);

                  tmp_noct->right = old_noct->right;
                  tmp_noct->left = new_noct->left;

                  new_noct = tmp_noct;
                } else {
                  // The joined cop don't exist.
                  // So these cop are not joined although these cop are doubtful
                  // mappings.
                  if (old_noct->cop->refer_flag < 1) {
                    old_noct->cop->refer_flag = 1;
                  }
                  tmp_xl = xl;
                  tmp_yl = yl;
                }
              } else {
                // These cop is not doubtful mapping or
                // both include del or ins in input data (in this case, don't
                // join).
                if (old_noct->cop->refer_flag < 1) {
                  old_noct->cop->refer_flag = 1;
                }
                tmp_xl = xl;
                tmp_yl = yl;
              }
            }
          } else { // These cop are not doubtful mapping.
            if (alignment_type == 100 && old_noct != start_and_end_noct &&
                old_noct->cop->refer_flag < 1) {
              old_noct->cop->refer_flag = 1;
            }
            tmp_xl = xl;
            tmp_yl = yl;
          }
        } else { // one side
          if (bigram->split == 0 &&
              !(info->delimiters_x[xl] == tmp_separate_char &&
                info->delimiters_y[yl] == tmp_separate_char) &&
              ((new_noct->right <= doubtful_context_size &&
                new_noct->cop->refer_flag <
                    2) // This cop is not previous knowledge.
               ||
               (old_noct->left <= doubtful_context_size &&
                old_noct->cop->refer_flag < 2))) {

            if (info->delimiters_x[xl] == tmp_separate_char &&
                info->delimiters_y[yl] == tmp_separate_char) {

              if (new_noct->right <= doubtful_context_size &&
                  new_noct->cop->refer_flag < 2) {

                for (tmp1 = xl - 1; tmp1 > 0; tmp1--) {
                  if (info->delimiters_x[tmp1] == inter_unknown) {
                    goto END_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE;
                  } else if (info->delimiters_x[tmp1] == tmp_separate_char) {
                    break;
                  }
                }

                for (tmp1 = yl - 1; tmp1 > 0; tmp1--) {
                  if (info->delimiters_y[tmp1] == inter_unknown) {
                    goto END_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE;
                  } else if (info->delimiters_y[tmp1] == tmp_separate_char) {
                    break;
                  }
                }
              }

              if (old_noct->left <= doubtful_context_size &&
                  old_noct->cop->refer_flag < 2) {

                for (tmp1 = xl + 1; tmp1 < pair_data->x_size; tmp1++) {
                  if (info->delimiters_x[tmp1] == inter_unknown) {
                    goto END_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE;
                  } else if (info->delimiters_x[tmp1] == tmp_separate_char) {
                    break;
                  }
                }

                for (tmp1 = yl + 1; tmp1 < tmp_y_size; tmp1++) {
                  if (info->delimiters_y[tmp1] == inter_unknown) {
                    goto END_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE;
                  } else if (info->delimiters_y[tmp1] == tmp_separate_char) {
                    break;
                  }
                }
              }

              bigram->split = 1;
            }
          END_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE:

            if (bigram->split == 0) {
              warn = 1; // detect doubtful mappings
            }
            if (alignment_type == 100) { // join doubtful mappings
              if (bigram->refer == 0) {
                for (tmp1 = 1; tmp1 <= old_noct->cop->x_len; tmp1++) {
                  for (tmp2 = 1; tmp2 <= old_noct->cop->y_len; tmp2++) {
                    if (tmp1 == old_noct->cop->x_len &&
                        tmp2 == old_noct->cop->y_len) {
                      break;
                    }
                    cop = COP_refer(old_noct->cop->x, tmp1, old_noct->cop->y,
                                    tmp2);

                    if (cop != NULL && cop->refer_flag != -1) {
                      tmp_noct = NumOfContextType_refer(cop);
                      if (tmp_noct != NULL) {
                        tmp_bigram = Bigram_refer(new_noct->cop, tmp_noct->cop);
                        if (tmp_bigram != NULL && tmp_bigram->split == 1) {
                          bigram->split = 1;
                          goto END_NEW_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE;
                        }
                      }
                    }
                  }
                }
              END_NEW_BIGRAM_SPLIT_CHECK_IN_ONT_SIDE:

                bigram->refer++;
              }

              /*
              fprintf(stderr,"check merge cop:%d\n",bigram->split);
              printJoinedString(stderr, join_char, new_noct->cop->x,
              new_noct->cop->x_len);
              fprintf(stderr,"\t");
              printJoinedString(stderr, join_char, new_noct->cop->y,
              new_noct->cop->y_len);
              fprintf(stderr,"\n");
              printJoinedString(stderr, join_char, old_noct->cop->x,
              old_noct->cop->x_len);
              fprintf(stderr,"\t");
              printJoinedString(stderr, join_char, old_noct->cop->y,
              old_noct->cop->y_len);
              fprintf(stderr,"\n");*/
              if (bigram->split == 0
                      // Both mappings are deletion or insertion char in input
                      // data, they are not joined
                  &&
                  (((vh.backX == pair_data->x_size ||
                     *(pair_data->x_part_strs[vh.backX]) == inter_del_ins) &&
                    tmp_xl == xl) ||
                   ((xl == pair_data->x_size ||
                     *(pair_data->x_part_strs[xl]) == inter_del_ins) &&
                    vh.backX == xl) ||
                   (*(pair_data->x_part_strs[vh.backX]) != inter_del_ins &&
                    (xl == pair_data->x_size ||
                     *(pair_data->x_part_strs[xl]) != inter_del_ins))) &&
                  (((vh.backY == tmp_y_size ||
                     *(pair_data->y_part_strs[vh.backY]) == inter_del_ins) &&
                    tmp_yl == yl) ||
                   ((yl == tmp_y_size ||
                     *(pair_data->y_part_strs[yl]) == inter_del_ins) &&
                    vh.backY == yl) ||
                   (*(pair_data->y_part_strs[vh.backY]) != inter_del_ins &&
                    (yl == tmp_y_size ||
                     *(pair_data->y_part_strs[yl]) != inter_del_ins)))) {

                if ((tmp_xl - vh.backX) == 0) {
                  cop = COP_refer(tmp_del_ins_char, 1,
                                  pair_data->y_part_strs[vh.backY],
                                  tmp_yl - vh.backY);
                } else if ((tmp_yl - vh.backY) == 0) {
                  cop = COP_refer(pair_data->x_part_strs[vh.backX],
                                  tmp_xl - vh.backX, tmp_del_ins_char, 1);
                } else {
                  cop = COP_refer(
                      pair_data->x_part_strs[vh.backX], tmp_xl - vh.backX,
                      pair_data->y_part_strs[vh.backY], tmp_yl - vh.backY);
                }

                if (cop != NULL && cop->refer_flag != -1) {
                  tmp_noct = NumOfContextType_get(cop);

                  tmp_noct->right = old_noct->right;
                  tmp_noct->left = new_noct->left;

                  new_noct = tmp_noct;
                } else {
                  // The joined cop don't exist.
                  // So these cop are not joined although these cop are doubtful
                  // mappings.
                  if (old_noct->cop->refer_flag < 1) {
                    old_noct->cop->refer_flag = 1;
                  }
                  tmp_xl = xl;
                  tmp_yl = yl;
                }
              } else {
                // These cop is not doubtful mapping or
                // both include del or ins in input data (in this case, don't
                // join) .
                if (old_noct->cop->refer_flag < 1) {
                  old_noct->cop->refer_flag = 1;
                }
                tmp_xl = xl;
                tmp_yl = yl;
              }
            }
          } else { // These cop are not doubtful mapping.
            if (alignment_type == 100 && old_noct != start_and_end_noct &&
                old_noct->cop->refer_flag < 1) {
              old_noct->cop->refer_flag = 1;
            }
            tmp_xl = xl;
            tmp_yl = yl;
          }
        }
        /*
        fprintf(stderr,"change old_noct=new_noct\n");
        printJoinedString(stderr, join_char, new_noct->cop->x,
        new_noct->cop->x_len);
        fprintf(stderr,"\t");
        printJoinedString(stderr, join_char, new_noct->cop->y,
        new_noct->cop->y_len);
        fprintf(stderr,"\n");*/
        old_noct = new_noct;
      }
      xl = vh.backX;
      yl = vh.backY;
      vh = viterbi_table[xl][yl].viterbi_hyp[vh.rank];
    }
    if (alignment_type == 1 || alignment_type == 2) {
      bigram = Bigram_get(start_and_end_noct->cop, old_noct->cop);
      if (bigram->refer == 0) {
        start_and_end_noct->right++;
        old_noct->left++;
      }

      bigram->split = 1;
      bigram->refer++;
    } else if (alignment_type == 100 && old_noct->cop->refer_flag < 1) {
      old_noct->cop->refer_flag = 1;
    } else if (output_used_mapping != NULL && alignment_type == 101 &&
               warn == 0) {
      vh = nvhl->viterbi_hyp[i];
      for (tmp1 = 0; tmp1 < vh.num_of_trans; tmp1++) {
        if (align_hyps[i].cop[tmp1]->refer_flag < 2) {
          align_hyps[i].cop[tmp1]->refer_flag = 2;
        }
      }
    }

    old_noct = start_and_end_noct;
  }

  return warn;
}

void requireAlignments(TOTAL_INFO *info) {
  FILE *fp, *err_fp;
  LEN i, j, k, max_trans;
  unsigned char max_nbest; // first alignment's nbest or second alignment's
                           // nbest
  char warn = 0, obtain_memory_for_output_file = 0;
  char *output_err_file;
  int AlignCount = 0, errAlignCount = 0;
  VITERBI_HYP_LIST *vhl, **viterbi_table;
  ALIGN_HYP *align_hyps;
  PAIR_DATA *pair_data;

  if (output_file == NULL) {
    obtain_memory_for_output_file = 1;
    i = strlen(input_file);
    if (alignment_type != 2) {
      if ((output_file = (char *)malloc(i + 7)) == NULL) {
        fprintf(stderr,
                "Don't get memory in malloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }

      strcpy(output_file, input_file);
      strcat(output_file, ".align");

      fprintf(stderr, "Write alignments: %s\n", output_file);
    } else {
      if ((output_file = (char *)malloc(i + 6)) == NULL) {
        fprintf(stderr,
                "Don't get memory in malloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }

      strcpy(output_file, input_file);
      strcat(output_file, ".safe");
      fprintf(stderr,
              "Write pair data that do not include doubtful mappings: %s\n",
              output_file);
    }
  }

  if ((fp = fopen(output_file, "w")) == NULL) {
    fprintf(stderr, "fopen error: %s\n", output_file);
    exit(EXIT_FAILURE);
  }

  if (alignment_type != 2) {
    if ((output_err_file = (char *)malloc(strlen(output_file) + 5)) == NULL) {
      fprintf(stderr,
              "Don't get memory in malloc.\nYou must need more memory.\n");
      exit(EXIT_FAILURE);
    }

    strcpy(output_err_file, output_file);
    strcat(output_err_file, ".err");
    fprintf(stderr, "Write error alignments: %s\n", output_err_file);
  } else {
    if (obtain_memory_for_output_file == 0) {
      if ((output_err_file = (char *)malloc(strlen(output_file) + 7)) == NULL) {
        fprintf(stderr,
                "Don't get memory in malloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }

      strcpy(output_err_file, output_file);
    } else {
      if ((output_err_file = (char *)malloc(i + 7)) == NULL) {
        fprintf(stderr,
                "Don't get memory in malloc.\nYou must need more memory.\n");
        exit(EXIT_FAILURE);
      }

      strcpy(output_err_file, input_file);
    }

    strcat(output_err_file, ".doubt");
    fprintf(stderr, "Write pair data that include doubtful mappings: %s\n",
            output_err_file);
  }

  if ((err_fp = fopen(output_err_file, "w")) == NULL) {
    fprintf(stderr, "fopen error: %s\n", output_err_file);
    exit(EXIT_FAILURE);
  }

  // get memory and set address for Viterbi algorithm
  max_nbest = (first_n_best > second_n_best) ? first_n_best : second_n_best;

  if ((viterbi_table = (VITERBI_HYP_LIST **)malloc(
           (maxX + 1) * (sizeof(VITERBI_HYP_LIST *) +
                         (maxY + 1) * (sizeof(VITERBI_HYP_LIST) +
                                       max_nbest * sizeof(VITERBI_HYP)) +
                         max_nbest * (sizeof(ALIGN_HYP) +
                                      maxLEN * sizeof(COP *))))) == NULL) {
    fprintf(stderr,
            "Don't get memory in malloc.\nYou must need more memory.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i <= maxX; i++) {
    viterbi_table[i] =
        (VITERBI_HYP_LIST *)((VITERBI_HYP *)(((VITERBI_HYP_LIST *)(viterbi_table +
                                                                   maxX + 1)) +
                                             i * (maxY + 1)) +
                             i * (maxY + 1) * max_nbest);
    for (j = 0; j <= maxY; j++) {
      viterbi_table[i][j].viterbi_hyp =
          (VITERBI_HYP *)(viterbi_table[i] + maxY + 1) + j * max_nbest;
    }
  }

  align_hyps =
      (ALIGN_HYP *)((VITERBI_HYP *)(((VITERBI_HYP_LIST *)(viterbi_table + maxX +
                                                          1)) +
                                    i * (maxY + 1)) +
                    i * (maxY + 1) * max_nbest);
  for (i = 0; i < max_nbest; i++) {
    align_hyps[i].cop = (COP **)(align_hyps + max_nbest) + i * maxLEN;
  }

  // Start alignments.
  if (alignment_type != 0) {
    // Initialization for context check.
    // The below 5 is heuristic.
    NumOfContextType_init(sqrt_hash_size * 5);
    Bigram_init(sqrt_hash_size, sqrt_hash_size);
    start_and_end_noct = NumOfContextType_get(
        COP_get(start_and_end_symbol, 1, start_and_end_symbol, 1));

    fprintf(stderr, (alignment_type == 1) ? "Start first alignments.\n"
                                          : "Start alignments.\n");
  } else {
    fprintf(stderr, "Start alignments.\n");
  }

  for (pair_data = info->pair_data; pair_data != NULL;
       pair_data = pair_data->next) {

    viterbi(info, pair_data, viterbi_table, first_n_best);
    backtrackForAlignment(info, pair_data, viterbi_table, align_hyps, &vhl);

    if (alignment_type == 0) {
      if (vhl->rank_size > 0) {
        AlignCount++;

        for (i = 0; i < vhl->rank_size && i < output_n_best; i++) {
          max_trans = vhl->viterbi_hyp[i].num_of_trans;

          for (j = 0; j < max_trans; j++) {
            printJoinedString(fp, join_char, align_hyps[i].cop[j]->x,
                              align_hyps[i].cop[j]->x_len);

            fprintf(fp, "%c", separate_char);
          }
          fprintf(fp, "\t");

          for (j = 0; j < max_trans; j++) {
            printJoinedString(fp, join_char, align_hyps[i].cop[j]->y,
                              align_hyps[i].cop[j]->y_len);

            fprintf(fp, "%c", separate_char);
          }

          if (printScore) {
            if (kind_of_cityblock == 0) {
              fprintf(
                  fp, "\t%d-best\tscore:%f\tscorePerChar:%f\tconsecutiveDel:%"
                      "d\tdelRatio:%f\ttrans:%d\n",
                  i + 1, align_hyps[i].score,
                  align_hyps[i].score / (pair_data->x_size + pair_data->y_size),
                  vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                  vhl->viterbi_hyp[i].num_of_del * 1.0 / (pair_data->x_size),
                  max_trans);
            } else if (kind_of_cityblock == 1) {
              fprintf(fp, "\t%d-best\tscore:%f\tscorePerY'sChar:%"
                          "f\tconsecutiveDel:%d\tdelRatio:%f\ttrans:%d\n",
                      i + 1, align_hyps[i].score,
                      align_hyps[i].score / pair_data->y_size,
                      vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                      vhl->viterbi_hyp[i].num_of_del * 1.0 /
                          (pair_data->x_size),
                      max_trans);
            } else {
              fprintf(fp, "\t%d-best\tscore:%f\tscorePerX'sChar:%"
                          "f\tconsecutiveDel:%d\tdelRatio:%f\ttrans:%d\n",
                      i + 1, align_hyps[i].score,
                      align_hyps[i].score / pair_data->x_size,
                      vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                      vhl->viterbi_hyp[i].num_of_del * 1.0 /
                          (pair_data->x_size),
                      max_trans);
            }
          } else {
            fprintf(fp, "\n");
          }
        }

        if ((AlignCount % 1000) == 0) {
          fprintf(stderr, "Aligned:%d\r", AlignCount);
        }
      } else {
        errAlignCount++;
        fprintf(stderr, "\nPair could not be aligned:%d\n", errAlignCount);
        fprintf(stderr, "x: ");
        printPairData(stderr, pair_data->x_part_strs[0]);
        fprintf(stderr, "\ny: ");
        printPairData(err_fp, pair_data->x_part_strs[0]);
        fprintf(err_fp, "\t");

        printPairData(stderr, pair_data->y_part_strs[0]);
        fprintf(stderr, "\n");
        printPairData(err_fp, pair_data->y_part_strs[0]);
        fprintf(err_fp, "\n");
      }
    } else {
      AlignCount++;
      if ((AlignCount % 1000) == 0) {
        fprintf(stderr, "Aligned:%d\r", AlignCount);
      }
    }
  }

  if (alignment_type == 0) {
    fprintf(stderr, "End alignments.\nAlignments:%d, Error alignments:%d\n\n",
            AlignCount, errAlignCount);
  } else {
    fprintf(stderr, (alignment_type == 1) ? "End first alignments.\n"
                                          : "End alignments.\n");
  }

  if (alignment_type != 0) {
    fprintf(stderr,
            (alignment_type == 1)
                ? "Start detectting doubtful mappings and merging them.\n"
                : "Start detectting doubtful mappings.\n");

    AlignCount = errAlignCount = 0;
    alignment_type =
        (alignment_type == 1) ? 100 : 101; // 100 means a state that detects
                                           // doubtful mappings and merges them.
    for (pair_data = info->pair_data; pair_data != NULL;
         pair_data = pair_data->next) {
      viterbi(info, pair_data, viterbi_table, first_n_best);
      warn = backtrackForAlignment(info, pair_data, viterbi_table, align_hyps,
                                   &vhl);

      if (alignment_type == 100 && (vhl->rank_size == 0 || warn)) {
        errAlignCount++;
        fprintf(stderr, "Detect pair data that includes doubtful mappings or "
                        "could not be aligned:%d\r",
                errAlignCount);
      } else if (alignment_type == 101) {
        vhl = &(viterbi_table[pair_data->x_size][pair_data->y_size]);
        if (vhl->rank_size > 0 && !warn) {
          AlignCount++;
          printPairData(fp, pair_data->x_part_strs[0]);
          fprintf(fp, "\t");
          printPairData(fp, pair_data->y_part_strs[0]);
          fprintf(fp, "\n");
        } else {
          errAlignCount++;
          printPairData(err_fp, pair_data->x_part_strs[0]);
          fprintf(err_fp, "\t");
          printPairData(err_fp, pair_data->y_part_strs[0]);
          fprintf(err_fp, "\n");
          fprintf(stderr, "Detect pair data that includes doubtful mappings or "
                          "could not be aligned:%d\r",
                  errAlignCount);
        }
      }
    }

    if (alignment_type == 100) {
      fprintf(stderr, "End detectting doubtful mappings and merging them.\n");
    } else {
      fprintf(stderr, "End detecting doubtful mappings.\n"
                      "Pair data that does not include doubtful mappings:%d\n"
                      "Pair data that includes doubtful mappings or could not "
                      "be aligned:%d\n",
              AlignCount, errAlignCount);
    }
  }

  if (alignment_type == 100) {
    fprintf(stderr, "Start second alignments.\n");

    alignment_type = 102;
    AlignCount = errAlignCount = 0;
    for (pair_data = info->pair_data; pair_data != NULL;
         pair_data = pair_data->next) {
      viterbi(info, pair_data, viterbi_table, second_n_best);
      backtrackForAlignment(info, pair_data, viterbi_table, align_hyps, &vhl);

      if (vhl->rank_size > 0) {
        AlignCount++;

        for (i = 0; i < vhl->rank_size && i < output_n_best; i++) {
          max_trans = vhl->viterbi_hyp[i].num_of_trans;

          for (j = 0; j < max_trans; j++) {
            printJoinedString(fp, join_char, align_hyps[i].cop[j]->x,
                              align_hyps[i].cop[j]->x_len);

            fprintf(fp, "%c", separate_char);
          }
          fprintf(fp, "\t");

          for (j = 0; j < max_trans; j++) {
            printJoinedString(fp, join_char, align_hyps[i].cop[j]->y,
                              align_hyps[i].cop[j]->y_len);

            fprintf(fp, "%c", separate_char);
          }

          if (printScore) {
            if (kind_of_cityblock == 0) {
              fprintf(
                  fp, "\t%d-best\tscore:%f\tscorePerChar:%f\tconsecutiveDel:%"
                      "d\tdelRatio:%f\ttrans:%d\n",
                  i + 1, align_hyps[i].score,
                  align_hyps[i].score / (pair_data->x_size + pair_data->y_size),
                  vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                  vhl->viterbi_hyp[i].num_of_del * 1.0 / (pair_data->x_size),
                  max_trans);
            } else if (kind_of_cityblock == 1) {
              fprintf(fp, "\t%d-best\tscore:%f\tscorePerY'sChar:%"
                          "f\tconsecutiveDel:%d\tdelRatio:%f\ttrans:%d\n",
                      i + 1, align_hyps[i].score,
                      align_hyps[i].score / pair_data->y_size,
                      vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                      vhl->viterbi_hyp[i].num_of_del * 1.0 /
                          (pair_data->x_size),
                      max_trans);
            } else {
              fprintf(fp, "\t%d-best\tscore:%f\tscorePerX'sChar:%"
                          "f\tconsecutiveDel:%d\tdelRatio:%f\ttrans:%d\n",
                      i + 1, align_hyps[i].score,
                      align_hyps[i].score / pair_data->x_size,
                      vhl->viterbi_hyp[i].max_num_of_consecutive_del,
                      vhl->viterbi_hyp[i].num_of_del * 1.0 /
                          (pair_data->x_size),
                      max_trans);
            }
          } else {
            fprintf(fp, "\n");
          }
        }

        if ((AlignCount % 1000) == 0) {
          fprintf(stderr, "Aligned:%d\r", AlignCount);
        }
      } else {
        errAlignCount++;
        fprintf(stderr, "\nPair could not be aligned:%d\n", errAlignCount);
        fprintf(stderr, "x: ");
        printPairData(stderr, pair_data->x_part_strs[0]);
        fprintf(stderr, "\ny: ");
        printPairData(err_fp, pair_data->x_part_strs[0]);
        fprintf(err_fp, "\t");
        printPairData(stderr, pair_data->y_part_strs[0]);
        fprintf(stderr, "\n");
        printPairData(err_fp, pair_data->y_part_strs[0]);
        fprintf(err_fp, "\n");
      }
    }

    fprintf(stderr,
            "End second alignments.\nAlignments:%d, Error alignments:%d\n",
            AlignCount, errAlignCount);
  }

  fclose(fp);
  if (obtain_memory_for_output_file == 1) {
    free(output_file);
  }
  fclose(err_fp);
  free(output_err_file);
  free(viterbi_table);
}


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
#include <string.h>
#define STRCMP(a, b) *(a) == *(b) && !strcmp(a, b)

extern void *options[][3];

int set_option(char *option, void *address);
void parse_argv(char **argv);


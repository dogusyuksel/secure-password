#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define OK			0
#define NOK			1

#define MAX_LINE_SIZE	1024

#define TABSIZE			5

#define HEADING_MAX		6

#define TAB			'\t'
#define SPACE		' '
#define BACKSPACE	127
#define DEL			126
#define ENTER		'\r'
#define NL			'\n'

#define COMMAND_QUIT		"quit"
#define COMMAND_HEADING_1	"heading1"
#define COMMAND_HEADING_2	"heading2"
#define COMMAND_HEADING_3	"heading3"
#define COMMAND_HEADING_4	"heading4"
#define COMMAND_HEADING_5	"heading5"
#define COMMAND_HEADING_6	"heading6"

#define debugf(...)		fprintf(stdout, __VA_ARGS__)
#define errorf(...)		fprintf(stderr, __VA_ARGS__)

#define FREE(p)			{										\
							if (p) {							\
								free(p);						\
								p = NULL;						\
							}									\
						}

#define FCLOSE(p)		{										\
							if (p) {							\
								fclose(p);						\
								p = NULL;						\
							}									\
						}

int fheading1(char *buf, char *filename);
int fheading2(char *buf, char *filename);
int fheading3(char *buf, char *filename);
int fheading4(char *buf, char *filename);
int fheading5(char *buf, char *filename);
int fheading6(char *buf, char *filename);

#endif //_MAIN_H___

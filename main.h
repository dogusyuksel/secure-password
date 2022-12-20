#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define VERSION		"01.00"

#define UNUSED(__val__)		((void)__val__)

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
#define ESC			'\e'

#define ASTERIKS	"*"

#define COMMAND_QUIT		"quit"
#define COMMAND_HEADING_1	"heading1"
#define COMMAND_HEADING_2	"heading2"
#define COMMAND_HEADING_3	"heading3"
#define COMMAND_HEADING_4	"heading4"
#define COMMAND_HEADING_5	"heading5"
#define COMMAND_HEADING_6	"heading6"
#define NEW_PARAGRAPHS		"paragraph"
#define LINE_BREAK			"linebreak"
#define BOLD				"bold"
#define ITALIC				"italic"
#define BOLDITALIC			"bolditalic"
#define QUOTE				"quote"
#define NESTED_QUOTE		"nestedquote"
#define ORDERED_LIST		"orderedlist"
#define UNORDERED_LIST		"unorderedlist"
#define START_CODEBLOCK		"startcode"
#define URL					"url"
#define NESTED_UNORDERED	"nestedunordered"
#define TABLE_START			"tablestart"
#define TABLE_ADD			"tableadd"

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
								fflush(p);						\
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

int fparagraph(char *buf, char *filename);

int flinebreak(char *buf, char *filename);

int fbold(char *buf, char *filename);

int fitalic(char *buf, char *filename);

int fbolditalic(char *buf, char *filename);

int fquote(char *buf, char *filename);

int fnestedquote(char *buf, char *filename);

int forderedlist(char *buf, char *filename);

int funorderedlist(char *buf, char *filename);
int fnestedunordered(char *buf, char *filename);

int fstartcode(char *buf, char *filename);

int furl(char *buf, char *filename);

int ftablestart(char *buf, char *filename);
int ftableadd(char *buf, char *filename);

#endif //_MAIN_H___

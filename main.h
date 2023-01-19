#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define VERSION		"00.01"
#define APP_NAME	"spwd"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define UNUSED(__val__)		((void)__val__)

#define OK			0
#define NOK			1

#define MAX_LINE_SIZE	1024
#define MAX_FILE_SIZE	4096

#define FILE_LOCATION		"FILE_LOCATION"
#define NEWLINE				"\n"
#define CONFIG_SEPERATOR	"="

#ifdef DEBUG_OPEN
#define debugf(...)		fprintf(stdout, "[%s:%d]", __func__, __LINE__);fprintf(stdout, __VA_ARGS__)
#else
#define debugf(...)		
#endif
#define errorf(...)		fprintf(stderr, "[%s:%d]", __func__, __LINE__);fprintf(stderr, __VA_ARGS__)

enum user_answer
{
	ANSWERED_YES,
	ANSWERED_NO,
	NOT_ANSWERED
};

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

#define CLOSE(p)		{										\
							if (p) {							\
								close(p);						\
								p = 0;							\
							}									\
						}

#endif //_MAIN_H___

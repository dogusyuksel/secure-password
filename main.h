#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define VERSION		"00.01"

#define APP_NAME	"spwd"

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

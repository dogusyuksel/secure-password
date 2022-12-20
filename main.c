#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/queue.h>
#include <limits.h>
#include <termios.h>

#include "main.h"

static struct termios *oldt = NULL;
static unsigned int ordered_list_counter = 0;
static bool code_started = false;

static char *commands[] = {COMMAND_HEADING_1, COMMAND_HEADING_2, COMMAND_HEADING_3,
							COMMAND_HEADING_4, COMMAND_HEADING_5, COMMAND_HEADING_6,
							NEW_PARAGRAPHS, LINE_BREAK, BOLD,
							ITALIC, BOLDITALIC, QUOTE,
							NESTED_QUOTE, ORDERED_LIST, UNORDERED_LIST,
							START_CODEBLOCK, URL, NESTED_UNORDERED,
							TABLE_START, TABLE_ADD};
static int (*operation[])(char *, char *) = {fheading1, fheading2, fheading3,
											fheading4, fheading5, fheading6,
											fparagraph, flinebreak, fbold,
											fitalic, fbolditalic, fquote,
											fnestedquote, forderedlist, funorderedlist,
											fstartcode, furl, fnestedunordered,
											ftablestart, ftableadd};

static struct option parameters[] = {
	{ "help",				no_argument,		0,	0x100	},
	{ "file-name",			required_argument,	0,	0x101	},
	{ "version",			no_argument,		0,	0x102	},
	{ NULL,					0,					0, 	0 		},
};

static void print_help_exit (void)
{
	debugf("please check README.md here: https://github.com/dogusyuksel/easy-markdown/blob/main/README.md\n");

	exit(OK);
}

int ftablestart(char *buf, char *filename)
{
	unsigned int i = 0;
	unsigned int column_count = 0;
	FILE *fp = NULL;
	char *rest = NULL;
	char *token;
	char delim = SPACE;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n");
	for (token = strtok_r(buf, &delim, &rest);
		token != NULL;
		token = strtok_r(NULL, &delim, &rest)) {
		fprintf(fp, " | %s ", token);
		column_count++;
	}
	fprintf(fp, "|\n");

	for (i = 0; i < column_count; i++) {
		fprintf(fp, " | :---: ");
	}
	fprintf(fp, "|\n");

	FCLOSE(fp);

	return OK;
}

int ftableadd(char *buf, char *filename)
{
	FILE *fp = NULL;
	char *rest = NULL;
	char *token;
	char delim = SPACE;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	for (token = strtok_r(buf, &delim, &rest);
		token != NULL;
		token = strtok_r(NULL, &delim, &rest)) {
		fprintf(fp, " | %s ", token);
	}
	fprintf(fp, "|\n");

	FCLOSE(fp);

	return OK;
}

int fnestedunordered(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "    - %s\n", buf ? buf : "");

	FCLOSE(fp);

	return OK;
}

int furl(char *buf, char *filename)
{
	FILE *fp = NULL;
	char *rest = NULL;
	char *token;
	char delim = SPACE;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "[");
	for (token = strtok_r(buf, &delim, &rest);
		token != NULL;
		token = strtok_r(NULL, &delim, &rest)) {
		if (strstr(token, "http")) {
			fprintf(fp, "](%s)", token);
		} else {
			fprintf(fp, "%s ", token);
		}
	}
	fprintf(fp, "\n");

	FCLOSE(fp);

	return OK;
}

int fstartcode(char *buf, char *filename)
{
	UNUSED(buf);
	UNUSED(filename);

	code_started = true;

	return OK;
}

int funorderedlist(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "- %s\n", buf ? buf : "");

	FCLOSE(fp);

	return OK;
}

int forderedlist(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	++ordered_list_counter;
	fprintf(fp, "%s%d. %s\n", (ordered_list_counter == 1) ? "\n" : "", ordered_list_counter, buf ? buf : "");

	FCLOSE(fp);

	return OK;
}

int fnestedquote(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n>> %s   ", buf ? buf : "");

	FCLOSE(fp);

	return OK;
}

int fquote(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n> %s   ", buf ? buf : "");

	FCLOSE(fp);

	return OK;
}

int fbolditalic(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	if (strstr(buf, ASTERIKS)) {
		fprintf(fp, " ___%s___", buf);
	} else {
		fprintf(fp, " ***%s***", buf);
	}

	FCLOSE(fp);

	return OK;
}

int fitalic(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	if (strstr(buf, ASTERIKS)) {
		fprintf(fp, " _%s_", buf);
	} else {
		fprintf(fp, " *%s*", buf);
	}

	FCLOSE(fp);

	return OK;
}

int fbold(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!buf || !filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	if (strstr(buf, ASTERIKS)) {
		fprintf(fp, " __%s__", buf);
	} else {
		fprintf(fp, " **%s**", buf);
	}

	FCLOSE(fp);

	return OK;
}

int flinebreak(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "   \n%s", (buf) ? buf : "");

	FCLOSE(fp);

	return OK;
}

int fparagraph(char *buf, char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n\n%s", (buf) ? buf : "");

	FCLOSE(fp);

	return OK;
}

static int heading_util(char *buf, char *filename, unsigned int headingcount)
{
	unsigned int i = 0;
	FILE *fp = NULL;

	if (!buf || !filename || headingcount > HEADING_MAX) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n");
	for (i = 0; i < headingcount; i++) {
		fprintf(fp, "#");
	}
	fprintf(fp, " %s\n", buf);

	FCLOSE(fp);

	return OK;
}

int fheading1(char *buf, char *filename)
{
	return heading_util(buf, filename, 1);
}

int fheading2(char *buf, char *filename)
{
	return heading_util(buf, filename, 2);
}

int fheading3(char *buf, char *filename)
{
	return heading_util(buf, filename, 3);
}

int fheading4(char *buf, char *filename)
{
	return heading_util(buf, filename, 4);
}

int fheading5(char *buf, char *filename)
{
	return heading_util(buf, filename, 5);
}

int fheading6(char *buf, char *filename)
{
	return heading_util(buf, filename, 6);
}

static int disable_icanon(struct termios **oldt)
{
	struct termios newt;

	FREE(*oldt);

	*oldt = (struct termios *)calloc(1, sizeof(struct termios));
	if (!*oldt) {
		errorf("calloc failed\n");
		return NOK;
	}

	tcgetattr(STDIN_FILENO, *oldt);

	newt = **oldt;

	newt.c_lflag &= ~(ICANON);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	return OK;
}

static void restore_icanon(struct termios **oldt)
{
	if (!*oldt) {
		errorf("parameter is null\n");
		return;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, *oldt);
	FREE(*oldt);
}

static void sigint_handler(__attribute__((unused)) int sig_num)
{
	restore_icanon(&oldt);
	exit(NOK);
}

static void process_command(char *buffer, char *filename)
{
	unsigned int i = 0, j = 0;
	char *rest = NULL;
	char *token;
	char delim = SPACE;

	if (!buffer || !filename) {
		errorf("arg is null\n");
		return;
	}

	token = strtok_r(buffer, &delim, &rest);
	if (token && strlen(token)) {

		while(rest && j < strlen(rest) && rest[j] == delim) {
			j++;
		}
		for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
			if (!strcmp(token, commands[i])) {
				if (i < sizeof(operation) / sizeof(operation[0])) {
					if (strcmp(commands[i], ORDERED_LIST)) {
						ordered_list_counter = 0;
					}
					operation[i](&rest[j], filename);
				}
			}
		}
	}
}

static void clear_screen(void)
{
	debugf("\033[2K");
	debugf("\r");
}

static void show_all_commands(void)
{
	int i = 0;
	int size = sizeof(commands) / sizeof(commands[0]);

	debugf("possible commands: ");
	for (i = 0; i < size; i++) {
		debugf("%s ", commands[i]);
	}
	debugf("\n");
}

static int codeblock_helper(char *buf, char *filename)
{
	FILE *fp = NULL;
	char *rest = NULL;
	char *token;
	char delim = NL;

	if (!filename) {
		errorf("parameters are wrong\n");
		return NOK;
	}

	fp = fopen(filename, "a+");
	if (!fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	fprintf(fp, "\n\n");
	for (token = strtok_r(buf, &delim, &rest);
		token != NULL;
		token = strtok_r(NULL, &delim, &rest)) {
		fprintf(fp, "    %s\n", token);
	}


	FCLOSE(fp);

	return OK;
}

static void read_and_process(char *filename)
{
	char c;
	char buffer[MAX_LINE_SIZE] = {0};
	int i = 0;
	int last_idx = -1;
	int count_count = 0;
	unsigned int counter = 0;
	bool exit_needed = false;
	bool command_got = false;

	if (!filename) {
		errorf("parameter is nulln");
		return;
	}

	memset(buffer, 0, sizeof(buffer));

	while(!exit_needed) {
		c = getc(stdin);

		if (c == TAB) {
			int size = sizeof(commands) / sizeof(commands[0]);
			count_count = 0;

			if (code_started) {
				goto cont_loop;
			}

			if (counter == 0) {
				show_all_commands();
				continue;
			}

			if (last_idx >= size - 1) {
				last_idx = -1;
			}

			for (i = last_idx + 1; i < size; i++) {
				if (strstr(commands[i], buffer) &&
					commands[i][0] == buffer[0]) {
					count_count++;
				}
			}
			for (i = last_idx + 1; i < size; i++) {
				if (strstr(commands[i], buffer) &&
					commands[i][0] == buffer[0]) {
					last_idx = i;
					break;
				}
			}

			if (counter == 0) {
				show_all_commands();
				continue;
			}

			if (count_count == 1) {
				clear_screen();
				memset(buffer, 0, sizeof(buffer));
				counter = strlen(commands[last_idx]);
				memcpy(buffer, commands[last_idx], counter);
				debugf("%s", buffer);
			} else if (count_count > 0) {
				clear_screen();
				if (last_idx >= 0) {
					debugf("%s", commands[last_idx]);
				}
			}

			continue;
		} else if (c == SPACE) {
			if (!command_got) {
				command_got = true;

				if (count_count > 1) {
					clear_screen();

					memset(buffer, 0, sizeof(buffer));
					counter = strlen(commands[last_idx]);
					memcpy(buffer, commands[last_idx], counter);
					debugf("%s ", buffer);
				}

				if (!strcmp(buffer, COMMAND_QUIT)) {
					exit_needed = true;
				}
			}
		} else if (c == ENTER || c == NL) {
			if (code_started) {
				goto cont_loop;
			}
			if (counter == 0) {
				fparagraph("", filename);
				continue;
			}

			process_command(buffer, filename);

			memset(buffer, 0, sizeof(buffer));
			counter = 0;
			command_got = false;
			clear_screen();
			last_idx = -1;
			count_count = 0;

			continue;
		} else if (c == ESC) {
			if (code_started) {
				code_started = false;
				codeblock_helper(buffer, filename);

				memset(buffer, 0, sizeof(buffer));
				counter = 0;
				command_got = false;
				clear_screen();
				last_idx = -1;
				count_count = 0;

				debugf("code mode ended\n");
			}

			continue;
		} else if (c == BACKSPACE) {
			clear_screen();
			if (counter == 0) {
				continue;
			}

			buffer[counter - 1] = '\0';
			counter--;
			debugf("%s", buffer);

			continue;
		} else if (c < SPACE || c >= DEL) {
			continue;
		}

cont_loop:
		buffer[counter++] = c;

		if (counter >= MAX_LINE_SIZE) {
			process_command(buffer, filename);

			memset(buffer, 0, sizeof(buffer));
			counter = 0;
			command_got = false;
			clear_screen();
			last_idx = -1;
			count_count = 0;

			continue;
		}

		if (!command_got && !strcmp(buffer, COMMAND_QUIT)) {
			exit_needed = true;
		}
	}
}

int main(int argc, char **argv)
{
	int ret = OK;
	int c, o;
	char *filename = NULL;
	
	if (disable_icanon(&oldt) == NOK) {
		errorf("disable_icanon failed\n");
		goto fail;
	}

	signal(SIGINT, sigint_handler);

	while ((c = getopt_long(argc, argv, "h", parameters, &o)) != -1) {
		switch (c) {
			case 0x100:
				print_help_exit();
				break;
			case 0x101:
				filename = strdup(optarg);
				if (!filename) {
					errorf("strdup failed\n");
					goto fail;
				}
				break;
			case 0x102:
				debugf("%s version %s\n", argv[0], VERSION);
				return OK;
				break;
			default:
				debugf("unknown argument\n");
				goto fail;
		}
	}

	if (!filename) {
		errorf("file-name is mandatory\n");
		print_help_exit();
	}

	debugf("******* %s started ******* \n", argv[0]);

	read_and_process(filename);

	goto out;

fail:
	ret = NOK;

out:
	restore_icanon(&oldt);

	debugf("\n******* %s ended ******* \n", argv[0]);

	return ret;
}

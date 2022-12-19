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

static char *commands[] = {COMMAND_HEADING_1, COMMAND_HEADING_2, COMMAND_HEADING_3,
							COMMAND_HEADING_4, COMMAND_HEADING_5, COMMAND_HEADING_6,
							NEW_PARAGRAPHS, LINE_BREAK, BOLD};
static int (*operation[])(char *, char *) = {fheading1, fheading2, fheading3,
											fheading4, fheading5, fheading6,
											fparagraph, flinebreak, fbold};

static struct option parameters[] = {
	{ "help",				no_argument,		0,	0x100	},
	{ "file-name",			required_argument,	0,	0x101	},
	{ NULL,					0,					0, 	0 		},
};

static void print_help_exit (const char *name)
{
	if (!name) {
		errorf("name is null\n");
		exit(NOK);
	}
	debugf("\n%s application is used \n", name);

	debugf("\nTODOn");

	debugf("\n");

	exit(NOK);
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

			clear_screen();
			if (counter == 0) {
				continue;
			}

			if (count_count == 1) {
				memset(buffer, 0, sizeof(buffer));
				counter = strlen(commands[last_idx]);
				memcpy(buffer, commands[last_idx], counter);
				debugf("%s", buffer);
			} else if (count_count >= 0) {
				debugf("%s", commands[last_idx]);
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

				if (strstr(buffer, COMMAND_QUIT)) {
					exit_needed = true;
				}
			}
		} else if (c == ENTER || c == NL) {
				process_command(buffer, filename);

				memset(buffer, 0, sizeof(buffer));
				counter = 0;
				command_got = false;
				clear_screen();
				last_idx = -1;
				count_count = 0;

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

		buffer[counter++] = c;

		if (!command_got && strstr(buffer, COMMAND_QUIT)) {
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
				print_help_exit(argv[0]);
				break;
			case 0x101:
				filename = strdup(optarg);
				if (!filename) {
					errorf("strdup failed\n");
					goto fail;
				}
				break;
			default:
				debugf("unknown argument\n");
				goto fail;
		}
	}

	if (!filename) {
		errorf("file-name is mandatory\n");
		print_help_exit(argv[0]);
	}

	debugf("%s started\n", argv[0]);

	read_and_process(filename);

	goto out;

fail:
	ret = NOK;

out:
	restore_icanon(&oldt);
	debugf("\n");

	return ret;
}

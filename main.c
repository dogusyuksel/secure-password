#define _GNU_SOURCE
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
#include <pwd.h>
#include <time.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#include "main.h"

#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && argv[optind][0] != '-') \
     ? (bool) (optarg = argv[optind++]) \
     : (optarg != NULL))

static const unsigned char key[] = "01234567890123456789012345678901";
static const unsigned char iv[]  = "0123456789012345";

static char *set = NULL;
static char *data = NULL;
static char *dump = NULL;
static char *del = NULL;

static struct option parameters[] = {
	{ "help",				no_argument,		0,	'h'	},
	{ "version",			no_argument,		0,	'v'	},
	{ "name",				required_argument,	0,	'n'	},
	{ "pwd",				required_argument,	0,	'p'	},
	{ "dump",				optional_argument,	0,	'd'	},
	{ "remove",				required_argument,	0,	'r'	},
	{ NULL,					0,					0, 	0 	},
};

static int print_help_exit (void)
{
	printf("\n%s - ver: %s\n", APP_NAME, VERSION);
	printf("This application is used to save your passwords securily in your PC.\n");

	printf("\nparameters;\n\n");
	printf("\t%s--version%s \t(-v): show version\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	printf("\t%s--name%s    \t(-n): reminder keyword of your password. Eg: github_pwd\n\t\tRequires argument\n\t\tShould be used with '--pwd' argument\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	printf("\t%s--pwd%s     \t(-p): password itself\n\t\tRequires argument\n\t\tShould be used with '--name' argument\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	printf("\t%s--dump%s    \t(-d): dump passwords\n\t\tOptional argument\n\t\tIf used without any argument then prints all passwods.\n\t\tIf used with any string argument then prints all similar 'named' passwords\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	printf("\t%s--remove%s  \t(-r): remove specified 'named' password\n\t\tRequires argument\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);

	printf("\n");

	return OK;
}

static void sigint_handler(__attribute__((unused)) int sig_num)
{
	FREE(set);
	FREE(data);
	FREE(dump);
	FREE(del);
	ERR_free_strings();
	fcloseall();

	exit(NOK);
}

static int encrypt(unsigned char *plaintext, int plaintext_len,
			unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, ciphertext_len = 0;
	unsigned char tag[16];

	if (!plaintext || !ciphertext) {
		errorf("args cannot be NULL\n");
		goto fail;
	}

	if(!(ctx = EVP_CIPHER_CTX_new())) {
		errorf("fail\n");
		goto fail;
	}

	if(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
		errorf("fail\n");
		goto fail;
	}

	if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1) {
		errorf("fail\n");
		goto fail;
	}

	if(EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
		errorf("fail\n");
		goto fail;
	}

	if(EVP_EncryptUpdate(ctx, NULL, &len, (const unsigned char *)APP_NAME, (int)strlen(APP_NAME)) != 1) {
		errorf("fail\n");
		goto fail;
	}

	if(EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
		errorf("fail\n");
		goto fail;
	}

	ciphertext_len = len;

	if(EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
		errorf("fail\n");
		goto fail;
	}
	ciphertext_len += len;

	if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
		errorf("fail\n");
		goto fail;
	}

	EVP_CIPHER_CTX_free(ctx);

	goto out;

fail:
	ciphertext_len = -1;;

out:
	return ciphertext_len;
}

static int decrypt(unsigned char *ciphertext, int ciphertext_len, 
			unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, plaintext_len = 0, ret;
	unsigned char tag[16];

	if (!ciphertext || !plaintext) {
		errorf("args cannot be NULL\n");
		goto fail;
	}

	if(!(ctx = EVP_CIPHER_CTX_new())) {
		goto fail;
	}

	if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
		goto fail;
	}

	if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL)) {
		goto fail;
	}

	if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
		goto fail;
	}

	if(!EVP_DecryptUpdate(ctx, NULL, &len, (const unsigned char *)APP_NAME, (int)strlen(APP_NAME))) {
		goto fail;
	}

	if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
		goto fail;
	}

	plaintext_len = len;

	if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)) {
		goto fail;
	}

	ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

 	EVP_CIPHER_CTX_free(ctx);

	if(ret > 0) {
		plaintext_len += len;
	}

	goto out;

fail:
	plaintext_len = -1;

out:
	return plaintext_len;
}

static int create_file(const char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		return NOK;
	}

	fp = fopen(filename, "w+");
	if (fp == NULL) {
		errorf("fopen() failed with: %s for the path: %s\n", strerror(errno), filename);
		return NOK;
	}
	FCLOSE(fp);

	return OK;
}

static bool is_file_exists(const char *filename)
{
	FILE *fp = NULL;

	if (!filename) {
		return false;
	}

	fp = fopen(filename, "r");
	if (fp == NULL) {
		return false;
	}

	FCLOSE(fp);

	return true;
}

static int get_plain_text(const char *filename, unsigned char *decryptedtext)
{
	int ret = OK;
	FILE *fp = NULL;
	long fsize = 0;
	unsigned char *new_text = NULL;

	if (!filename || !decryptedtext) {
		errorf("args cannot be NULL\n");
		goto fail;
	}

	errno = 0;
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		errorf("fopen() failed with: %s for the path: %s\n", strerror(errno), filename);
		goto fail;
	}

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (!fsize) {
		debugf("fsize is zero, file is empty for now\n");
		goto fail;
	}

	debugf("new_text_size: %d\n", (int)fsize);

	new_text = calloc(1, fsize + 1);
	if (!new_text) {
		errorf("calloc failed\n");
		goto fail;
	}

	if (fsize != (long)fread(new_text, 1, fsize, fp)) {
		errorf("read error\n");
		goto fail;
	}
	debugf("read size: %d\n", (int)fsize);
	debugf("read text: %s\n", new_text);

 	if (decrypt(new_text, fsize, decryptedtext) <= 0) {
		errorf("decrypt failed\n");
		goto fail;
	}

	debugf("decrypted text: '%s'\n", decryptedtext);

	goto out;

fail:
	ret = NOK;

out:
	FCLOSE(fp);
	FREE(new_text);

	return ret;
}

static int set_text(const char *filename, unsigned char *plain_text)
{
	int ret = OK;
	FILE *fp = NULL;
	int ciphertext_len = 0;
	unsigned char ciphertext[MAX_FILE_SIZE];

	if (!filename || !plain_text) {
		errorf("args cannot be NULL\n");
		return NOK;
	}

	memset(ciphertext, 0, sizeof(ciphertext));

	debugf("plain_text: '%s' and len: '%d'\n", plain_text, (int)strlen((char *)plain_text));

	ciphertext_len = encrypt(plain_text, strlen((char *)plain_text), ciphertext);
	debugf("encrypted text: %s\n", ciphertext);

	if (ciphertext_len < 0) {
		errorf("encryption failed\n");
		goto fail;
	}

	if (!is_file_exists(filename)) {
		if(create_file(filename) == NOK) {
			errorf("create_file() failed\n");
			goto fail;
		}
	}

	errno = 0;
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		errorf("fopen() failed with: %s for the path: %s\n", strerror(errno), filename);
		goto fail;
	}

	if (strlen((char *)ciphertext) != fwrite(ciphertext, 1, strlen((char *)ciphertext), fp)) {
		errorf("write error\n");
		goto fail;
	}

	goto out;

fail:
	ret = NOK;

out:
	FCLOSE(fp);

	return ret;
}

static bool case_insen_strstr(char *str1, char *str2)
{
	int i = 0;
	int len = 0;

	if (!str1 || !str2) {
		return false;
	}

	len = strlen(str1);
	if (len > (int)strlen(str2)) {
		len = strlen(str2);
	}

	for (i = 0; i < len; i++) {
		if (tolower(str1[i]) != tolower(str2[i])) {
			return false;
		}
	}

	return true;
}

static int dump_keywords(const char *filename, char *includes)
{
	int ret = OK;
	unsigned char plain_text[MAX_FILE_SIZE] = {0};
	char *token = NULL, *token2 = NULL;
	char *rest = NULL, *rest2 = NULL;

	if (!filename) {
		errorf("args are wrong\n");
		return NOK;
	}

	memset(plain_text, 0, sizeof(plain_text));

	if (get_plain_text(filename, plain_text) == NOK) {
		errorf("password content cannot be read or its empty\n");
		goto fail;
	}

	for (token = strtok_r((char *)plain_text, NEWLINE, &rest); token != NULL; token = strtok_r(NULL, NEWLINE, &rest)) {
		if (token && strlen(token)) {
			debugf("token: %s len: %d\n", token, (int)strlen(token));
			token2 = strtok_r(token, CONFIG_SEPERATOR, &rest2);
			if (token2 && strlen(token2)) {
				debugf("token2: %s len: %d\n", token2, (int)strlen(token2));
				if (includes && case_insen_strstr(token2, includes)) {
					printf("%s\t%s\n", token2, rest2);
				} else if (!includes) {
					printf("%s\t%s\n", token2, rest2);
				}
			}
		}
	}

	goto out;

fail:
	ret = NOK;

out:

	return ret;
}

static int del_password(const char *filename, char *includes)
{
	int ret = OK;
	unsigned char plain_text[MAX_FILE_SIZE];
	char *plain_text_dup = NULL;
	char *token = NULL, *token2 = NULL;
	char *rest = NULL, *rest2 = NULL;

	if (!filename) {
		errorf("args are wrong\n");
		return NOK;
	}

	memset(plain_text, 0, sizeof(plain_text));

	if (get_plain_text(filename, plain_text) == NOK) {
		errorf("password content cannot be read or its empty\n");
		goto fail;
	}

	plain_text_dup = calloc(1, strlen((char *)plain_text) + 1);
	if (!plain_text_dup) {
		errorf("calloc failed\n");
		goto fail;
	}

	for (token = strtok_r((char *)plain_text, NEWLINE, &rest); token != NULL; token = strtok_r(NULL, NEWLINE, &rest)) {
		if (token && strlen(token)) {
			debugf("token: %s len: %d\n", token, (int)strlen(token));
			token2 = strtok_r(token, CONFIG_SEPERATOR, &rest2);
			if (token2 && strlen(token2)) {
				debugf("token2: %s len: %d\n", token2, (int)strlen(token2));
				if (includes && case_insen_strstr(token2, includes)) {
ask_again:
					printf("do you want to delete the %s? [Y/n] :", token2);
					char c = getc(stdin);
					if (c == 'Y') {
						continue;
					} else if (c != 'n') {
						goto ask_again;
					}
				}
				strcat(plain_text_dup, token2);
				strcat(plain_text_dup, CONFIG_SEPERATOR);
				strcat(plain_text_dup, rest2);
				strcat(plain_text_dup, NEWLINE);
				debugf("plain_text_dup: %s  len: %d\n", plain_text_dup, (int)strlen(plain_text_dup));
			}
		}
	}

	debugf("try to add plain_text_dup: %s\n", plain_text_dup);
	if (set_text(filename, (unsigned char *)plain_text_dup) == NOK) {
		errorf("set_text() failed\n");
		goto fail;
	}

	goto out;

fail:
	ret = NOK;

out:
	FREE(plain_text_dup);

	return ret;
}

static int set_password(const char *filename, const char *set, const char *data)
{
	int ret = OK;
	unsigned char plain_text[MAX_FILE_SIZE];
	char *plain_text_dup = NULL;
	char *token = NULL, *token2 = NULL;
	char *rest = NULL, *rest2 = NULL;
	enum user_answer answer_is_yes = NOT_ANSWERED;

	if (!filename || !set || !data) {
		errorf("args are wrong\n");
		return NOK;
	}

	if (!filename) {
		errorf("args are wrong\n");
		return NOK;
	}

	memset(plain_text, 0, sizeof(plain_text));

	if (get_plain_text(filename, plain_text) == NOK) {
		errorf("password content cannot be read, or empty\n");
	}
	debugf("plain_text: %s\n", plain_text);

	plain_text_dup = calloc(1, strlen((char *)plain_text) + strlen(set) + strlen(data) + strlen(CONFIG_SEPERATOR) + strlen(NEWLINE) + 1);
	if (!plain_text_dup) {
		errorf("calloc failed\n");
		goto fail;
	}

	for (token = strtok_r((char *)plain_text, NEWLINE, &rest); token != NULL; token = strtok_r(NULL, NEWLINE, &rest)) {
		if (token && strlen(token)) {
			debugf("token: %s  len: %d\n", token, (int)strlen(token));
			token2 = strtok_r(token, CONFIG_SEPERATOR, &rest2);
			if (token2 && strlen(token2)) {
				debugf("token2: %s  len: %d\n", token2, (int)strlen(token2));
				debugf("rest2: %s  len: %d\n", rest2, (int)strlen(rest2));
				if (strcmp(set, token2) == 0) {
ask_again1:
					printf("there is already saved password for %s\n", token2);
					printf("do you want to overwrite the old: %s with new: %s? [Y/n] :", rest2, data);
					char c = getc(stdin);
					if (c == 'Y') {
						answer_is_yes = ANSWERED_YES;
						continue;
					} else if (c == 'n') {
						answer_is_yes = ANSWERED_NO;
					} else {
						goto ask_again1;
					}
				}
				strcat(plain_text_dup, token2);
				strcat(plain_text_dup, CONFIG_SEPERATOR);
				strcat(plain_text_dup, rest2);
				strcat(plain_text_dup, NEWLINE);
				debugf("plain_text_dup: %s  len: %d\n", plain_text_dup, (int)strlen(plain_text_dup));
			}
		}
	}

	if (answer_is_yes == NOT_ANSWERED) {
ask_again2:
		printf("do you want to save %s for %s? [Y/n] :", data, set);
		char c = getc(stdin);
		if (c == 'Y') {
			//add new entry
			strcat(plain_text_dup, set);
			strcat(plain_text_dup, CONFIG_SEPERATOR);
			strcat(plain_text_dup, data);
			strcat(plain_text_dup, NEWLINE);
		} else if (c != 'n') {
			goto ask_again2;
		}
	} else if (answer_is_yes == ANSWERED_YES) {
		//add new entry
		strcat(plain_text_dup, set);
		strcat(plain_text_dup, CONFIG_SEPERATOR);
		strcat(plain_text_dup, data);
		strcat(plain_text_dup, NEWLINE);
	} else {
		goto out;
	}

	debugf("try to add plain_text_dup: %s\n", plain_text_dup);
	if (set_text(filename, (unsigned char *)plain_text_dup) == NOK) {
		errorf("set_text() failed\n");
		goto fail;
	}

	goto out;

fail:
	ret = NOK;

out:
	FREE(plain_text_dup);

	return ret;
}

static int sudo_check(void)
{
	if (!getuid() && !geteuid()) {
		return OK;
	}

	return NOK;
}

static int get_user_home_path(char *path, unsigned int len)
{
	FILE *p;
	size_t sz = 0;
	char username[MAX_LINE_SIZE] = {0};
	char buffer[MAX_LINE_SIZE] = {0};

	p = popen("pstree -lu -s $$ | grep --max-count=1 -o '([^)]*)' | head -n 1 | sed 's/[()]//g'", "r");
	if (!p) {
		errorf("Failed popen");
		return NOK;
	}

	sz = fread (username, sizeof(char), sizeof(username) - 1, p);
	if (ferror(p)) {
		errorf("pipe reading error");
		pclose(p);
		return NOK;
	}

	username[sz - 1] = '\0'; //eat \n
	debugf("username:: %s\n", username);
	pclose(p);

	if (!strlen(username)) {
		return NOK;
	}

	snprintf(buffer, MAX_LINE_SIZE, "eval echo ~%s", username);
	p = popen(buffer, "r");
	if (!p) {
		errorf("Failed popen");
		return NOK;
	}

	sz = fread (buffer, sizeof(char), sizeof(buffer) - 1, p);
	if (ferror(p)) {
		errorf("pipe reading error");
		pclose(p);
		return NOK;
	}

	buffer[sz - 1] = '\0'; //eat \n
	debugf("userhome:: %s\n", buffer);
	pclose(p);

	if (!strlen(buffer)) {
		return NOK;
	}

	strncpy(path, buffer, len);

	return OK;
}

static int config_file_check(char *filename)
{
	int ret = OK;
	FILE *fp;
	char *token = NULL;
	char buffer[MAX_LINE_SIZE] = {0};
	char home_path[MAX_LINE_SIZE] = {0};

	if (!filename) {
		errorf("arg cannot be NULL\n");
		return NOK;
	}

	memset(buffer, 0, MAX_LINE_SIZE);
	memset(home_path, 0, MAX_LINE_SIZE);

	if (get_user_home_path(home_path, MAX_LINE_SIZE) == NOK) {
		errorf("get_user_home_path() failed\n");
		return NOK;
	}

	snprintf(buffer, MAX_LINE_SIZE, "%s/.%s.conf", home_path, APP_NAME);
	debugf("buffer: %s\n", buffer);
	fp = fopen(buffer, "r");
	if (fp != NULL) {
		debugf("config file exists\n");

		memset(buffer, 0, sizeof(buffer));
		if (fgets(buffer, sizeof(buffer) - 1, fp)) {
			buffer[strcspn(buffer, "\n")] = 0;

			token = strtok(buffer, CONFIG_SEPERATOR);
			if (strcmp(token, FILE_LOCATION) != 0) {
				errorf("key is wrong\n");
				goto fail;
			}
			token = strtok(NULL, CONFIG_SEPERATOR);
			if (token == NULL) {
				errorf("FILE_LOCATION is null");
				goto fail;
			}
			strncpy(filename, token, MAX_LINE_SIZE);

			if (!is_file_exists(filename)) {
				if(create_file(filename) == NOK) {
					errorf("create_file() failed\n");
					goto fail;
				}
			}

			goto out;
		}

		goto fail;
	}

	errno = 0;
	fp = fopen(buffer, "w+");
	if (fp == NULL) {
		errorf("fopen() failed with: %s for the path: %s\n", strerror(errno), buffer);
		goto fail;
	}

	snprintf(buffer, MAX_LINE_SIZE, "%s%s%s/%s.pwd\n", FILE_LOCATION, CONFIG_SEPERATOR, home_path, APP_NAME);
	debugf("buffer: %s", buffer);
	if (strlen(buffer) != fwrite(buffer, 1, strlen(buffer), fp)) {
		errorf("write error\n");
		goto fail;
	}
	snprintf(filename, MAX_LINE_SIZE, "%s/%s.pwd", home_path, APP_NAME);

	if (!is_file_exists(filename)) {
		if(create_file(filename) == NOK) {
			errorf("create_file() failed\n");
			goto fail;
		}
	}

	goto out;

fail:
	ret = NOK;

out:
	FCLOSE(fp);
	return ret;
}

int main(int argc, char **argv)
{
	int ret = OK;
	int c, o;
	bool is_dump = false;
	char *set = NULL;
	char *data = NULL;
	char *dump = NULL;
	char *del = NULL;
	char filename[MAX_LINE_SIZE] = {0};

	signal(SIGINT, sigint_handler);

	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();   

	while ((c = getopt_long(argc, argv, "hvn:p:r:d::", parameters, &o)) != -1) {
		switch (c) {
			case 'h':
				return print_help_exit();
				break;
			case 'v':
				printf("%s version is %s\n", APP_NAME, VERSION);
				return OK;
				break;
			case 'n':
				set = strdup(optarg);
				if (!set) {
					errorf("strdup failed\n");
					goto fail;
				}
				break;
			case 'r':
				del = strdup(optarg);
				if (!del) {
					errorf("strdup failed\n");
					goto fail;
				}
				break;
			case 'd':
				is_dump = true;
				if (OPTIONAL_ARGUMENT_IS_PRESENT) {
					dump = strdup(optarg);
					if (!dump) {
						errorf("strdup failed\n");
						goto fail;
					}
				}
				break;
			case 'p':
				data = strdup(optarg);
				if (!data) {
					errorf("strdup failed\n");
					goto fail;
				}
				break;
			default:
				debugf("unknown argument\n");
				goto fail;
		}
	}

	if (sudo_check() == NOK) {
		errorf("you need to use this app with 'sudo'\n");
		goto fail;
	}

	if (config_file_check(filename) == NOK) {
		errorf("config_file_check() failed\n");
		goto fail;
	}
	if (!strlen(filename)) {
		errorf("filename could not be handled\n");
		goto fail;
	}
	debugf("proceed with the file: %s\n", filename);

	if (is_dump) {
		if (dump_keywords(filename, dump)) {
			errorf("dump_keywords() failed\n");
			goto fail;
		}

		goto out;
	}

	if (set && data) {
		if (set_password(filename, set, data)) {
			errorf("set_password() failed\n");
			goto fail;
		}

		goto out;
	}

	if (del) {
		if (del_password(filename, del)) {
			errorf("del_password() failed\n");
			goto fail;
		}

		goto out;
	}

	goto out;

fail:
	ret = NOK;

out:
	FREE(set);
	FREE(data);
	FREE(dump);
	FREE(del);
	ERR_free_strings();
	fcloseall();

	return ret;
}

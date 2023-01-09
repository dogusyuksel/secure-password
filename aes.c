#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#include "aes.h"
#include "main.h"

static const unsigned char key[] = "01234567890123456789012345678901";
static const unsigned char iv[] = "0123456789012345";

int encrypt(unsigned char *plaintext, int plaintext_len,
			unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, ciphertext_len = 0;
	unsigned char tag[16];

	if (!plaintext || !ciphertext) {
		errorf("args cannot be NULL\n");
		goto fail;
	}

	// memcpy(ciphertext, plaintext, plaintext_len);
	// return plaintext_len;

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

int decrypt(unsigned char *ciphertext, int ciphertext_len, 
			unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, plaintext_len = 0, ret;
	unsigned char tag[16];

	if (!ciphertext || !plaintext) {
		errorf("args cannot be NULL\n");
		goto fail;
	}
	// memcpy(plaintext, ciphertext, ciphertext_len);
	// return ciphertext_len;

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

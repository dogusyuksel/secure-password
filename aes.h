#ifndef AES_H_
#define AES_H_

int encrypt(unsigned char *plaintext, int plaintext_len, 
		unsigned char *ciphertext);

int decrypt(unsigned char *ciphertext, int ciphertext_len, 
		unsigned char *plaintext);

#endif /* AES_H_ */
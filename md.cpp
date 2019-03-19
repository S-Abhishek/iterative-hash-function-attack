#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
using namespace std;

// in bytes
#define HASH_SIZE 4
#define MSG_BLOCK 4

#define K 16

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext){
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv))
    handleErrors();

  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  // if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  // ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext){
  
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

int padded_length(int l, int k){
  return l + k - l%k;
}


// Pad given plain text of length l to k bytes
void pad(unsigned char* plain_text, int l, int k, unsigned char* newplain){

  int i;
  int pad_len = k - l%k;
  // unsigned char* newplain = new unsigned char[l + pad_len]();
  copy(plain_text, plain_text + l, newplain);
  
  for( i = l; i < l + pad_len; i++ )
    newplain[i] = (unsigned char)pad_len;
}

void characters(long long b, unsigned char* str)
{
    str[3] = b & 0xFF;
    b = b >> 8;
    str[2] = b & 0xFF;
    b = b >> 8;
    str[1] = b & 0xFF;
    b = b >> 8;
    str[0] = b & 0xFF;
    b = b >> 8;
} 

void iterative_hash(unsigned char *plain_text, unsigned char *initial_hash, unsigned char *h0){
    
    int i;
    int length_message = strlen((char *)plain_text);
    int no_of_blocks = length_message/MSG_BLOCK;

    unsigned char* iv = new unsigned char[16]();
    // unsigned char* h0 = new unsigned char[HASH_SIZE]();
    unsigned char* h1 = new unsigned char[HASH_SIZE]();

    // Buffers to hold the padded input and obtained output
    unsigned char* h0_temp = new unsigned char[16]();
    unsigned char* h1_temp = new unsigned char[16]();

    unsigned char* mblock = new unsigned char[16]();
    
    copy(initial_hash, initial_hash + HASH_SIZE, h0);
    for(i = 0 ; i < no_of_blocks ; i++)
    {
        copy(plain_text + i*MSG_BLOCK, plain_text + (i+1)*MSG_BLOCK, mblock);

        // Padding the input to 128 bits
        pad(h0, HASH_SIZE, 16, h0_temp);

        // Encryption with the padded values
        encrypt(mblock, 16, h0_temp, iv, h1_temp );

        // Dropping bits 
        copy(h1_temp, h1_temp + HASH_SIZE, h0);
        BIO_dump_fp (stdout, (const char *)h0, 4);
    }

    delete iv;
    delete h1;
    delete h0_temp;
    delete h1_temp;
    delete mblock;
}

int main (void){

  unsigned char* plaintext = (unsigned char *)"12345845784534757767664566834";
  unsigned char* initial_hash = (unsigned char *)"aaaa";
  unsigned char* padded = new unsigned char[padded_length(strlen((char*)plaintext), 4)];

  unsigned char* final_hash = new unsigned char[4]();

  pad(plaintext, strlen ((char *)plaintext), 4, padded);
  iterative_hash(padded, initial_hash, final_hash);
  BIO_dump_fp (stdout, (const char*)final_hash, 4);
}

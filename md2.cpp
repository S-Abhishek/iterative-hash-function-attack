#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <array>
#include <map>
using namespace std;

// in bytes
#define HASH_SIZE 4
#define K 2
#define MSG_BLOCK 4

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

// Pad given plain text of length l to k bytes
unsigned char* pad(unsigned char* plain_text, int l, int k){

  int i;
  int pad_len = k - l%k;
  unsigned char* newplain = new unsigned char[l + pad_len]();

  copy(plain_text, plain_text + l, newplain);
  
  for( i = l; i < l + pad_len; i++ )
    newplain[i] = (unsigned char)pad_len;
  
  return newplain;
}


unsigned char* iterative_hash(unsigned char *plain_text, unsigned char *initial_hash){
    
    int i;
    int length_message = strlen((char *)plain_text);
    int no_of_blocks = length_message/MSG_BLOCK;

    unsigned char* iv = new unsigned char[16]();
    unsigned char* h0 = new unsigned char[HASH_SIZE]();
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
        h0_temp = pad(h0, HASH_SIZE, 16);

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
    
    return h0;
}
unsigned char* characters(long long b){

    unsigned char* str = new unsigned char[4]();
    str[3] = (char) b & 0xFF;
    b = b >> 8;
    str[2] = (char) b & 0xFF;
    b = b >> 8;
    str[1] = (char) b & 0xFF;
    b = b >> 8;
    str[0] = (char) b & 0xFF;
    b = b >> 8;
    return str;
}



void brute_force(unsigned char *hash1, unsigned char *hash2){
    

    map<string,string> collision;
    for(long long i = 0 ; i < ((long long)1<<32) ; i++)
    {
        unsigned char *plaintext = characters(i);
        string plaintext_s(reinterpret_cast<char *>(plaintext));
        plaintext = pad(plaintext, strlen ((char *)plaintext), 4);
        unsigned char *h1 = iterative_hash(plaintext, hash1); 
        string h1_s(reinterpret_cast<char *>(h1));
        map<string,string>::iterator pos1;
        pos1 = collision.find(h1_s);
        if(pos1 == collision.end())
        {
          collision[h1_s] = plaintext_s;
        }
        else
        {
          cout<<"Collision found:"<<endl;
          BIO_dump_fp (stdout, plaintext_s.c_str(), 4);
          cout<<endl;
          BIO_dump_fp (stdout, (pos1->second).c_str(), 4);
          cout<<endl;
          BIO_dump_fp (stdout, h1_s.c_str(), 4);
          cout<<endl;
          break;
        }
        
        unsigned char *h2 = (unsigned char *)iterative_hash(plaintext, hash2); 
        string h2_s(reinterpret_cast<char *>(h2));
        map<string,string>::iterator pos2;
        pos2 = collision.find(h2_s);
        if(pos2 == collision.end())
        {
          collision[h2_s]=plaintext_s;
        }
        else
        {
          cout<<"Collision found:"<<endl;
          BIO_dump_fp (stdout, (pos2->second).c_str(), 4);
          cout<<endl;
          BIO_dump_fp (stdout, plaintext_s.c_str(), 4);
          cout<<endl;
          BIO_dump_fp (stdout, h2_s.c_str(), 4);
          cout<<endl;
          break;
          
        }
        


    }

    
}

int main (void){

  //unsigned char* plaintext = (unsigned char *)"12345845784534757767664566834";
  //unsigned char* initial_hash = (unsigned char *)"aaaa";
  
  //plaintext = pad(plaintext, strlen ((char *)plaintext), 4);
  //BIO_dump_fp (stdout, (const char *)iterative_hash(plaintext, initial_hash), 4);

  unsigned char *hash1 = (unsigned char *)"abcd";
  unsigned char *hash2 = (unsigned char *)"efgh";
  brute_force(hash1,hash2);


}
#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <cstring>
#include "utils.h"
using namespace std;

// Pad given plain text of length l to k bytes
unsigned char* pad(unsigned char* plain_text, int l, int k){

  int pad_len = k - l%k;
  unsigned char* newplain = new unsigned char[l + pad_len]();
  copy(plain_text, plain_text + l, newplain);
  int i;
  for( i = l; i < l + pad_len; i++ )
    newplain[i] = (unsigned char)pad_len;

  return newplain;
}


unsigned char* iterative_hash(unsigned char *plain_text, unsigned char *initial_hash){
    
    int i;
    int length_message = strlen((char *)plain_text); 
    int no_of_blocks = length_message/16;
    unsigned char* iv = new unsigned char[16]();

    unsigned char* h0 = new unsigned char[32]();
    copy(initial_hash, initial_hash + 32, h0);

    unsigned char* h1 = new unsigned char[32]();

    for(i = 0 ; i < no_of_blocks ; i++)
    {
        string message_block(plain_text+i*16, plain_text+(i+1)*(16));
        encrypt((unsigned char*)message_block.c_str(), strlen((char *)message_block.c_str()), h0, iv, h1 );
        copy(h1, h1 + 32, h0);
    }

    return h0;
}

int main (void)
{
  unsigned char* plaintext = (unsigned char *)"123458457845847577676645678";
  unsigned char* initial_hash = (unsigned char *)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  
  plaintext = pad(plaintext, strlen ((char *)plaintext), 16);
  BIO_dump_fp (stdout, (const char *)iterative_hash(plaintext, initial_hash), 32);
}
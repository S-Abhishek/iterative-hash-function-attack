#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <fstream> 
#include <limits>
#include <omp.h>
#include <random>
#include <unistd.h>
#include <unordered_map> 
#include <unordered_set>

using namespace std;

// in bytes
#define HASH_SIZE 4
#define MSG_BLOCK 4
#define COMPARISON_BATCH 1024
#define GLUE_SIZE 4

#define K 10

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

long long one = 1;
long long MAX = (long long) 1 << 32;


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

void to_chars(long long b, unsigned char* str){
    str[3] = b & 0xFF;
    b = b >> 8;
    str[2] = b & 0xFF;
    b = b >> 8;
    str[1] = b & 0xFF;
    b = b >> 8;
    str[0] = b & 0xFF;
}

long long to_long(unsigned char* str){

    long long b = 0;
    b += str[0];
    b = b << 8;
    b += str[1];
    b = b << 8;
    b += str[2];
    b = b << 8;
    b += str[3];

    // delete str;
    return b;
}

void iterative_hash(unsigned char *plain_text, int length_message, unsigned char *initial_hash, unsigned char *h0){
    
    int i;
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
        // BIO_dump_fp (stdout, (const char *)h0, 4);
    }

    delete iv;
    delete h1;
    delete h0_temp;
    delete h1_temp;
    delete mblock;
}

void GotoLine(fstream& file, unsigned int num){
    file.seekg(ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(numeric_limits<streamsize>::max(),'\n');
    }
}

int parent(int index)
{
  return (((one<<(10+1)) + index)/2);
}

int main (void){

  fstream in("data/hash_ds.txt");
  fstream min("data/msg_ds.txt");

  int num_hashes = 2047;

  unordered_set<long long> hash_set;
  long long hash_ds[num_hashes];
  long long msg_ds[num_hashes];

  long long hash, mesg;

  for(int i = 0; i < num_hashes; i++){
      in>>hash;
      min>>mesg;

      hash_ds[i] = hash;
      msg_ds[i] = mesg;
      hash_set.insert(hash);
  }

  in.close();
  min.close();

  unsigned char* msg = (unsigned char*)"Hello there, This is Johnathan. This is an important message. Please keep this safe";
  int len = strlen((char*)msg);
  int newlen = padded_length(len + GLUE_SIZE, MSG_BLOCK);
  bool found = false;
  long long found_glue;
  long long found_hash;

  #pragma omp parallel
  {
    // Initiaze buffers for each thread here

    // Initial hash = 0
    unsigned char* hash1_buffer = new unsigned char[HASH_SIZE]();
    unsigned char* padded_msg = new unsigned char[newlen];
    pad(msg, len + GLUE_SIZE, MSG_BLOCK, padded_msg);

    unsigned char* final_hash = new unsigned char[HASH_SIZE]();

    #pragma omp for
    for( long long j = 0; j < (one << 32); j++)
    {
      if(found) continue;
      #pragma omp critical
      if( j%(one << 23) == 0 ) cout<<"current :"<<j<<endl;
      

      long long glue = j;

      // Pasting glue at end of message
      to_chars(glue, padded_msg + len);
      iterative_hash(padded_msg, newlen, hash1_buffer, final_hash);

      // Searching through hash set
      long long final_hash_val = to_long(final_hash);
      auto res = hash_set.find(final_hash_val);
      if( res != hash_set.end() ){
          #pragma omp critical
          {
              cout<<"Glue found :"<<glue<<", hashes to "<<*res<<endl;
              found_glue = glue;
              found_hash = final_hash_val;
          }

          found = true;
      }
    }

  }
  
  // Find index of hash
  uint index = -1;
  for(int i = 0; i < num_hashes; i++){
    if(hash_ds[i] == found_hash)
      index = i;
  }

  uint level = index / (one << 10);
  
  // Old msg + glue + m1 + m2 . . . 
  int final_len = newlen + GLUE_SIZE +  (10 - 1 - level)*MSG_BLOCK;
  unsigned char* newmsg = new unsigned char[final_len];
  
  copy(msg, msg + len, newmsg);
  pad(msg, len + GLUE_SIZE, MSG_BLOCK, newmsg);
  to_chars(found_glue, newmsg + len);

  unsigned char* final_hash = new unsigned char[HASH_SIZE]();
  unsigned char* initial_hash = new unsigned char[HASH_SIZE]();
  int i = 0;
  
  while(index < (one << 10 + 1) - 2){
    to_chars(msg_ds[index], newmsg + newlen + i*MSG_BLOCK);
    iterative_hash(newmsg, newlen + (i+1)*MSG_BLOCK, initial_hash, final_hash);
    cout<<"Adding message block "<<msg_ds[index]<<", New hash :"<<to_long(final_hash)<<endl;
    i++;
    index = parent(index);
  }  

  cout<<endl<<"Final message length : "<<newlen + i*MSG_BLOCK<<endl;
  iterative_hash(newmsg, newlen + i*MSG_BLOCK, initial_hash, final_hash);
  cout<<"The new message"<<endl;
  BIO_dump_fp (stdout, (const char *)newmsg, newlen + i*MSG_BLOCK);
  cout<<endl<<"The final hash "<<to_long(final_hash)<<endl;
}

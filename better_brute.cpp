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

using namespace std;

// in bytes
#define HASH_SIZE 4
#define MSG_BLOCK 4
#define COMPARISON_BATCH 1024

#define K 16

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

long long message_ds[2047];
long long hash_ds[2047];

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

void iterative_hash(unsigned char *plain_text, unsigned char *initial_hash, unsigned char *h0){
    
    int i;
    int length_message = MSG_BLOCK;
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

void construct(int num_hashes,int offset,int level)
{
  random_device mno;
  mt19937 rng(mno());
  uniform_int_distribution<mt19937::result_type> dist(0,MAX);
  
  string filename = "msgouto"+to_string(level);
  
  fstream out(filename, fstream::out);
  
  
  // export OMP_NUM_THREADS=<number of threads to use>.
  #pragma omp parallel
  {

    // Initiaze buffers for each thread here
    unsigned char* hash1_buffer = new unsigned char[HASH_SIZE]();
    unsigned char* hash2_buffer = new unsigned char[HASH_SIZE]();

    unsigned char* final_hash = new unsigned char[HASH_SIZE]();
    unsigned char* msg1_buffer = new unsigned char[MSG_BLOCK]();
    unsigned char* msg2_buffer = new unsigned char[MSG_BLOCK]();
    // unsigned char* op1 = new unsigned char[HASH_SIZE]();

    // for each pair of hash
    #pragma omp for
    for(int i = offset ; i < offset + num_hashes; i += 2){
        
      long hash1 = hash_ds[i];
      long hash2 = hash_ds[i+1];

      #pragma omp critical
      cout<<hash1<<" "<<hash2<<" "<<i<<" "<<i+1<<endl;

      to_chars(hash1, hash1_buffer);
      to_chars(hash2, hash2_buffer);

      // Message map to store 
      unordered_map<long long, long long> msg1_map;
      for(int i = 0; i < COMPARISON_BATCH; i++ ){

          // Generate a random messgae
          long long msg1 = dist(rng);
          to_chars(msg1, msg1_buffer);

          // Find its hash
          iterative_hash(msg1_buffer, hash1_buffer, final_hash);

          // Insert (hash, msg) into map
          msg1_map.insert( {to_long(final_hash), msg1} );
      }


      // Try 2^32 random messages
      for( long long j = 0; j < (one << 32); j++)
      {
        #pragma omp critical
        if( j%(one << 23) == 0 ) cout<<"current :"<<j<<endl;
        
        // Select another random message
        // do{
        //     msg2 = dist(rng);
        // }
        // while(msg1 == msg2);
        long long msg2 = j;
        to_chars(msg2, msg2_buffer);

        // Calculate hash for the message with initial hash hash2
        iterative_hash(msg2_buffer, hash2_buffer, final_hash);
        long long final_hash_val = to_long(final_hash);
        auto res = msg1_map.find(final_hash_val);
        if( res != msg1_map.end() ){
          
          // Write results to file -> hash1, message1, hash2, message2, colliding hash
          //#pragma omp critical
          //out<<hash1<<","<<res->second<<","<<hash2<<","<<msg2<<","<<final_hash_val<<endl;
          #pragma omp critical
          {
            hash_ds[parent(i)] = final_hash_val;
            //cout<<parent(i)<<" "<<hash_ds[parent(i)]<<" hash value inserted"<<endl;
            message_ds[i] = res->second;
            message_ds[i+1] = msg2;
            cout<<hash1<<","<<res->second<<","<<hash2<<","<<msg2<<","<<final_hash_val<<endl;
            out<<hash1<<","<<res->second<<","<<hash2<<","<<msg2<<","<<final_hash_val<<endl;

          }
          //#pragma omp critical
          
          break;
        }
        // else
        // {
        //     BIO_dump_fp (stdout, (const char*)hash2_buffer, 4);
        //     BIO_dump_fp (stdout, (const char*)final_hash, 4);
        //     cout<<endl;
        // }

      }

    }

    // Cleanup
    delete hash1_buffer;
    delete hash2_buffer;
    delete msg1_buffer;
    delete msg2_buffer;
    delete final_hash;
  }
}



int main (void){

  fstream in("message.txt");
  fstream out("msgout2.txt", fstream::out);

  int num_hashes = 1024;
  int start_line = 1;
  int k = 10;


  // Seeking to specified line
  GotoLine(in, start_line);

  // Reading hashes from file and storing in array
  long long hash;
  for(int i = 0; i < num_hashes; i++){
      in>>hash;
      hash_ds[i] = hash;
      cout<<hash_ds[i]<<endl;
    }
  int offset = 0;
  
  for(int i = 0 ; i < k ; i++)
  {
    if(i!=0)
    {
      //cout<<"before value "<<offset<<endl;
      offset = offset + (one<<(k-i+1)); //Mistake here, it computed (offset + one)<<(k-i+1) instead of (offset) + (one<<(k-i+1))
      //cout<<(one<<(k-i+1))<<"is added to offset "<<(k-i+1)<<" final value"<<offset<<endl;
    }
    //cout<<offset<<" is the offset num of hashes: "<<num_hashes<<endl;
    construct(num_hashes,offset,i);
    num_hashes = num_hashes/2;
  }
  
  
  
  

}

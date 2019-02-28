#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
using namespace std;

char *iterative_hash(unsigned char *plain_text,unsigned char *h)
{
    
    for(int i = 0 ; i < (strlen((char *)plain_text)/128) ; i++)
    {
        string message_block(i*128,i*(128)*(2)-1);
        h = encrypt(plain_text,strlen((char *)plain_text,h);
    }
    if((plain_text)%128 != 0)
    {
        string message_block(i*128);
        pad(message_block);
        h = encrypt(plain_text,strlen((char *)plain_text,h));
    }
    return h;
}

int main()
{
    unsigned char *h = (unsigned char *)"0123456789012345";  //Cast to char * , normally strings cannot be converted to char *
    unsigned char *plain_text = (unsigned char *)"He my polite be object oh change. Consider no mr am overcame yourself throwing sociable children. Hastily her totally conduct may. My solid by stuff first smile fanny. Humoured how advanced mrs elegance sir who. Home sons when them dine do want to. Estimating themselves unsatiable imprudence an he at an. Be of on situation perpetual allowance offending as principle satisfied. Improved carriage securing are desirous too.";
    unsigned char *iv = (unsigned char *)"0000000000000000";
    unsigned char hash_value[128];
    
    //cout<<strlen((char *)h);

    hash_value = iterative_hash(plain_text, h);
    cout<<hash_value;

    
    return 0;
}
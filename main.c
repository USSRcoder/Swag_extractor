//cl main.c crc16.c lh1_decoder.c lha_decoder.c
#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef uint8_t u8; 

#include "public/lha_decoder.h"

static u8   *lha_decoder_in;
static u8   *lha_decoder_inl;

static size_t myLHADecoderCallback(void *buf, size_t buf_len, void *user_data) {
	if(buf_len > (lha_decoder_inl - lha_decoder_in)) buf_len = (lha_decoder_inl - lha_decoder_in);
	memcpy(buf, lha_decoder_in, buf_len);
    //printf("buflen=%d\n", buf_len);
	lha_decoder_in += buf_len;
	return buf_len;
}

int lha_decoder(u8 *in, int insz, u8 *out, int outsz, u8 *type) {
    lha_decoder_in  = in;
    lha_decoder_inl = in + insz;
    u8 *o   = out;
    u8 *ol  = out + outsz;

    LHADecoder *ctx;
    LHADecoderType  *lha_type;

    if(!type) type = "-lh1-";
    lha_type = lha_decoder_for_name(type);
    if(!lha_type) return -1;

    ctx = lha_decoder_new(lha_type, myLHADecoderCallback, NULL, outsz);
    if(!ctx) return -2;

    int     len;
    for(;;) {
        len = lha_decoder_read(ctx, o, ol - o);
        if(!len) break;
        o += len;
    }

    lha_decoder_free(ctx);

    return o - out;
}

#pragma pack(1)
typedef struct SwagHeader{
    unsigned char      HeadSize          ; //{ size of header }
    unsigned char      HeadChk           ; //{ checksum for header }
    char               HeadID[5]         ; //{ compression type tag }
    unsigned int       NewSize           ; //{ compressed size }
    unsigned int       OrigSize          ; //{ original size }
    unsigned short     Time              ; //{ packed time }
    unsigned short     Date              ; //{ packed date }
    unsigned short     Attr              ; //{ file attributesand flags }
    unsigned int       BufCRC            ; //{ 32 - CRC of the Buffer }
    char               Swag[12]          ; //{ stored SWAG filename }
    char               Subject[40]       ; //{ snipet subject }
    char               Contrib[35]       ; //{ contributor }
    char               Keys[70]          ; //{ search keys, comma deliminated }
    unsigned short     PathStrLen        ;
    char               PathStr[_MAX_PATH]; //{ filename(variable length) }
    unsigned short     CRC               ; //{ 16 - bit CRC(immediately follows FName) }
} SwagHeader                             ;
#pragma pack(pop)

int fpos(FILE* _Stream){
    fpos_t pos;
    fgetpos(_Stream, &pos);
    return pos;
}

int main(int argc, char* argv[]) {

    char input[_MAX_PATH] = "default.SWG";
    char output[_MAX_PATH] = "outputDir";

    if (argc >= 2){
        if (strcmp(argv[1],"-?") == 0 ||
            strcmp(argv[1], "/?") == 0 ||
            strcmp(argv[1], "--h") == 0) {
                printf("Usage: %s [%s] [%s]\n", argv[0], input, output);
                return 0;
        }
    }

    if (argc >= 2) {
        
        strncpy(input, argv[1], strlen(argv[1]));
        input[strlen(argv[1])] = 0;
    }

    if (argc >= 3){
        
        strncpy(output, argv[2], strlen(argv[2]));
        output[strlen(argv[2])] = 0;
    }

    
    int index = 0;
    FILE* f = fopen(input, "rb");
    if (f == 0) {
        printf("Error opening %s\n", input);
        return 1;
    } else {
        printf("Open %s:\n", input);
    }

    CreateDirectoryA(output, NULL);
    //size_t swg_size = fgetsize(f);
    do {
        fseek(f, index, SEEK_SET);
        SwagHeader hdr = {0};

        fread(&hdr,offsetof(SwagHeader, Swag),1,f);
        if (hdr.HeadSize == 0)
        //if (index > swg_size)
            break;

        unsigned char l = 0;
        fread(&l, 1, 1, f);
        fread(&hdr.Swag[0], l, 1, f);
        fseek(f, 12-l, SEEK_CUR);

        l = 0;
        fread(&l, 1, 1, f);
        fread(&hdr.Subject[0], l, 1, f);
        fseek(f, 40 - l, SEEK_CUR);

        l = 0;
        fread(&l, 1, 1, f);
        fread(&hdr.Contrib[0], l, 1, f);
        fseek(f, 35 - l, SEEK_CUR);

        l = 0;
        fread(&l, 1, 1, f);
        fread(&hdr.Keys[0], l, 1, f);
        fseek(f, 69 - l, SEEK_CUR);

        fread(&hdr.PathStrLen, 2, 1, f);
        hdr.PathStrLen = _byteswap_ushort(hdr.PathStrLen);
        fread(&hdr.PathStr[0], hdr.PathStrLen, 1, f);

        fread(&hdr.CRC, 2, 1, f);


        u8* in = calloc(1, hdr.NewSize + 1);
        fread(in, hdr.NewSize, 1, f);

        printf("lh1: %d\n", hdr.NewSize);

        u8* out = calloc(1, hdr.OrigSize + 1);
        int size = hdr.OrigSize;
        size = lha_decoder(in, hdr.NewSize, out, size, "-lh1-");

        printf("unp: %d\n", size);
        //printf("%s\n", out);
        //printf("HeadSize       : %d\n", hdr.HeadSize);
        //printf("HeadChk        : %d\n", hdr.HeadChk);
        //printf("HeadID[5]      : %.5s\n", hdr.HeadID);
        //printf("CompSize       : %d\n", hdr.NewSize);
        //printf("OrigSize       : %d\n", hdr.OrigSize);
        //printf("Time           : %d\n", hdr.Time);
        //printf("Date           : %d\n", hdr.Date);
        //printf("Attr           : %d\n", hdr.Attr);
        //printf("BufCRC         : %d\n", hdr.BufCRC);
        printf("[12]Swag       : %s\n", hdr.Swag);
        printf("[40]Subject    : %s\n", hdr.Subject);
        printf("[35]Contrib    : %s\n", hdr.Contrib);
        printf("[70]Keys       : %s\n", hdr.Keys);
        //printf("PathStrLen     : %d\n", hdr.PathStrLen);
        printf("PathStr        : %s\n", hdr.PathStr);
        //printf("CRC            : %d\n", hdr.CRC);
        
        char output_fn[MAX_PATH]  = {0};
        strncpy(output_fn,output,strlen(output));
        strcat(output_fn,"\\");
        strcat(output_fn, hdr.PathStr);
        printf("Extracrting to : %s\n", output_fn);

        FILE* f = fopen(output_fn, "a+b");
        //printf(":%d\n", fwrite(out, size, 1, f) );
        fwrite(out, size, 1, f);
        fclose(f);

        free(out);
        free(in);
        printf(" --- \n");
        index = index + hdr.HeadSize + hdr.NewSize + 2;
    } while (1);
    fclose(f);

    return 0;
}

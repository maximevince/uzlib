/*
 * tgunzip  -  gzip decompressor example
 *
 * Copyright (c) 2003 by Joergen Ibsen / Jibz
 * All Rights Reserved
 *
 * http://www.ibsensoftware.com/
 *
 * Copyright (c) 2014-2016 by Paul Sokolovsky
 *
 * This software is provided 'as-is', without any express
 * or implied warranty.  In no event will the authors be
 * held liable for any damages arising from the use of
 * this software.
 *
 * Permission is granted to anyone to use this software
 * for any purpose, including commercial applications,
 * and to alter it and redistribute it freely, subject to
 * the following restrictions:
 *
 * 1. The origin of this software must not be
 *    misrepresented; you must not claim that you
 *    wrote the original software. If you use this
 *    software in a product, an acknowledgment in
 *    the product documentation would be appreciated
 *    but is not required.
 *
 * 2. Altered source versions must be plainly marked
 *    as such, and must not be misrepresented as
 *    being the original software.
 *
 * 3. This notice may not be removed or altered from
 *    any source distribution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/*

    This example is a use case for esp8266 where all compressed data is accessible from flash
    and are inflated to flash (like in OTA operations).
    Flash constraints are:
    - only 32 bit access
    - wear leveling: reduce the number of write on flash blocks (512 bytes in this example)
    - not much ram (dictionary disabled: uncompressed data = flash are used for dictionary)

    This example uncompresses data to a temporary 512 bytes block, and move
    it to flash everytime it is full.

    The align_read() function goes get data from already inflated data from
    flash when they are outside from the 512bytes temp buffer.

*/


#define NO_DICT 1
#define NO_CB 1

#define TMPSZ 512
unsigned char tmp [TMPSZ];

unsigned int len, outlen;
const unsigned char *source;
unsigned char *dest;
off_t tmpshift;

#define FLASH 1

#define WITHIN(p,start,len) ((((p) - (start)) >= 0) && (((start) - (p)) + (len) > 0))

#if FLASH

unsigned char* flashed;

unsigned char align_read (const unsigned char* s)
{
	if (WITHIN(s, flashed, TMPSZ))
		return *(s - flashed + tmp);

	assert(WITHIN(s, source, len) || WITHIN(s, dest, flashed - dest));
	
	return *s;
}

void align_write (unsigned char* d, unsigned char v)
{
	// ensure we are always writing in tmp
	assert(((long)d - (long)tmp) >= 0 && ((long)tmp + TMPSZ - (long)d) < outlen);

	tmp[d - dest - tmpshift] = v;
}

#define ALIGN_READ(x) align_read(x)
#define ALIGN_WRITE(x,y) align_write((x), (y))

#else
#pragma message "NO FLASH"
#endif
	
#include "adler32.c"
#include "crc32.c"

#include "tinflate.c"
#include "tinfgzip.c"

/* produce decompressed output in chunks of this size */
/* defauly is to decompress byte by byte; can be any other length */
#define OUT_CHUNK_SIZE TMPSZ

void exit_error(const char *what)
{
   printf("ERROR: %s\n", what);
   exit(1);
}

int main(int argc, char *argv[])
{
    FILE *fin, *fout;
    unsigned int dlen;
    int res;

    printf("tgunzip - example from the tiny inflate library (www.ibsensoftware.com)\n\n");

    if (argc < 3)
    {
       printf(
          "Syntax: tgunzip <source> <destination>\n\n"
          "Both input and output are kept in memory, so do not use this on huge files.\n");

       return 1;
    }

    uzlib_init();

    /* -- open files -- */

    if ((fin = fopen(argv[1], "rb")) == NULL) exit_error("source file");

    if ((fout = fopen(argv[2], "wb")) == NULL) exit_error("destination file");

    /* -- read source -- */

    fseek(fin, 0, SEEK_END);

    len = ftell(fin);

    fseek(fin, 0, SEEK_SET);

    source = (unsigned char *)malloc(len);

    if (source == NULL) exit_error("memory");

    if (fread((unsigned char*)source, 1, len, fin) != len) exit_error("read");

    fclose(fin);

    if (len < 4) exit_error("file too small");

    /* -- get decompressed length -- */

    dlen =            source[len - 1];
    dlen = 256*dlen + source[len - 2];
    dlen = 256*dlen + source[len - 3];
    dlen = 256*dlen + source[len - 4];

    outlen = dlen;

    /* there can be mismatch between length in the trailer and actual
       data stream; to avoid buffer overruns on overlong streams, reserve
       one extra byte */
    dlen++;

    dest = (unsigned char *)malloc(dlen);

    if (dest == NULL) exit_error("memory");
    
#if !NO_DICT || !NO_CB
#error
#endif

    /* -- decompress data -- */

    struct uzlib_uncomp d;
    uzlib_uncompress_init(&d);

    /* all 2 fields below must be initialized by user */
    d.source = source;
    d.source_limit = source + len - 4;

    res = uzlib_gzip_parse_header(&d);
    if (res != TINF_OK) {
        printf("Error parsing header: %d\n", res);
        exit(1);
    }

    d.dest_start = d.dest = dest;
    
#if FLASH
    flashed = dest;
#endif

    while (dlen) {
        unsigned int chunk_len = dlen < OUT_CHUNK_SIZE ? dlen : OUT_CHUNK_SIZE;
#if FLASH
	tmpshift = d.dest - dest;
#endif
        d.dest_limit = d.dest + chunk_len;
        res = uzlib_uncompress(&d);
        dlen -= chunk_len;
        
	//printf("CHUNK\n");
#if FLASH
        // FLASH CHUNK - copy tmp to realdest
        memcpy(flashed, tmp, chunk_len);
        flashed += chunk_len;
#endif
        
        if (res != TINF_OK) {
            break;
        }
    }

    if (res != TINF_DONE) {
        printf("Error during decompression: %d\n", res);
        exit(-res);
    }

    printf("decompressed %lu bytes\n", d.dest - dest);

    /* -- write output -- */

    fwrite(dest, 1, outlen, fout);

    fclose(fout);

    return 0;
}

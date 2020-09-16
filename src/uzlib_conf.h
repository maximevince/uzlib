/*
 * uzlib  -  tiny deflate/inflate library (deflate, gzip, zlib)
 *
 * Copyright (c) 2014-2018 by Paul Sokolovsky
 */

#ifndef UZLIB_CONF_H_INCLUDED
#define UZLIB_CONF_H_INCLUDED

#ifndef UZLIB_CONF_DEBUG_LOG
/* Debug logging level 0, 1, 2, etc. */
#define UZLIB_CONF_DEBUG_LOG 0
#endif

#ifndef UZLIB_CONF_PARANOID_CHECKS
/* Perform extra checks on the input stream, even if they aren't proven
   to be strictly required (== lack of them wasn't proven to lead to
   crashes). */
#define UZLIB_CONF_PARANOID_CHECKS 0
#endif

/* Custom TINF_DEST_PUTC / TINF_DEST_GETC implementation */

//#define TINF_DEST_PUTC(c) do { *d->dest = c; } while(0)
#define TINF_DEST_PUTC uzlib_dest_putc
extern void uzlib_dest_putc(uint8_t c);

//#define TINF_DEST_GETC(offset) ({ char ret = d->dest[offset]; ret; })
#define TINF_DEST_GETC uzlib_dest_getc
extern uint8_t uzlib_dest_getc(const uint8_t *addr);

#endif /* UZLIB_CONF_H_INCLUDED */

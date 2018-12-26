/*
 * rift.h
 *
 * Copyright (C) 2018 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef RIFT_H
#define RIFT_H

#include <stdlib.h>

typedef struct {
        char *buf;
        size_t siz;
        size_t lb;
        size_t rb;
} rift_buffer_t;


typedef enum {
    RIFT_SUCCESS = 0,
    RIFT_ENOMEM,     /* Out of memory    */
    RIFT_ERANGE,     /* Range error      */
    RIFT_EARG,       /* Invalid argument */
    RIFT_EMAX        /* Max error number */
} rift_error_t;


rift_error_t rift_alloc(rift_buffer_t** gap,
                        const char* content,
                        const size_t len);


rift_error_t rift_free(rift_buffer_t *buf);


size_t rift_read(const rift_buffer_t* gap,
                 char* buf,
                 const size_t n);


rift_error_t rift_insert(rift_buffer_t* gb,
		         const char* buf,
			 const size_t n);

rift_error_t rift_delete(rift_buffer_t* gap,
                         size_t n);


rift_error_t rift_fwd(rift_buffer_t* gap,
                      size_t n);


rift_error_t rift_rwd(rift_buffer_t* gap,
                      size_t n);


void rift_inspect(rift_buffer_t*);

#endif /* !RIFT_H */

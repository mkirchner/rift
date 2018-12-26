/*
 * rift.c
 *
 * Copyright (C) 2018 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rift.h"

#if ( __WORDSIZE == 64 )
#define BUILD_64
#else
#define BUILD_32
#endif

/**
 * @brief Calculates the capacity of the gap buffer given
 *        a requested size.
 *
 * The final size corresponds to the first power of two
 * that is larger than the requested size n.
 */
static size_t _rift_calc_buf_capacity(const size_t n)
{
    size_t v = n - 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#ifdef BUILD_64
    v |= v >> 32;
#endif
    v++;
    return v;
}

/**
 * @brief Grows the buffer such that `n` more
 *        characters fit in the gap.
 *
 * The functions tests if adding `n` chars to the buffer would
 * overflow and then reallocates buffer capacity if necessary.
 *
 * @param gap Pointer to the gap buffer instance.
 * @param n   The additional number of chars to provision for.
 *
 * @return Nonzero on any failure. Consult `errno`.
 */
static int _rift_provision_buf_capacity(rift_buffer_t* gap, const size_t n)
{
    /* check if we need to extend the size of the buffer */
    // printf("Request to provision for %lu chars.", n);
    if (gap->lb + n >= gap->rb) {
        // printf("Provisioning for %lu chars.", n);
        /* allocate new buffer */
        size_t new_siz = _rift_calc_buf_capacity(gap->siz + n);
        char *new_buf = NULL;
        if ((new_buf = malloc(new_siz)) == NULL) {
            return -1; /* ENOMEM */
        }
        /* copy contents */
        size_t new_rb = gap->lb + new_siz - gap->siz;
        size_t rlen = gap->siz - gap->rb - 1;
        if (gap->buf) {
            memcpy(new_buf, gap->buf, gap->lb);
            memcpy(new_buf+new_rb, gap->buf+gap->rb, rlen);
        }
        /* update gap buffer struct */
        char *tmp = gap->buf;
        gap->buf = new_buf;
        gap->rb = new_rb;
        gap->siz = new_siz;
        /* free old buffer */
        free(tmp);
    }
    // printf("Capacity: %lu chars.", gap->siz);
    return 0;
}


rift_error_t rift_alloc(rift_buffer_t** gb, const char* content, const size_t len)
{
    if ((*gb = malloc(sizeof(rift_buffer_t))) == NULL) {
        return RIFT_ENOMEM;
    }
    (*gb)->siz = 0;
    (*gb)->lb = 0;
    (*gb)->rb = 0;
    (*gb)->buf = NULL;
    if (content && len > 0) {
        return rift_insert(*gb, content, len);
    }
    return RIFT_SUCCESS;
}


rift_error_t rift_free(rift_buffer_t* gb)
{
    // printf("%p\n", (void *) gb);
    if (gb) {
	    if (gb->buf) {
                    // printf("%p\n", (void*) gb->buf);
		    free(gb->buf);
	    }
	    free(gb);
    }
    return RIFT_SUCCESS;
}


size_t rift_read(const rift_buffer_t* gb, char* buf, const size_t bufsiz)
{
    if (!(gb && buf) || (buf && !bufsiz)) {
        return RIFT_EARG;
    }
    /* copy lhs */
    size_t lsiz = gb->lb;
    size_t n = (bufsiz < lsiz) ? bufsiz : lsiz;
    memcpy(buf, gb->buf, n);
    /* copy rhs */
    size_t rsiz = bufsiz > 0 ? gb->siz - 1 - gb->rb : 0;
    n = (n < rsiz) ? n : rsiz;
    memcpy(buf+gb->lb, gb->buf+gb->rb, n);
    /* terminate string */
    size_t total_len = gb->lb + rsiz;
    size_t term_index = total_len < bufsiz ? total_len : bufsiz;
    buf[term_index] = '\0';
    return term_index;
}


rift_error_t rift_insert(rift_buffer_t* gb, const char* buf, const size_t n)
{
    if (!(gb && buf)) {
        return RIFT_EARG;
    }
    if (_rift_provision_buf_capacity(gb, n) != 0) {
        return RIFT_ENOMEM;
    };
    memcpy(gb->buf + gb->lb, buf, n);
    gb->lb += n;
    return RIFT_SUCCESS;
}


rift_error_t rift_delete(rift_buffer_t* gb, size_t n)
{
    if (n > gb->lb) {
        /* cannot move beyond left boundary */
        return RIFT_ERANGE;
    }
    gb->lb -= n;
    return RIFT_SUCCESS;
}


rift_error_t rift_fwd(rift_buffer_t* gb, size_t n)
{
    if (n + gb->rb > gb->siz) {
        return RIFT_ERANGE;
    }
    memmove(gb->buf + gb->lb, gb->buf + gb->rb, n);
    gb->lb += n;
    gb->rb += n;
    return RIFT_SUCCESS;
}


rift_error_t rift_rwd(rift_buffer_t* gb, size_t n)
{
    if (n > gb->lb) {
        return RIFT_ERANGE;
    }
    memmove(gb->buf + gb->rb - n, gb->buf + gb->lb - n, n);
    gb->lb -= n;
    gb->rb -= n;
    return RIFT_SUCCESS;
}


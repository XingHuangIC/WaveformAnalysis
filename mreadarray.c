/*
 * Copyright (c) 2011, 2012, 2016
 *
 *     Yuan Mei
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "mreadarray.h"

#ifdef LINE_MAX
#undef LINE_MAX
#define LINE_MAX 4096
#endif

#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

#ifndef blankq
#define blankq(a) ((a)==' ' || (a)=='\t')
#endif

static const int sizeof_size_t = sizeof(size_t);

mrdary_hdl *mrdary_init_f(const char *fname, size_t rowmax)
{
    mrdary_hdl *hdl=NULL;
    char buf[LINE_MAX]={0}, *p;
    int i;
    FILE *fp;

    if((fp=fopen(fname, "r"))==NULL) {
        perror(fname);
        return NULL;
    }

    hdl = malloc(sizeof(mrdary_hdl));
    hdl->marray = NULL;
    hdl->fp = fp;

    /* read the first effective line
     * and determine the number of columns
     */
    hdl->column = 0;
    while(!feof(fp)) {
        if(fgets(buf, LINE_MAX, fp)==NULL)
            break;
        if(buf[0]=='#' || buf[0]=='%')
            continue;
        p = buf;
        while((p-buf) < strlen(buf)) {
            for(;blankq(*p);p++);
            for(;!blankq(*p);p++);
            hdl->column++;
        }
        break;
    }
    // fprintf(stderr, "Number of columns in file: %zd\n", hdl->column);
    /* rewind the file */
    rewind(fp);
    hdl->row = 0;
    hdl->rowmax = rowmax;
    /* allocate array with the default rowmax */
    hdl->marray = calloc(hdl->column * hdl->rowmax, sizeof(double));
    hdl->min = malloc(hdl->column * sizeof(double));
    hdl->max = malloc(hdl->column * sizeof(double));
    for(i=0; i<hdl->column; i++) {
        hdl->min[i] = DBL_MAX;
        hdl->max[i] = -DBL_MIN;
    }

    return hdl;
}

size_t mrdary_read_all(mrdary_hdl *hdl)
{
    char buf[LINE_MAX]={0}, *p, *pn;
    FILE *fp;
    double v;
    int i;

    if(hdl->row != 0) {
        fprintf(stderr, "%s error: hdl->row = %zd != 0\n", __FUNCTION__, hdl->row);
        return 0;
    }

    fp = hdl->fp;

    while(!feof(fp)) {
        if(fgets(buf, LINE_MAX, fp)==NULL)
            break;
        if(buf[0]=='#' || buf[0]=='%')
            continue;
        p = buf;
        i = 0;
        while((p-buf) < strlen(buf)) {
            v = strtod(p, &pn);
            if(pn == p) break; /* no conversion occured */
            p = pn;
            /* replaed by the above three lines
            for(;blankq(*p);p++);
            sscanf(p, "%lf", &v);
            for(;!blankq(*p);p++);
            */
            /* convert things like nan to 0.0 */
            if(!isfinite(v)) v = 0.0;
            /* update min and max */
            if(v < hdl->min[i]) hdl->min[i] = v;
            if(v > hdl->max[i]) hdl->max[i] = v;

            hdl->marray[hdl->column*hdl->row + i] = v;
            i++;
            if(i > hdl->column) {
                fprintf(stderr, "%s error: i=%d > hdl->column=%zd\n", __FUNCTION__, i, hdl->column);
                hdl->row++;
                return hdl->row;
            }
        }
        hdl->row++;
        if(hdl->row >= hdl->rowmax) {
            /* re-allocate memory */
            hdl->rowmax *= 2;
            hdl->marray = realloc(hdl->marray, hdl->column * hdl->rowmax * sizeof(double));
        }
    }
    return hdl->row;
}

int mrdary_free(mrdary_hdl *hdl)
{
    /* lisp may give a freed non-NULL hdl,
       so we check based on the value of hdl->marray */
    if(hdl && hdl->marray) {
        fclose(hdl->fp);
        free(hdl->marray);
        hdl->marray = NULL;
        free(hdl->min);
        free(hdl->max);
        free(hdl);
        hdl = NULL;
        return 1;
    }
    return 0;
}

double *mrdary_value_mn(mrdary_hdl *hdl, size_t m, size_t n)
{
    if(m>=hdl->row || n>=hdl->column) {
        fprintf(stderr, "%s error: (%zd, %zd) not inside of (%zd, %zd)\n",
                __FUNCTION__, m, n, hdl->row, hdl->column);
        return NULL;
    }
    return (hdl->marray + m*hdl->column + n);
}

double *mrdary_min(mrdary_hdl *hdl, size_t i)
{
    return (hdl->min + i);
}

double *mrdary_max(mrdary_hdl *hdl, size_t i)
{
    return (hdl->max + i);
}

#ifdef MREADARRAY_DEBUG_ENABLEMAIN

int main(int argc, char **argv)
{
    mrdary_hdl *hdl;

    hdl = mrdary_init_f(argv[1], 1000005);
    mrdary_read_whole(hdl);
    mrdary_free(hdl);
    return EXIT_SUCCESS;
}

#endif /* MREADARRAY_DEBUG_ENABLEMAIN */

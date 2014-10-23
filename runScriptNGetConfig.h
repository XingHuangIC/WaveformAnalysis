#ifndef __RUNSCRIPTNGETCONFIG_H__
#define __RUNSCRIPTNGETCONFIG_H__
#include "common.h"
#include "scheme.h"

enum scmvar_type {
    scmvart_integer = 0,
    scmvart_real = 1, /* usually means double */
};

typedef struct scmvar_config_elem 
{
    char *scmSymName;
    enum scmvar_type scmVart;
    pointer symbol;
} scmvar_config_t;

scheme *tinyscheme_init(const char *fName);
int tinyscheme_close(scheme *sc);
/* init and close tinyscheme in a single run, and get the parameters as the result */
config_parameters_t *get_config_parameters(const char *fName);
int free_config_parameters(config_parameters_t *cParms);

#endif /* __RUNSCRIPTNGETCONFIG_H__ */

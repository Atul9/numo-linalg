/*
!!!!!!!!! Editing !!!!!!!!!!!

  lapack.c
  LAPACK wrapper for Ruby/Numo
    (C) Copyright 2017 by Masahiro TANAKA

  This program is free software.
  NO WARRANTY.
*/
#include <assert.h>
#include <ruby.h>
#include "numo/narray.h"
#include "numo/template.h"

// from ruby/ext/fiddle/fiddle.h
#if defined(HAVE_DLFCN_H)
# include <dlfcn.h>
# /* some stranger systems may not define all of these */
#ifndef RTLD_LAZY
#define RTLD_LAZY 0
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif
#else
# if defined(_WIN32)
#   include <windows.h>
#   define dlopen(name,flag) ((void*)LoadLibrary(name))
#   define dlerror() strerror(rb_w32_map_errno(GetLastError()))
#   define dlsym(handle,name) ((void*)GetProcAddress((handle),(name)))
#   define RTLD_LAZY -1
#   define RTLD_NOW  -1
#   define RTLD_GLOBAL -1
# endif
#endif

#define lapack_complex_float scomplex
#define lapack_complex_double dcomplex
#include "lapacke.h"
#include "lapacke_t.h"

static void *lapack_handle = 0;
static char *lapack_prefix = 0;


int
numo_lapacke_option_order(VALUE trans)
{
    int opt;
    char *ptr;

    switch(TYPE(trans)) {
    case T_NIL:
    case T_FALSE:
        return LAPACK_ROW_MAJOR;
    case T_TRUE:
        return LAPACK_COL_MAJOR;
    case T_FIXNUM:
        opt = FIX2INT(trans);
        if (opt == LAPACK_ROW_MAJOR || opt == LAPACK_COL_MAJOR) {
            return opt;
        }
        break;
    case T_SYMBOL:
        trans = rb_sym2str(trans);
    case T_STRING:
        ptr = RSTRING_PTR(trans);
        if (RSTRING_LEN(trans) > 0) {
            switch(ptr[0]){
            case 'R': case 'r':
                return LAPACK_ROW_MAJOR;
            case 'C': case 'c':
                return LAPACK_COL_MAJOR;
            }
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for LAPACKE_ORDER");
    return 0;
}

char
numo_lapacke_option_job(VALUE job)
{
    char *ptr, c;

    switch(TYPE(job)) {
    case T_NIL:
    case T_FALSE:
        return 0;
    case T_SYMBOL:
        job = rb_sym2str(job);
    case T_STRING:
        ptr = RSTRING_PTR(job);
        if (RSTRING_LEN(job) > 0) {
            c = ptr[0];
            if (c >= 'a' && c <= 'z') {
                c -= 'a'-'A';
            }
            return c;
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for JOB option");
    return 0;
}

char
numo_lapacke_option_trans(VALUE trans)
{
    int opt;
    char *ptr;

    switch(TYPE(trans)) {
    case T_NIL:
    case T_FALSE:
        return 'N';
    case T_TRUE:
        return 'T';
    case T_FIXNUM:
        opt = FIX2INT(trans);
        if (opt >= 'N' && opt <= 'C') {
            return opt;
        }
        break;
    case T_SYMBOL:
        trans = rb_sym2str(trans);
    case T_STRING:
        ptr = RSTRING_PTR(trans);
        if (RSTRING_LEN(trans) > 0) {
            switch(ptr[0]){
            case 'N': case 'n':
                return 'N';
            case 'T': case 't':
                return 'T';
            case 'C': case 'c':
                return 'C';
            }
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for LAPACKE_TRANSPOSE");
    return 0;
}

char
numo_lapacke_option_uplo(VALUE uplo)
{
    int opt;
    char *ptr;

    switch(TYPE(uplo)) {
    case T_NIL:
    case T_FALSE:
        return 'U';
    case T_TRUE:
        return 'L';
    case T_FIXNUM:
        opt = FIX2INT(uplo);
        switch(opt){
        case 'U':
        case 'L':
            return opt;
        }
        break;
    case T_SYMBOL:
        uplo = rb_sym2str(uplo);
    case T_STRING:
        ptr = RSTRING_PTR(uplo);
        if (RSTRING_LEN(uplo) > 0) {
            switch(ptr[0]){
            case 'U': case 'u':
                return 'U';
            case 'L': case 'l':
                return 'L';
            }
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for LAPACKE_UPLO");
    return 0;
}

char
numo_lapacke_option_diag(VALUE diag)
{
    int opt;
    char *ptr;

    switch(TYPE(diag)) {
    case T_NIL:
    case T_FALSE:
        return 'N';
    case T_TRUE:
        return 'U';
    case T_FIXNUM:
        opt = FIX2INT(diag);
        switch(opt){
        case 'N':
        case 'U':
            return opt;
        }
        break;
    case T_SYMBOL:
        diag = rb_sym2str(diag);
    case T_STRING:
        ptr = RSTRING_PTR(diag);
        if (RSTRING_LEN(diag) > 0) {
            switch(ptr[0]){
            case 'N': case 'n':
                return 'N';
            case 'U': case 'u':
                return 'U';
            }
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for LAPACKE_DIAG");
    return 0;
}

char
numo_lapacke_option_side(VALUE side)
{
    int opt;
    char *ptr;

    switch(TYPE(side)) {
    case T_NIL:
    case T_FALSE:
        return 'L';
    case T_TRUE:
        return 'R';
    case T_FIXNUM:
        opt = FIX2INT(side);
        switch(opt){
        case 'L':
        case 'R':
            return opt;
        }
        break;
    case T_SYMBOL:
        side = rb_sym2str(side);
    case T_STRING:
        ptr = RSTRING_PTR(side);
        if (RSTRING_LEN(side) > 0) {
            switch(ptr[0]){
            case 'L': case 'l':
                return 'L';
            case 'R': case 'r':
                return 'R';
            }
        }
        break;
    }
    rb_raise(rb_eArgError,"invalid value for LAPACKE_SIDE");
    return 0;
}

void
numo_lapacke_check_func(void **func, const char *name)
{
    char *s, *error;

    if (*func==0) {
        if (lapack_handle==0) {
            rb_raise(rb_eRuntimeError,"LAPACK library is not loaded");
        }
        if (lapack_prefix==0) {
            rb_raise(rb_eRuntimeError,"LAPACKE prefix is not set");
        }
        s = alloca(strlen(lapack_prefix)+strlen(name)+1);
        strcpy(s,lapack_prefix);
        strcat(s,name);
        dlerror();
        *func = dlsym(lapack_handle, s);
        error = dlerror();
        if (error != NULL) {
            rb_raise(rb_eRuntimeError, "%s", error);
        }
    }
}

/*
  module definition: Numo::Linalg
*/
static VALUE mLinalg;

/*
  module definition: Numo::Linalg::Lapack
*/
static VALUE mLapack;


static VALUE
lapack_s_dlopen(int argc, VALUE *argv, VALUE mod)
{
    int i, f;
    VALUE lib, flag;
    char *error;

    i = rb_scan_args(argc, argv, "11", &lib, &flag);
    if (i==2) {
        f = NUM2INT(flag);
    } else {
        f = RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND;
    }
    dlerror();
    lapack_handle = dlopen(StringValueCStr(lib), f);
    error = dlerror();
    if (error != NULL) {
        rb_raise(rb_eRuntimeError, "%s", error);
    }
    return Qnil;
}

static VALUE
lapack_s_require(int argc, VALUE *argv, VALUE mod)
{
    int i, f;
    VALUE lib, flag;
    char *error;

    i = rb_scan_args(argc, argv, "11", &lib, &flag);
    if (i==2) {
        f = NUM2INT(flag);
    } else {
        f = RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND;
    }
    dlerror();
    dlopen(StringValueCStr(lib), f);
    error = dlerror();
    if (error != NULL) {
        rb_raise(rb_eRuntimeError, "%s", error);
    }
    return Qnil;
}


static VALUE
lapack_s_prefix_set(VALUE mod, VALUE prefix)
{
    long len;

    if (TYPE(prefix) != T_STRING) {
        rb_raise(rb_eTypeError,"argument must be string");
    }
    if (lapack_prefix) {
        free(lapack_prefix);
    }
    len = RSTRING_LEN(prefix);
    lapack_prefix = malloc(len+1);
    strcpy(lapack_prefix, StringValueCStr(prefix));
    return prefix;
}


void Init_numo_linalg_lapack_s();
void Init_numo_linalg_lapack_d();
void Init_numo_linalg_lapack_c();
void Init_numo_linalg_lapack_z();

void
Init_lapack(void)
{
    VALUE mN;

    mN = rb_define_module("Numo");
    /*
      Document-module: Numo::Linalg
    */
    mLinalg = rb_define_module_under(mN, "Linalg");
    mLapack = rb_define_module_under(mLinalg, "Lapack");

    rb_define_module_function(mLapack, "dlopen", lapack_s_dlopen, -1);
    rb_define_module_function(mLapack, "require", lapack_s_require, -1);
    rb_define_module_function(mLapack, "prefix=", lapack_s_prefix_set, 1);

    lapack_prefix = malloc(strlen("LAPACKE_")+1); // default prefix
    strcpy(lapack_prefix,"LAPACKE_");

    Init_numo_linalg_lapack_s();
    Init_numo_linalg_lapack_d();
    Init_numo_linalg_lapack_c();
    Init_numo_linalg_lapack_z();
}

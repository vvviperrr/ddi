#pragma once

#include <jni.h>
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "base.h"
#include "log.h"

#define MAX_PARAMS 128

enum NATIVE_TYPE {
    TYPE_INVALID,
    TYPE_VOID,
    TYPE_BOOLEAN,
    TYPE_BYTE,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_OBJECT,
};

struct hook_entry {
    struct dalvik_hook_t *h;
    void *func;
    bool used;
    size_t id;
    char *handler_signature;
    jclass handler_class;
    jobject handler_object;
    jmethodID handler_method;

    size_t params_count;
    enum NATIVE_TYPE params[MAX_PARAMS];
    enum NATIVE_TYPE ret_type;
};

struct hook_entry *new_hook_entry(void);
struct hook_entry *get_hook_entry_by_id(size_t);
void free_hook_entry(JNIEnv *, struct hook_entry *);

void *hook_handler_generic(struct hook_entry *, JNIEnv *, jobject, va_list);

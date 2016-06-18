#include "hook_entry.h"

#define HOOK_HANDLER_NAME hook_handler_
#define HOOK_STRUCT_NAME dalvik_hook_

#define glue(x, y) x ## y

#define HOOK_HANDLER(id) glue(HOOK_HANDLER_NAME, id)
#define DALVIK_HOOK(id) glue(HOOK_STRUCT_NAME, id)

struct dalvik_hook_wrapper {
    struct dalvik_hook_t dh;
    struct hook_entry *entry;
};

#define DECLARE_HOOK_1(N) \
static struct dalvik_hook_wrapper DALVIK_HOOK(N); \
static void *HOOK_HANDLER(N)(JNIEnv *env, jobject obj, ...) \
{ \
    va_list args; \
    va_start(args, obj); \
    void * ret = hook_handler_generic(DALVIK_HOOK(N).entry, env, obj, args); \
    va_end(args); \
    return ret; \
}

#define DECLARE_HOOK_10(N) \
    DECLARE_HOOK_1(glue(N,0)) \
    DECLARE_HOOK_1(glue(N,1)) \
    DECLARE_HOOK_1(glue(N,2)) \
    DECLARE_HOOK_1(glue(N,3)) \
    DECLARE_HOOK_1(glue(N,4)) \
    DECLARE_HOOK_1(glue(N,5)) \
    DECLARE_HOOK_1(glue(N,6)) \
    DECLARE_HOOK_1(glue(N,7)) \
    DECLARE_HOOK_1(glue(N,8)) \
    DECLARE_HOOK_1(glue(N,9))

#define DECLARE_HOOK_100(N) \
    DECLARE_HOOK_10(glue(N,0)) \
    DECLARE_HOOK_10(glue(N,1)) \
    DECLARE_HOOK_10(glue(N,2)) \
    DECLARE_HOOK_10(glue(N,3)) \
    DECLARE_HOOK_10(glue(N,4)) \
    DECLARE_HOOK_10(glue(N,5)) \
    DECLARE_HOOK_10(glue(N,6)) \
    DECLARE_HOOK_10(glue(N,7)) \
    DECLARE_HOOK_10(glue(N,8)) \
    DECLARE_HOOK_10(glue(N,9))

#define DECLARE_HOOK_ENTRY_1(N) \
{ \
    .h = &DALVIK_HOOK(N).dh, \
    .func = HOOK_HANDLER(N), \
    .used = false, \
    .id = 0, \
    .handler_signature = NULL, \
    .handler_class = NULL, \
    .handler_object = NULL, \
    .handler_method = NULL, \
    .params_count = 0, \
    .params = { 0 }, \
    .ret_type = 0, \
},

#define DECLARE_HOOK_ENTRY_10(N) \
    DECLARE_HOOK_ENTRY_1(glue(N,0)) \
    DECLARE_HOOK_ENTRY_1(glue(N,1)) \
    DECLARE_HOOK_ENTRY_1(glue(N,2)) \
    DECLARE_HOOK_ENTRY_1(glue(N,3)) \
    DECLARE_HOOK_ENTRY_1(glue(N,4)) \
    DECLARE_HOOK_ENTRY_1(glue(N,5)) \
    DECLARE_HOOK_ENTRY_1(glue(N,6)) \
    DECLARE_HOOK_ENTRY_1(glue(N,7)) \
    DECLARE_HOOK_ENTRY_1(glue(N,8)) \
    DECLARE_HOOK_ENTRY_1(glue(N,9)) \

#define DECLARE_HOOK_ENTRY_100(N) \
    DECLARE_HOOK_ENTRY_10(glue(N,0)) \
    DECLARE_HOOK_ENTRY_10(glue(N,1)) \
    DECLARE_HOOK_ENTRY_10(glue(N,2)) \
    DECLARE_HOOK_ENTRY_10(glue(N,3)) \
    DECLARE_HOOK_ENTRY_10(glue(N,4)) \
    DECLARE_HOOK_ENTRY_10(glue(N,5)) \
    DECLARE_HOOK_ENTRY_10(glue(N,6)) \
    DECLARE_HOOK_ENTRY_10(glue(N,7)) \
    DECLARE_HOOK_ENTRY_10(glue(N,8)) \
    DECLARE_HOOK_ENTRY_10(glue(N,9)) \

/* max 100 hooks supported */
DECLARE_HOOK_100(0);

/* and 100 entries */
static struct hook_entry entries[] = {
    DECLARE_HOOK_ENTRY_100(0)
};

#define ENTRIES_SIZE (sizeof(entries) / sizeof(entries[0]))

struct hook_entry *new_hook_entry(void)
{
    size_t i;

    for (i = 0; i < ENTRIES_SIZE; i++) {
        if (!entries[i].used) {
            entries[i].used = true;
            return &entries[i];
        }
    }

    return NULL;
}

struct hook_entry *get_hook_entry_by_id(size_t id)
{
    return id >= ENTRIES_SIZE ? NULL : &entries[id];
}

void free_hook_entry(JNIEnv *env, struct hook_entry *entry)
{
    /* TODO */
    free(entry->handler_signature);

    if (entry->handler_class) {
        (*env)->DeleteGlobalRef(env, entry->handler_class);
    }

    if (entry->handler_object) {
        (*env)->DeleteGlobalRef(env, entry->handler_object);
    }

    entry->used = false;
}

void __attribute__ ((constructor)) hook_entry_init(void)
{
    size_t i;
    struct dalvik_hook_wrapper *hw;

    for (i = 0; i < ENTRIES_SIZE; i++) {
        hw = (struct dalvik_hook_wrapper * ) entries[i].h;
        hw->entry = &entries[i];
        entries[i].id = i;
    }
}


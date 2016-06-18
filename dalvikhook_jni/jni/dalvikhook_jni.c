#include <jni.h>

#include "hook_entry.h"
#include "com_nobody_dalvikhook_Handler.h"
#include "com_nobody_dalvikhook_Hook.h"

#undef LOG_TAG
#define LOG_TAG "libdalvikhook_jni"
#include "android_jni/log.h"

static struct dexstuff_t d;

static void my_log2(char *msg)
{
    LOGV("%s", msg);
}

/* create handler signature from hooking method signature */
static char *get_handler_signature(const char *method_signature)
{
    /* TODO: remove */
    char *ret = malloc((strlen(method_signature) * 2) + 32 + 1);
    char *tmp = strdup(method_signature);
    char *it = tmp;
    char *c;

    *ret = 0;
    strcat(ret, "(Ljava/lang/Object;"); /* concat Object class (self) */

    if ((c = strchr(it, '(')) == NULL) { /* find begin params pos */
        goto bad_signature;
    }

    it = c + 1; /* it points next symbol after '(' */

    while ((c = strchr(it, 'L')) != NULL) { /* while class def exists */
        *c = 0; /* terminate on 'L' */
        strcat(ret, it); /* concat all before 'L' */
        strcat(ret, "Ljava/lang/Object;"); /* concat Object class */
        it = c + 1; /* it points next symbol after 'L' */

        if ((c = strchr(it, ';')) == NULL) { /* find end class def pos */
            goto bad_signature;
        }

        it = c + 1; /* it points next symbol after ';' */
    }

    /* no more class defs, concat the rest */
    strcat(ret, it);

    free(tmp);

    return ret;

bad_signature:
    LOGE("Bad method signature: %s", method_signature);
    free(tmp);
    free(ret);
    return NULL;
}

static enum NATIVE_TYPE get_param_type(char param)
{
    switch (param) {
    case 'V':
        return TYPE_VOID;
    case 'Z':
        return TYPE_BOOLEAN;
    case 'B':
        return TYPE_BYTE;
    case 'C':
        return TYPE_CHAR;
    case 'S':
        return TYPE_SHORT;
    case 'I':
        return TYPE_INT;
    case 'J':
        return TYPE_LONG;
    case 'F':
        return TYPE_FLOAT;
    case 'D':
        return TYPE_DOUBLE;
    case 'L':
    case '[':
        return TYPE_OBJECT;
    default:
        return TYPE_INVALID;
    }
}

static bool parse_method_params(struct hook_entry *entry, const char *method_signature)
{
    int params_count = 0;
    enum NATIVE_TYPE param = TYPE_INVALID;

    char *it = strchr(method_signature, '(');
    char *end = strrchr(method_signature, ')');

    if (!(it && end)) {
        goto bad_signature;
    }

    ++it; /* ptr to first param */

    while (it != end) {
        param = get_param_type(*it);

        switch (param) {
        case TYPE_INVALID:
            goto bad_signature;
            break;
        case TYPE_OBJECT:
            if (*it == 'L') { /* it's object */
                if ((it = strchr(it, ';')) == NULL) {
                    goto bad_signature;
                }
            } else { /* it's array */
                while (*(++it) == '['); /* pass array signs */
            }
            break;
        default:
            /* it's buildin param */
            break;
        }

        /* store param type */
        entry->params[params_count++] = param;

        ++it; /* ptr to next param */
    }

    /* store params count */
    entry->params_count = params_count;

    /* get return type param */
    if ((param = get_param_type(*(it + 1))) == TYPE_INVALID) {
        goto bad_signature;
    }

    /* store return param type */
    entry->ret_type = param;

    return true;

bad_signature:
    LOGE("Bad method signature: %s", method_signature);
    return false;
}

static bool va_list_to_jvalue(struct hook_entry *entry, va_list args, jvalue *value)
{
    int i;

    for (i = 0; i < entry->params_count; i++) {
        switch (entry->params[i]) {
        case TYPE_BOOLEAN:
            value->z = va_arg(args, int);
            break;
        case TYPE_BYTE:
            value->b = va_arg(args, int);
            break;
        case TYPE_CHAR:
            value->c = va_arg(args, int);
            break;
        case TYPE_SHORT:
            value->s = va_arg(args, int);
            break;
        case TYPE_INT:
            value->i = va_arg(args, int);
            break;
        case TYPE_LONG:
            value->j = va_arg(args, jlong);
            break;
        case TYPE_FLOAT:
            value->f = va_arg(args, jdouble);
            break;
        case TYPE_DOUBLE:
            value->d = va_arg(args, jdouble);
            break;
        case TYPE_OBJECT:
            value->l = va_arg(args, jobject);
            break;
        default:
            LOGE("Invalid param type: %d", entry->params[i]);
            return false;
        }

        ++value;
    }

    return true;
}

static bool object_array_to_jvalue(JNIEnv *env, struct hook_entry *entry, jobjectArray args, jvalue *value)
{
    int i;

    jint args_size = (*env)->GetArrayLength(env, args);

    if (args_size != entry->params_count) {
        LOGE("invalid params count");
        return false;
    }

    for (i = 0; i < entry->params_count; i++) {
        jobject object = (*env)->GetObjectArrayElement(env, args, i);

        switch (entry->params[i]) {
        case TYPE_BOOLEAN:
            value->z = object_to_boolean(env, object);
            break;
        case TYPE_BYTE:
            value->b = object_to_byte(env, object);
            break;
        case TYPE_CHAR:
            value->c = object_to_char(env, object);
            break;
        case TYPE_SHORT:
            value->s = object_to_short(env, object);
            break;
        case TYPE_INT:
            value->i = object_to_int(env, object);
            break;
        case TYPE_LONG:
            value->j = object_to_long(env, object);
            break;
        case TYPE_FLOAT:
            value->f = object_to_float(env, object);
            break;
        case TYPE_DOUBLE:
            value->d = object_to_double(env, object);
            break;
        case TYPE_OBJECT:
            value->l = object;
            break;
        default:
            LOGE("Invalid param type: %d", entry->params[i]);
            return false;
        }

        ++value;
    }

    return true;
}

static jvalue call_method_by_ret_type(struct hook_entry *entry,
        JNIEnv *env,
        jobject obj,
        jmethodID method,
        jvalue *args,
        enum NATIVE_TYPE ret_type)
{
    jvalue ret = { 0 };

    switch (ret_type) {
    case TYPE_VOID:
        (*env)->CallVoidMethodA(env, obj, method, args);
        break;
    case TYPE_BOOLEAN:
        ret.z = (*env)->CallBooleanMethodA(env, obj, method, args);
        break;
    case TYPE_BYTE:
        ret.b = (*env)->CallByteMethodA(env, obj, method, args);
        break;
    case TYPE_CHAR:
        ret.c = (*env)->CallCharMethodA(env, obj, method, args);
        break;
    case TYPE_SHORT:
        ret.s = (*env)->CallShortMethodA(env, obj, method, args);
        break;
    case TYPE_INT:
        ret.i = (*env)->CallIntMethodA(env, obj, method, args);
        break;
    case TYPE_LONG:
        ret.j = (*env)->CallLongMethodA(env, obj, method, args);
        break;
    case TYPE_FLOAT:
        ret.f = (*env)->CallFloatMethodA(env, obj, method, args);
        break;
    case TYPE_DOUBLE:
        ret.d = (*env)->CallDoubleMethodA(env, obj, method, args);
        break;
    case TYPE_OBJECT:
        ret.l = (*env)->CallObjectMethodA(env, obj, method, args);
        break;
    default:
        LOGE("Invalid return type");
        assert(false);
        break;
    }

    return ret;
}

static size_t get_hook_object_filed_id(JNIEnv *env, jobject obj)
{
    jclass cls = (*env)->GetObjectClass(env, obj);
    jfieldID fid = (*env)->GetFieldID(env, cls, "id", "I");
    jint id = (*env)->GetIntField(env, obj, fid);
    return id;
}

static bool set_hook_object_filed_id(JNIEnv *env, jobject obj, size_t id)
{
    jclass cls = (*env)->GetObjectClass(env, obj);
    jfieldID fid = (*env)->GetFieldID(env, cls, "id", "I");
    (*env)->SetIntField(env, obj, fid, id);
    return true;
}

void *hook_handler_generic(struct hook_entry *entry,
        JNIEnv *env, jobject self_object, va_list va)
{
    /* !!! FIXME !!! */

    jvalue ret;

    jvalue args[MAX_PARAMS] = { 0 };
    args[0].l = self_object; /* first is self object */
    va_list_to_jvalue(entry, va, args + 1); /* real params are the rest */

    ret = call_method_by_ret_type(
            entry,
            env,
            entry->handler_object,
            entry->handler_method,
            args,
            entry->ret_type
    );

    /* FIXME void, long, double */
    return (void *) ret.l;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    LOGV(__func__);

    JNIEnv *env = NULL;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    assert(env != NULL);

    bool ret = unboxing_init(env);
    assert(ret);

 	// set log function for  libbase (very important!)
    set_logfunction((void *) my_log2);

	// set log function for libdalvikhook (very important!)
    dalvikhook_set_logfunction((void *) my_log2);

    dexstuff_resolv_dvm(&d);

    return JNI_VERSION_1_6;
}

JNIEXPORT jboolean JNICALL Java_com_nobody_dalvikhook_Hook_setHook
  (JNIEnv *env, jclass jHookClass, jstring jClassName, jstring jMethodName, jstring jMethodSignature, jobject jHookHandlerObject)
{
    LOGV(__func__);

    bool ret = true;

    struct hook_entry *entry = new_hook_entry();
    if (!entry) {
        LOGE("No free hook entries left. Cannot create hook.");
        return false;
    }

    const char *class_name = (*env)->GetStringUTFChars(env, jClassName, 0);
    const char *method_name = (*env)->GetStringUTFChars(env, jMethodName, 0);
    const char *method_signature = (*env)->GetStringUTFChars(env, jMethodSignature, 0);

    char *handler_method_name = malloc(strlen(method_name) + 2);
    *handler_method_name = '_';
    strcpy(handler_method_name + 1, method_name);

    if (!parse_method_params(entry, method_signature)) {
        LOGE("Cannot parse method signature params");
        ret = false;
        goto end;
    }

    if (entry->params_count > MAX_PARAMS) {
        LOGE("Too many params in method");
        ret = false;
        goto end;
    }

    char *handler_signature = get_handler_signature(method_signature);
    if (handler_signature == NULL) {
        LOGE("Cannot create handler signature from method signature");
        ret = false;
        goto end;
    }

    jclass handlerClass = (*env)->GetObjectClass(env, jHookHandlerObject);
    if (!handlerClass) {
        LOGE("Cannot get Handler class");
        (*env)->ExceptionClear(env);
        ret = false;
        goto end;
    }

    jmethodID handlerMethod = (*env)->GetMethodID(env, handlerClass, handler_method_name, handler_signature);
    if (!handlerMethod) {
        LOGE("Cannot get handler method named %s with signature %s",
                handler_method_name, handler_signature);
        (*env)->ExceptionClear(env);
        ret = false;
        goto end;
    }

    /* fill entry */
    entry->handler_class = (*env)->NewGlobalRef(env, handlerClass);
    entry->handler_object = (*env)->NewGlobalRef(env, jHookHandlerObject);
    entry->handler_method = handlerMethod;
    entry->handler_signature = handler_signature;

    if (!set_hook_object_filed_id(env, entry->handler_object, entry->id)) {
        LOGE("Cannot set object field id");
        ret = false;
        goto end;
    }

    dalvik_hook_setup(entry->h,
            (char *) class_name,
            (char *) method_name,
            (char *) method_signature,
            entry->params_count + 1,
            entry->func);

    dalvik_hook(&d, entry->h);

end:
    free(handler_signature);
    free(handler_method_name);
    (*env)->ReleaseStringUTFChars(env, jMethodSignature, method_signature);
    (*env)->ReleaseStringUTFChars(env, jMethodName, method_name);
    (*env)->ReleaseStringUTFChars(env, jClassName, class_name);

    if (!ret) {
        free_hook_entry(env, entry);
    }

    return ret;
}

static jvalue call_type_method_generic(JNIEnv *env,
        jobject handler_object,
        jobject self_object,
        jobjectArray object_array,
        enum NATIVE_TYPE type)
{
    struct hook_entry *entry = get_hook_entry_by_id(get_hook_object_filed_id(env, handler_object));
    assert(entry);

    jvalue args[MAX_PARAMS] = { 0 };
    object_array_to_jvalue(env, entry, object_array, args);

    dalvik_prepare(&d, entry->h, env);
    jvalue ret = call_method_by_ret_type(entry, env, self_object, entry->h->mid, args, type);
    dalvik_postcall(&d, entry->h);

    return ret;
}

JNIEXPORT void JNICALL Java_com_nobody_dalvikhook_Handler_callVoidMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    call_type_method_generic(env, handler, self, args, TYPE_VOID);
}

JNIEXPORT jboolean JNICALL Java_com_nobody_dalvikhook_Handler_callBooleanMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_BOOLEAN).z;
}

JNIEXPORT jbyte JNICALL Java_com_nobody_dalvikhook_Handler_callByteMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_BYTE).b;
}

JNIEXPORT jchar JNICALL Java_com_nobody_dalvikhook_Handler_callCharacterMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_CHAR).c;
}

JNIEXPORT jshort JNICALL Java_com_nobody_dalvikhook_Handler_callShortMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_SHORT).s;
}

JNIEXPORT jint JNICALL Java_com_nobody_dalvikhook_Handler_callIntMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_INT).i;
}

JNIEXPORT jlong JNICALL Java_com_nobody_dalvikhook_Handler_callLongMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_LONG).j;
}

JNIEXPORT jfloat JNICALL Java_com_nobody_dalvikhook_Handler_callFloatMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_FLOAT).f;
}

JNIEXPORT jdouble JNICALL Java_com_nobody_dalvikhook_Handler_callDoubleMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_DOUBLE).d;
}

JNIEXPORT jobject JNICALL Java_com_nobody_dalvikhook_Handler_callObjectMethod
  (JNIEnv *env, jobject handler, jobject self, jobjectArray args)
{
    return call_type_method_generic(env, handler, self, args, TYPE_OBJECT).l;
}


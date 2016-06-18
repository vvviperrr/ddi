#include "unboxing.h"
#include <jni.h>

#undef LOG_TAG
#define LOG_TAG "libdalvikhook_unboxing"
#include "android_jni/log.h"

struct {
    const char *class_name;
    const char *method_name;
    const char *signature;
    jclass class;
    jmethodID methodId;
} info[] = {
    [TYPE_INT] = {
        .class_name = "java/lang/Integer",
        .method_name = "intValue",
        .signature = "()I",
    },

    [TYPE_LONG] = {
        .class_name = "java/lang/Long",
        .method_name = "longValue",
        .signature = "()J",
    },

    [TYPE_SHORT] = {
        .class_name = "java/lang/Short",
        .method_name = "shortValue",
        .signature = "()S",
    },

    [TYPE_CHAR] = {
        .class_name = "java/lang/Character",
        .method_name = "charValue",
        .signature = "()C",
    },

    [TYPE_FLOAT] = {
        .class_name = "java/lang/Float",
        .method_name = "floatValue",
        .signature = "()F",
    },

    [TYPE_DOUBLE] = {
        .class_name = "java/lang/Double",
        .method_name = "doubleValue",
        .signature = "()D",
    },

    [TYPE_BYTE] = {
        .class_name = "java/lang/Byte",
        .method_name = "byteValue",
        .signature = "()B",
    },

    [TYPE_BOOLEAN] = {
        .class_name = "java/lang/Boolean",
        .method_name = "booleanValue",
        .signature = "()Z",
    },
};

jint object_to_int(JNIEnv *env, jobject obj)
{ return (*env)->CallIntMethod(env, obj, info[TYPE_INT].methodId); }

jlong object_to_long(JNIEnv *env, jobject obj)
{ return (*env)->CallLongMethod(env, obj, info[TYPE_LONG].methodId); }

jshort object_to_short(JNIEnv *env, jobject obj)
{ return (*env)->CallShortMethod(env, obj, info[TYPE_SHORT].methodId); }

jchar object_to_char(JNIEnv *env, jobject obj)
{ return (*env)->CallCharMethod(env, obj, info[TYPE_CHAR].methodId); }

jfloat object_to_float(JNIEnv *env, jobject obj)
{ return (*env)->CallFloatMethod(env, obj, info[TYPE_FLOAT].methodId); }

jdouble object_to_double(JNIEnv *env, jobject obj)
{ return (*env)->CallDoubleMethod(env, obj, info[TYPE_DOUBLE].methodId); }

jbyte object_to_byte(JNIEnv *env, jobject obj)
{ return (*env)->CallByteMethod(env, obj, info[TYPE_BYTE].methodId); }

jboolean object_to_boolean(JNIEnv *env, jobject obj)
{ return (*env)->CallBooleanMethod(env, obj, info[TYPE_BOOLEAN].methodId); }

bool unboxing_init(JNIEnv *env)
{
    size_t i;
    jclass clazz;
    jmethodID method;

    for (i = TYPE_BOOLEAN; i <= TYPE_DOUBLE; i++) {

        clazz = (*env)->FindClass(env, info[i].class_name);
        if (!clazz) {
            LOGV("Unable find class %s", info[i].class_name);
            return false;
        }

        method = (*env)->GetMethodID(env, clazz, info[i].method_name, info[i].signature);
        if (!method) {
            LOGV("Unable find method %s signature %s", info[i].method_name, info[i].signature);
            return false;
        }

        info[i].class = (*env)->NewGlobalRef(env, clazz);
        info[i].methodId = method;
    }

    return true;
}


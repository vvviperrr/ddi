#pragma once

#include "hook_entry.h"

bool unboxing_init(JNIEnv *env);

jint object_to_int(JNIEnv *, jobject);
jlong object_to_long(JNIEnv *, jobject);
jshort object_to_short(JNIEnv *, jobject);
jchar object_to_char(JNIEnv *, jobject);
jfloat object_to_float(JNIEnv *, jobject);
jdouble object_to_double(JNIEnv *, jobject);
jbyte object_to_byte(JNIEnv *, jobject);
jboolean object_to_boolean(JNIEnv *, jobject );

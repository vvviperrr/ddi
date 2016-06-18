package com.nobody.dalvikhook;

import android.util.Log;

public class Test {
    private static final String TAG = "DalvikHookJNITest";

    public Test() {
        Log.w(TAG, "Test ctor called");

        Hook.setHook("Ljava/lang/String;", "indexOf", "(Ljava/lang/String;I)I",
            new Handler() {
                public int _indexOf(Object self, Object str, int i) {
                    int ret = callIntMethod(self, str, i);
                    StringBuilder builder = new StringBuilder()
                        .append("id = ")
                        .append(Integer.toString(getID()))
                        .append(", ")
                        .append("indexOf: ")
                        .append((String) self)
                        .append(": ")
                        .append((String) str)
                        .append(": ")
                        .append(Integer.toString(i));
                    Log.w(TAG, builder.toString());
                    return ret;
                }
            }
        );

        Hook.setHook("Ljava/lang/String;", "startsWith", "(Ljava/lang/String;I)Z",
            new Handler() {
                public boolean _startsWith(Object self, Object str, int i) {
                    boolean ret = callBooleanMethod(self, str, i);
                    StringBuilder builder = new StringBuilder()
                        .append("id = ")
                        .append(Integer.toString(getID()))
                        .append(", ")
                        .append("startsWith: ")
                        .append((String) self)
                        .append(": ")
                        .append((String) str)
                        .append(": ")
                        .append(Integer.toString(i));
                    Log.w(TAG, builder.toString());
                    return ret;
                }
            }
        );

        Hook.setHook("Ljava/lang/String;", "endsWith", "(Ljava/lang/String;)Z",
            new Handler() {
                public boolean _endsWith(Object self, Object str) {
                    boolean ret = callBooleanMethod(self, str);
                    StringBuilder builder = new StringBuilder()
                        .append("id = ")
                        .append(Integer.toString(getID()))
                        .append(", ")
                        .append("endsWith: ")
                        .append((String) self)
                        .append(": ")
                        .append((String) str);
                    Log.w(TAG, builder.toString());
                    return ret;
                }
            }
        );

        Hook.setHook("Ljava/lang/StringBuffer;", "toString", "()Ljava/lang/String;",
            new Handler() {
                public Object _toString(Object self) {
                    String ret = (String) callObjectMethod(self);
                    StringBuilder builder = new StringBuilder()
                        .append("id = ")
                        .append(Integer.toString(getID()))
                        .append(", ")
                        .append("toString: ret = ")
                        .append((String) ret);
                    Log.w(TAG, builder.toString());
                    return ret;
                }
            }
        );
    }
}


package com.nobody.dalvikhook;

public class Hook {
    public static native boolean setHook(
            String className,
            String methodName,
            String methodSignature,
            Handler handler
    );

    public static void loadLibrary(String path) {
        System.load(path + "/libdalvikhook_jni.so");
    }
}

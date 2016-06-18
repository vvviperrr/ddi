package com.nobody.dalvikhook;

public abstract class Handler {
    private int id;

    public Handler() {
        id = -1;
    }

    public int getID() {
        return id;
    }

    public native final void callVoidMethod(Object self, Object... args);
    public native final boolean callBooleanMethod(Object self, Object... args);
    public native final byte callByteMethod(Object self, Object... args);
    public native final char callCharacterMethod(Object self, Object... args);
    public native final short callShortMethod(Object self, Object... args);
    public native final int callIntMethod(Object self, Object... args);
    public native final long callLongMethod(Object self, Object... args);
    public native final float callFloatMethod(Object self, Object... args);
    public native final double callDoubleMethod(Object self, Object... args);
    public native final Object callObjectMethod(Object self, Object... args);
}


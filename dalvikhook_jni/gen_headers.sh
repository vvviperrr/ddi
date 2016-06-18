#!/bin/bash

mkdir tmp 2>/dev/null
javac -d tmp/ src/com/nobody/dalvikhook/Handler.java src/com/nobody/dalvikhook/Hook.java && \
javah -d jni -classpath tmp/ com.nobody.dalvikhook.Handler com.nobody.dalvikhook.Hook
rm -rf tmp 2>/dev/null

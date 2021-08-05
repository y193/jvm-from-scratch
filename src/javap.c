/*
 * Copyright (c) 2021 y193
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>

#include "classfile.h"
#include "file.h"
#include "verifier.h"

void print_usage(void) {
    printf("USAGE: javap <classes>");
}

int main(int argc, char const *argv[]) {
    if (argc <= 1) {
        print_usage();
        return 0;
    }

    uint8_t *bytes;
    long nbytes = fread_all(&bytes, argv[1]);

    struct classfile classfile;
    parse_classfile(&classfile, nbytes, bytes);

    if (!verify_class_format(&classfile)) {
        fprintf(stderr, "ClassFormatError\n");
        return 0;
    }

    char *source_file_name = "Object.java";

    printf("Compiled from \"%s\"\n", source_file_name);
    printf("public class java.lang.Object\n");
    printf("  minor version: 3\n");
    printf("  major version: 45\n");
    printf("  flags: ACC_PUBLIC\n");
    printf("Constant pool:\n");
    printf("   #1 = Integer            500000\n");
    printf("   #2 = String             #41            // @\n");
    printf("   #3 = Class              #61            // "
           "java/lang/InterruptedException\n");
    printf("...\n");
    printf("  #60 = Utf8               getClass\n");
    printf("  #61 = Utf8               java/lang/InterruptedException\n");
    printf("  #62 = Utf8               ()V\n");
    printf("{\n");
    printf("  public final native java.lang.Class getClass();\n");
    printf("    descriptor: ()Ljava/lang/Class;\n");
    printf("    flags: ACC_PUBLIC, ACC_FINAL, ACC_NATIVE\n");
    printf("...\n");
    printf("  public java.lang.Object();\n");
    printf("    descriptor: ()V\n");
    printf("    flags: ACC_PUBLIC\n");
    printf("    Code:\n");
    printf("      stack=0, locals=1, args_size=1\n");
    printf("         0: return\n");
    printf("      LineNumberTable:\n");
    printf("        line 29: 0\n");
    printf("}\n");
    printf("SourceFile: \"Object.java\"\n");

    return 0;
}

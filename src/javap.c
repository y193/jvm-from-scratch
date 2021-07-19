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
        fprintf(stderr, "XXXX\n");
        return 0;
    }

    u1 *source_file_name;

    printf("\n----------------------------------------\n");
    printf("magic: 0x%08X\n", classfile.magic);
    printf("minor_version: %u\n", classfile.minor_version);
    printf("major_version: %u\n", classfile.major_version);
    printf("constant_pool_count: %u\n", classfile.constant_pool_count);
    printf("constant_pool: [%s]\n",
           classfile.constant_pool == NULL ? "" : "...");
    printf("access_flags: 0x%04X\n", classfile.access_flags);
    printf("this_class: %u\n", classfile.this_class);
    printf("super_class: %u\n", classfile.super_class);
    printf("interfaces_count: %u\n", classfile.interfaces_count);
    printf("interfaces: [%s]\n", classfile.interfaces == NULL ? "" : "...");
    printf("fields_count: %u\n", classfile.fields_count);
    printf("fields: [%s]\n", classfile.fields == NULL ? "" : "...");
    printf("methods_count: %u\n", classfile.methods_count);
    printf("methods: [%s]\n", classfile.methods == NULL ? "" : "...");
    printf("attributes_count: %u\n", classfile.attributes_count);
    printf("attributes: [%s]\n", classfile.attributes == NULL ? "" : "...");
    printf("----------------------------------------\n");

    return 0;
}

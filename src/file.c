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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "file.h"

static long file_size(const char *path) {
    struct stat st;

    if (stat(path, &st) != 0)
        return -1;

    if ((st.st_mode & S_IFMT) != S_IFREG)
        return -1;

    if (st.st_size > LONG_MAX)
        return -1;

    return st.st_size;
}

long fread_all(uint8_t **bytes_ptr, const char *path) {
    *bytes_ptr = NULL;

    long size = file_size(path);

    if (size <= 0)
        return -1;

    uint8_t *bytes = malloc(size);

    if (bytes == NULL)
        return -1;

    FILE *stream = fopen(path, "rb");

    if (stream != NULL) {
        size_t num = fread(bytes, sizeof(uint8_t), size, stream);
        fclose(stream);

        if (num <= LONG_MAX && (long)num == size) {
            *bytes_ptr = bytes;
            return size;
        }
    }

    free(bytes);

    return -1;
}

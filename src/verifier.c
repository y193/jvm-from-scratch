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
#include "verifier.h"
#include "classfile.h"

#define JAVA_MIN_SUPPORTED_VERSION 45
#define JAVA_MAX_SUPPORTED_VERSION 49

static bool verify_class_format_cp(u2 cp_count, const struct cp_info *cp) {
    if (cp_count == 0)
        return false;

    for (int i = 1; i < cp_count; i++) {
        struct cp_info cp_entry = cp[i];
        u1 tag = cp_entry.tag;
        u2 cp_index;

        switch (tag) {
        case CONSTANT_Utf8:
            break;

        case CONSTANT_Integer:
        case CONSTANT_Float:
            break;

        case CONSTANT_Long:
        case CONSTANT_Double:
            if (i >= cp_count - 1)
                return false;

            i++;
            break;

        case CONSTANT_Class:
        case CONSTANT_String:
            cp_index = get_u2(&cp_entry.info[0]);

            if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Utf8)
                return false;

            break;

        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            cp_index = get_u2(&cp_entry.info[0]);

            if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Class)
                return false;

            cp_index = get_u2(&cp_entry.info[2]);

            if (cp_index >= cp_count ||
                cp[cp_index].tag != CONSTANT_NameAndType)
                return false;

            break;

        case CONSTANT_NameAndType:
            cp_index = get_u2(&cp_entry.info[0]);

            if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Utf8)
                return false;

            cp_index = get_u2(&cp_entry.info[2]);

            if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Utf8)
                return false;

            break;

        default:
            return false;
        }
    }

    return true;
}

static bool verify_class_format_fields(u2 fields_count,
                                       const struct field_info *fields,
                                       u2 cp_count, const struct cp_info *cp) {
    for (int i = 0; i < fields_count; i++) {
        struct field_info field = fields[i];
        u2 cp_index;

        cp_index = field.name_index;

        if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Utf8)
            return false;

        cp_index = field.descriptor_index;

        if (cp_index >= cp_count || cp[cp_index].tag != CONSTANT_Utf8)
            return false;
    }

    return true;
}

bool verify_class_format(const struct classfile *classfile) {
    if (classfile->magic != 0xCAFEBABEL)
        return false;

    if (!verify_class_format_cp(classfile->constant_pool_count,
                                classfile->constant_pool))
        return false;

    if (!verify_class_format_fields(classfile->fields_count, classfile->fields,
                                    classfile->constant_pool_count,
                                    classfile->constant_pool))
        return false;

    return true;
}

bool verify_class_version(const struct classfile *classfile) {
    u2 minor = classfile->minor_version;
    u2 major = classfile->major_version;

    if (major < JAVA_MIN_SUPPORTED_VERSION ||
        JAVA_MAX_SUPPORTED_VERSION < major)
        return false;

    if (major == JAVA_MAX_SUPPORTED_VERSION && minor > 0)
        return false;

    return true;
}

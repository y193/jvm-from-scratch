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
#include <stdlib.h>

#include "classfile.h"

#define CLASSFILE_DEFUN_PARSE(type, member)                                    \
    static bool parse_##member(u2 member##_count, type *member,                \
                               const u1 **current_ptr, const u1 *end) {        \
        const u1 *current = *current_ptr;                                      \
                                                                               \
        for (u2 i = 0; i < member##_count; i++)                                \
            member[i].attributes = NULL;                                       \
                                                                               \
        for (u2 i = 0; i < member##_count; i++) {                              \
            if (end - current < 8)                                             \
                return false;                                                  \
                                                                               \
            u2 access_flags = read_u2(&current);                               \
            u2 name_index = read_u2(&current);                                 \
            u2 descriptor_index = read_u2(&current);                           \
            u2 attributes_count = read_u2(&current);                           \
            struct attribute_info *attributes = NULL;                          \
                                                                               \
            if (attributes_count > 0) {                                        \
                attributes =                                                   \
                    malloc(sizeof(struct attribute_info) * attributes_count);  \
                                                                               \
                if (attributes == NULL)                                        \
                    return false;                                              \
                                                                               \
                if (!parse_attributes(attributes_count, attributes, &current,  \
                                      end)) {                                  \
                    free(attributes);                                          \
                    return false;                                              \
                }                                                              \
            }                                                                  \
                                                                               \
            member[i] = (type){access_flags, name_index, descriptor_index,     \
                               attributes_count, attributes};                  \
        }                                                                      \
                                                                               \
        *current_ptr = current;                                                \
                                                                               \
        return true;                                                           \
    }

#define CLASSFILE_DEFUN_READ_U(n)                                              \
    u##n read_u##n(const u1 **current_ptr) {                                   \
        const u1 *current = *current_ptr;                                      \
        u##n value = get_u##n(current);                                        \
        *current_ptr += n;                                                     \
                                                                               \
        return value;                                                          \
    }

#define CLASSFILE_SUBROUTINE_PARSE(type, member)                               \
    do {                                                                       \
        if (end - current < 2)                                                 \
            goto failure;                                                      \
                                                                               \
        classfile->member##_count = read_u2(&current);                         \
                                                                               \
        if (classfile->member##_count > 0) {                                   \
            classfile->member =                                                \
                malloc(sizeof(type) * classfile->member##_count);              \
                                                                               \
            if (classfile->member == NULL)                                     \
                goto failure;                                                  \
                                                                               \
            if (!parse_##member(classfile->member##_count, classfile->member,  \
                                &current, end))                                \
                goto failure;                                                  \
        }                                                                      \
    } while (0)

static bool parse_attributes(u2 attributes_count,
                             struct attribute_info *attributes,
                             const u1 **current_ptr, const u1 *end) {
    const u1 *current = *current_ptr;

    for (u2 i = 0; i < attributes_count; i++) {
        if (end - current < 6)
            return false;

        u2 attribute_name_index = read_u2(&current);
        u4 attribute_length = read_u4(&current);

        attributes[i] = (struct attribute_info){attribute_name_index,
                                                attribute_length, current};

        if (end - current < attribute_length)
            return false;

        current += attribute_length;
    }

    *current_ptr = current;

    return true;
}

CLASSFILE_DEFUN_PARSE(struct method_info, methods)
CLASSFILE_DEFUN_PARSE(struct field_info, fields)

static bool parse_interfaces(u2 interfaces_count, u2 *interfaces,
                             const u1 **current_ptr, const u1 *end) {
    const u1 *current = *current_ptr;

    for (u2 i = 0; i < interfaces_count; i++) {
        if (end - current < 2)
            return false;

        interfaces[i] = read_u2(&current);
    }

    *current_ptr = current;

    return true;
}

static bool parse_constant_pool(u2 constant_pool_count,
                                struct cp_info *constant_pool,
                                const u1 **current_ptr, const u1 *end) {
    const u1 info_lengths[] = {0, 0, 0, 4, 4, 8, 8, 2, 2, 4, 4, 4, 4};
    const u1 *current = *current_ptr;

    constant_pool[0] = (struct cp_info){0, NULL};

    for (u2 i = 1; i < constant_pool_count; i++) {
        if (end - current < 1)
            return false;

        u1 tag = read_u1(&current);
        u2 utf8_length;

        constant_pool[i] = (struct cp_info){tag, current};

        switch (tag) {
        case CONSTANT_Utf8:
            if (end - current < 2)
                return false;

            utf8_length = read_u2(&current);

            if (end - current < utf8_length)
                return false;

            current += utf8_length;
            break;

        case CONSTANT_Long:
        case CONSTANT_Double:
            if (i >= constant_pool_count - 1)
                return false;

            i++;
            constant_pool[i] = (struct cp_info){0, NULL};

        case CONSTANT_Integer:
        case CONSTANT_Float:
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_NameAndType:
            if (end - current < info_lengths[tag])
                return false;

            current += info_lengths[tag];
            break;

        default:
            return false;
        }
    }

    *current_ptr = current;

    return true;
}

static void free_classfile(struct classfile *classfile) {
    free(classfile->constant_pool);
    free(classfile->interfaces);

    if (classfile->fields != NULL)
        for (int i = 0; i < classfile->fields_count; i++)
            free(classfile->fields[i].attributes);

    free(classfile->fields);

    if (classfile->methods != NULL)
        for (int i = 0; i < classfile->methods_count; i++)
            free(classfile->methods[i].attributes);

    free(classfile->methods);
    free(classfile->attributes);
}

static void init_classfile(struct classfile *classfile) {
    classfile->magic = 0;
    classfile->minor_version = 0;
    classfile->major_version = 0;
    classfile->constant_pool_count = 0;
    classfile->constant_pool = NULL;
    classfile->access_flags = 0;
    classfile->this_class = 0;
    classfile->super_class = 0;
    classfile->interfaces_count = 0;
    classfile->interfaces = NULL;
    classfile->fields_count = 0;
    classfile->fields = NULL;
    classfile->methods_count = 0;
    classfile->methods = NULL;
    classfile->attributes_count = 0;
    classfile->attributes = NULL;
}

u1 get_u1(const u1 *current) {
    return current[0];
}

u2 get_u2(const u1 *current) {
    return (current[0] << 8) | current[1];
}

u4 get_u4(const u1 *current) {
    return (current[0] << 24) | (current[1] << 16) | (current[2] << 8) |
           current[3];
}

CLASSFILE_DEFUN_READ_U(1)
CLASSFILE_DEFUN_READ_U(2)
CLASSFILE_DEFUN_READ_U(4)

bool parse_classfile(struct classfile *classfile, long nbytes,
                     const u1 *bytes) {
    init_classfile(classfile);

    const u1 *current = bytes;
    const u1 *end = bytes + nbytes;

    if (end - current < 4)
        goto failure;

    classfile->magic = read_u4(&current);

    if (classfile->magic != 0xCAFEBABEL)
        goto failure;

    if (end - current < 6)
        goto failure;

    classfile->minor_version = read_u2(&current);
    classfile->major_version = read_u2(&current);
    classfile->constant_pool_count = read_u2(&current);

    if (classfile->constant_pool_count == 0)
        goto failure;

    classfile->constant_pool =
        malloc(sizeof(struct cp_info) * classfile->constant_pool_count);

    if (classfile->constant_pool == NULL)
        goto failure;

    if (!parse_constant_pool(classfile->constant_pool_count,
                             classfile->constant_pool, &current, end))
        goto failure;

    if (end - current < 6)
        goto failure;

    classfile->access_flags = read_u2(&current);
    classfile->this_class = read_u2(&current);
    classfile->super_class = read_u2(&current);

    CLASSFILE_SUBROUTINE_PARSE(u2, interfaces);
    CLASSFILE_SUBROUTINE_PARSE(struct field_info, fields);
    CLASSFILE_SUBROUTINE_PARSE(struct method_info, methods);
    CLASSFILE_SUBROUTINE_PARSE(struct attribute_info, attributes);

    if (current == end)
        return true;

failure:
    free_classfile(classfile);
    init_classfile(classfile);

    return false;
}

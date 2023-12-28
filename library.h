#ifndef LIBS_LIBRARY_H
#define LIBS_LIBRARY_H

#include <printf.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>


typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef float f32;
typedef double f64;
typedef int bool;
#define false 0
#define true 1


// ======= ArrayList LIBRARY =======

#define ARRAY_LIST_DEFAULT_CAP 10
typedef struct ArrayList {
    void * _Nonnull data;
    u32 len;
    u32 cap;
    const u32 elem_size;
} ArrayList;

ArrayList arraylist_new(u32 elem_size);

#define array_push(list, elem) _Generic((elem), \
    u32: arraylist_push(list, &(u32){elem}),    \
    i32: arraylist_push(list, &(i32){elem}),    \
    u64: arraylist_push(list, &(u64){elem}),    \
    i64: arraylist_push(list, &(i64){elem}),    \
    f32: arraylist_push(list, &(f32){elem}),    \
    f64: arraylist_push(list, &(f64){elem}),    \
    u8: arraylist_push(list, &(u8){elem}),      \
    i8: arraylist_push(list, &(i8){elem}),      \
    u16: arraylist_push(list, &(u16){elem}),    \
    i16: arraylist_push(list, &(i16){elem}))//,    \
//    default: arraylist_push(list, elem))    \

void arraylist_push(ArrayList * _Nonnull list, void * _Nonnull elem);

void arraylist_remove(ArrayList * _Nonnull list, u32 index);

void arraylist_free(ArrayList * _Nonnull list);

#define arraylist_last(list, type) ((type *)list.data)[list.len - 1]
#define forward_it(list, type) for (type *it = list.data; it != (type *)list.data + list.len; it++)
#define reverse_it(list, type) for (type *it = &arraylist_last(list, type); it != (type *)list.data-1; it--)
#define arraylist_at(list, index, type) ((type *)list.data)[index]
#define arraylist_sort(list, compare) qsort((list).data, (list).len, (list).elem_size, compare);


// ======= STRING LIBRARY =======

typedef struct Str {
    const char * _Nonnull str;
    u32 len;
} Str;

typedef struct FormatOption {
    Str format;
    void (* _Nonnull printer)(va_list * _Nonnull args);
} FormatOption;
#define MAX_FORMAT_OPTIONS 20

void __attribute__((overloadable)) print(char *_Nonnull format, ...);
void __attribute__((overloadable)) print(const Str format, ...);

bool str_eq(const Str a, const Str b);
const Str str_concat(const Str a, const Str b);
const Str str_from_cstr(const char * _Nonnull cstr);
bool str_a_contains_b(const Str a, const Str b);

void str_register(const char * _Nonnull format, void (* _Nonnull printer)(va_list * _Nonnull args));

#ifndef LIBS_IMPLEMENTATION
#define LIBS_IMPLEMENTATION

ArrayList arraylist_new(u32 elem_size) {
    ArrayList list = {
        .len = 0,
        .elem_size = elem_size,
        .cap = ARRAY_LIST_DEFAULT_CAP,
        .data = malloc(ARRAY_LIST_DEFAULT_CAP * elem_size)
    };
    assert(list.data); // Failed to allocate memory.
    return list;
}

void arraylist_free(ArrayList * _Nonnull list) {
    free(list->data);
}

void arraylist_push(ArrayList * _Nonnull list, void * _Nonnull elem) {
    if (list->len == list->cap) {
        list->cap *= 2;
        list->data = realloc(list->data, list->cap * list->elem_size);
        assert(list->data); // Failed to reallocate memory.
    }
    u32 offset = list->len * list->elem_size;
    for (u32 i = 0; i < list->elem_size; i++) {
        ((char *) list->data)[offset + i] = ((char *) elem)[i];
    }
    list->len++;
}

void arraylist_remove(ArrayList * _Nonnull list, u32 index) {
    assert(index < list->len); // Index out of bounds.
    u32 offset = index * list->elem_size;
    for (u32 j = index; j < list->len; j++) {
        for (u32 i = 0; i < list->elem_size; i++) {
            ((char *) list->data)[offset + i] = ((char *) list->data)[offset + list->elem_size + i];
        }
        offset += list->elem_size;
    }

    list->len--;
}

static u32 _num_options = 0;
static FormatOption options[MAX_FORMAT_OPTIONS] = {0};
void str_register(const char * _Nonnull format, void (* _Nonnull printer)(va_list * _Nonnull args)) {
    assert(_num_options < MAX_FORMAT_OPTIONS); // Ensure that enough space has been allocated for the format options.
    options[_num_options].format = str_from_cstr(format);
    options[_num_options].printer = printer;
    _num_options++;
}

void i32_Printer(va_list * _Nonnull args) {
    i32 i = va_arg(*args, i32);
    printf("%d", i);
}

void u32_Printer(va_list * _Nonnull args) {
    u32 i = va_arg(*args, u32);
    printf("%u", i);
}

void i64_Printer(va_list * _Nonnull args) {
    i64 i = va_arg(*args, i64);
    printf("%lld", i);
}

void u64_Printer(va_list * _Nonnull args) {
    u64 i = va_arg(*args, u64);
    printf("%llu", i);
}

void f32_Printer(va_list * _Nonnull args) {
    f32 i = va_arg(*args, f64);
    printf("%f", i);
}

void f64_Printer(va_list * _Nonnull args) {
    f64 i = va_arg(*args, f64);
    printf("%f", i);
}

void u8_Printer(va_list * _Nonnull args) {
    u8 i = va_arg(*args, u32);
    printf("%u", i);
}

void i8_Printer(va_list * _Nonnull args) {
    i8 i = va_arg(*args, i32);
    printf("%d", i);
}

void u16_Printer(va_list * _Nonnull args) {
    u16 i = va_arg(*args, u32);
    printf("%u", i);
}

void i16_Printer(va_list * _Nonnull args) {
    i16 i = va_arg(*args, i32);
    printf("%d", i);
}

void bool_Printer(va_list * _Nonnull args) {
    bool i = va_arg(*args, bool);
    if (i) {
        printf("true");
    } else {
        printf("false");
    }
}

void str_Printer(va_list * _Nonnull args) {
    Str str = va_arg(*args, Str);
    printf("%.*s", str.len, str.str);
}

#if defined(__clang__)
static void init_printers(void) __attribute__((constructor)) {
    str_register("{i32}", i32_Printer);
    str_register("{u32}", u32_Printer);
    str_register("{i64}", i64_Printer);
    str_register("{u64}", u64_Printer);
    str_register("{f32}", f32_Printer);
    str_register("{f64}", f64_Printer);
    str_register("{u8}", u8_Printer);
    str_register("{i8}", i8_Printer);
    str_register("{u16}", u16_Printer);
    str_register("{i16}", i16_Printer);
    str_register("{bool}", bool_Printer);
    str_register("{str}", str_Printer);
}
#endif

static inline void print_impl(const Str format, va_list args) {
    u32 last_printed = 0;
    // Scan through the format string and discover any registered format options.
    for (u32 i = 0; i < format.len; i++) {
        if (format.str[i] == '\\' && format.str[i + 1] == '{') {
            // We want to print upto here, but not the next character.
            printf("%.*s{", i - last_printed, format.str + last_printed);
            i++;
            last_printed = i + 1;
            continue;
        }
        if (format.str[i] == '{') {
            printf("%.*s", i - last_printed, format.str + last_printed);
            u32 j = i;
            bool found = false;
            while (j < format.len) {
                j++;
                if (format.str[j] == '}') {
                    found = true;
                    break;
                }
            }
            if ( found ) {
                last_printed = j + 1;
                Str option = {format.str + i, j - i + 1};
                // Match for any registered options.
                found = false;
                for ( u32 k = 0; k < _num_options; k++ ) {
                    if ( str_eq( option, options[k].format ) ) {
                        options[k].printer(&args);
                        i = j + 1;
                        found = true;
                        break;
                    }
                    assert(k < _num_options); // No matching format option found.
                }
                assert(found); // Did not find a matching format option.
            } else {
                assert(false); // No matching '}' found.
            }
        }
    }
    printf("%.*s", format.len - last_printed, format.str + last_printed);
}

void __attribute__((overloadable)) print(char * _Nonnull format_c, ...) {
    va_list args;
    va_start(args, format_c);
    const Str format = str_from_cstr(format_c);
    print_impl(format, args);
    va_end(args);
}

void __attribute__((overloadable)) print(const Str format, ...) {
    va_list args;
    va_start(args, format);
    print_impl(format, args);
    va_end(args);
}

const Str str_concat(const Str a, const Str b) {
    u32 len = a.len + b.len;
    char *str = malloc(len + 1);
    for (u32 i = 0; i < a.len; i++) {
        str[i] = a.str[i];
    }
    for (u32 i = 0; i < b.len; i++) {
        str[a.len + i] = b.str[i];
    }
    str[len] = '\0';
    Str c = {str, len};
    return c;
}

const Str str_from_cstr(const char * _Nonnull cstr) {
    Str str = {cstr, 0};
    while (cstr[str.len] != '\0') {
        str.len++;
    }
    return str;
}

bool str_eq(Str a, Str b) {
    if (a.len != b.len) {
        return 0;
    }
    for (u32 i = 0; i < a.len; i++) {
        if (a.str[i] != b.str[i]) {
            return 0;
        }
    }
    return 1;
}

bool str_a_contains_b(Str a, Str b) {
    if (a.len < b.len) {
        return 0;
    }
    for (u32 i = 0; i < a.len - b.len; i++) {
        bool found = 1;
        for (u32 j = 0; j < b.len; j++) {
            if (a.str[i + j] != b.str[j]) {
                found = 0;
                break;
            }
        }
        if (found) {
            return 1;
        }
    }
    return 0;
}

#endif

#endif //LIBS_LIBRARY_H

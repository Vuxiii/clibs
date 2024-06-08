#ifndef JLIB_H
#define JLIB_H
#include <printf.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef float f32;
typedef double f64;

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
#define forward_it(list, type) for (type *it = (type *)(list).data; it != (type *)(list).data + (list).len; it++)
#define reverse_it(list, type) for (type *it = &arraylist_last(list, type); it != (type *)list.data-1; it--)
#define arraylist_at(list, index, type) ((type *)list.data)[index]
#define arraylist_sort(list, compare) qsort((list).data, (list).len, (list).elem_size, compare);


// ======= STRING LIBRARY =======


// MARK: - Str View

#define _j_str_view_base_end(view) ((view)->base->str + (view)->base->len)
#define _j_str_view_end(view) ((view)->current.str + (view)->current.len)
#ifdef J_STR_VIEW_SHOULD_CLAMP
#define _j_str_view_clamp(view) ((view).current.str = _j_str_view_end(view) > _j_str_view_base_end(view) ? (view).base->str + (view).base->len - (view).current.len : (view).current.str)
#else
#define _j_str_view_clamp(view) ((void)0)
#endif
#define j_str_view_set_width(view, n) ((view)->current.len = (n), _j_str_view_clamp(view))
#define j_str_view_reset(view) ((view)->current = *(view)->base)
#define j_str_view_advance(view, n) ((view)->current.str += (n), _j_str_view_clamp(view), &(view)->current)
#define j_str_view_next(view) ((view)->current.str++, &(view)->current)
#define j_str_view_prev(view) ((view)->current.str--, &(view)->current)
#define j_str_view_at(view, n) ((view)->current.str[n])
#define j_str_view_eq(view, str) (str_eq((view)->current, str))
#define j_str_view_eq_cstr(view, cstr) (str_eq((view)->current, str_from_cstr(cstr)))
#define j_str_view_current(view) (&((view)->current))
#define j_str_view_has_next(view) (_j_str_view_end(view) <= _j_str_view_base_end(view))
#define j_str_view_has_next_n(view, n) (_j_str_view_end(view) + (n) <= _j_str_view_base_end(view))

#define j_list(type) type * _Nullable

typedef struct Str {
    char * _Nonnull str;
    u32 len;
} Str;

typedef struct Str_View {
    const Str * _Nonnull base;
    Str current;
} Str_View;

typedef Str IStr;

typedef struct FormatOption {
    Str format;
    const Str (* _Nonnull printer)(va_list * _Nonnull args);
} FormatOption;

void __attribute__((overloadable)) print(char *_Nonnull format, ...);
void __attribute__((overloadable)) print(const Str format, ...);

bool str_eq(const Str a, const Str b);
const Str str_concat(const Str a, const Str b);
Str str_from_cstr(const char * _Nonnull cstr);
Str_View str_view(Str *str);
bool str_a_contains_b(const Str a, const Str b);

Str str_build_from_arraylist( const j_list(Str) list );

const Str __attribute__((overloadable)) str_format(char * _Nonnull format, ...);
const Str __attribute__((overloadable)) str_format(const Str format, ...);

void str_register(const char * _Nonnull format, const Str (* _Nonnull printer)(va_list * _Nonnull args));

#define cast(type, value) ((type)(value))
#define JASSERT(condition, message) (assert(condition && message))
#define jassert(condition, message) (JASSERT(condition, message))
#endif
#ifdef JLIB_IMPL

// MARK: - Argument Parser

// MARK: - FileSystem

enum J_FS_WALK_OPTIONS {
    J_FS_WALK_INCLUDE_FOLDER,
    J_FS_WALK_INCLUDE_REGULAR,
    J_FS_WALK_ORDER_LEXICOGRAPHIC, // TODO: William Implement me
    J_FS_WALK_COUNT_FOLDER_ENTRIES, // TODO: William Implement me
    J_FS_WALK_INCLUDE_DOT, // TODO: William Implement me
    J_FS_WALK_INCLUDE_DOTDOT, // TODO: William Implement me
};

typedef struct Path {
    Str  * _Nullable components;
} Path;

typedef struct FS_Dir {
    DIR * _Nullable dir;
    i32 remaining_entries;
} FS_Dir;

// Currently only supports directories and files.
// TODO: William Give this an arena allocator to cleanup the memory easier.
typedef struct FS_Walker {
    FS_Dir * _Nullable open_directories;
    Path path;
    u32 options;
} FS_Walker;

typedef struct FS_Entry {
    bool is_last;
    struct dirent * _Nullable dirent;
} FS_Entry;

// MARK: - Bit Field

#define j_bit_set(field, bit) ((field) |= (cast(typeof(field), 1) << (bit)))
#define j_bit_check(field, bit) (!!((field) & (cast(typeof(field), 1) << (bit))))



// MARK: - Basic Data Structures Stamps

#define EXPAND(x) x
#define CONCAT(x, y) x ## y
#define J_MAYBE(type) Maybe##type
#define J_PAIR(ft, st) Pair_##ft##_##st
#define J_LIST(type) type * _Nullable

#define _J_STAMP_PAIR(ft, st) \
typedef struct Pair_##ft##_##st { \
    ft first;                 \
    st second;                \
} Pair_##ft##_##st

typedef struct Maybe {
    bool is_present;
    void *value;
} Maybe;

#define _J_STAMP_MAYBE(type) \
typedef struct Maybe ## type { \
    bool is_present;         \
    type value;              \
} Maybe##type

// Mark: - Pair
#define j_pair(ft, st) J_PAIR(ft, st)

#define _j_stamp_pair(ft, st) _J_STAMP_PAIR(ft, st)
#define _j_stamp_maybe(type) _J_STAMP_MAYBE(type)

_j_stamp_pair(u32, u32);
_j_stamp_pair(Str, Str);

// MARK: - Maybe
_j_stamp_maybe(Str);
_j_stamp_maybe(u32);
_j_stamp_maybe(FS_Walker);
_j_stamp_maybe(FS_Entry);
_j_stamp_maybe(j_pair(Str, Str));

#define j_maybe(type) J_MAYBE(type)

//TODO: William Also do a j_view(type)

// MARK: - ArrayList New
#define EMPTY_ARRAY NULL
//#define j_list(type) J_LIST(type)

typedef struct ArrHeader {
    u32 len;
    u32 cap;
} ArrHeader;

#define j_al_header(list) ((list) ? cast(ArrHeader *, list) - 1 : EMPTY_ARRAY)
#define j_al_len(list) ((list) ? j_al_header(list)->len : 0)
#define j_al_cap(list) ((list) ? j_al_header(list)->cap : 0)
#define _j_al_init(list) do\
{                       \
    if ((list) == EMPTY_ARRAY) { \
        (list) = malloc(10 * sizeof(list[0]) + sizeof(ArrHeader)) + sizeof(ArrHeader); \
        j_al_header(list)->len = 0;      \
        j_al_header(list)->cap = 10;      \
    }                   \
} while(0)
#define _j_al_next_size(list) (j_al_cap(list) * sizeof(list[0]) * 2)
#define _j_al_realloc_new_size(list) (sizeof(ArrHeader) + _j_al_next_size(list))
#define _j_al_realloc(list) do \
{                             \
    if(j_al_len(list) == j_al_cap(list)) { \
        void *p = realloc(j_al_header(list), _j_al_realloc_new_size(list)); \
        jassert(p, "Failed to realloc\n"); \
        (list) = p + sizeof(ArrHeader);      \
        j_al_header(list)->cap *= 2;       \
    }                          \
} while(0)
#define j_al_append(list, elem) do \
{                                  \
    _j_al_init(list);               \
    _j_al_realloc(list);            \
    list[j_al_len(list)] = (elem);   \
    j_al_header(list)->len += 1;   \
} while(0)
#define j_al_swap(list, i, j) do \
{                               \
    typeof(list[i]) temp = list[i]; \
    list[i] = list[j];            \
    list[j] = temp;               \
} while(0)
#define j_al_removeLast(list) (j_al_header(list)->len -= 1, list[j_al_len(list)])
#define j_al_removeFirst(list) (memmove(list, list+1, sizeof(list[0]) * (j_al_len(list)-1)), j_al_header(list)->len -= 1)
#define j_al_last(list) ((list)[j_al_len(list)-1])
#define j_al_free(list) (free(j_al_header(list)), list = NULL)



// MARK: - HashMap
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
static u64 j_hmap_generic_hash(const void *key, size_t len) {
    uint64_t hash = FNV_OFFSET;
    size_t count = 0;
    for (const char* p = key; count < len; ++count) { // TODO: William check if we are indeed using NULL terminated strings.
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

static bool j_hmap_generic_compare(const void *lhs, const void *rhs, size_t len) {
    return memcmp(lhs, rhs, len) == 0;
}

#define j_hmap(ktp, vtp) j_maybe( j_pair(ktp,vtp) ) * _Nullable
#define EMPTY_HMAP NULL

typedef struct HMapHeader {
    u32 cap; // It is going to be quite expensive to realloc the map because we need to rehash all the keys.
    u32 len;
    bool (*compare)(const void *lhs, const void *rhs, size_t len);
    u64 (*hasher)(const void *key, size_t len);
} HMapHeader;

u32 j_hmap_get_slot_for_key(HMapHeader *map, void *key, size_t key_size, size_t entry_size) {
    jassert(map->len < map->cap, "Precondition: The map must have space for the new entry.\n");

    u32 index = map->hasher(key, key_size) % map->cap;
    char *entries = cast(char *, map + 1);
    // NOTE: Here we are assuming that the entry is a Maybe and the first element is the is_present field.
    // Here we also want to check if they are the same. If they are we return that index.
    while (cast(bool, *(entries + index * entry_size)) == true) {
        char *value_entry = entries + index *entry_size + offsetof(struct Maybe, value);
        // Again, Here we are assuming that the first entry is the key aka j_pair.first.
        if (map->compare(value_entry, key, key_size) == true) {
            print("Update on Index: {u32}\n", index);
            return index;
        }
        index = (index + 1) % map->cap;
    }

    print("New on Index: {u32}\n", index);
    map->len++;
    return index;
}

#define j_hmap_header(map) ((map) ? cast(HMapHeader *, map) - 1 : EMPTY_HMAP)
#define _j_hmap_init_compare(map, compare_func) j_hmap_header(map)->compare = (compare_func)
#define _j_hmap_init_hasher(map, hasher_func) j_hmap_header(map)->hasher = (hasher_func)
#define j_hmap_init(map, hasher_func, compare_func, capacity) \
({                       \
    if ((map) == EMPTY_HMAP) { \
        (map) = calloc(capacity, sizeof(map[0]) + sizeof(HMapHeader)) + sizeof(HMapHeader); \
        j_hmap_header(map)->len = 0;                                                   \
        j_hmap_header(map)->cap = (capacity);      \
        _j_hmap_init_compare(map, compare_func);                                        \
        _j_hmap_init_hasher(map, hasher_func);                                        \
    }                                      \
})

#define j_hmap_cap(map) ((map) ? j_hmap_header(map)->cap : 0)
#define j_hmap_len(map) ((map) ? j_hmap_header(map)->len : 0)
#define _j_hmap_next_size(map) (j_hmap_cap(map) * sizeof(map[0]) * 2)
#define j_hmap_put(map, key, valuet) \
({                                  \
    j_hmap_init(map, j_hmap_generic_hash, j_hmap_generic_compare, 10); \
    jassert(j_hmap_len(map) < j_hmap_cap(map), "Precondition: There must be room for the entry inside the hashmap"); \
    u32 index = j_hmap_get_slot_for_key(j_hmap_header(map), &key, sizeof(key), sizeof(map[0])); \
    map[index].value.first = key;   \
    map[index].value.second = (valuet);\
    map[index].is_present = true;   \
})
#define j_hmap_remove(map, key) \
({                              \
    u32 index = j_hmap_get_slot_for_key(j_hmap_header(map), &key, sizeof(key), sizeof(map[0])); \
    jassert(map[index].is_present == true, "Precondition: The key MUST be in the map before removing it!\n"); \
    map[index].is_present = false;                                                              \
    j_hmap_header(map)->len--;  \
})
#define j_hmap_removeAll(map) do \
{                               \
    for (u32 i = 0; i < j_hmap_cap(map); i++) { \
        map[i].is_present = false; \
    }                           \
    j_hmap_header(map)->len = 0; \
} while(0)
#define j_hmap_get(map, key) ({ \
    u32 index = j_hmap_header(map)->hasher(&key, sizeof(key)) % j_hmap_cap(map); \
    u32 offset = 0;                            \
    while (j_hmap_header(map)->compare(&map[(index+offset)].value.first, &key, sizeof(key)) == false) { \
        offset += 1;            \
        if (index+offset == j_hmap_cap(map)) {                                   \
            index = 0;                        \
        }                        \
        jassert(offset < j_hmap_cap(map), "Precondition: The key must exist in the hmap before calling this function.\n"); \
    }                           \
    print("Get on Index: {u32}\n", (index + offset));           \
    map[(index+offset)].value.second;                          \
})
#define j_hmap_is_empty(map) (j_hmap_len(map) == 0)
#define j_hmap_iter_next(map, it) ({ \
    u32 index = (it).value + 1; \
    while (index < j_hmap_cap(map)) { \
        if (map[index].is_present) { \
            break; \
        } \
        index++; \
    }                       \
    Maybeu32 res = {0};                        \
    if (index == j_hmap_cap(map)) {   \
        res.is_present = false;\
    } else {                \
        res.is_present = true;        \
        res.value = index;  \
    }                       \
    res;                    \
})
#define j_hmap_iter(map) j_hmap_iter_next(map, ((Maybeu32){false, -1}))
#define j_hmap_iter_get(map, it) (jassert((it).is_present == true, "Precondition: Cannot call get on an nil value"), map[(it).value].value)

bool j_hmap_compare_str(const void *lhs, const void *rhs, size_t len) {
    return str_eq(*(Str *)lhs, *(Str *)rhs);
}

u64 j_hmap_hash_str(const void *key, size_t len) {
    return j_hmap_generic_hash(((Str *)key)->str, ((Str *)key)->len);
}


// MARK: - Red Black Tree




// MARK: - Regex

typedef struct Regex_Edge {
    i32 c: 8;           // Some character
    u32 is_wildcard: 1; // .
    u32 is_optional: 1; // ?
    u32 is_plus: 1;     // +
    u32 is_star: 1;     // *
    u32 is_union: 1;    // |
    u32 is_digit: 1;    // \d [:digit:] [0-9]
    u32 is_alpha: 1;    // \w [:alpha:] [a-zA-Z]
    u32 is_alnum: 1;    //    [:alnum:] [a-zA-Z0-9]
    u32 is_space: 1;    // \s [:space:] [ \t\n\r\f\v]
    u32 is_lower: 1;    // \l [:lower:] [a-z]
    u32 is_upper: 1;    // \u [:upper:] [A-Z]

    // For implementation...
    u32 _is_epsilon: 1;
} Regex_Edge;
typedef u32 Vertex_Descriptor;

_j_stamp_pair(Regex_Edge, Vertex_Descriptor);

typedef struct Regex_Vertex {
    j_list(j_pair(Regex_Edge, Vertex_Descriptor)) edges;
} Regex_Vertex;

typedef struct Regex {
    j_list(Regex_Edge) edges;
    j_list(Regex_Vertex) vertices;
    j_list(Vertex_Descriptor) accepting_states;
    Vertex_Descriptor start_state;
    Vertex_Descriptor current_state;
} Regex;

_j_stamp_maybe(Regex);


j_maybe(Regex) j_regex(Str pattern);
bool j_regex_tokenize_next(j_list(Regex_Edge) *tokens, Str_View *pattern);

bool j_regex_tokenize_next(j_list(Regex_Edge) *tokenss, Str_View *pattern) {
    if (j_str_view_has_next(pattern) == false) {
        return false;
    }
    j_list(Regex_Edge) tokens = *tokenss;
    if (j_str_view_eq_cstr(pattern, ".")) {
        j_al_append(tokens, (Regex_Edge) {.is_wildcard = true});
    } else if (j_str_view_eq_cstr(pattern, "|")) {
        j_al_append(tokens, (Regex_Edge) {.is_union = true});
    } else if (j_str_view_eq_cstr(pattern, "*")) {
        j_al_append(tokens, (Regex_Edge) {.is_star = true});
    } else if (j_str_view_eq_cstr(pattern, "+")) {
        j_al_append(tokens, (Regex_Edge) {.is_plus = true});
    } else if (j_str_view_eq_cstr(pattern, "?")) {
        j_al_append(tokens, (Regex_Edge) {.is_optional = true});
    } else if (j_str_view_eq_cstr(pattern, "\\")) {
        j_str_view_set_width(pattern, 2);
        if (j_str_view_eq_cstr(pattern, "\\d")) {
            j_al_append(tokens, (Regex_Edge) {.is_digit = true});
        } else if (j_str_view_eq_cstr(pattern, "\\w")) {
            j_al_append(tokens, (Regex_Edge) {.is_alpha = true});
        } else if (j_str_view_eq_cstr(pattern, "\\s")) {
            j_al_append(tokens, (Regex_Edge) {.is_space = true});
        } else if (j_str_view_eq_cstr(pattern, "\\l")) {
            j_al_append(tokens, (Regex_Edge) {.is_lower = true});
        } else if (j_str_view_eq_cstr(pattern, "\\u")) {
            j_al_append(tokens, (Regex_Edge) {.is_upper = true});
        } else if (j_str_view_eq_cstr(pattern, "\\.")) {
            j_al_append(tokens, (Regex_Edge) {.c = '.'});
        } else if (j_str_view_eq_cstr(pattern, "\\|")) {
            j_al_append(tokens, (Regex_Edge) {.c = '|'});
        } else if (j_str_view_eq_cstr(pattern, "\\*")) {
            j_al_append(tokens, (Regex_Edge) {.c = '*'});
        } else if (j_str_view_eq_cstr(pattern, "\\+")) {
            j_al_append(tokens, (Regex_Edge) {.c = '+'});
        } else if (j_str_view_eq_cstr(pattern, "\\?")) {
            j_al_append(tokens, (Regex_Edge) {.c = '?'});
        } else {
            jassert(false, "Unknown escape sequence\n");
        }
        j_str_view_next(pattern);
        j_str_view_set_width(pattern, 1);
    } else {
        char c = j_str_view_current(pattern)->str[0];
        j_al_append(tokens, (Regex_Edge) {.c = c });
    }
    j_str_view_next(pattern);
    *tokenss = tokens;
    return true;
}

MaybeRegex j_regex(Str pattern) {

    //TODO: William Fix this macro text subst....
    j_list(Regex_Edge) tokens = EMPTY_ARRAY;

    Str_View view = str_view(&pattern);
    j_str_view_set_width(&view, 1);

    Regex re = {
            .edges = EMPTY_ARRAY,
            .vertices = EMPTY_ARRAY,
            .accepting_states = EMPTY_ARRAY,
            .start_state = 0,
            .current_state = 0,
    };

    bool success;
    Vertex_Descriptor previous;
    while ((success = j_regex_tokenize_next(&tokens, &view)) == true) {

        Regex_Edge edge = j_al_last(tokens);
        Regex_Vertex vertex = {
                .edges = EMPTY_ARRAY
        };

        j_al_append(re.edges, edge);
        j_al_append(re.vertices, vertex);

        previous = j_al_len(re.vertices) - 1;
    }

    return (MaybeRegex) { .is_present = false };
}


// MARK: - ArrayList
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
        void *p = realloc(list->data, list->cap * list->elem_size);
        assert(p); // Failed to reallocate memory.
        list->data = p;
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

static j_list(FormatOption) options = EMPTY_ARRAY;

void str_register(const char * _Nonnull format, const Str (* _Nonnull printer)(va_list * _Nonnull args)) {
    j_al_append(options, ((FormatOption) {.format = str_from_cstr(format), .printer = printer}));
}

static const Str i32_Printer(va_list * _Nonnull args) {
    i32 number = va_arg(*args, i32);
    u32 len = 0;
    i32 temp = number;
    if (number == 0) {
        return str_from_cstr("0");
    }
    if (temp < 0) {
        len++;
        temp = -temp;
    }
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)malloc(len + 1);
    str[len] = '\0';
    temp = number;
    if (temp < 0) {
        str[0] = '-';
        temp = -temp;
    }
    for (u32 i = len - 1; i > 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    if (number >= 0) {
        str[0] = (temp % 10) + '0';
    }
    return (Str){str, len};
}

static const Str u32_Printer(va_list * _Nonnull args) {
    u32 number = va_arg(*args, u32);
    if (number == 0) {
        char *str = (char *)malloc(2);
        str[0] = '0';
        str[1] = '\0';
        return (Str){str, 1};
    }
    u32 len = 0;

    u32 temp = number;
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)malloc(len + 1);
    str[len+1] = '\0';
    temp = number;
    for (i32 i = len - 1; i >= 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    return (Str){str, len};
}

static const Str i64_Printer(va_list * _Nonnull args) {
    i64 number = va_arg(*args, i64);
    u32 len = 0;
    i64 temp = number;
    if (number == 0) {
        return str_from_cstr("0");
    }
    if (temp < 0) {
        len++;
        temp = -temp;
    }
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)malloc(len + 1);
    str[len] = '\0';
    temp = number;
    if (temp < 0) {
        str[0] = '-';
        temp = -temp;
    }
    for (u32 i = len - 1; i > 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    if (number >= 0) {
        str[0] = (temp % 10) + '0';
    }
    return (Str){str, len};
}

static const Str u64_Printer(va_list * _Nonnull args) {
    u64 number = va_arg(*args, u64);
    if (number == 0) {
        char *str = (char *)malloc(2);
        str[0] = '0';
        str[1] = '\0';
        return (Str){str, 1};
    }
    u32 len = 0;

    u64 temp = number;
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)malloc(len + 1);
    str[len+1] = '\0';
    temp = number;
    for (i32 i = len - 1; i >= 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    return (Str){str, len};
}

static const Str f32_Printer(va_list * _Nonnull args) {
    f64 number = va_arg(*args, f64);

    i32 int_part = (i32)number;
    f32 frac_part = (number - int_part);
    i32 pad_with_zero = -1;
    u32 frac_len = 0;
    while (frac_part != (i32)frac_part) {
        if ((i32)frac_part == 0) {
            pad_with_zero++;
        } else {
            ++frac_len;
        }
        frac_part *= 10;
    }
    i32 frac_part_i32 = (i32)frac_part;

    Str int_str;
    {
        u32 len = 0;
        i64 temp = int_part;
        if (temp == 0) {
            int_str = str_from_cstr("0");
        } else {
            if (temp < 0) {
                len++;
                temp = -temp;
            }
            while (temp > 0) {
                len++;
                temp /= 10;
            }
            char *str = (char *)malloc(len + 1);
            str[len] = '\0';
            temp = int_part;
            if (temp < 0) {
                str[0] = '-';
                temp = -temp;
            }
            for (u32 i = len - 1; i > 0; i--) {
                str[i] = (temp % 10) + '0';
                temp /= 10;
            }
            if (temp >= 0) {
                str[0] = (temp % 10) + '0';
            }
            int_str = (Str) {str, len};
        }
    }
    Str frac_str;
    {
        u32 len = 1 + frac_len;
        i64 temp = frac_part_i32;
        if (number == 0) {
            frac_str = str_from_cstr("0");
        } else {
            if (temp < 0) {
                temp = -temp;
            }
            len += pad_with_zero;
            char *str = (char *)malloc(len + 1);
            str[len] = '\0';
            temp = frac_part_i32;
            if (temp < 0) {
                temp = -temp;
            }
            for (i32 i = len - 1; i > 0; i--) {
                str[i] = (temp % 10) + '0';
                temp /= 10;
            }
            if (temp >= 0) {
                str[0] = (temp % 10) + '0';
            }
            for (i32 i = 0; i < pad_with_zero; i++) {
                str[i] = '0';
            }
            frac_str = (Str) {str, len};
        }
    }

    const Str dot_str = str_from_cstr(".");
    const Str out = str_concat(int_str, str_concat(dot_str, frac_str));
    return out;
}

static const Str f64_Printer(va_list * _Nonnull args) {
    f64 number = va_arg(*args, f64);

    i64 int_part = (i64)number;
    f64 frac_part = (number - int_part);
    i32 pad_with_zero = -1;
    u32 frac_len = 0;
    while (frac_part != (i64)frac_part) {
        if ((i64)frac_part == 0) {
            pad_with_zero++;
        } else {
            ++frac_len;
        }
        frac_part *= 10;
    }
    i64 frac_part_i32 = (i64)frac_part;

    Str int_str;
    {
        u32 len = 0;
        i64 temp = int_part;
        if (temp == 0) {
            int_str = str_from_cstr("0");
        } else {
            if (temp < 0) {
                len++;
                temp = -temp;
            }
            while (temp > 0) {
                len++;
                temp /= 10;
            }
            char *str = (char *)malloc(len + 1);
            str[len] = '\0';
            temp = int_part;
            if (temp < 0) {
                str[0] = '-';
                temp = -temp;
            }
            for (u32 i = len - 1; i > 0; i--) {
                str[i] = (temp % 10) + '0';
                temp /= 10;
            }
            if (temp >= 0) {
                str[0] = (temp % 10) + '0';
            }
            int_str = (Str) {str, len};
        }
    }
    Str frac_str;
    {
        u32 len = 1 + frac_len;
        i64 temp = frac_part_i32;
        if (number == 0) {
            frac_str = str_from_cstr("0");
        } else {
            if (temp < 0) {
                temp = -temp;
            }
            len += pad_with_zero;
            char *str = (char *)malloc(len + 1);
            str[len] = '\0';
            temp = frac_part_i32;
            if (temp < 0) {
                temp = -temp;
            }
            for (u32 i = len - 1; i > 0; i--) {
                str[i] = (temp % 10) + '0';
                temp /= 10;
            }
            if (temp >= 0) {
                str[0] = (temp % 10) + '0';
            }
            for (i32 i = 0; i < pad_with_zero; i++) {
                str[i] = '0';
            }
            frac_str = (Str) {str, len};
        }
    }

    const Str dot_str = str_from_cstr(".");
    const Str out = str_concat(int_str, str_concat(dot_str, frac_str));
    return out;
}

static const Str bool_Printer(va_list * _Nonnull args) {
    bool i = va_arg(*args, u32);
    if (i) {
        return str_from_cstr("true");
    } else {
        return str_from_cstr("false");
    }
}

static const Str str_Printer(va_list * _Nonnull args) {
    Str str = va_arg(*args, Str);
    return str;
}

#if defined(__clang__)
static void init_printers(void) __attribute__((constructor)) {
    str_register("{i32}", i32_Printer);
    str_register("{u32}", u32_Printer);
    str_register("{i64}", i64_Printer);
    str_register("{u64}", u64_Printer);
    str_register("{f32}", f32_Printer);
    str_register("{f64}", f64_Printer);
    str_register("{bool}", bool_Printer);
    str_register("{str}", str_Printer);
}
#endif
// TODO: William Fix me -> There is a bug... {str}{str} prints "correct here{str}"
static inline const Str str_format_impl(const Str format, va_list args ) {
//    ArrayList strs = arraylist_new(sizeof(Str));
    Str *strs = EMPTY_ARRAY;
    u32 last_printed = 0;
    // Scan through the format string and discover any registered format options.
    for (u32 i = 0; i < format.len; i++) {
        if (format.str[i] == '\\' && format.str[i + 1] == '{') {
            // We want to print upto here, but not the next character.
            j_al_append(strs, ((Str) { .str = format.str + last_printed, .len = i - last_printed }));
            i++;
            last_printed = i;
            continue;
        }
        if (format.str[i] == '{') {
            j_al_append(strs, ((Str) { .str = format.str + last_printed, .len = i - last_printed }));
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
                for ( u32 k = 0; k < j_al_len(options); k++ ) {
                    if ( str_eq( option, options[k].format ) ) {
                        const Str fmt = options[k].printer(&args);
                        j_al_append(strs, fmt);
                        i = j;
                        found = true;
                        break;
                    }
                }
                assert(found); // Did not find a matching format option.
            } else {
                assert(false); // No matching '}' found.
            }
        }
    }
    Str temp_str = {
        .str = format.str + last_printed,
        .len = format.len - last_printed
    };
    j_al_append(strs, temp_str);

    Str out = str_build_from_arraylist(strs);

    // j_al_free(strs);
    return out;
}

const Str __attribute__((overloadable)) str_format(char * _Nonnull format_c, ...) {
    va_list args;
    va_start(args, format_c);
    const Str format = str_from_cstr(format_c);
    const Str out = str_format_impl(format, args);
    va_end(args);
    return out;
}

const Str __attribute__((overloadable)) str_format(const Str format, ...) {
    va_list args;
    va_start(args, format);
    const Str out = str_format_impl(format, args);
    va_end(args);
    return out;
}

void __attribute__((overloadable)) print(char * _Nonnull format_c, ...) {
    va_list args;
    va_start(args, format_c);
    const Str string = str_format_impl(str_from_cstr(format_c), args);

    write(STDOUT_FILENO, string.str, string.len);
    va_end(args);
}

void __attribute__((overloadable)) print(const Str format, ...) {
    va_list args;
    va_start(args, format);
    const Str string = str_format_impl(format, args);
    write(STDOUT_FILENO, string.str, string.len);
    va_end(args);
}

const Str str_concat(const Str a, const Str b) {
    u32 len = a.len + b.len;
    char *str = (char*)malloc(len + 1);
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


/**
 * @brief Constructs and owns a Str from a c-string.
 * @param cstr The c-string to construct the Str from.
 * @return The constructed Str.
 */
Str str_from_cstr(const char * _Nonnull cstr) {
    //TODO: (William) We can use sizeof(cstr) here.
    size_t len = strlen(cstr);
    char *p = (char *)malloc(len + 1);
    jassert(p, "Failed to allocate memory for string.");
    strncpy(p, cstr, len);
    return (Str) {.str = p, .len = len};
}

/**
 * @brief Constructs a non-owning View of a Str.
 * @param str The Str to construct the Str_View from.
 * @return The constructed Str_View.
 */
Str_View str_view(Str *str) {
    return (Str_View) {
        .base = str,
        .current = *str
    };
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

Str str_build_from_arraylist( const j_list(Str) list ) {
    u32 total_len = 0;
    for (u32 i = 0; i < j_al_len(list); i++) {
        total_len += list[i].len;
    }
    char *str = (char*)malloc(total_len + 1);
    u32 offset = 0;
    for (u32 i = 0; i < j_al_len(list); ++i) {
        for (u32 j = 0; j < list[i].len; j++) {
            str[offset + j] = list[i].str[j];
        }
        offset += list[i].len;
    }
    str[total_len] = '\0';
    return (Str){str, total_len};
}

#endif
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




// ======= STRING LIBRARY =======

#define j_list(type) type * _Nullable

#define cast(type, value) ((type)(value))
#define JASSERT(condition, message) (assert(condition && message))
#define jassert(condition, message) (JASSERT(condition, message))


typedef struct Str {
    char * _Nonnull str;
    u32 len;
} Str;

#define str_from_lit(cstr) ((Str) { .str = cstr, .len = sizeof(cstr) - 1})
#define str_from_cstr(cstr) ((Str) { .str = cstr, .len = strlen(cstr) })


typedef struct SubStr {
    const Str * _Nonnull base;
    Str str;
} SubStr;

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
#define NIL { .is_present = false }
_j_stamp_maybe(Str);
_j_stamp_maybe(SubStr);
_j_stamp_maybe(u32);

_j_stamp_maybe(j_pair(Str, Str));

#define j_maybe(type) J_MAYBE(type)


#define if_let(unwrap_name, maybe_val, block) if ((maybe_val).is_present == true) { typeof((maybe_val).value) unwrap_name = (maybe_val).value; block }
#define if_let_expr(unwrap_name, maybe_val, expr, block) if ((maybe_val = (expr)).is_present == true) { typeof(maybe_val.value) unwrap_name = maybe_val.value; block }

#define while_let(unwrap_name, maybe_val, block) while ((maybe_val).is_present == true) { typeof((maybe_val).value) unwrap_name = (maybe_val).value; block }
#define while_let_expr(unwrap_name, maybe_val, expr, block) while ((maybe_val = (expr)).is_present == true) { typeof(maybe_val.value) unwrap_name = maybe_val.value; block}

typedef Str IStr;

typedef struct FormatOption {
    Str format;
    const Str (* _Nonnull printer)(va_list * _Nonnull args);
} FormatOption;

void __attribute__((overloadable)) print(char *_Nonnull format, ...);
void __attribute__((overloadable)) print(const Str format, ...);

bool str_eq(const Str a, const Str b);
bool str_start_with(const Str str, const Str prefix);
const Str str_concat(const Str prefix, const Str suffix);
//Str str_from_cstr(const char * _Nonnull cstr);
bool str_contains(const Str hay, const Str needle);
Str str_build_from_arraylist( const j_list(Str) list );

const Str __attribute__((overloadable)) str_format(char * _Nonnull format, ...);
const Str __attribute__((overloadable)) str_format(const Str format, ...);

void str_register(const char * _Nonnull format, const Str (* _Nonnull printer)(va_list * _Nonnull args));


/**
 * @brief Returns a substring from the start of the substring up to and including the specified position
 * Precondition: The position must be less than the length of the substring
 */
SubStr j_ss_prefix_through(SubStr ss, u32 n);
/**
 * @brief Returns a substring from the start of the substring up to, but not including, the specified position
 * Precondition: The position must be less than the length of the substring
 */
SubStr j_ss_prefix_up_to(SubStr ss, u32 n);

/**
 * @brief Returns a substring containing all but the first element
 */
SubStr j_ss_drop_first(SubStr ss);
/**
 * @brief Returns a substring containing all but the first n elements
 * @param ss The substring
 * @param n The first n elements to drop
 * @return The resulting substring
 */
SubStr j_ss_drop_first_n(SubStr ss, u32 n);
/**
 * @brief Remove and return the first element from the substring
 */
char j_ss_remove_first(SubStr *ss);
/**
 * @brief Remove the first n elements from the substring
 */
void j_ss_remove_first_n(SubStr *ss, u32 n);
/**
 * @brief returns the first index of the character in the substring
 */
Maybeu32 j_ss_first_index_of_c(SubStr ss, char c);
/**
 * @brief returns the first index of the string in the substring
 */
Maybeu32 j_ss_first_index_of_str(SubStr ss, Str needle);

/**
 * @brief Returns the substring up to, but not including, the seperator and removes it from the substring
 * @param ss The SubString
 * @param seperator The seperator to split on
 * @return The substring up to the seperator.
 */
MaybeSubStr j_ss_split_on_first_str(SubStr *ss, Str seperator);

/**
 * @brief Removes the whitespace from the front of the substring.
 * @param ss
 */
void j_ss_trim_front_whitespace(SubStr *ss);



#endif
#ifdef JLIB_IMPL

// MARK: - SubString implementation

SubStr j_ss_drop_first(SubStr ss) {
    assert(ss.str.len > 0);
    ss.str.str++;
    ss.str.len--;
    return ss;
}

SubStr j_ss_drop_first_n(SubStr ss, u32 n) {
    assert(ss.str.len >= n);
    ss.str.str += n;
    ss.str.len -= n;
    return ss;
}

char j_ss_remove_first(SubStr *ss) {
    ss->str.str++;
    ss->str.len--;
    return *(ss->str.str - 1);
}

void j_ss_remove_first_n(SubStr *ss, u32 n) {
    jassert(ss->str.len >= n, "Precondition: The number of elements to remove must be less than the length of the substring\n");
    ss->str.str += n;
    ss->str.len -= n;
}

/**
 * @brief Skips the elements from the substring that satisfies the predicate
 */
#define j_ss_remove_while(ss, pred) do { \
    char j_ss_it;                                   \
    while ((ss)->str.len > 0 && (j_ss_it = *(ss)->str.str, (pred))) { \
        j_ss_remove_first(ss);                                  \
    }\
} while(0)

Maybeu32 j_ss_first_index_of_c(SubStr ss, char c) {
    u32 j_ss_index = 0;
    while (ss.str.str[j_ss_index] != c) {
        j_ss_index++;
        if (ss.str.len == j_ss_index)
            return (Maybeu32) { .is_present = false };
    }
    return (Maybeu32) { .is_present = true, .value = j_ss_index };
}

Maybeu32 j_ss_first_index_of_str(SubStr ss, Str needle) {
    if (ss.str.len < needle.len || ss.str.len == 0)
        return (Maybeu32) { .is_present = false };
    u32 index = 0;
    while (str_start_with(j_ss_prefix_through(ss, needle.len).str, needle) == false) {
        index++;
        ss.str.str++;
        ss.str.len--;
        if (ss.str.len < needle.len)
            return (Maybeu32) { .is_present = false };
    }
    return (Maybeu32) { .is_present = true, .value = index };
}

/**
 * @brief returns the first index where the predicate is true
 */
#define j_ss_first_index_where(ss, pred) ({ \
    Str j_ss_it = (ss).str; \
    u32 j_ss_index = 0; \
    while (j_ss_index < (ss).str.len && !(pred)) { \
        j_ss_index++;                       \
        j_ss_it.str++;                      \
        j_ss_it.len--;                      \
    } \
    j_ss_index; \
})

SubStr j_ss_prefix_up_to(SubStr ss, u32 n) {
    jassert(n > 1 && n <= ss.str.len, "Precondition: The position must not be larger than the length of the substring\n");
    ss.str.len = n-1;
    return ss;
}

SubStr j_ss_prefix_through(SubStr ss, u32 n) {
//    jassert(n <= ss.str.len, "Precondition: The position must be less than the length of the substring\n");
    if (ss.str.len == 0) return ss;
    ss.str.len = n;
    return ss;
}

/**
 * @brief Returns a substring from the start of the substring up until the predicate returns false.
 */
#define j_ss_prefix_while(ss, pred) ({ \
    SubStr temp = (ss);                \
    temp.str.len = 0;                  \
    char j_ss_it;                      \
    while (temp.str.len < (ss).str.len && (j_ss_it = temp.str.str[temp.str.len], (pred))) { \
        temp.str.len++;                \
    }                                  \
    temp;                              \
})

/**
 * @brief resets the SubString to the original String
 */
#define j_ss_reset(ss) (ss).str = *(ss).base

MaybeSubStr j_ss_split_on_first_str(SubStr *ss, Str seperator) {
    Maybeu32 mindex = j_ss_first_index_of_str(*ss, seperator);
    if (mindex.is_present == false)
        return (MaybeSubStr) { .is_present = false };
    u32 index = mindex.value;
    SubStr line = j_ss_prefix_up_to(*ss, index + seperator.len);
    j_ss_remove_first_n(ss, index + seperator.len);

    return (MaybeSubStr) { .is_present = true, .value = line };
}

void j_ss_trim_front_whitespace(SubStr *ss) {
    j_ss_remove_while(ss, j_ss_it == ' ' || j_ss_it == '\n' || j_ss_it == '\r' || j_ss_it == '\t');
}
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

_j_stamp_maybe(FS_Walker);
_j_stamp_maybe(FS_Entry);
// MARK: - Bit Field

#define j_bit_set(field, bit) ((field) |= (cast(typeof(field), 1) << (bit)))
#define j_bit_check(field, bit) (!!((field) & (cast(typeof(field), 1) << (bit))))



// MARK: - Basic Data Structures Stamps



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

static j_list(FormatOption) options = EMPTY_ARRAY;

void str_register(const char * _Nonnull format, const Str (* _Nonnull printer)(va_list * _Nonnull args)) {
    j_al_append(options, ((FormatOption) {.format = str_from_cstr(format), .printer = printer}));
}

static const Str i32_Printer(va_list * _Nonnull args) {
    i32 number = va_arg(*args, i32);
    u32 len = 0;
    i32 temp = number;
    if (number == 0) {
        return str_from_lit("0");
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
        return str_from_lit("0");
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
            int_str = str_from_lit("0");
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
            frac_str = str_from_lit("0");
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

    const Str dot_str = str_from_lit(".");
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
            int_str = str_from_lit("0");
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
            frac_str = str_from_lit("0");
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

    const Str dot_str = str_from_lit(".");
    const Str out = str_concat(int_str, str_concat(dot_str, frac_str));
    return out;
}

static const Str bool_Printer(va_list * _Nonnull args) {
    bool i = va_arg(*args, u32);
    if (i) {
        return str_from_lit("true");
    } else {
        return str_from_lit("false");
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
static inline const Str str_format_impl(const Str format, va_list args ) {
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
    j_al_append(strs, ((Str) { .str = format.str + last_printed, .len = format.len - last_printed}));

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

const Str str_concat(const Str prefix, const Str suffix) {
    u32 len = prefix.len + suffix.len;
    char *str = (char*)malloc(len + 1);
    for (u32 i = 0; i < prefix.len; i++) {
        str[i] = prefix.str[i];
    }
    for (u32 i = 0; i < suffix.len; i++) {
        str[prefix.len + i] = suffix.str[i];
    }
    str[len] = '\0';
    Str c = {str, len};
    return c;
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

bool str_start_with(const Str str, const Str prefix) {
    if (str.len < prefix.len) {
        return 0;
    }
    for (u32 i = 0; i < prefix.len; i++) {
        if (str.str[i] != prefix.str[i]) {
            return 0;
        }
    }
    return 1;
}

bool str_contains(const Str hay, const Str needle) {
    if (hay.len < needle.len) {
        return 0;
    }
    for (u32 i = 0; i < hay.len - needle.len; i++) {
        bool found = 1;
        for (u32 j = 0; j < needle.len; j++) {
            if (hay.str[i + j] != needle.str[j]) {
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
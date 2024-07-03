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


#define J_KB(x) ((x) * 1024)
#define J_MB(x) ((x) * 1024 * 1024)
#define J_GB(x) ((x) * 1024 * 1024 * 1024)
#define J_TB(x) ((x) * 1024 * 1024 * 1024 * 1024)


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

// MARK: - Arena Allocator

typedef struct Stack {
    u64 size;
    u64 used;
    void *memory;
} Stack;

typedef struct ArenaFreeNode {
    struct ArenaFreeNode *next;
    u64 size;
    void *memory;
} ArenaFreeNode;


/**
 * @brief This struct represents an allocator that can be used to allocate memory. The flags determine the behavior of the allocator and which allocation scheme that should be used.
 *
 * The flags are as follows:
 * - zero_initialized: If true, all memory allocated will be zero initialized.
 * - storage_duration_permanent: If true, indicates that the memory allocated will only be freed when the program exits.
 * - storage_duration_scratch: If true, indicates that the memory allocated will not last throughout the duration of the program. Additionally the j_free function will be a noop. To reset the memory, use j_reset_arena.
 * - use_free_list: If true, indicates that the allocator will use a free list to reclaim freed memory. Uses Best Fit.
 * - allocation_scheme_pool: If true, indicates that the allocator will allocate blocks of fixed size and the parameter in j_alloc will default to count.
 * - allocation_scheme_linear: If true, indicates that the allocator will allocate memory linearly. The parameter in j_alloc will default to size.
 * - allocated_with_malloc: If true, indicates that the memory was allocated with malloc and should be freed with free.
 * - block_size: The size of the blocks to allocate if using the pool allocation scheme.
 * - free_list: A pointer into the stack allocator that is used to keep track of free memory.
 */
typedef struct Arena {
    Stack stack;
    ArenaFreeNode *free_list;
    u64 block_size;
    // TODO: Maybe include a is_valid bool that points to the arena that is was allocated from. And if that becomes invalid, then the arena is invalid.
    struct {
        u8 zero_initialized: 1;
        u8 storage_duration_permanent: 1;
        u8 storage_duration_scratch: 1; // j_free will be noop if true.
        u8 use_free_list: 1;            // Uses best_fit_strategy for now.
        u8 allocation_scheme_pool: 1;
        u8 allocation_scheme_linear: 1;
        u8 allocated_with_malloc: 1;
    } flags;
} Arena;

typedef Arena * ArenaPtr;
_j_stamp_maybe(Arena);
_j_stamp_maybe(ArenaPtr);

/**
 * @brief Allocates size bytes of memory from the arena. For arenas using pool allocation scheme, the size determines the number of elements to allocate of the specified type by the arena.
 */
void *j_alloc(Arena *arena, u64 size_or_count);
void j_free(Arena *arena, void *ptr, u64 size);
void j_init_scratch(Arena *program_memory, i32 arena_count, u64 total_scratch_available);
j_maybe(ArenaPtr) j_get_scratch();
void j_release_scratch(Arena *scratch);
static j_list(j_maybe(Arena)) SCRATCH_ARENAS = NULL; // is_present indicates whether or not it is in use.

void j_reset_arena(Arena *arena);

/**
 * @brief Allocates a new permanent arena with the specified size using malloc
 */
Arena j_make_arena(u64 size, bool zero_initialized);

Stack j_alloc_stack(Arena *arena, u64 size);

Arena j_make_scratch(Arena *arena, u64 size);

Arena j_make_pool(Arena *arena, u64 size, u64 block_size);

// MARK: - Swift Syntax
#define if_let(unwrap_name, maybe_val, block) if ((maybe_val).is_present == true) { typeof((maybe_val).value) unwrap_name = (maybe_val).value; block }
#define if_let_expr(unwrap_name, maybe_val, expr, block) if ((maybe_val = (expr)).is_present == true) { typeof(maybe_val.value) unwrap_name = maybe_val.value; block }

#define while_let(unwrap_name, maybe_val, block) while ((maybe_val).is_present == true) { typeof((maybe_val).value) unwrap_name = (maybe_val).value; block }
#define while_let_expr(unwrap_name, maybe_val, expr, block) while ((maybe_val = (expr)).is_present == true) { typeof(maybe_val.value) unwrap_name = maybe_val.value; block}

typedef Str IStr;

typedef struct FormatOption {
    Str format;
    const Str (* _Nonnull printer)(Arena *arena, va_list * _Nonnull args);
} FormatOption;

void __attribute__((overloadable)) print(char *_Nonnull format, ...);
void __attribute__((overloadable)) print(const Str format, ...);

bool str_eq(const Str a, const Str b);
bool str_start_with(const Str str, const Str prefix);
const Str str_concat(Arena *arena, const Str prefix, const Str suffix);
//Str str_from_cstr(const char * _Nonnull cstr);
bool str_contains(const Str hay, const Str needle);
Str str_build_from_arraylist(Arena *arena, const j_list(Str) list);

const Str __attribute__((overloadable)) str_format(char * _Nonnull format, Arena *arena, ...);
const Str __attribute__((overloadable)) str_format(const Str format, Arena *arena, ...);

void str_register(Arena *perm_arena,
                  const char * _Nonnull format,
                  const Str (* _Nonnull printer)(Arena *arena, va_list * _Nonnull args));


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

typedef enum J_RB_COLOR {
    J_RB_RED,
    J_RB_BLACK,
} J_RB_COLOR;

_j_stamp_pair(u32, bool);
_j_stamp_maybe(j_pair(u32, bool));
_j_stamp_maybe(J_RB_COLOR);

#define EMPTY_RB NULL
typedef struct RBTreeHeader {
    u32 len;
    u32 cap;
    j_maybe(u32) root;
    // Below can be consolidated into a single array of structs.
    j_maybe(u32) * _Nullable parent_for;
    j_maybe(u32) * _Nullable left_child_for;
    j_maybe(u32) * _Nullable right_child_for;
    J_RB_COLOR * _Nullable color_for;

    // 0 -> Equal
    // - -> Left is less
    // + -> Left is greater
    i32 (*compare_func)(u32 lhs, u32 rhs);
} RBTreeHeader;

#define j_rb(key, value) j_pair(key, value) * _Nullable
#define j_rb_header(tree) (cast(RBTreeHeader *, tree)-1)
#define j_rb_cap(tree) (tree ? (cast(RBTreeHeader *, tree)-1)->cap : 0)
#define j_rb_len(tree) (tree ? (cast(RBTreeHeader *, tree)-1)->len : 0)
#define j_rb_compare(tree, lhs, rhs) (j_rb_header(tree)->compare_func(lhs, rhs))
#define j_rb_root(tree) (j_rb_header(tree)->root)
#define j_rb_key(tree, descriptor) (tree[descriptor].first)
#define j_rb_value(tree, descriptor) (tree[descriptor].second)
#define j_rb_left(tree, descriptor) (j_rb_header(tree)->left_child_for[descriptor])
#define _j_rb_left(tree, descriptor) (j_rb_header(tree)->left_child_for[descriptor].value)
#define j_rb_right(tree, descriptor) (j_rb_header(tree)->right_child_for[descriptor])
#define _j_rb_right(tree, descriptor) (j_rb_header(tree)->right_child_for[descriptor].value)
#define j_rb_parent(tree, descriptor) (j_rb_header(tree)->parent_for[descriptor])
#define _j_rb_parent(tree, descriptor) (j_rb_header(tree)->parent_for[descriptor].value)
#define j_rb_color(tree, descriptor) (j_rb_header(tree)->color_for[descriptor])
#define _j_rb_init(tree, compare_function) ({   \
    if ((tree) == EMPTY_RB) { \
        u32 cap = 10;         \
        (tree) = malloc(sizeof(RBTreeHeader) + sizeof(tree[0]) * cap) + sizeof(RBTreeHeader);\
        j_rb_header(tree)->len = 0;  \
        j_rb_header(tree)->cap = cap;\
        j_rb_header(tree)->root = ((typeof(j_rb_header(tree)->root)) {.is_present = false});\
        j_rb_header(tree)->parent_for = calloc(cap, sizeof(j_maybe(u32)));       \
        j_rb_header(tree)->left_child_for = calloc(cap, sizeof(j_maybe(u32)));   \
        j_rb_header(tree)->right_child_for = calloc(cap, sizeof(j_maybe(u32)));  \
        j_rb_header(tree)->color_for = calloc(cap, sizeof(J_RB_COLOR)); \
        j_rb_header(tree)->compare_func = (compare_function);\
    }                       \
})

#define j_rb_put(tree, key, val) ({ \
    j_maybe(j_pair(u32, bool)) result = NIL; \
    _j_rb_init(tree, j_rb_u32_comp); \
    \
    j_maybe(u32) mcurrent = j_rb_root(tree); \
    j_maybe(u32) mparent = {.is_present = false}; \
    while_let(current, mcurrent, { \
        mparent = mcurrent; \
        i32 compare_result = j_rb_compare(tree, key, j_rb_key(tree, current)); \
        if (compare_result == 0) { \
            j_rb_value(tree, current) = (val); \
            result.is_present = true; \
            result.value.first = current; \
            result.value.second = false; \
            break; \
        } else if (compare_result < 0) { \
            mcurrent = j_rb_left(tree, current); \
        } else { \
            mcurrent = j_rb_right(tree, current); \
        } \
    }) \
    if (result.is_present == false) { \
        u32 current = j_rb_header(tree)->len++; \
        mcurrent.is_present = true; \
        mcurrent.value = current; \
        tree[current] = (typeof(tree[0])) { .first = key, .second = (val) }; \
        j_rb_parent(tree, current) = mparent; \
        if_let(parent, mparent, { \
            if (j_rb_compare(tree, key, j_rb_key(tree, parent)) < 0) { \
                j_rb_left(tree, parent) = mcurrent; \
            } else { \
                j_rb_right(tree, parent) = mcurrent; \
            } \
        }) else { \
            j_rb_header(tree)->root = mcurrent; \
        } \
        j_rb_color(tree, current) = J_RB_RED; \
        _j_rb_insert_fixup(tree, current); \
        result.value.first = current;\
        result.value.second = true;\
    } \
    result.value; \
})

#define j_rb_find(tree, key) ({ \
    j_maybe(u32) mcurrent = j_rb_root(tree); \
    bool found = false; \
    while_let(current, mcurrent, { \
        i32 compare_result = j_rb_compare(tree, key, j_rb_key(tree, current)); \
        if (compare_result == 0) { \
            found = true; \
            break; \
        } else if (compare_result < 0) { \
            mcurrent = j_rb_left(tree, current); \
        } else { \
            mcurrent = j_rb_right(tree, current); \
        } \
    }) \
    mcurrent.is_present = found; \
    mcurrent; \
});
#define _j_rb_is_left_sibling(tree, u) (j_rb_parent(tree, u).is_present && j_rb_left(tree, j_rb_parent(tree, u).value).is_present && j_rb_left(tree, j_rb_parent(tree, u).value).value == u)
#define _j_rb_is_right_sibling(tree, u) (j_rb_parent(tree, u).is_present && j_rb_right(tree, j_rb_parent(tree, u).value).is_present && j_rb_right(tree, j_rb_parent(tree, u).value).value == u)
#define _j_rb_left_sibling(tree, u) (j_rb_left(tree, j_rb_parent(tree, u).value))
#define _j_rb_right_sibling(tree, u) (j_rb_right(tree, j_rb_parent(tree, u).value))
#define _j_rb_grandparent(tree, u) (j_rb_parent(tree, j_rb_parent(tree, u).value).value)
#define _j_rb_transplant(tree, u, v) ({ \
    if (j_rb_parent(tree, u).is_present == false) { \
        j_rb_root(tree) = ((j_maybe(u32)) { .is_present = true, .value = v }); \
    } else if (_j_rb_is_left_sibling(tree, u)) { \
        _j_rb_left_sibling(tree, u) = ((j_maybe(u32)) { .is_present = true, .value = v }); \
    } else { \
        _j_rb_right_sibling(tree, u) = ((j_maybe(u32)) { .is_present = true, .value = v }); \
    } \
    j_rb_parent(tree, v) = j_rb_parent(tree, u); \
})

void _j_rb_left_rotate(void *tree, u32 descriptor);
void _j_rb_right_rotate(void *tree, u32 descriptor);
void _j_rb_insert_fixup(void *tree, u32 descriptor);
void _j_rb_delete_fixup(void *tree, u32 descriptor);
void j_rb_delete(void *tree, u32 descriptor);


#endif
#ifdef JLIB_IMPL

// MARK: - Red Black Tree Implementation


void j_rb_delete(void *tree, u32 descriptor) {
    u32 y = descriptor;
    u32 x;
    J_RB_COLOR y_original_color = j_rb_color(tree, y);
    if (j_rb_left(tree, descriptor).is_present == false) {
        x = _j_rb_right(tree, descriptor);
        _j_rb_transplant(tree, descriptor, x);
    } else if (j_rb_right(tree, descriptor).is_present == false) {
        x = _j_rb_left(tree, descriptor);
        _j_rb_transplant(tree, descriptor, x);
    } else {
        y = _j_rb_right(tree, descriptor);
        while (j_rb_left(tree, y).is_present) {
            y = _j_rb_left(tree, y);
        }
        y_original_color = j_rb_color(tree, y);
        x = _j_rb_right(tree, y);
        if (_j_rb_parent(tree, y) == descriptor) {
            _j_rb_parent(tree, x) = y;
        } else {
            _j_rb_transplant(tree, y, x);
            _j_rb_right(tree, y) = _j_rb_right(tree, descriptor);
            j_rb_parent(tree, _j_rb_right(tree, y)) = ((j_maybe(u32)) { .is_present = true, .value = y });
        }
        _j_rb_transplant(tree, descriptor, y);
        j_rb_left(tree, y) = j_rb_left(tree, descriptor);
        j_rb_parent(tree, _j_rb_left(tree, y)) = ((j_maybe(u32)) { .is_present = true, .value = y });
        j_rb_color(tree, y) = j_rb_color(tree, descriptor);
    }
    if (y_original_color == J_RB_BLACK) {
        _j_rb_delete_fixup(tree, x);
    }
}

void _j_rb_left_rotate(void *tree, u32 descriptor) {
    // We know y is present;
    j_maybe(u32) y = j_rb_right(tree, descriptor);
    j_maybe(u32) yleft = j_rb_left(tree, y.value);
    j_rb_right(tree, descriptor) = yleft;

    if_let(left, yleft, {
        j_rb_parent(tree, left) = ((j_maybe(u32)) { .is_present = true, .value = descriptor});
    })
    j_rb_parent(tree, y.value) = j_rb_parent(tree, descriptor);
    if (j_rb_parent(tree, descriptor).is_present == false) {
        j_rb_root(tree) = y;
    } else if (_j_rb_is_left_sibling(tree, descriptor)) {
        j_rb_left(tree, j_rb_parent(tree, descriptor).value) = y;
    } else {
        j_rb_right(tree, j_rb_parent(tree, descriptor).value) = y;
    }

    j_rb_left(tree, y.value) = ((j_maybe(u32)) { .value = descriptor, .is_present = true });
    j_rb_parent(tree, descriptor) = y;
}

void _j_rb_right_rotate(void *tree, u32 descriptor) {
    // We know y is present;
    j_maybe(u32) y = j_rb_left(tree, descriptor);
    j_maybe(u32) yright = j_rb_right(tree, y.value);
    j_rb_left(tree, descriptor) = yright;

    if_let(right, yright, {
        j_rb_parent(tree, right) = ((j_maybe(u32)) { .is_present = true, .value = descriptor});
    })
    j_rb_parent(tree, y.value) = j_rb_parent(tree, descriptor);
    if (j_rb_parent(tree, descriptor).is_present == false) {
        j_rb_root(tree) = y;
    } else if (_j_rb_is_right_sibling(tree, descriptor)) {
        j_rb_right(tree, j_rb_parent(tree, descriptor).value) = y;
    } else {
        j_rb_left(tree, j_rb_parent(tree, descriptor).value) = y;
    }

    j_rb_right(tree, y.value) = ((j_maybe(u32)) { .value = descriptor, .is_present = true });
    j_rb_parent(tree, descriptor) = y;
}

void _j_rb_insert_fixup(void *tree, u32 descriptor) {
    while (j_rb_parent(tree, descriptor).is_present && j_rb_color(tree, _j_rb_parent(tree, descriptor)) == J_RB_RED) {
        j_maybe(u32) my;
        if (_j_rb_parent(tree, descriptor) == _j_rb_left(tree, _j_rb_grandparent(tree, descriptor))) {
            u32 y = _j_rb_right(tree, _j_rb_grandparent(tree, descriptor));
            if (j_rb_color(tree, y) == J_RB_RED) {
                j_rb_color(tree, _j_rb_parent(tree, descriptor)) = J_RB_BLACK;
                j_rb_color(tree, y) = J_RB_BLACK;
                j_rb_color(tree, _j_rb_grandparent(tree, descriptor)) = J_RB_RED;
                descriptor = _j_rb_grandparent(tree, descriptor);
            } else {
                if (_j_rb_is_right_sibling(tree, descriptor)) {
                    descriptor = _j_rb_parent(tree, descriptor);
                    _j_rb_left_rotate(tree, descriptor);
                }
                j_rb_color(tree, _j_rb_parent(tree, descriptor)) = J_RB_BLACK;
                j_rb_color(tree, _j_rb_grandparent(tree, descriptor)) = J_RB_RED;
                _j_rb_right_rotate(tree, _j_rb_grandparent(tree, descriptor));
            }
        } else  {
            u32 y = _j_rb_left(tree, _j_rb_grandparent(tree, descriptor));
            if (j_rb_color(tree, y) == J_RB_RED) {
                j_rb_color(tree, _j_rb_parent(tree, descriptor)) = J_RB_BLACK;
                j_rb_color(tree, y) = J_RB_BLACK;
                j_rb_color(tree, _j_rb_grandparent(tree, descriptor)) = J_RB_RED;
                descriptor = _j_rb_grandparent(tree, descriptor);
            } else {
                if (_j_rb_is_left_sibling(tree, descriptor)) {
                    descriptor = _j_rb_parent(tree, descriptor);
                    _j_rb_right_rotate(tree, descriptor);
                }
                j_rb_color(tree, _j_rb_parent(tree, descriptor)) = J_RB_BLACK;
                j_rb_color(tree, _j_rb_grandparent(tree, descriptor)) = J_RB_RED;
                _j_rb_left_rotate(tree, _j_rb_grandparent(tree, descriptor));
            }
        }

    }
    j_rb_color(tree, j_rb_root(tree).value) = J_RB_BLACK;
}

void _j_rb_delete_fixup(void *tree, u32 descriptor) {
    while (descriptor != j_rb_root(tree).value && j_rb_color(tree, descriptor) == J_RB_BLACK) {
        j_maybe(u32) mparent = j_rb_parent(tree, descriptor);
        if_let(parent, mparent, {
            if (j_rb_left(tree, parent).is_present && descriptor == j_rb_left(tree, parent).value) {
                j_maybe(u32) msibling = j_rb_right(tree, parent);
                if_let(sibling, msibling, {
                        if (j_rb_color(tree, sibling) == J_RB_RED) {
                            j_rb_color(tree, sibling) = J_RB_BLACK;
                            j_rb_color(tree, parent) = J_RB_RED;
                            _j_rb_left_rotate(tree, parent);
                            msibling = j_rb_right(tree, parent);
                        }
                        if_let(sibling, msibling, {
                            if (j_rb_left(tree, sibling).is_present && j_rb_color(tree, j_rb_left(tree, sibling).value) == J_RB_BLACK &&
                                j_rb_right(tree, sibling).is_present && j_rb_color(tree, j_rb_right(tree, sibling).value) == J_RB_BLACK) {
                                j_rb_color(tree, sibling) = J_RB_RED;
                                descriptor = parent;
                            } else {
                                if (j_rb_right(tree, sibling).is_present && j_rb_color(tree, j_rb_right(tree, sibling).value) == J_RB_BLACK) {
                                    j_rb_color(tree, j_rb_left(tree, sibling).value) = J_RB_BLACK;
                                    j_rb_color(tree, sibling) = J_RB_RED;
                                    _j_rb_right_rotate(tree, sibling);
                                    msibling = j_rb_right(tree, parent);
                                }
                                if_let(sibling, msibling, {
                                        j_rb_color(tree, sibling) = j_rb_color(tree, parent);
                                        j_rb_color(tree, parent) = J_RB_BLACK;
                                        j_rb_color(tree, j_rb_right(tree, sibling).value) = J_RB_BLACK;
                                        _j_rb_left_rotate(tree, parent);
                                        descriptor = j_rb_root(tree).value;
                                })
                            }
                        })
                })
            } else {
                j_maybe(u32) msibling = j_rb_left(tree, parent);
                if_let(sibling, msibling, {
                        if (j_rb_color(tree, sibling) == J_RB_RED) {
                            j_rb_color(tree, sibling) = J_RB_BLACK;
                            j_rb_color(tree, parent) = J_RB_RED;
                            _j_rb_right_rotate(tree, parent);
                            msibling = j_rb_left(tree, parent);
                        }
                        if_let(sibling, msibling, {
                            if (j_rb_right(tree, sibling).is_present && j_rb_color(tree, j_rb_right(tree, sibling).value) == J_RB_BLACK &&
                                j_rb_left(tree, sibling).is_present && j_rb_color(tree, j_rb_right(tree, sibling).value) == J_RB_BLACK) {
                                j_rb_color(tree, sibling) = J_RB_RED;
                                descriptor = parent;
                            } else {
                                if (j_rb_left(tree, sibling).is_present && j_rb_color(tree, j_rb_left(tree, sibling).value) == J_RB_BLACK) {
                                    j_rb_color(tree, j_rb_right(tree, sibling).value) = J_RB_BLACK;
                                    j_rb_color(tree, sibling) = J_RB_RED;
                                    _j_rb_left_rotate(tree, sibling);
                                    msibling = j_rb_left(tree, parent);
                                }
                                if_let(sibling, msibling, {
                                        j_rb_color(tree, sibling) = j_rb_color(tree, parent);
                                        j_rb_color(tree, parent) = J_RB_BLACK;
                                        j_rb_color(tree, j_rb_left(tree, sibling).value) = J_RB_BLACK;
                                        _j_rb_right_rotate(tree, parent);
                                        descriptor = j_rb_root(tree).value;
                                })
                            }
                        })
                })
            }
        })
    }
    j_rb_color(tree, descriptor) = J_RB_BLACK;
}

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
    u64 len;
    u64 cap;
} ArrHeader;

#define j_al_header(list) ((list) ? cast(ArrHeader *, list) - 1 : EMPTY_ARRAY)
#define j_al_len(list) ((list) ? j_al_header(list)->len : 0)
#define j_al_cap(list) ((list) ? j_al_header(list)->cap : 0)
#define _j_al_init(list, arena) ({\
    if ((list) == EMPTY_ARRAY) {  \
        (list) = j_alloc(arena, sizeof(list[0]) * 10 + sizeof(ArrHeader)) + sizeof(ArrHeader); \
        j_al_header(list)->len = 0;                                                                   \
        j_al_header(list)->cap = 10;                                                                  \
    }                             \
})
#define _j_al_realloc(list, arena) ({ \
    if (j_al_len(list) == j_al_cap(list)) { \
        u64 new_cap = j_al_cap(list)*2;     \
        u64 size = j_al_len(list);    \
        typeof(list) ptr = j_alloc(arena, sizeof(list[0]) * new_cap + sizeof(ArrHeader)) + sizeof(ArrHeader); \
        memcpy(ptr, list, sizeof(list[0]) * j_al_cap(list));\
        j_free(arena, j_al_header(list), sizeof(list[0]) * j_al_cap(list) + sizeof(ArrHeader));     \
        list = ptr;                   \
        j_al_header(list)->cap = new_cap;   \
        j_al_header(list)->len = size;\
    }                                 \
})
#define j_al_append(list, arena, elem) ({ \
    _j_al_init(list, arena);              \
    _j_al_realloc(list, arena);           \
    list[j_al_len(list)] = (elem);        \
    j_al_header(list)->len += 1;          \
})
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


// MARK: - Arena Allocator

inline void *j_alloc(Arena *arena, u64 size_or_count) {
    jassert(arena->flags.allocation_scheme_linear + arena->flags.allocation_scheme_pool == 1,
            "An arena should either use a linear or a pool allocation scheme\n");
    u64 allocation_amount = size_or_count;
    if (arena->flags.allocation_scheme_pool) {
        allocation_amount *= arena->block_size;
    }

    if (arena->flags.use_free_list) {
        if (arena->free_list != NULL) {
            // Strategy: Best Fit
            ArenaFreeNode *node = arena->free_list;
            ArenaFreeNode *prev = NULL;
            ArenaFreeNode *best_fit = NULL;
            ArenaFreeNode *best_fit_prev = NULL;
            while (node != NULL) {
                if (node->size <= allocation_amount) {
                    if (best_fit == NULL || node->size < best_fit->size) {
                        best_fit = node;
                        best_fit_prev = prev;
                    }
                }
                prev = node;
            }
            // Did we find a size that fits?
            if (best_fit != NULL) {
                void *ptr = best_fit->memory;
                if (best_fit_prev == NULL) {
                    arena->free_list = best_fit->next;
                } else {
                    best_fit_prev->next = best_fit->next;
                }
                return ptr;
            }
        }
        jassert(arena->stack.size - arena->stack.used >= allocation_amount,
                "The arena is out of memory\n");

        // We have not found a free block that fits the size. We need to allocate a new block.
        void *ptr = arena->stack.memory + arena->stack.used;
        arena->stack.used += sizeof(ArenaFreeNode) + allocation_amount;

        ArenaFreeNode *node = ptr;
        node->size = allocation_amount;
        node->next = NULL;
        node->memory = ptr + sizeof(ArenaFreeNode);
        return arena->flags.zero_initialized ? memset(node->memory, 0, allocation_amount) : node->memory;
    } else {
        jassert(arena->stack.size - arena->stack.used >= allocation_amount,
                "The arena is out of memory\n");

        void *ptr = arena->stack.memory + arena->stack.used;
        arena->stack.used += size_or_count;
        return arena->flags.zero_initialized ? memset(ptr, 0, allocation_amount) : ptr;
    }
}

inline void j_reset_arena(Arena *arena) {
    arena->free_list = NULL;
    arena->stack.used = 0;
}

inline void j_free(Arena *arena, void *ptr, u64 size) {
    jassert(ptr >= arena->stack.memory && ptr < arena->stack.memory + arena->stack.used,
            "The pointer is not in the arena\n");
    if (arena->flags.storage_duration_scratch) {
        // Noop for scratch allocation scheme.
        return;
    }

    if (arena->flags.use_free_list) {
        //TODO: Ensure that below math is correct...
        ArenaFreeNode *node = ptr - offsetof(ArenaFreeNode, memory);

        // Find the position just before the above node that we want to free.
        ArenaFreeNode *current = arena->free_list;
        if (current == NULL) {
            arena->free_list = node;
            return;
        }
        // We assume that current != NULL for now.
        while (current->next != NULL && cast(u8 *, current->memory) + current->size < cast(u8 *, node)) {
            current = current->next;
        }
        ArenaFreeNode *next = current->next;
        current->next = node;
        node->next = next;
    } else {
        // We can only free the memory if we are trying to free the last block that was allocated.
        if (ptr + size == arena->stack.memory + arena->stack.used) {
            arena->stack.used -= size;
        }
    }
}

void j_init_scratch(Arena *program_memory, i32 arena_count, u64 total_scratch_available) {
    jassert(program_memory->stack.size - program_memory->stack.used >= total_scratch_available,
            "The total scratch available is larger than the program_memory size\n");

    void *memory = j_alloc(program_memory, total_scratch_available);
    u64 scratch_size = total_scratch_available / arena_count;
    for (i32 i = 0; i < arena_count; ++i) {
        j_al_append(SCRATCH_ARENAS, program_memory, ((j_maybe(Arena)) {
                .is_present = true,
                .value = {
                        .stack = {
                                .used = 0,
                                .size = scratch_size,
                                .memory = memory + i * scratch_size
                        },
                        .flags = {
                                .storage_duration_scratch = 1,
                                .allocation_scheme_linear = 1,
                        }
                }
        }));
    }
}

j_maybe(ArenaPtr) j_get_scratch() {
    for (i32 i = 0; i < j_al_len(SCRATCH_ARENAS); ++i) {
        j_maybe(Arena) *m = &SCRATCH_ARENAS[i];
        if (m->is_present) {
            SCRATCH_ARENAS[i].is_present = false;
            return (j_maybe(ArenaPtr)) { .is_present = true, .value = &m->value };
        }
    }
    return (j_maybe(ArenaPtr)) { .is_present = false };
}

void j_release_scratch(Arena *scratch) {
    u64 total_scratch_memory = 0;
    for (i32 i = 0; i < j_al_len(SCRATCH_ARENAS); ++i) {
        total_scratch_memory += SCRATCH_ARENAS[i].value.stack.size;
    }
    jassert(cast(void *, scratch) >= cast(void *, SCRATCH_ARENAS) && cast(void *, scratch) < cast(void *, SCRATCH_ARENAS) + total_scratch_memory, "The scratch is not a valid scratch arena\n");
    scratch->stack.used = 0;
    // Find the index of the scratch
    i32 index = 0;
    while (&SCRATCH_ARENAS[index].value != scratch) {
        index++;
    }
    SCRATCH_ARENAS[index].is_present = true;
}

Arena j_make_arena(u64 size, bool zero_initialize) {
    void *ptr = malloc(size);
    jassert(ptr != NULL, "Could not allocate memory for the arena\n");
    return (Arena) {
            .stack = {
                    .used = 0,
                    .size = size,
                    .memory = ptr
            },
            .flags = {
                    .allocated_with_malloc = 1,
                    .storage_duration_permanent = 1,
                    .zero_initialized = zero_initialize,
            },
            .free_list = NULL
    };
}

Stack j_alloc_stack(Arena *arena, u64 size) {
    return (Stack) {
            .used = 0,
            .size = size,
            .memory = j_alloc(arena, size)
    };
}

Arena j_make_scratch(Arena *arena, u64 size) {
    return (Arena) {
            .stack = j_alloc_stack(arena, size),
            .flags = {
                    .allocation_scheme_linear = true,
                    .storage_duration_scratch = true,
            }
    };
}

Arena j_make_pool(Arena *arena, u64 size, u64 block_size) {
    return (Arena) {
            .stack = j_alloc_stack(arena, size),
            .flags = {
                    .allocation_scheme_pool = true,
            },
            .block_size = block_size
    };
}


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

void str_register(Arena *perm_arena, const char * _Nonnull format, const Str (* _Nonnull printer)(Arena *arenaz, va_list * _Nonnull args)) {
    j_al_append(options, perm_arena, ((FormatOption) {
        .format = str_from_cstr(format),
        .printer = printer
    }));
}

static const Str i32_Printer(Arena *arena, va_list * _Nonnull args) {
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
    char *str = (char *)j_alloc(arena, len + 1);
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

static const Str u32_Printer(Arena *arena, va_list * _Nonnull args) {
    u32 number = va_arg(*args, u32);
    if (number == 0) {
        return str_from_lit("0");
    }
    u32 len = 0;

    u32 temp = number;
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)j_alloc(arena, len + 1);
    str[len+1] = '\0';
    temp = number;
    for (i32 i = len - 1; i >= 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    return (Str){str, len};
}

static const Str i64_Printer(Arena *arena, va_list * _Nonnull args) {
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
    char *str = (char *)j_alloc(arena, len + 1);
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

static const Str u64_Printer(Arena *arena, va_list * _Nonnull args) {
    u64 number = va_arg(*args, u64);
    if (number == 0) {
        return str_from_lit("0");
    }
    u32 len = 0;

    u64 temp = number;
    while (temp > 0) {
        len++;
        temp /= 10;
    }
    char *str = (char *)j_alloc(arena, len + 1);
    str[len+1] = '\0';
    temp = number;
    for (i32 i = len - 1; i >= 0; i--) {
        str[i] = (temp % 10) + '0';
        temp /= 10;
    }
    return (Str){str, len};
}

static const Str f32_Printer(Arena *arena, va_list * _Nonnull args) {
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
            char *str = (char *)j_alloc(arena, len + 1);
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
            char *str = (char *)j_alloc(arena, len + 1);
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
    const Str out = str_concat(arena, int_str, str_concat(arena, dot_str, frac_str));
    return out;
}

static const Str f64_Printer(Arena *arena, va_list * _Nonnull args) {
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
            char *str = (char *)j_alloc(arena, len + 1);
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
            char *str = (char *)j_alloc(arena, len + 1);
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
    const Str out = str_concat(arena, int_str, str_concat(arena, dot_str, frac_str));
    return out;
}

static const Str bool_Printer(Arena *arena, va_list * _Nonnull args) {
    bool i = va_arg(*args, u32);
    if (i) {
        return str_from_lit("true");
    } else {
        return str_from_lit("false");
    }
}

static const Str str_Printer(Arena *arena, va_list * _Nonnull args) {
    Str str = va_arg(*args, Str);
    return str;
}

//#if defined(__clang__)
static void init_printers(Arena *arena) { // __attribute__((constructor)) {
    str_register(arena, "{i32}", i32_Printer);
    str_register(arena, "{u32}", u32_Printer);
    str_register(arena, "{i64}", i64_Printer);
    str_register(arena, "{u64}", u64_Printer);
    str_register(arena, "{f32}", f32_Printer);
    str_register(arena, "{f64}", f64_Printer);
    str_register(arena, "{bool}", bool_Printer);
    str_register(arena, "{str}", str_Printer);
}
//#endif
static inline const Str str_format_impl(Arena *arena, const Str format, va_list args ) {
    Str *strs = EMPTY_ARRAY;
    j_maybe(ArenaPtr) mscratch = j_get_scratch();
    jassert(mscratch.is_present, "Precondition: The scratch space must be initialized before calling this function.\n");
    Arena *scratch = mscratch.value;
    u32 last_printed = 0;
    // Scan through the format string and discover any registered format options.
    for (u32 i = 0; i < format.len; i++) {
        if (format.str[i] == '\\' && format.str[i + 1] == '{') {
            // We want to print upto here, but not the next character.
            j_al_append(strs, scratch, ((Str) { .str = format.str + last_printed, .len = i - last_printed }));
            i++;
            last_printed = i;
            continue;
        }
        if (format.str[i] == '{') {
            j_al_append(strs, scratch, ((Str) { .str = format.str + last_printed, .len = i - last_printed }));
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
                        const Str fmt = options[k].printer(scratch, &args);
                        j_al_append(strs, scratch, fmt);
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
    j_al_append(strs, scratch, ((Str) { .str = format.str + last_printed, .len = format.len - last_printed}));

    Str out = str_build_from_arraylist(arena, strs);
    j_release_scratch(scratch);
    return out;
}

const Str __attribute__((overloadable)) str_format(Arena *arena, char * _Nonnull format_c, ...) {
    va_list args;
    va_start(args, format_c);
    const Str format = str_from_cstr(format_c);
    const Str out = str_format_impl(arena, format, args);
    va_end(args);
    return out;
}

const Str __attribute__((overloadable)) str_format(Arena *arena, const Str format, ...) {
    va_list args;
    va_start(args, format);
    const Str out = str_format_impl(arena, format, args);
    va_end(args);
    return out;
}

void __attribute__((overloadable)) print(char * _Nonnull format_c, ...) {
    va_list args;
    va_start(args, format_c);
    j_maybe(ArenaPtr) mscratch = j_get_scratch();
    jassert(mscratch.is_present, "Precondition: The scratch space must be initialized before calling this function.\n");
    Arena *scratch = mscratch.value;
    const Str string = str_format_impl(scratch, str_from_cstr(format_c), args);
    write(STDOUT_FILENO, string.str, string.len);
    j_release_scratch(scratch);
    va_end(args);
}

void __attribute__((overloadable)) print(const Str format, ...) {
    va_list args;
    va_start(args, format);
    j_maybe(ArenaPtr) mscratch = j_get_scratch();
    jassert(mscratch.is_present, "Precondition: The scratch space must be initialized before calling this function.\n");
    Arena *scratch = mscratch.value;
    const Str string = str_format_impl(scratch, format, args);
    write(STDOUT_FILENO, string.str, string.len);
    j_release_scratch(scratch);
    va_end(args);
}

const Str str_concat(Arena *arena, const Str prefix, const Str suffix) {
    u32 len = prefix.len + suffix.len;
    char *str = (char*)j_alloc(arena, len + 1);
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

Str str_build_from_arraylist(Arena *arena, const j_list(Str) list) {
    u32 total_len = 0;
    for (u32 i = 0; i < j_al_len(list); i++) {
        total_len += list[i].len;
    }
    char *str = (char*)j_alloc(arena, total_len + 1);
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

#undef _j_rb_is_left_sibling
#undef _j_rb_is_right_sibling
#undef _j_rb_left_sibling
#undef _j_rb_right_sibling
#undef _j_rb_grandparent
#undef _j_rb_transplant

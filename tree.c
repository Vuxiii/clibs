//
// Created by William Juhl on 21/05/2024.
//
#define JLIB_IMPL

#include <string.h>
#include <dirent.h>
#include "jlib.h"

typedef struct ArgParser {
    IStr program;
    IStr program_description;
    int _argc;
    char **_argv;
    Arena arena;
    j_list(Str) args;
    j_list(Str) flags;
    j_list(Str) alternates;
    j_list(Str) descriptions;
} ArgParser;


ArgParser j_parser_init(char *program, char *program_description, int argc, char **argv);
void j_arg_usage(ArgParser *parser, char *program);
u32 j_arg_option(ArgParser *parser, char *flag, char *alternate, char *description);
bool j_arg_has_next(ArgParser *parser);
j_maybe(u32) j_arg_get_flag(ArgParser *parser);
j_maybe(Str) j_arg_current_input(ArgParser *parser);

ArgParser j_parser_init(char *program, char *program_description, int argc, char **argv) {
    return (ArgParser) {
            .program = str_from_cstr(program),
            .program_description = str_from_cstr(program_description),
            ._argc = argc,
            ._argv = argv,
            .args = EMPTY_ARRAY,
            .flags = EMPTY_ARRAY,
            .alternates = EMPTY_ARRAY,
            .descriptions = EMPTY_ARRAY,
    };
}

u32 j_arg_option(ArgParser *parser, char *flag, char *alternate, char *description) {
    assert(j_al_len(parser->flags) < 32);
    Str flags = str_concat(&parser->arena, str_from_lit("-"), str_from_lit(flag));
    Str alt = str_concat(&parser->arena, str_from_lit("--"), str_from_lit(alternate));
    j_al_append(parser->flags, &parser->arena, flags);
    j_al_append(parser->alternates, &parser->arena, alt);
    j_al_append(parser->descriptions, &parser->arena, str_from_cstr(description));
    return j_al_len(parser->flags) - 1;
}

bool j_arg_has_next(ArgParser *parser) {
    return parser->_argc > 1;
}

j_maybe(u32) j_arg_get_flag(ArgParser *parser) {
    Str flag = str_from_cstr(parser->_argv[parser->_argc - 1]);
    // Check if we have the flag registered..
    for (u32 i = 0; i < j_al_len(parser->flags); ++i) {
        if (str_eq(flag, parser->flags[i]) || str_eq(flag, parser->alternates[i])) {
            parser->_argc--;
            return (j_maybe(u32)) { .value = i, .is_present = true };
        }
    }
    return (j_maybe(u32)) { .is_present = false};
}

j_maybe(Str) j_arg_current_input(ArgParser *parser) {
    if (parser->_argc > 1) {
        return (j_maybe(Str)) { .value = str_from_cstr(parser->_argv[parser->_argc - 1]), .is_present = true };
    }
    return (j_maybe(Str)) { .is_present = false };
}



/**
 * Walks the file system starting at the given path
 * @param path The root directory to walk
 * @param options The options for the walker
 * @return nil if the path does not exist
 */
j_maybe(FS_Walker) j_fs_walk(Arena *arena, Str path, u32 options);
/**
 * Open a folder in the walk
 * @param walker The Walker
 * @param folder The folder to open
 * @return false if the folder does not exist
 */
bool j_fs_walk_open_folder(Arena *scratch, FS_Walker *walker, Str folder);
/**
 * Get the next file in the walk
 * The FS_Entry is only valid until the next call to this function. So copy it if you need it after.
 * @param walker The Walker
 * @return nil if there are no more files
 */
struct j_maybe(FS_Entry) j_fs_walk_next(Arena *arena, FS_Walker *walker);

void j_fs_path_push(Arena *arena, Path *path, Str entry);
j_maybe(Str) j_fs_path_pop(Path *path);
/**
 * Builds the path from the components separated by '/'
 * @param path The path
 * @return nil if the path is empty
 */
j_maybe(Str) j_fs_path_build(Path *path);

void j_fs_path_push(Arena *arena, Path *path, Str entry) {
    j_al_append(path->components, arena, entry);
}

j_maybe(Str) j_fs_path_pop(Path *path) {
    if (j_al_len(path->components) == 0) {
        return (j_maybe(Str)) { .is_present = false };
    }
    return (j_maybe(Str)) { .value = j_al_removeLast(path->components), .is_present = true };
}

j_maybe(Str) j_fs_path_build(Path *path) { //TODO: William Add an arena here.
    u32 len = j_al_len(path->components) - 1; // The amount of slashes needed.
    if (len == -1) {
        return (j_maybe(Str)) { .is_present = false };
    }
    for (u32 i = 0; i < j_al_len(path->components); ++i) {
        len += path->components[i].len;
    }

    char *buffer = malloc(len + 1);
    if (buffer == NULL) {
        return (j_maybe(Str)) { .is_present = false };
    }
    Str result = (Str) { .str = buffer, .len = len };

    for (u32 i = 0; i < j_al_len(path->components); ++i, ++buffer) {
        memmove(buffer, path->components[i].str, path->components[i].len);
        buffer += path->components[i].len;
        buffer[0] = '/';
    }
    buffer[0] = '\0';
    return (j_maybe(Str)) { .value = result, .is_present = true };
}

j_maybe(FS_Walker) j_fs_walk(Arena *arena, Str path, u32 options) {
    FS_Walker walker = {
            .open_directories = EMPTY_ARRAY,
            .path = { .components = EMPTY_ARRAY },
            .options = options,
    };
    if (j_fs_walk_open_folder(arena, &walker, path) == false) {
        return (j_maybe(FS_Walker)) { .is_present = false };
    }
    return (j_maybe(FS_Walker)) { .value = walker, .is_present = true };
}

bool j_fs_walk_open_folder(Arena *arena, FS_Walker *walker, Str folder) {
    j_fs_path_push(arena, &walker->path, folder);
    j_maybe(Str) mStr = j_fs_path_build(&walker->path);
    if (mStr.is_present == false) {
        j_fs_path_pop(&walker->path);
        return false;
    }
    Str path = mStr.value;
    DIR *dir = opendir(path.str);
    if (dir == NULL) {
        j_fs_path_pop(&walker->path);
        return false;
    }
    j_al_append(walker->open_directories, arena, ((FS_Dir) { .dir = dir, .remaining_entries = 0 }) );
    // Count the number of entries in the folder.
    if (j_bit_check(walker->options, J_FS_WALK_COUNT_FOLDER_ENTRIES)) {
        Str dot = str_from_lit(".");
        Str dotdot = str_from_lit("..");
        struct dirent *entry = NULL;
        u32 count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_FOLDER) == false && entry->d_type == DT_DIR) {
                continue;
            }
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_REGULAR) == false && entry->d_type == DT_REG) {
                continue;
            }
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_DOT) == false && str_eq(str_from_cstr(entry->d_name), dot)) {
                continue;
            }
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_DOTDOT) == false && str_eq(str_from_cstr(entry->d_name), dotdot)) {
                continue;
            }
            count++;
        }
        rewinddir(dir);
        j_al_last(walker->open_directories).remaining_entries = count;
    }
    return true;
}

j_maybe(FS_Entry) j_fs_walk_next(Arena *arena, FS_Walker *walker) {
    struct dirent *dir = NULL;
    jassert(j_al_len(walker->open_directories) > 0, "Precondition: When calling this method the walker must have a folder open.\n");
    if (j_bit_check(walker->options, J_FS_WALK_COUNT_FOLDER_ENTRIES)) {
        j_al_last(walker->open_directories).remaining_entries--;
    }
    Str dot = str_from_lit(".");
    Str dotdot = str_from_lit("..");
    u32 parent_index = j_al_len(walker->open_directories) - 1;
    do {
        dir = readdir(j_al_last(walker->open_directories).dir);
        if (dir == NULL) {
            closedir(j_al_removeLast(walker->open_directories).dir);
            parent_index--;
            // Go one folder up...
            j_fs_path_pop(&walker->path);

            if (j_al_len(walker->open_directories) == 0) {
                return (j_maybe(FS_Entry) ) {.is_present = false};
            }
            // TODO: William Check if this is correct...
            j_al_last(walker->open_directories).remaining_entries--;
            continue;
        }
        if (dir->d_type == DT_DIR) {
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_FOLDER) == false) {
                dir = NULL;
                continue;
            }
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_DOT) == false && str_eq(str_from_cstr(dir->d_name), dot)) {
                dir = NULL;
                continue;
            }
            if (j_bit_check(walker->options, J_FS_WALK_INCLUDE_DOTDOT) == false && str_eq(str_from_cstr(dir->d_name), dotdot)) {
                dir = NULL;
                continue;
            }
            j_fs_walk_open_folder(arena, walker, str_from_cstr(dir->d_name));
        }
        if (dir->d_type == DT_REG && j_bit_check(walker->options, J_FS_WALK_INCLUDE_REGULAR) == false) {
            dir = NULL;
            continue;
        }
    } while (dir == NULL);
    i32 remaining = walker->open_directories[parent_index].remaining_entries;
    return (j_maybe(FS_Entry)) { .is_present = true, .value = { .dirent = dir, .is_last = remaining == 0 } };
}



/**
 * Concatenates the strings into one string
 * @param strings The strings to concatenate
 * @return The concatenated string
 */
Str build_str(Str *strings) {
    u32 len = 0;
    for (u32 i = 0; i < j_al_len(strings); ++i) {
        len += strings[i].len;
    }
    char *buffer = malloc(len + 1);

    Str result = (Str) { .str = buffer, .len = len };
    for (u32 i = 0; i < j_al_len(strings); ++i) {
        memmove(buffer, strings[i].str, strings[i].len);
        buffer += strings[i].len;
    }
    buffer[0] = '\0';
    return result;
}

void print_a(int *a) {
    print("[");
    for (int i = 0; i < j_al_len(a); ++i) {
        if (i != 0) {
            print(", ");
        }
        print("{i32}", a[i]);
    }
    print("]\n");
}

void print_a_len(int *a) {
    print("{u32}\n", j_al_len(a));
}


int hmap_test(int argc, char **argv) {

    // Hmmm, figure out what to do here. I want type safety.
    // But again, what is the type. j_pair<Key, Value>? Or just Value *?

    // Below can be j_hmap_init(Str, Str)
    Arena program_memory = j_make_arena(J_MB(5));
    j_init_scratch(&program_memory, 2, J_MB(1));
    init_printers(&program_memory);

    Arena arena = {
            .stack = {
                    .memory = j_alloc(&arena, J_MB(1)),
                    .used = 0,
                    .size = J_MB(1)
            },
            .free_list = NULL
    };

    j_hmap(Str, Str) map = EMPTY_HMAP;
    j_hmap_init(map, j_hmap_generic_hash, j_hmap_generic_compare, 100);
//    j_hmap_init(map, j_hmap_hash_str, j_hmap_compare_str, 100);
    printf("The address of our map is %p\n", map);
    // Perhaps we can also do, _j_hmap_init(Str, Str, map); -> j_hmap(Str, Str) map = j_hmap_init(Str, Str);

    Str key1 = str_from_lit("key1");
    Str value = str_from_lit("value1");

    Str key2 = str_from_lit("Some awesome String Key");
    Str value2 = str_from_lit("Some awesome String Value");

    Str key3 = str_from_lit("Some key");
    Str value3 = str_from_lit("Some value");

    Str key4 = str_from_lit("Yet another key way");
    Str value4 = str_from_lit("Yet another value");



    j_hmap_put(map, key1, value);
    j_hmap_put(map, key2, value2);
    j_hmap_put(map, key3, value3);
    j_hmap_put(map, key4, value4);

    Str stored1 = j_hmap_get(map, key1);
    print("{str} -> {str}\n", key1, stored1);
    j_hmap_put(map, key1, str_from_lit("We just updated the value of key1 to this"));
    stored1 = j_hmap_get(map, key1);
    print("{str} -> {str}\n", key1, stored1);
    Str stored2 = j_hmap_get(map, key2);
    print("{str} -> {str}\n", key2, stored2);
    Str stored3 = j_hmap_get(map, key3);
    print("{str} -> {str}\n", key3, stored3);
    Str stored4 = j_hmap_get(map, key4);
    print("{str} -> {str}\n", key4, stored4);

    print("Iterating over the map\n\n\n");

    for (Maybeu32 it = j_hmap_iter(map); it.is_present; it = j_hmap_iter_next(map, it)) {
        j_maybe(j_pair(Str, Str)) mentry = map[it.value];
        j_pair(Str, Str) entry = j_hmap_iter_get(map, it);

        print("{str} -> {str}\n", mentry.value.first, mentry.value.second);
        print("{str} -> {str}\n", entry.first, entry.second);
    }

    print("count = {u32}\n", j_hmap_len(map));

    j_hmap_removeAll(map);
    print("After removing all entries\n");
    print("count = {u32}\n", j_hmap_len(map));

    print("Inserting more than the cap elements\n");
    for (u32 i = 0; i < 101; ++i) {

        Str key = str_format(&arena, "Key{u32}", i);
        Str value = str_format(&arena, "value{u32}", i);

        j_hmap_put(map, key, value);
        print("The count is {u32} and i is {u32}\n", j_hmap_len(map), i);
    }
    free(program_memory.stack.memory);
    return 0;
}



int substring_Test(void) {

    Str input = str_from_lit("   \t        Hello, World, This, is, a, test");

    SubStr sub = { .base = &input, .str = input };

    print("The first instance of the letter 'H' is at index {u32}\n", j_ss_first_index_of_c(sub, 'H'));

    j_ss_remove_while(&sub, j_ss_it == ' ');

    print("The substring is '{str}'\n", sub.str);
    j_ss_reset(sub);

    j_ss_trim_front_whitespace(&sub);
    print("The substring is '{str}'\n", sub.str);

    print("The first instance of the letter 'H' is at index {u32}\n", j_ss_first_index_of_c(sub, 'H'));

    Str search_for_me = str_from_lit("This");
    print("The word '{str}' appears at index {u32}\n", search_for_me, j_ss_first_index_where(sub, str_start_with(j_ss_it, search_for_me) == true));
    print("{str}\n", sub.str);

    j_ss_reset(sub);

    print("The prefix of spaces is is:'{str}'\n", j_ss_prefix_while(sub, j_ss_it == ' ').str);

    Str csvInput = str_from_lit("William,Juhl,24,Odense\nMarcell,Klitten,28,Odense\n");

    SubStr csvSub = { .base = &csvInput, .str = csvInput };
    j_maybe(SubStr) field;
    j_maybe(SubStr) line;
    while ((line = j_ss_split_on_first_str(&csvSub, str_from_lit("\n"))).is_present == true) {
        while ((field = j_ss_split_on_first_str(&line.value, str_from_lit(","))).is_present == true) {
            print("Field: '{str}'\n", field.value.str);
        }
        print("\n");
    }

    {
        j_ss_reset(csvSub);
        j_maybe(SubStr) mline;
        while_let_expr(line, mline, j_ss_split_on_first_str(&csvSub, str_from_lit("\n")), {
            j_maybe(SubStr) mfield;
            while_let_expr(field, mfield, j_ss_split_on_first_str(&line, str_from_lit(",")), {
                print("Field: '{str}'\n", field.str);
            })
            print("\n");
        })
    }

    {
        Maybeu32 mindex = {.is_present = true, .value = 69 };
        u32 index;
        if (true) {

        }
    }
    Maybeu32 mindex = {.is_present = true, .value = 69 };
    if_let(index, mindex, {
        print("{u32}\n", index);
    }) else {
        print("Nope\n");
    }

    if_let_expr(index, mindex, ((Maybeu32) { .is_present = true, .value = 69}), {
        print("Never nicer method maybe. {u32}\n", index);
    }) else {
        print("Nope\n");
    }

    return 0;
}



i32 j_rb_u32_comp(u32 lhs, u32 rhs) {
    return lhs - rhs;
}




static const Str maybeu32_Printer(Arena *arena, va_list * _Nonnull args) {
    j_maybe(u32) m = va_arg(*args, j_maybe(u32));
    if (m.is_present) {
        return str_format(arena, "{u32}", m.value);
    }
    return str_from_lit("Nil");
}

static const Str pair_u32bool_Printer(Arena *arena, va_list * _Nonnull args) {
    j_pair(u32, bool) p = va_arg(*args, j_pair(u32, bool));
    return str_format(arena, "({u32}, {bool})", p.first, p.second);
}

int redblacktree_test(void) {
    Arena program_memory = j_make_arena(J_MB(5));
    j_init_scratch(&program_memory, 2, J_MB(1));
    init_printers(&program_memory);



    str_register(&program_memory, "{maybeu32}", maybeu32_Printer);
    str_register(&program_memory, "{pair_u32bool}", pair_u32bool_Printer);

    // Red Black tree
    j_rb(u32, u32) tree = EMPTY_RB;

    // RBInsert
    j_pair(u32, bool) it1 = j_rb_put(tree, 42, 69);
    print("The cap of the tree {u32}\n", j_rb_cap(tree));
    print("The len of the tree {u32}\n", j_rb_len(tree));

    print("The first entry is: {u32}, {u32}\n", tree[0].first, tree[0].second);
    
    j_pair(u32, bool) it2 = j_rb_put(tree, 42, 420);
    
    print("The cap of the tree {u32}\n", j_rb_cap(tree));
    print("The len of the tree {u32}\n", j_rb_len(tree));

    print("The first entry is: {u32}, {u32}\n", tree[0].first, tree[0].second);

    j_pair(u32, bool) it3 = j_rb_put(tree, 1, 2);
    print("it1: {pair_u32bool}\n", it1);
    print("it2: {pair_u32bool}\n", it2);
    print("it3: {pair_u32bool}\n", it3);

    // Find
    u32 key = 42;
    j_maybe(u32) descriptor = j_rb_find(tree, key);

    print("Trying the find key {u32} -> {maybeu32}\n", key, descriptor);

    tree = EMPTY_RB;
    j_rb_put(tree, 24, 24);
    j_rb_put(tree, 18, 18);
    j_rb_put(tree, 26, 26);
    j_rb_put(tree, 5, 5);
    j_rb_put(tree, 20, 20);
    j_rb_put(tree, 27, 27);
    j_rb_put(tree, 2, 2);
    j_rb_put(tree, 7, 7);
    j_rb_put(tree, 23, 23);
    j_rb_put(tree, 21, 21);

    jassert(j_rb_len(tree) == 10, "The length of the tree should be 10\n");
    u32 root = j_rb_root(tree).value;
    jassert(j_rb_key(tree, root) == 24, "The root should be 24\n");
    jassert(j_rb_color(tree, root) == J_RB_BLACK, "The root should be black\n");

    j_maybe(u32) mk2 = j_rb_find(tree, 2);
    j_maybe(u32) mk5 = j_rb_find(tree, 5);
    j_maybe(u32) mk7 = j_rb_find(tree, 7);
    j_maybe(u32) mk18 = j_rb_find(tree, 18);
    j_maybe(u32) mk20 = j_rb_find(tree, 20);
    j_maybe(u32) mk21 = j_rb_find(tree, 21);
    j_maybe(u32) mk23 = j_rb_find(tree, 23);
    j_maybe(u32) mk24 = j_rb_find(tree, 24);
    j_maybe(u32) mk26 = j_rb_find(tree, 26);
    j_maybe(u32) mk27 = j_rb_find(tree, 27);

    jassert(mk2.is_present, "The key 2 should be present\n");
    jassert(mk5.is_present, "The key 5 should be present\n");
    jassert(mk7.is_present, "The key 7 should be present\n");
    jassert(mk18.is_present, "The key 18 should be present\n");
    jassert(mk20.is_present, "The key 20 should be present\n");
    jassert(mk21.is_present, "The key 21 should be present\n");
    jassert(mk23.is_present, "The key 23 should be present\n");
    jassert(mk24.is_present, "The key 24 should be present\n");
    jassert(mk26.is_present, "The key 26 should be present\n");
    jassert(mk27.is_present, "The key 27 should be present\n");

    u32 k2 = mk2.value;
    u32 k5 = mk5.value;
    u32 k7 = mk7.value;
    u32 k18 = mk18.value;
    u32 k20 = mk20.value;
    u32 k21 = mk21.value;
    u32 k23 = mk23.value;
    u32 k24 = mk24.value;
    u32 k26 = mk26.value;
    u32 k27 = mk27.value;

    jassert(j_rb_parent(tree, k2).value == k5, "The parent of 2 should be 5\n");
    jassert(j_rb_parent(tree, k7).value == k5, "The parent of 7 should be 5\n");
    jassert(j_rb_parent(tree, k20).value == k21, "The parent of 20 should be 21\n");
    jassert(j_rb_parent(tree, k23).value == k21, "The parent of 23 should be 21\n");
    jassert(j_rb_parent(tree, k5).value == k18, "The parent of 5 should be 18\n");
    jassert(j_rb_parent(tree, k21).value == k18, "The parent of 21 should be 18\n");
    jassert(j_rb_parent(tree, k18).value == k24, "The parent of 18 should be 24\n");
    jassert(j_rb_parent(tree, k27).value == k26, "The parent of 27 should be 26\n");
    jassert(j_rb_parent(tree, k26).value == k24, "The parent of 26 should be 24\n");

    jassert(_j_rb_left(tree, k24) == k18, "The left child of 24 should be 18\n");
    jassert(_j_rb_left(tree, k18) == k5, "The left child of 18 should be 5\n");
    jassert(_j_rb_left(tree, k5) == k2, "The left child of 5 should be 2\n");
    jassert(_j_rb_left(tree, k21) == k20, "The left child of 21 should be 20\n");

    jassert(_j_rb_right(tree, k5) == k7, "The right child of 5 should be 7\n");
    jassert(_j_rb_right(tree, k21) == k23, "The right child of 21 should be 23\n");
    jassert(_j_rb_right(tree, k24) == k26, "The right child of 24 should be 26\n");
    jassert(_j_rb_right(tree, k26) == k27, "The right child of 26 should be 27\n");
    return 0;
}

inline Stack j_stack_init(Arena *arena, u64 size) {
    return (Stack) {
            .size = size,
            .used = 0,
            .memory = j_alloc(arena, size)
    };
}

inline void *j_alloc(Arena *arena, u64 size) {
    jassert(arena->stack.size - arena->stack.used >= size, "The arena is out of memory\n");
    // Check what kind of allocation scheme we want to use.
    // For now, we ignore the freelist.
    
    void *ptr = arena->stack.memory + arena->stack.used;
    arena->stack.used += size;
    return ptr;
}

inline void j_free(Arena *arena, void *ptr, u64 size) {
    jassert(ptr >= arena->stack.memory && ptr < arena->stack.memory + arena->stack.used, "The pointer is not in the arena\n");
    // Check what kind of allocation scheme we want to use.
    // For now, we simply noop.
}


void do_stuff(Arena a) {
    print("Begin do_stuff\n");

    j_list(int) ints = EMPTY_ARRAY;
    j_al_append(ints, &a, 1);

    print("Using {u64} bytes memory\n", a.stack.used);

    print("End do_stuff\n");
}

void j_init_scratch(Arena *program_memory, i32 arena_count, u64 total_scratch_available) {
    jassert(program_memory->stack.size - program_memory->stack.used >= total_scratch_available, "The total scratch available is larger than the program_memory size\n");

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
            return (j_maybe(ArenaPtr)) { .is_present = true, .value = { .arena = &m->value } };
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

Arena j_make_arena(u64 size) {
    void *ptr = malloc(size);
    jassert(ptr != NULL, "Could not allocate memory for the arena\n");
    return (Arena) {
            .stack = {
                    .used = 0,
                    .size = size,
                    .memory = ptr
            },
            .free_list = NULL
    };
}

int main(void) {
     // Arena allocator
    u64 total_program_memory = J_MB(10);
    Arena program_memory = j_make_arena(total_program_memory);

    j_init_scratch(&program_memory, 2, J_MB(1));
    init_printers(&program_memory);

    Arena temp = {
            .stack = {
                    .used = 0,
                    .size = J_KB(1),
                    .memory = j_alloc(&program_memory, J_KB(1))
            },
            .free_list = NULL,
    };

    print("{u32}\n", temp.stack.used);
    do_stuff(temp);
    print("{u32}\n", temp.stack.used);

    // Pool style Allocator -> Fixed size, different lifetimes
    // Temporary Style Allocator -> Random size, short lifetime
    // Static style Allocator -> Random Size, long lifetime



    return 0;
}





void print_current_prefix(Str *prefix) {
    for (u32 i = 0; i < j_al_len(prefix); ++i) {
        print("{str}", prefix[i]);
    }
}

int tree(int argc, char **argv) {

    Arena program_memory = j_make_arena(J_MB(5));
    j_init_scratch(&program_memory, 2, J_MB(1));
    init_printers(&program_memory);

    char *program = argv[0];

    ArgParser parser = j_parser_init(program,
                                     "A copy of the unix tree command.\nIt walks the directory and prints the layout like a tree.\n",
                                     argc,
                                     argv);

    u32 HELP_FLAG = j_arg_option(&parser,
                                 "h",
                                 "help",
                                 "Print help message");
    u32 INCLUDE_FOLDER_FLAG = j_arg_option(&parser, "r", "recursive", "Walk the directory recursively");
    u32 INCLUDE_FILE_FLAG = j_arg_option(&parser, "f", "file", "Include files in the walk");
    u32 LEXICOGRAPHIC_FLAG = j_arg_option(&parser, "l", "lexicographic", "Walk the directory lexicographically");

    u32 options = 0;
    while (j_arg_has_next(&parser)) {
        j_maybe(u32) flag = j_arg_get_flag(&parser);
        if (flag.is_present){
            print("Flag: {str}\n", parser.flags[flag.value]);
            if (flag.value == HELP_FLAG) {
                print("{str}\n", parser.program_description);
                exit(1);
            }
            j_bit_set(options, flag.value);
        } else {
            j_maybe(Str) input = j_arg_current_input(&parser);
            if (input.is_present) {
                print("Unknown flag: {str}\n", input.value);
                exit(1);
            } else {
                print("No more flags...\n");
            }
        }
    }
    print("{u32}\n", options);
    for (i32 i = 31; i >= 0; --i) {
        print("{u32}", j_bit_check(options, i));
    }

    print("\n------------------\n");

    u32 walk_options = 0;
    if (j_bit_check(options, INCLUDE_FOLDER_FLAG)) {
        j_bit_set(walk_options, J_FS_WALK_INCLUDE_FOLDER);
    }
    if (j_bit_check(options, INCLUDE_FILE_FLAG)) {
        j_bit_set(walk_options, J_FS_WALK_INCLUDE_REGULAR);
    }
    if (j_bit_check(options, LEXICOGRAPHIC_FLAG)) {
        j_bit_set(walk_options, J_FS_WALK_ORDER_LEXICOGRAPHIC);
    }
    j_bit_set(walk_options, J_FS_WALK_COUNT_FOLDER_ENTRIES);


    Str dot = str_from_lit(".");
    Str dotdot = str_from_lit("..");
    j_maybe(FS_Walker) mwalker = j_fs_walk(&program_memory, dot, walk_options);
    if (mwalker.is_present == false) {
        print("Path does not exist\n");
        exit(1);
    }

    FS_Walker walker = mwalker.value;

    Str T     = str_from_lit("├── ");
    Str pipe  = str_from_lit("│   ");
    Str end   = str_from_lit("└── ");
    Str space = str_from_lit("    ");

    Str *prefix = EMPTY_ARRAY;
    bool first_iter = true;
    struct dirent current_entry = {0};
    bool current_entry_is_last = false;
    j_maybe(FS_Entry) peek_entry = {0};
    u32 current_depth = 0;
    while ((peek_entry = j_fs_walk_next(&program_memory, &walker)).is_present) {
        j_maybe(Str) m = j_fs_path_build(&walker.path);
        if (first_iter) {
            first_iter = false;
            current_entry = *peek_entry.value.dirent;
            current_depth = j_al_len(walker.path.components);
            current_entry_is_last = peek_entry.value.is_last;

            if (peek_entry.value.is_last) {
                j_al_append(prefix, &program_memory, end);
            } else {
                j_al_append(prefix, &program_memory, T);
            }
            continue;
        }

        u32 peek_depth = j_al_len(walker.path.components);

        Str entry_name = str_from_cstr(current_entry.d_name);

        print_current_prefix(prefix);
        print("{str}\n", entry_name);
        // Don't forget to add the regular files to the path......
        if (peek_entry.value.dirent->d_type == DT_REG) {
            peek_depth++;
        }
        if (current_entry.d_type == DT_REG) {
            current_depth++;
        }

        if (peek_depth > current_depth) {
            // Because we are going a step deeper, we have to update the current prefix.
            if (current_entry_is_last) {
                j_al_last(prefix) = space;
            } else {
                j_al_last(prefix) = pipe;
            }
            if (peek_entry.value.is_last) {
                j_al_append(prefix, &program_memory, end);
            } else {
                j_al_append(prefix, &program_memory, T);
            }
        } else {
            if (peek_depth < current_depth) {
                for (u32 i = 0; i < current_depth - peek_depth; ++i) {
                    j_al_removeLast(prefix);
                }
            }
            if (peek_entry.value.is_last) {
                j_al_last(prefix) = end;
            } else {
                j_al_last(prefix) = T;
            }
        }
        current_entry_is_last = peek_entry.value.is_last;
        current_entry = *peek_entry.value.dirent;
        current_depth = j_al_len(walker.path.components);
    }
    print_current_prefix(prefix);
    print("{str}\n", str_from_cstr(current_entry.d_name));
    return 0;
}
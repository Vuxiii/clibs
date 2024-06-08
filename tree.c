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
    Str flags = str_concat(str_from_cstr("-"), str_from_cstr(flag));
    Str alt = str_concat(str_from_cstr("--"), str_from_cstr(alternate));
    j_al_append(parser->flags, flags);
    j_al_append(parser->alternates, alt);
    j_al_append(parser->descriptions, str_from_cstr(description));
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
j_maybe(FS_Walker) j_fs_walk(Str path, u32 options);
/**
 * Open a folder in the walk
 * @param walker The Walker
 * @param folder The folder to open
 * @return false if the folder does not exist
 */
bool j_fs_walk_open_folder(FS_Walker *walker, Str folder);
/**
 * Get the next file in the walk
 * The FS_Entry is only valid until the next call to this function. So copy it if you need it after.
 * @param walker The Walker
 * @return nil if there are no more files
 */
struct j_maybe(FS_Entry) j_fs_walk_next(FS_Walker *walker);

void j_fs_path_push(Path *path, Str entry);
j_maybe(Str) j_fs_path_pop(Path *path);
/**
 * Builds the path from the components separated by '/'
 * @param path The path
 * @return nil if the path is empty
 */
j_maybe(Str) j_fs_path_build(Path *path);

void j_fs_path_push(Path *path, Str entry) {
    j_al_append(path->components, entry);
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

j_maybe(FS_Walker) j_fs_walk(Str path, u32 options) {
    FS_Walker walker = {
            .open_directories = EMPTY_ARRAY,
            .path = { .components = EMPTY_ARRAY },
            .options = options,
    };
    if (j_fs_walk_open_folder(&walker, path) == false) {
        return (j_maybe(FS_Walker)) { .is_present = false };
    }
    return (j_maybe(FS_Walker)) { .value = walker, .is_present = true };
}

bool j_fs_walk_open_folder(FS_Walker *walker, Str folder) {
    j_fs_path_push(&walker->path, folder);
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
    j_al_append(walker->open_directories, ((FS_Dir) { .dir = dir, .remaining_entries = 0 }) );
    // Count the number of entries in the folder.
    if (j_bit_check(walker->options, J_FS_WALK_COUNT_FOLDER_ENTRIES)) {
        Str dot = str_from_cstr(".");
        Str dotdot = str_from_cstr("..");
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

j_maybe(FS_Entry) j_fs_walk_next(FS_Walker *walker) {
    struct dirent *dir = NULL;
    jassert(j_al_len(walker->open_directories) > 0, "Precondition: When calling this method the walker must have a folder open.\n");
    if (j_bit_check(walker->options, J_FS_WALK_COUNT_FOLDER_ENTRIES)) {
        j_al_last(walker->open_directories).remaining_entries--;
    }
    Str dot = str_from_cstr(".");
    Str dotdot = str_from_cstr("..");
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
            j_fs_walk_open_folder(walker, str_from_cstr(dir->d_name));
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

//typedef u64 *HashFunction(const void *);
//
//typedef struct HMapStrStr {
//    HashFunction *hasher;
//    j_list(j_pair(Str, Str)) entries;
//} HMapStrStr;


int hmap_test(int argc, char **argv) {

    // Hmmm, figure out what to do here. I want type safety.
    // But again, what is the type. j_pair<Key, Value>? Or just Value *?

    // Below can be j_hmap_init(Str, Str)
    j_hmap(Str, Str) map = EMPTY_HMAP;
    j_hmap_init(map, j_hmap_generic_hash, j_hmap_generic_compare, 100);
//    j_hmap_init(map, j_hmap_hash_str, j_hmap_compare_str, 100);
    printf("The address of our map is %p\n", map);
    // Perhaps we can also do, _j_hmap_init(Str, Str, map); -> j_hmap(Str, Str) map = j_hmap_init(Str, Str);

    Str key1 = str_from_cstr("key1");
    Str value = str_from_cstr("value1");

    Str key2 = str_from_cstr("Some awesome String Key");
    Str value2 = str_from_cstr("Some awesome String Value");

    Str key3 = str_from_cstr("Some key");
    Str value3 = str_from_cstr("Some value");

    Str key4 = str_from_cstr("Yet another key way");
    Str value4 = str_from_cstr("Yet another value");



    j_hmap_put(map, key1, value);
    j_hmap_put(map, key2, value2);
    j_hmap_put(map, key3, value3);
    j_hmap_put(map, key4, value4);

    Str stored1 = j_hmap_get(map, key1);
    print("{str} -> {str}\n", key1, stored1);
    j_hmap_put(map, key1, str_from_cstr("We just updated the value of key1 to this"));
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

        Str key = str_format("Key{u32}", i);
        Str value = str_format("value{u32}", i);

        j_hmap_put(map, key, value);
        print("The count is {u32} and i is {u32}\n", j_hmap_len(map), i);
    }

    return 0;
}





int main(void) {

    print("{str}{str}", str_from_cstr("Hello"), str_from_cstr(" World\n"));

    return 0;
}












void print_current_prefix(Str *prefix) {
    for (u32 i = 0; i < j_al_len(prefix); ++i) {
        print("{str}", prefix[i]);
    }
}

int tree(int argc, char **argv) {
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


    Str dot = str_from_cstr(".");
    Str dotdot = str_from_cstr("..");
    j_maybe(FS_Walker) mwalker = j_fs_walk(dot, walk_options);
    if (mwalker.is_present == false) {
        print("Path does not exist\n");
        exit(1);
    }

    FS_Walker walker = mwalker.value;

    Str T     = str_from_cstr("├── ");
    Str pipe  = str_from_cstr("│   ");
    Str end   = str_from_cstr("└── ");
    Str space = str_from_cstr("    ");

    Str *prefix = EMPTY_ARRAY;
    bool first_iter = true;
    struct dirent current_entry = {0};
    bool current_entry_is_last = false;
    j_maybe(FS_Entry) peek_entry = {0};
    u32 current_depth = 0;
    while ((peek_entry = j_fs_walk_next(&walker)).is_present) {
        j_maybe(Str) m = j_fs_path_build(&walker.path);
        if (first_iter) {
            first_iter = false;
            current_entry = *peek_entry.value.dirent;
            current_depth = j_al_len(walker.path.components);
            current_entry_is_last = peek_entry.value.is_last;

            if (peek_entry.value.is_last) {
                j_al_append(prefix, end);
            } else {
                j_al_append(prefix, T);
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
                j_al_append(prefix, end);
            } else {
                j_al_append(prefix, T);
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
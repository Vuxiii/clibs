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

    Str *args;
    Str *flags;
    Str *alternates;
    Str *descriptions;
} ArgParser;


ArgParser j_parser_init(char *program, char *program_description, int argc, char **argv);
void j_arg_usage(ArgParser *parser, char *program);
u32 j_arg_option(ArgParser *parser, char *flag, char *alternate, char *description);
bool j_arg_has_next(ArgParser *parser);
Maybeu32 j_arg_get_flag(ArgParser *parser);
MaybeStr j_arg_current_input(ArgParser *parser);

ArgParser j_parser_init(char *program, char *program_description, int argc, char **argv) {
    return (ArgParser) {
            .program = str_from_cstr(program),
            .program_description = str_from_cstr(program_description),
            ._argc = argc,
            ._argv = argv,
            .args = NULL,
            .flags = NULL,
            .alternates = NULL,
            .descriptions = NULL,
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

Maybeu32 j_arg_get_flag(ArgParser *parser) {
    Str flag = str_from_cstr(parser->_argv[parser->_argc - 1]);
    // Check if we have the flag registered..
    for (u32 i = 0; i < j_al_len(parser->flags); ++i) {
        if (str_eq(flag, parser->flags[i]) || str_eq(flag, parser->alternates[i])) {
            parser->_argc--;
            return (Maybeu32) { .value = i, .is_present = true };
        }
    }
    return (Maybeu32) { .is_present = false};
}

MaybeStr j_arg_current_input(ArgParser *parser) {
    if (parser->_argc > 1) {
        return (MaybeStr) { .str = str_from_cstr(parser->_argv[parser->_argc - 1]), .is_present = true };
    }
    return (MaybeStr) { .is_present = false };
}



/**
 * Walks the file system starting at the given path
 * @param path The root directory to walk
 * @param options The options for the walker
 * @return nil if the path does not exist
 */
MaybeFS_Walker j_fs_walk(Str path, u32 options);
/**
 * Open a folder in the walk
 * @param walker The Walker
 * @param folder The folder to open
 * @return false if the folder does not exist
 */
bool j_fs_walk_open_folder(FS_Walker *walker, Str folder);
/**
 * Get the next file in the walk
 * @param walker The Walker
 * @return nil if there are no more files
 */
MaybeDirent j_fs_walk_next(FS_Walker *walker);

void j_fs_path_push(Path *path, Str entry);
MaybeStr j_fs_path_pop(Path *path);
MaybeStr j_fs_path_build(Path *path);

void j_fs_path_push(Path *path, Str entry) {
    j_al_append(path->components, entry);
}

MaybeStr j_fs_path_pop(Path *path) {
    if (j_al_len(path->components) == 0) {
        return (MaybeStr) { .is_present = false };
    }
    return (MaybeStr) { .str = j_al_removeLast(path->components), .is_present = true };
}

MaybeStr j_fs_path_build(Path *path) {
    u32 len = j_al_len(path->components) - 1; // The amount of slashes needed.
    if (len == -1) {
        return (MaybeStr) { .is_present = false };
    }
    for (u32 i = 0; i < j_al_len(path->components); ++i) {
        len += path->components[i].len;
    }

    char *buffer = malloc(len + 1);
    if (buffer == NULL) {
        return (MaybeStr) { .is_present = false };
    }
    Str result = (Str) { .str = buffer, .len = len };

    for (u32 i = 0; i < j_al_len(path->components); ++i) {
        memmove(buffer, path->components[i].str, path->components[i].len);
        buffer += path->components[i].len;
        buffer[0] = '/';
        buffer++;
    }
    --buffer;
    buffer[0] = '\0';
    return (MaybeStr) { .str = result, .is_present = true };
}

MaybeFS_Walker j_fs_walk(Str path, u32 options) {
    FS_Walker walker = {
            .current_directory = NULL,
            .path = NULL,
            .options = options,
    };
    if (j_fs_walk_open_folder(&walker, path) == false) {
        return (MaybeFS_Walker) { .is_present = false };
    }
    return (MaybeFS_Walker) { .walker = walker, .is_present = true };
}

bool j_fs_walk_open_folder(FS_Walker *walker, Str folder) {
    j_fs_path_push(&walker->path, folder);
    MaybeStr mStr = j_fs_path_build(&walker->path);
    if (mStr.is_present == false) {
        j_fs_path_pop(&walker->path);
        return false;
    }
    Str path = mStr.str;
    DIR *dir = opendir(path.str);
    if (dir == NULL) {
        j_fs_path_pop(&walker->path);
        return false;
    }
    j_al_append(walker->current_directory, dir);
    return true;
}

MaybeDirent j_fs_walk_next(FS_Walker *walker) {
    struct dirent *dir = NULL;
    assert(("Precondition: When calling this method the walker must have a folder open.\n",
            j_al_len(walker->current_directory) > 0));
    Str dot = str_from_cstr(".");
    Str dotdot = str_from_cstr("..");
    do {
        dir = readdir(j_al_last(walker->current_directory));
        if (dir == NULL) {
            closedir(j_al_removeLast(walker->current_directory));
            // Go one folder up...
            j_fs_path_pop(&walker->path);

            if (j_al_len(walker->current_directory) == 0) {
                return (MaybeDirent) {.is_present = false};
            }
            continue;
        }
//        print("Looking at Entry: {str}\n", str_from_cstr(dir->d_name));
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
    return (MaybeDirent) { .is_present = true, .dirent = dir };
}

int main(int argc, char **argv) {
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
        Maybeu32 flag = j_arg_get_flag(&parser);
        if (flag.is_present){
            print("Flag: {str}\n", parser.flags[flag.value]);
            if (flag.value == HELP_FLAG) {
                print("{str}\n", parser.program_description);
                exit(1);
            }
            j_bit_set(options, flag.value);
        } else {
            MaybeStr input = j_arg_current_input(&parser);
            if (input.is_present) {
                print("Unknown flag: {str}\n", input.str);
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
//    if (j_bit_check(options, INCLUDE_FILE_FLAG)) {
        j_bit_set(walk_options, J_FS_WALK_INCLUDE_REGULAR);
//    }
    if (j_bit_check(options, LEXICOGRAPHIC_FLAG)) {
        j_bit_set(walk_options, J_FS_WALK_ORDER_LEXICOGRAPHIC);
    }
    Str dot = str_from_cstr(".");
    Str dotdot = str_from_cstr("..");
    MaybeFS_Walker mwalker = j_fs_walk(dot, walk_options);
    if (mwalker.is_present == false) {
        print("Path does not exist\n");
        exit(1);
    }

    FS_Walker walker = mwalker.walker;

    Str T = str_from_cstr("├── ");
    Str pipe = str_from_cstr("│  ");
    Str end = str_from_cstr("└── ");
    Str space = str_from_cstr("   ");

    u32 last_depth = 0;
    bool is_last = false;
    Str *prefix = NULL;

    j_al_append(prefix, T);
    MaybeDirent current_entry = {0};

    Str last_path = str_from_cstr(".");
    // NOTE: we can save the entry for a single iter and compare it to the last entry to determine if we are the last entry.
    // Which is the last thing i need. Also, figure out why the +2 is needed below.... I'm too tired lmao.
    while ((current_entry = j_fs_walk_next(&walker)).is_present) {
        MaybeStr m = j_fs_path_build(&walker.path);
        u32 d = j_al_len(walker.path.components) - 1;
        if (d < last_depth) {
            for (u32 i = d; i < last_depth+2; ++i) {
                j_al_removeLast(prefix);
            }


            j_al_append(prefix, T);
        }
        struct dirent *dirent = current_entry.dirent;
        Str entry_name = str_from_cstr(dirent->d_name);
        if (str_eq(entry_name, dot) || str_eq(entry_name, dotdot)) {
            continue;
        }
        for (u32 i = 0; i < j_al_len(prefix); ++i) {
            print("{str}", prefix[i]);
        }
        print("{str}\n", str_from_cstr(current_entry.dirent->d_name));
        if (dirent->d_type == DT_REG && !is_last) {
            if (j_al_len(prefix) > 0) {
                j_al_removeLast(prefix);
            }
            j_al_append(prefix, T);
        } else if (dirent->d_type == DT_DIR && !is_last) {
            if (j_al_len(prefix) > 0) {
                j_al_removeLast(prefix);
            }
            j_al_append(prefix, pipe);
            j_al_append(prefix, T);
        }
        last_path = m.str;
        last_depth = d;

    }

    return 0;
}
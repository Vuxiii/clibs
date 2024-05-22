//
// Created by William Juhl on 21/05/2024.
//
#define JLIB_IMPL

#include <string.h>
#include "jlib.h"

typedef struct ArgParser {
    IStr program;
    Str *args;

    Str *flags;
    Str *alternates;
    Str *descriptions;
} ArgParser;


ArgParser j_parser_init(char *program);
void j_arg_set_name(ArgParser *parser, char *program);
void j_arg_usage(ArgParser *parser, char *program);
void j_arg_option(ArgParser *parser, char *flag, char *alternate, char *description);

//ArgParser j_parser_init(char *program) {
//    ArgParser parser = {
//            .program = str_from_cstr(program),
//            .args = arraylist_new(sizeof (Str)),
//            .flags = arraylist_new(sizeof(Str)),
//            .alternates = arraylist_new(sizeof(Str)),
//            .descriptions = arraylist_new(sizeof(Str)),
//    };
//    return parser;
//}
//
//void j_arg_set_name(ArgParser *parser, char *program) {
//    parser->program = str_from_cstr(program);
//}
//
//void j_arg_option(ArgParser *parser, char *flag, char *alternate, char *description) {
//    arraylist_push(&parser->flags, str_from_cstr(flag));
//    arraylist_push(&parser->alternates, str_from_cstr(alternate));
//    arraylist_push(&parser->descriptions, str_from_cstr(description));
//}

int main(int argc, char **argv) {
    char *program = argv[0];
//
//    ArgParser parser = j_parser_init(program);
//
//    j_arg_set_name(&parser, program);
//    j_arg_option(&parser, "h", "help", "Print help message");
//

    int *ints = NULL;

    j_al_append(ints, 1);
    j_al_append(ints, 2);
    j_al_append(ints, 3);
    j_al_append(ints, 4);
    j_al_append(ints, 5);
    j_al_append(ints, 6);
    j_al_append(ints, 7);
    j_al_append(ints, 8);
    j_al_append(ints, 9);
    j_al_append(ints, 10);
    j_al_append(ints, 11);
    print("Capacity: {u32}\n", j_al_cap(ints));
    print("Length:   {u32}\n", j_al_len(ints));
    print("[");
    for (u32 i = 0; i < j_al_len(ints); i++) {
        if (i != 0) print(", ");
        print("{i32}", ints[i]);
    }
    print("]\n");
    int removedElement = j_al_removeLast(ints);
    print("Removed: {i32}\n", removedElement);
    print("Capacity: {u32}\n", j_al_cap(ints));
    print("Length:   {u32}\n", j_al_len(ints));
    print("[");
    for (u32 i = 0; i < j_al_len(ints); i++) {
        if (i != 0) print(", ");
        print("{i32}", ints[i]);
    }
    print("]\n");
    j_al_removeFirst(ints);
    print("Capacity: {u32}\n", j_al_cap(ints));
    print("Length:   {u32}\n", j_al_len(ints));
    print("[");

    print("]\nFinished\n");

    j_al_free(ints);

    assert(ints == NULL);

    // Init
//    if (ints == NULL) {
//         Base case
//        ints = malloc(10 * sizeof(ints[0]) + sizeof(ArrHeader)) + sizeof(ArrHeader);
//        ArrHeader *header = j_al_header(ints);
//        u32 *len = &j_al_header(ints)->len;
//        u32 *cap = &j_al_header(ints)->cap;
//        *len = 0;
//        *cap = 10;
//    }
    // Realloc
//    u32 len = j_al_len(ints);
//    u32 cap = j_al_cap(ints);
//    if (j_al_len(ints) == j_al_cap(ints)) {
//        void *p = realloc(cast(void *, ints) - sizeof(ArrHeader), sizeof(ArrHeader) + j_al_cap(ints) * sizeof(ints[0]) * 2);
//        jassert(p, "Failed to realloc");
//        ints = p + sizeof(ArrHeader);
//        *cast(u32 *, ints - sizeof(ArrHeader) + offsetof(ArrHeader, cap)) *= 2;
//    }
    // Insert
//    ints[j_al_len(ints)] = 2;
//    *cast(u32 *, cast(char *, ints) - sizeof(ArrHeader) + offsetof(ArrHeader, len)) += 1;
//    ints[j_al_len(ints)] = 2;
//    j_al_append(ints, 1);
    return 0;
}
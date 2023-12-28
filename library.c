#include "library.h"

#include <stdio.h>
typedef struct {
    double x;
    double y;
} Point;

void Point_Printer( va_list *args ) {
    Point point = va_arg(*args, Point);
    printf( "{%f, %f}", point.x, point.y );
}

int int_less( const void *a, const void *b ) {
    return *(int*)a - *(int*)b;
}

int double_less( const void *a, const void *b ) {
    return *(double*)a - *(double*)b;
}

int main(void) {

    const Str s = str_format("My number is {i32}\n", 42);

//    print(s);
//
//    print("{u64}\n", -1);
//    print("{u32}\n", -1);
//    print("{i32}\n", 0);
//    print("{i32}\n", -0);
//    print("{u32}\n", 10);
//    print("{u32}\n", 0);
//    print("{u32}\n", -0);
    print("{f32}\n", 7.0045000301);

//    ArrayList arr = arraylist_new(sizeof(i32));
//    array_push(&arr, -1);
//    array_push(&arr, 9);
//    array_push(&arr, -2);
//    array_push(&arr, 4);
//    array_push(&arr, -8);
//
//    forward_it(arr, i32) {
//        print("{i32}\n", *it);
//    }
//    arraylist_sort(arr, int_less);
//    forward_it(arr, i32) {
//        print("{i32}\n", *it);
//    }
//
//    str_register( "{Point}", Point_Printer);
//
//    ArrayList points = arraylist_new(sizeof(Point));
//    arraylist_push(&points, &(Point){1.0, 2.0});
//    arraylist_push(&points, &(Point){3.0, 4.0});
//    arraylist_push(&points, &(Point){5.0, 6.0});
//
//    forward_it(points, Point) {
//        print("{Point}\n", *it);
//    }
//
//    Point *last_point = &arraylist_at(points, 2, Point);
//
//    print("Last point: {Point}\n", *last_point);
//
//    reverse_it(points, Point) {
//        print("{Point}\n", *it);
//    }
//
//    for ( Point *it = points.data; it <= &arraylist_last(points, Point); ++it ) {
//        print("Point {Point}\n", *it);
//    }
//
//    print("Hello {Point}\n", arraylist_at(points, 1, Point));
//
//    arraylist_free(&points);
//
//    print("Hello {Point}\n", (Point){7.4, 2.0});
//
////    Str s;
//    print(str_from_cstr("Hello {Point}\n"), (Point){1.0, 2.0});
//    print("Hello {Point}\n", (Point){1.0, 2.0});

//    bool a = false;
//    print("{str}\n", a);
//    print("{bool}\n", !a);

//    print("String in braces: {str}\n", str_from_cstr("Hello World"));

    return 0;

}
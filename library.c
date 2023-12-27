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



int main(void) {


    str_register( "{Point}", Point_Printer);

    print(str_from_cstr("Hello {Point}\n"), (Point){1.0, 2.0});
    print("Hello {Point}\n", (Point){1.0, 2.0});

    bool a = false;
    print("{str}\n", a);
    print("{bool}\n", !a);

    print("String in braces: {str}\n", str_from_cstr("Hello World"));

    return 0;

}
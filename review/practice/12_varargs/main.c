// Variadic Functions (Variable-Length Arguments) in C

#include <stdio.h>
#include <stdarg.h> // va_list, va_start, va_arg, va_end

// =========================================================
// Basic structure of a variadic function
//
// 1. At least one fixed argument is required (passed to va_start)
// 2. Begin with va_start(args, last_fixed)
// 3. Retrieve arguments one by one with va_arg(args, type)
// 4. End with va_end(args) — mandatory
// 5. Types are the caller's responsibility (wrong type = UB)
// =========================================================

// Example 1: Sum of ints (pattern: pass the argument count explicitly)
int sum_ints(int count, ...) {
    va_list args;
    va_start(args, count);  // count is the last fixed argument

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);  // retrieve as int
    }

    va_end(args);
    return total;
}

// Example 2: Print all strings (NULL sentinel pattern)
// The end of the argument list is indicated by NULL
void print_all(const char *first, ...) {
    va_list args;
    va_start(args, first);

    const char *s = first;
    while (s != NULL) {
        printf("%s ", s);
        s = va_arg(args, const char *);  // retrieve the next string
    }
    printf("\n");

    va_end(args);
}

// Example 3: Custom log function (format string pattern)
// Accepts the same arguments as printf and forwards them to vprintf
void my_log(const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("[%s] ", level);
    vprintf(fmt, args);  // use vprintf, which accepts a va_list directly
    printf("\n");

    va_end(args);
}

// Example 4: Forwarding a va_list to another function
// Making a function that accepts va_list enables reuse
int my_vsnprintf_len(const char *fmt, va_list args) {
    // vsnprintf returns the number of bytes that would be written
    return vsnprintf(NULL, 0, fmt, args);  // pass NULL buffer to compute length only
}

int my_format_len(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = my_vsnprintf_len(fmt, args);
    va_end(args);
    return len;
}

// =========================================================
// Dangerous pattern: type-unsafe usage
// =========================================================

// NOTE: float arguments are promoted to double, so use double in va_arg
void wrong_float(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        // float is promoted to double when passed as a variadic argument
        // va_arg(args, float) is NG -> va_arg(args, double) is correct
        double v = va_arg(args, double);  // OK
        printf("%.2f ", v);
    }
    va_end(args);
    printf("\n");
}

int main(void) {

    printf("=== sum_ints (pass count explicitly) ===\n");
    printf("sum(1,2,3)      = %d\n", sum_ints(3, 1, 2, 3));
    printf("sum(10,20,30,40)= %d\n", sum_ints(4, 10, 20, 30, 40));

    printf("\n=== print_all (NULL sentinel pattern) ===\n");
    print_all("hello", "world", "foo", NULL);
    print_all("one", NULL);

    printf("\n=== my_log (format string pattern) ===\n");
    my_log("INFO",  "server started on port=%d", 8080);
    my_log("ERROR", "file not found: %s", "config.toml");
    my_log("DEBUG", "x=%d, y=%.2f", 42, 3.14);

    printf("\n=== pre-compute formatted string length ===\n");
    int len = my_format_len("value=%d, name=%s", 42, "Alice");
    printf("formatted length: %d\n", len);

    printf("\n=== float is promoted to double ===\n");
    wrong_float(3, 1.1f, 2.2f, 3.3f);  // passed as float, received as double

    printf("\n=== wrong type causes UB (commented out) ===\n");
    printf("e.g. sum_ints(2, 1.0, 2.0) with va_arg(int) -> UB\n");
    // detectable at runtime with -fsanitize=undefined

    return 0;
}

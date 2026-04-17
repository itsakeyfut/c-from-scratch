// Preprocessor / Macro in C

#include <stdio.h>
#include <assert.h>

// =========================================================
// Constant macros (#define)
// No type information, unlike const int.
// Prefer enum or static const for type safety.
// =========================================================
#define PI        3.14159265358979
#define MAX_BUF   256
#define APP_NAME "MyApp"

// =========================================================
// Function-like macros — replaced verbatim at expansion time
// 1. Always wrap arguments in ()
// 2. Always wrap the entire macro in ()
// 3. Watch out for side effects from multiple evaluations
// =========================================================
#define SQUARE(x) ((x) * (x))     // OK: properly parenthesized
#define SQUARE_NG(x) x * x        // NG: missing parentheses
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define ABS(x)     ((x) < 0 ? -(x) : (x))

// Array length (only valid before array decay)
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// =========================================================
// do { ... } while(0) idiom
// Required to safely use multi-statement macros inside if/else
// =========================================================
// Explicit-type version (works in standard C11)
#define SWAP(type, a, b) do { \
    type _tmp = (a);          \
    (a) = (b);                \
    (b) = _tmp;               \
} while(0)

// =========================================================
// Conditional compilation (#ifdef / #ifndef / #if)
// =========================================================
#ifdef DEBUG
    #define LOG(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)  ((void)0) // no-op (eliminated by the optimizer)
#endif

// =========================================================
// Stringification (#) and token pasting (##)
// =========================================================
#define STRINGIFY(x) #x    // converts x to a string literal
#define CONCAT(a, b) a##b  // pastes a and b together as a single token

// =========================================================
// Predefined macros (provided by the compiler)
// =========================================================
// __FILE__   : current file name (string)
// __LINE__   : current line number (integer)
// __func__   : current function name (C99 and later)
// __DATE__   : compilation date (e.g., "Jan  1 2025")
// __TIME__   : compilation time (e.g., "12:34:56")
// __STDC__   : 1 if the compiler conforms to the C standard

int main(void) {
    printf("=== Constant Macros ===\n");
    printf("PI      = %f\n", PI);
    printf("MAX_BUF = %d\n", MAX_BUF);
    printf("APP_NAME = %s\n", APP_NAME);

    // =========================================================
    // Side effect pitfalls with function-like macros
    // =========================================================
    printf("\n=== Macro side effect pitfalls ===\n");
    int v = 3;

    printf("SQUARE(v)      = %d\n", SQUARE(v));     // 9  (OK)
    printf("SQUARE(v + 1)  = %d\n", SQUARE(v + 1)); // 16 (OK: parentheses present)
    printf("SQUARE_NG(v+1) = %d\n", SQUARE_NG(v+1));// 7  (NG: expands to 3+1*3+1 = 7)

    // Side effect: ++ is evaluated twice
    int x = 3;
    int result = MAX(x++, 2);  // x++ evaluated twice -> x becomes 5
    printf("MAX(x++, 2): result=%d, x=%d (x incremented twice)\n", result, x);
    // -> never pass expressions with side effects as macro arguments

    printf("\n=== MAX / MIN / ABS ===\n");
    printf("MAX(4, 7)  = %d\n", MAX(4, 7));
    printf("MIN(4, 7)  = %d\n", MIN(4, 7));
    printf("ABS(-5)    = %d\n", ABS(-5));

    printf("\n=== ARRAY_LEN ===\n");
    int arr[] = {10, 20, 30, 40, 50};
    printf("ARRAY_LEN(arr) = %zu\n", ARRAY_LEN(arr));

    printf("\n=== SWAP (do-while macro) ===\n");
    int a = 1, b = 2;
    printf("before: a=%d, b=%d\n", a, b);
    SWAP(int, a, b);
    printf("after:  a=%d, b=%d\n", a, b);

    // =========================================================
    // Conditional compilation
    // =========================================================
    printf("\n=== Conditional Compilation ===\n");
    LOG("debug message: x=%d", x);  // printed only when compiled with -DDEBUG
#ifdef DEBUG
    printf("DEBUG build\n");
#else
    printf("RELEASE build (use -DDEBUG for a debug build)\n");
#endif

    // =========================================================
    // Stringification and token pasting
    // =========================================================
    printf("\n=== Stringification (#) ===\n");
    printf("STRINGIFY(hello) = %s\n", STRINGIFY(hello));
    printf("STRINGIFY(42)    = %s\n", STRINGIFY(42));
    printf("STRINGIFY(a + b) = %s\n", STRINGIFY(a + b));

    printf("\n=== Token Pasting (##) ===\n");
    int CONCAT(my_, value) = 42;   // expands to: int my_value = 42;
    printf("my_value = %d\n", my_value);

    // =========================================================
    // Predefined macros
    // =========================================================
    printf("\n=== Predefined Macros ===\n");
    printf("__FILE__ = %s\n", __FILE__);
    printf("__LINE__ = %d\n", __LINE__);
    printf("__func__ = %s\n", __func__);
    printf("__DATE__ = %s\n", __DATE__);
    printf("__TIME__ = %s\n", __TIME__);

    // =========================================================
    // assert — aborts the program if the condition is false
    // Disabled when NDEBUG is defined (e.g., in release builds)
    // =========================================================
    printf("\n=== assert ===\n");
    int val = 5;
    assert(val > 0);         // OK: passes
    assert(val == 5);        // OK: passes
    printf("assert passed\n");
    // assert(val == 0);     // NG: would abort here

    return 0;
}

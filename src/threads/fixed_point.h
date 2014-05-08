#ifndef THREADS_FIXED_POINT
#define THREADS_FIXED_POINT

#define FP_Q 14
#define FP_F (1 << FP_Q)

#define INT_TO_FP(n) (n * FP_F)
#define FP_TO_INT(x) (x / FP_F)
#define FP_TO_INT_ROUND(x) (x>=0? (x + FP_F / 2) / FP_F \
        : (x - FP_F / 2) / FP_F)
#define FP_ADD(x, y) (x + y)
#define FP_SUBSTRACT(x, y) (x - y)
#define FP_ADD_N(x, n) (x + n * FP_F)
#define FP_SUB_N(x, n) (x - n * FP_F)
#define FP_MULT(x, y) (((int64_t) x) * y / FP_F)
#define FP_MULT_N(x, n) (x * n)
#define FP_DIV(x, y) (((int64_t) x) * FP_F / y)
#define FP_DIV_N(x, n) (x / n)
#endif // THREADS_FIXED_POINT

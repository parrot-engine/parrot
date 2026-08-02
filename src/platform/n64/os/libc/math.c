#include <math.h>

double acos(double x) {
    return atan2(sqrt(1.0 - x * x), x);
}

float acosf(float x) {
    return atan2f(sqrtf(1.0f - x * x), x);
}

double asin(double x) {
    return atan2(x, sqrt(1.0 - x * x));
}

float asinf(float x) {
    return atan2f(x, sqrtf(1.0f - x * x));
}

double atan(double x) {
    return atan2(x, 1.0);
}

float atanf(float x) {
    return atan2f(x, 1.0f);
}

double atan2(double y, double x) {
    if (x == 0.0 && y == 0.0)
        return 0.0;
    double ax = fabs(x), ay = fabs(y);
    double a = (ax > ay) ? ay / ax : ax / ay;
    double s = a * a;
    double r = ((-0.0464964749 * s + 0.15931422) * s - 0.327622764) * s * a + a;
    if (ay > ax)
        r = M_PI / 2 - r;
    if (x < 0.0)
        r = M_PI - r;
    if (y < 0.0)
        r = -r;
    return r;
}

float atan2f(float y, float x) {
    if (x == 0.0f && y == 0.0f)
        return 0.0f;
    float ax = fabsf(x), ay = fabsf(y);
    float a = (ax > ay) ? ay / ax : ax / ay;
    float s = a * a;
    float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
    if (ay > ax)
        r = (float)(M_PI / 2) - r;
    if (x < 0.0f)
        r = (float)M_PI - r;
    if (y < 0.0f)
        r = -r;
    return r;
}

double cos(double x) {
    return sin(x + M_PI / 2);
}

float cosf(float x) {
    return sinf(x + M_PI / 2);
}

float sinf(float x) {
    return sin(x);
}

double tan(double x) {
    return sin(x) / cos(x);
}

float tanf(float x) {
    return sinf(x) / cosf(x);
}

double acosh(double x) {
    return log(x + sqrt(x * x - 1.0));
}

float acoshf(float x) {
    return logf(x + sqrtf(x * x - 1.0f));
}

double asinh(double x) {
    return log(x + sqrt(x * x + 1.0));
}

float asinhf(float x) {
    return logf(x + sqrtf(x * x + 1.0f));
}

double atanh(double x) {
    return 0.5 * log((1.0 + x) / (1.0 - x));
}

float atanhf(float x) {
    return 0.5f * logf((1.0f + x) / (1.0f - x));
}

double cosh(double x) {
    return (exp(x) + exp(-x)) * 0.5;
}

float coshf(float x) {
    return (expf(x) + expf(-x)) * 0.5f;
}

double sinh(double x) {
    return (exp(x) - exp(-x)) * 0.5;
}

float sinhf(float x) {
    return (expf(x) - expf(-x)) * 0.5f;
}

double tanh(double x) {
    double e = exp(2.0 * x);
    return (e - 1.0) / (e + 1.0);
}

float tanhf(float x) {
    float e = expf(2.0f * x);
    return (e - 1.0f) / (e + 1.0f);
}

double exp(double x) {
    if (x == 0.0)
        return 1.0;
    int n = (int)(x * 1.4426950408889634);
    double r = x - n * 0.6931471805599453;
    double y =
        1.0 + r * (1.0 + r * (0.5 + r * (1.0 / 6.0 + r * (1.0 / 24.0 +
                                                          r * (1.0 / 120.0 + r * (1.0 / 720.0 + r * (1.0 / 5040.0)))))));
    return ldexp(y, n);
}

float expf(float x) {
    if (x == 0.0f)
        return 1.0f;
    int n = (int)(x * 1.4426950408889634f);
    float r = x - n * 0.6931471805599453f;
    float y =
        1.0f +
        r * (1.0f +
             r * (0.5f + r * (1.0f / 6.0f +
                              r * (1.0f / 24.0f + r * (1.0f / 120.0f + r * (1.0f / 720.0f + r * (1.0f / 5040.0f)))))));
    return ldexpf(y, n);
}

double exp2(double x) {
    return exp(x * 0.6931471805599453);
}

float exp2f(float x) {
    return expf(x * 0.6931471805599453f);
}

double expm1(double x) {
    return exp(x) - 1.0;
}

float expm1f(float x) {
    return expf(x) - 1.0f;
}

double frexp(double value, int *exp) {
    if (value == 0.0) {
        *exp = 0;
        return 0.0;
    }
    *exp = (int)(log2(fabs(value))) + 1;
    return value * exp2(-(*exp));
}

float frexpf(float value, int *exp) {
    if (value == 0.0f) {
        *exp = 0;
        return 0.0f;
    }
    *exp = (int)(log2f(fabsf(value))) + 1;
    return value * exp2f(-(*exp));
}

int ilogb(double x) {
    if (x == 0.0)
        return 0x80000000;
    return (int)logb(x);
}

int ilogbf(float x) {
    if (x == 0.0f)
        return 0x80000000;
    return (int)logbf(x);
}

double ldexp(double x, int exp) {
    return x * exp2(exp);
}

float ldexpf(float x, int exp) {
    return x * exp2f(exp);
}

double log(double x) {
    if (x <= 0.0)
        return 0.0;
    int e;
    double m = frexp(x, &e);
    m = (m - 1.0) / (m + 1.0);
    double m2 = m * m;
    double y = 2.0 * m * (1.0 + m2 * (1.0 / 3.0 + m2 * (1.0 / 5.0 + m2 * (1.0 / 7.0 + m2 * (1.0 / 9.0)))));
    return y + e * 0.6931471805599453;
}

float logf(float x) {
    if (x <= 0.0f)
        return 0.0f;
    int e;
    float m = frexpf(x, &e);
    m = (m - 1.0f) / (m + 1.0f);
    float m2 = m * m;
    float y = 2.0f * m * (1.0f + m2 * (1.0f / 3.0f + m2 * (1.0f / 5.0f + m2 * (1.0f / 7.0f + m2 * (1.0f / 9.0f)))));
    return y + e * 0.6931471805599453f;
}

double log10(double x) {
    return log(x) * 0.4342944819032518;
}

float log10f(float x) {
    return logf(x) * 0.4342944819032518f;
}

double log1p(double x) {
    return log(1.0 + x);
}

float log1pf(float x) {
    return logf(1.0f + x);
}

double log2(double x) {
    return log(x) * 1.4426950408889634;
}

float log2f(float x) {
    return logf(x) * 1.4426950408889634f;
}

double logb(double x) {
    int e;
    frexp(x, &e);
    return (double)(e - 1);
}

float logbf(float x) {
    int e;
    frexpf(x, &e);
    return (float)(e - 1);
}

double modf(double value, double *iptr) {
    *iptr = trunc(value);
    return value - *iptr;
}

float modff(float value, float *iptr) {
    *iptr = truncf(value);
    return value - *iptr;
}

double scalbn(double x, int n) {
    return ldexp(x, n);
}

float scalbnf(float x, int n) {
    return ldexpf(x, n);
}

double scalbln(double x, long int n) {
    return ldexp(x, (int)n);
}

float scalblnf(float x, long int n) {
    return ldexpf(x, (int)n);
}

double cbrt(double x) {
    if (x == 0.0)
        return 0.0;
    double y = exp(log(fabs(x)) / 3.0);
    return (x < 0.0) ? -y : y;
}

float cbrtf(float x) {
    if (x == 0.0f)
        return 0.0f;
    float y = expf(logf(fabsf(x)) / 3.0f);
    return (x < 0.0f) ? -y : y;
}

double fabs(double x) {
    double result;
    __asm__("abs.d %0, %1" : "=f"(result) : "f"(x));
    return result;
}

float fabsf(float x) {
    float result;
    __asm__("abs.s %0, %1" : "=f"(result) : "f"(x));
    return result;
}

double hypot(double x, double y) {
    return sqrt(x * x + y * y);
}

float hypotf(float x, float y) {
    return sqrtf(x * x + y * y);
}

double pow(double x, double y) {
    if (x == 0.0)
        return (y > 0.0) ? 0.0 : 1.0;
    return exp(y * log(x));
}

float powf(float x, float y) {
    if (x == 0.0f)
        return (y > 0.0f) ? 0.0f : 1.0f;
    return expf(y * logf(x));
}

double sqrt(double x) {
    double result;
    __asm__("sqrt.d %0, %1" : "=f"(result) : "f"(x));
    return result;
}

float sqrtf(float x) {
    float result;
    __asm__("sqrt.s %0, %1" : "=f"(result) : "f"(x));
    return result;
}

double erf(double x) {
    double t = 1.0 / (1.0 + 0.3275911 * fabs(x));
    double y = 1.0 - (((((1.061405429 * t - 1.453152027) * t) + 1.421413741) * t - 0.284496736) * t + 0.254829592) * t *
                         exp(-x * x);
    return (x >= 0.0) ? y : -y;
}

float erff(float x) {
    float t = 1.0f / (1.0f + 0.3275911f * fabsf(x));
    float y = 1.0f - (((((1.061405429f * t - 1.453152027f) * t) + 1.421413741f) * t - 0.284496736f) * t + 0.254829592f) *
                         t * expf(-x * x);
    return (x >= 0.0f) ? y : -y;
}

double erfc(double x) {
    return 1.0 - erf(x);
}

float erfcf(float x) {
    return 1.0f - erff(x);
}

double lgamma(double x) {
    return log(tgamma(x));
}

float lgammaf(float x) {
    return logf(tgammaf(x));
}

double tgamma(double x) {
    if (x < 0.5)
        return M_PI / (sin(M_PI * x) * tgamma(1.0 - x));
    x -= 1.0;
    double a = 0.99999999999980993 + 676.5203681218851 / (x + 1.0) - 1259.1392167224028 / (x + 2.0) +
               771.32342877765313 / (x + 3.0) - 176.61502916214059 / (x + 4.0) + 12.507343278686905 / (x + 5.0) -
               0.13857109526572012 / (x + 6.0) + 9.984369654078991e-6 / (x + 7.0) + 1.5056327351493116e-7 / (x + 8.0);
    return sqrt(2.0 * M_PI) * pow(x + 7.5, x + 0.5) * exp(-(x + 7.5)) * a;
}

float tgammaf(float x) {
    return (float)tgamma(x);
}

double ceil(double x) {
    long long int truncated = x;
    int with_fraction = (x < truncated);
    return truncated + with_fraction;
}

float ceilf(float x) {
    long long int truncated = x;
    int with_fraction = (x < truncated);
    return truncated + with_fraction;
}

double floor(double x) {
    long long int truncated = x;
    int with_fraction = (x < truncated);
    return truncated - with_fraction;
}

float floorf(float x) {
    long long int truncated = x;
    int with_fraction = (x < truncated);
    return truncated - with_fraction;
}

double nearbyint(double x) {
    return round(x);
}

float nearbyintf(float x) {
    return roundf(x);
}

double rint(double x) {
    return round(x);
}

float rintf(float x) {
    return roundf(x);
}

long int lrint(double x) {
    return (long int)round(x);
}

long int lrintf(float x) {
    return (long int)roundf(x);
}

long long int llrint(double x) {
    return (long long int)round(x);
}

long long int llrintf(float x) {
    return (long long int)roundf(x);
}

double round(double x) {
    return floor(x + 0.5);
}

float roundf(float x) {
    return floorf(x + 0.5);
}

long int lround(double x) {
    return floor(x + 0.5);
}

long int lroundf(float x) {
    return floorf(x + 0.5);
}

long long int llround(double x) {
    return floor(x + 0.5);
}

long long int llroundf(float x) {
    return floorf(x + 0.5);
}

double trunc(double x) {
    return (x < 0.0) ? ceil(x) : floor(x);
}

float truncf(float x) {
    return (x < 0.0f) ? ceilf(x) : floorf(x);
}

double fmod(double x, double y) {
    return x - y * trunc(x / y);
}

float fmodf(float x, float y) {
    return x - y * truncf(x / y);
}

double remainder(double x, double y) {
    double n = round(x / y);
    return x - n * y;
}

float remainderf(float x, float y) {
    float n = roundf(x / y);
    return x - n * y;
}

double remquo(double x, double y, int *quo) {
    double n = round(x / y);
    *quo = (int)n;
    return x - n * y;
}

float remquof(float x, float y, int *quo) {
    float n = roundf(x / y);
    *quo = (int)n;
    return x - n * y;
}

double copysign(double x, double y) {
    return (y < 0.0) ? -fabs(x) : fabs(x);
}

float copysignf(float x, float y) {
    return (y < 0.0f) ? -fabsf(x) : fabsf(x);
}

double nan(const char *tagp) {
    (void)tagp;
    return 0.0 / 0.0;
}

float nanf(const char *tagp) {
    (void)tagp;
    return 0.0f / 0.0f;
}

double nextafter(double x, double y) {
    if (x == y)
        return y;
    double d = (y > x) ? 1e-15 : -1e-15;
    return x + d;
}

float nextafterf(float x, float y) {
    if (x == y)
        return y;
    float d = (y > x) ? 1e-7f : -1e-7f;
    return x + d;
}

double nexttoward(double x, long double y) {
    return nextafter(x, (double)y);
}

float nexttowardf(float x, long double y) {
    return nextafterf(x, (float)y);
}

double fdim(double x, double y) {
    return (x > y) ? x - y : 0.0;
}

float fdimf(float x, float y) {
    return (x > y) ? x - y : 0.0f;
}

double fmax(double x, double y) {
    return (x > y) ? x : y;
}

float fmaxf(float x, float y) {
    return (x > y) ? x : y;
}

double fmin(double x, double y) {
    return (x < y) ? x : y;
}

float fminf(float x, float y) {
    return (x < y) ? x : y;
}

double fma(double x, double y, double z) {
    return x * y + z;
}

float fmaf(float x, float y, float z) {
    return x * y + z;
}
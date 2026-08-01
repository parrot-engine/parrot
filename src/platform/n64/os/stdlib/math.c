#include "platform/n64/stdlib/math.h"
#include <stdbool.h>
#include <stddef.h>

#define PARROT_SINE_LENGTH 256
double Parrot_sine_table[PARROT_SINE_LENGTH];

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

double sin(double x) {
    size_t index = (size_t)round((x * PARROT_SINE_LENGTH * 2) / M_PI) % (PARROT_SINE_LENGTH * 4);

    bool flip = false;
    if (index > 0) {
        if (index > PARROT_SINE_LENGTH * 2) {
            index -= PARROT_SINE_LENGTH * 2;
            flip = true;
        }

        if (index > PARROT_SINE_LENGTH) {
            index = (PARROT_SINE_LENGTH * 2) - index;
        }
    }

    double value = Parrot_sine_table[index];
    if (flip) {
        value = -value;
    }
    return value;
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

double Parrot_sine_table[PARROT_SINE_LENGTH] = {0.0,
                                                0.006135884649154475,
                                                0.012271538285719925,
                                                0.01840672990580482,
                                                0.024541228522912288,
                                                0.030674803176636626,
                                                0.03680722294135883,
                                                0.04293825693494082,
                                                0.049067674327418015,
                                                0.055195244349689934,
                                                0.06132073630220858,
                                                0.06744391956366405,
                                                0.07356456359966743,
                                                0.07968243797143013,
                                                0.0857973123444399,
                                                0.09190895649713272,
                                                0.0980171403295606,
                                                0.10412163387205459,
                                                0.11022220729388306,
                                                0.11631863091190475,
                                                0.1224106751992162,
                                                0.12849811079379317,
                                                0.13458070850712617,
                                                0.1406582393328492,
                                                0.14673047445536175,
                                                0.15279718525844344,
                                                0.15885814333386145,
                                                0.16491312048996992,
                                                0.17096188876030122,
                                                0.17700422041214875,
                                                0.18303988795514095,
                                                0.1890686641498062,
                                                0.19509032201612825,
                                                0.2011046348420919,
                                                0.20711137619221856,
                                                0.21311031991609136,
                                                0.2191012401568698,
                                                0.22508391135979283,
                                                0.2310581082806711,
                                                0.2370236059943672,
                                                0.24298017990326387,
                                                0.24892760574572015,
                                                0.25486565960451457,
                                                0.2607941179152755,
                                                0.26671275747489837,
                                                0.272621355449949,
                                                0.27851968938505306,
                                                0.2844075372112719,
                                                0.29028467725446233,
                                                0.2961508882436238,
                                                0.3020059493192281,
                                                0.30784964004153487,
                                                0.3136817403988915,
                                                0.3195020308160157,
                                                0.3253102921622629,
                                                0.33110630575987643,
                                                0.33688985339222005,
                                                0.3426607173119944,
                                                0.34841868024943456,
                                                0.35416352542049034,
                                                0.3598950365349881,
                                                0.36561299780477385,
                                                0.37131719395183754,
                                                0.37700741021641826,
                                                0.3826834323650898,
                                                0.38834504669882625,
                                                0.3939920400610481,
                                                0.3996241998456468,
                                                0.40524131400498986,
                                                0.4108431710579039,
                                                0.41642956009763715,
                                                0.4220002707997997,
                                                0.4275550934302821,
                                                0.43309381885315196,
                                                0.43861623853852766,
                                                0.4441221445704292,
                                                0.44961132965460654,
                                                0.45508358712634384,
                                                0.46053871095824,
                                                0.4659764957679662,
                                                0.47139673682599764,
                                                0.4767992300633221,
                                                0.4821837720791227,
                                                0.487550160148436,
                                                0.49289819222978404,
                                                0.49822766697278187,
                                                0.5035383837257176,
                                                0.508830142543107,
                                                0.5141027441932217,
                                                0.5193559901655896,
                                                0.524589682678469,
                                                0.5298036246862946,
                                                0.5349976198870972,
                                                0.5401714727298929,
                                                0.5453249884220465,
                                                0.5504579729366048,
                                                0.5555702330196022,
                                                0.560661576197336,
                                                0.5657318107836131,
                                                0.5707807458869673,
                                                0.5758081914178453,
                                                0.5808139580957645,
                                                0.5857978574564389,
                                                0.5907597018588742,
                                                0.5956993044924334,
                                                0.600616479383869,
                                                0.6055110414043255,
                                                0.6103828062763095,
                                                0.6152315905806268,
                                                0.6200572117632891,
                                                0.6248594881423863,
                                                0.629638238914927,
                                                0.6343932841636455,
                                                0.6391244448637757,
                                                0.6438315428897914,
                                                0.6485144010221124,
                                                0.6531728429537768,
                                                0.6578066932970786,
                                                0.6624157775901718,
                                                0.6669999223036375,
                                                0.6715589548470183,
                                                0.6760927035753159,
                                                0.680600997795453,
                                                0.6850836677727004,
                                                0.6895405447370668,
                                                0.693971460889654,
                                                0.6983762494089729,
                                                0.7027547444572253,
                                                0.7071067811865475,
                                                0.7114321957452164,
                                                0.7157308252838186,
                                                0.7200025079613817,
                                                0.7242470829514669,
                                                0.7284643904482252,
                                                0.7326542716724128,
                                                0.7368165688773698,
                                                0.7409511253549591,
                                                0.745057785441466,
                                                0.7491363945234593,
                                                0.7531867990436124,
                                                0.7572088465064845,
                                                0.7612023854842618,
                                                0.765167265622459,
                                                0.7691033376455796,
                                                0.773010453362737,
                                                0.7768884656732324,
                                                0.7807372285720944,
                                                0.7845565971555752,
                                                0.7883464276266062,
                                                0.7921065773002124,
                                                0.7958369046088835,
                                                0.799537269107905,
                                                0.8032075314806448,
                                                0.8068475535437992,
                                                0.8104571982525948,
                                                0.8140363297059483,
                                                0.8175848131515837,
                                                0.8211025149911046,
                                                0.8245893027850253,
                                                0.8280450452577558,
                                                0.8314696123025452,
                                                0.83486287498638,
                                                0.838224705554838,
                                                0.8415549774368983,
                                                0.844853565249707,
                                                0.8481203448032971,
                                                0.8513551931052652,
                                                0.8545579883654005,
                                                0.8577286100002721,
                                                0.8608669386377673,
                                                0.8639728561215867,
                                                0.8670462455156926,
                                                0.8700869911087113,
                                                0.8730949784182901,
                                                0.8760700941954066,
                                                0.8790122264286334,
                                                0.8819212643483549,
                                                0.8847970984309378,
                                                0.8876396204028539,
                                                0.8904487232447579,
                                                0.8932243011955153,
                                                0.8959662497561851,
                                                0.8986744656939538,
                                                0.901348847046022,
                                                0.9039892931234433,
                                                0.9065957045149153,
                                                0.9091679830905223,
                                                0.9117060320054299,
                                                0.9142097557035307,
                                                0.9166790599210427,
                                                0.9191138516900578,
                                                0.9215140393420419,
                                                0.9238795325112867,
                                                0.9262102421383113,
                                                0.9285060804732155,
                                                0.9307669610789837,
                                                0.9329927988347388,
                                                0.9351835099389475,
                                                0.937339011912575,
                                                0.9394592236021899,
                                                0.9415440651830208,
                                                0.9435934581619604,
                                                0.9456073253805213,
                                                0.9475855910177411,
                                                0.9495281805930367,
                                                0.9514350209690083,
                                                0.9533060403541938,
                                                0.9551411683057707,
                                                0.9569403357322089,
                                                0.9587034748958716,
                                                0.9604305194155658,
                                                0.9621214042690416,
                                                0.9637760657954398,
                                                0.9653944416976894,
                                                0.9669764710448521,
                                                0.9685220942744173,
                                                0.970031253194544,
                                                0.9715038909862518,
                                                0.9729399522055601,
                                                0.9743393827855759,
                                                0.9757021300385286,
                                                0.9770281426577544,
                                                0.9783173707196277,
                                                0.9795697656854405,
                                                0.9807852804032304,
                                                0.9819638691095552,
                                                0.9831054874312163,
                                                0.984210092386929,
                                                0.9852776423889412,
                                                0.9863080972445987,
                                                0.9873014181578584,
                                                0.9882575677307495,
                                                0.989176509964781,
                                                0.9900582102622971,
                                                0.99090263542778,
                                                0.9917097536690995,
                                                0.99247953459871,
                                                0.9932119492347945,
                                                0.9939069700023561,
                                                0.9945645707342554,
                                                0.9951847266721968,
                                                0.9957674144676598,
                                                0.996312612182778,
                                                0.9968202992911657,
                                                0.9972904566786902,
                                                0.9977230666441916,
                                                0.9981181129001492,
                                                0.9984755805732948,
                                                0.9987954562051724,
                                                0.9990777277526454,
                                                0.9993223845883495,
                                                0.9995294175010931,
                                                0.9996988186962042,
                                                0.9998305817958234,
                                                0.9999247018391445,
                                                0.9999811752826011};
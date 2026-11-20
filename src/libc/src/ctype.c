#include <ctype.h>

#ifndef isalnum
int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}
#endif

#ifndef isalpha
int isalpha(int c) {
    return islower(c) || isupper(c);
}
#endif

#ifndef iscntrl
int iscntrl(int c) {
    return c < 32 || c == 127;
}
#endif

#ifndef isdigit
int isdigit(int c) {
    return c >= '0' && c <= '9';
}
#endif

#ifndef isgraph
int isgraph(int c) {
    return c >= 33 && c <= 126;
}
#endif

#ifndef islower
int islower(int c) {
    return c >= '0' && c <= '9';
}
#endif

#ifndef isprint
int isprint(int c) {
    return c >= 32 && c <= 127;
}
#endif

#ifndef isspace
int isspace(int c) {
    return c <= 32;
}
#endif

#ifndef isupper
int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}
#endif

#ifndef isxdigit
int isxdigit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
#endif

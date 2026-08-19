#include "parrot/drivers/window_driver.h"
#include "parrot/core/util.h"
#include "parrot/drivers/window_driver.keys.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/dbe.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ParrotWindowDriverWindow {
    Display *display;

    int screen;
    Window window;

    int width;
    int height;

    Atom wm_delete;

    bool close_requested;

    GC gc;
    XdbeBackBuffer back_buffer;

    uint32_t *image_data;
    XImage *image;

    bool mod_lctrl;
    bool mod_lshift;
    bool mod_lalt;
    bool mod_rctrl;
    bool mod_rshift;
    bool mod_ralt;
};

static ParrotWindowDriverEventKey x_keysym_to_driver_key(KeySym keysym);

static void driver_init(void) {
}

static void driver_shutdown(void) {
}

static ParrotWindowDriverWindow *driver_create_window(int width, int height) {
    ParrotWindowDriverWindow *self = malloc(sizeof(ParrotWindowDriverWindow));
    PARROT_RET_COND_V(!self, NULL);
    memset(self, 0, sizeof(ParrotWindowDriverWindow));

    self->width = width;
    self->height = height;

    self->display = XOpenDisplay(NULL);
    PARROT_FAIL_NULL_MSG(self->display, "Failed to open X11 display");

    self->screen = DefaultScreen(self->display);
    Window root = RootWindow(self->display, self->screen);

    self->window = XCreateSimpleWindow(self->display,
                                       root,
                                       0,
                                       0,
                                       width,
                                       height,
                                       1,
                                       BlackPixel(self->display, self->screen),
                                       BlackPixel(self->display, self->screen));
    XSelectInput(self->display, self->window, ExposureMask | KeyPress | KeyRelease);

    XMapWindow(self->display, self->window);

    self->wm_delete = XInternAtom(self->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(self->display, self->window, &self->wm_delete, 1);

    self->gc = XCreateGC(self->display, self->window, 0, NULL);
    self->back_buffer = XdbeAllocateBackBufferName(self->display, self->window, XdbeBackground);

    self->image_data = calloc(width * height, sizeof(uint32_t));
    self->image = XCreateImage(self->display,
                               DefaultVisual(self->display, self->screen),
                               DefaultDepth(self->display, self->screen),
                               ZPixmap,
                               0,
                               (char *)self->image_data,
                               width,
                               height,
                               32,
                               0);

    XPutImage(self->display, self->window, self->gc, self->image, 0, 0, 0, 0, width, height);
    XFlush(self->display);

    return self;
}

static void driver_delete_window(ParrotWindowDriverWindow *self) {
    XDestroyImage(self->image);

    XdbeDeallocateBackBufferName(self->display, self->back_buffer);
    XFreeGC(self->display, self->gc);

    XDestroyWindow(self->display, self->window);

    XCloseDisplay(self->display);

    free(self);
}

static bool driver_poll_events(ParrotWindowDriverWindow *self, ParrotWindowDriverEvent *out_event) {
    XEvent event;
    while (XPending(self->display)) {
        XNextEvent(self->display, &event);
        switch (event.type) {
        case ClientMessage: {
            if ((Atom)event.xclient.data.l[0] == self->wm_delete) {
                *out_event = (ParrotWindowDriverEvent){
                    .type = ParrotWindowDriverEventType_QUIT,
                };
                return true;
            }
        } break;
        case Expose: {
            XWindowAttributes attrs;
            XGetWindowAttributes(self->display, self->window, &attrs);

            if (attrs.width != self->width || attrs.height != self->height) {
                *out_event = (ParrotWindowDriverEvent){
                    .type = ParrotWindowDriverEventType_RESIZE,
                    .data.resize =
                        {
                            .width = attrs.width,
                            .height = attrs.height,
                            .old_width = self->width,
                            .old_height = self->height,
                        },
                };

                XDestroyImage(self->image);

                self->image_data = calloc(attrs.width * attrs.height, sizeof(uint32_t));
                self->image = XCreateImage(self->display,
                                           DefaultVisual(self->display, self->screen),
                                           DefaultDepth(self->display, self->screen),
                                           ZPixmap,
                                           0,
                                           (char *)self->image_data,
                                           attrs.width,
                                           attrs.height,
                                           32,
                                           0);

                self->width = attrs.width;
                self->height = attrs.height;

                return true;
            }
        } break;
        case KeyPress:
        case KeyRelease: {
            bool key_down = event.type == KeyPress;

            char buf[32];
            KeySym logical_key;

            int len = XLookupString(&event.xkey, buf, sizeof(buf), &logical_key, NULL);

            switch (logical_key) {
            case XK_Control_L:
                self->mod_lctrl = key_down;
                break;
            case XK_Shift_L:
                self->mod_lshift = key_down;
                break;
            case XK_Alt_L:
                self->mod_lalt = key_down;
                break;
            case XK_Control_R:
                self->mod_rctrl = key_down;
                break;
            case XK_Shift_R:
                self->mod_rshift = key_down;
                break;
            case XK_Alt_R:
                self->mod_ralt = key_down;
                break;

            default:
                break;
            }

            *out_event = (ParrotWindowDriverEvent){
                .type = ParrotWindowDriverEventType_KEY,
                .data.key =
                    {
                        .physical_key = x_keysym_to_driver_key(logical_key),
                        .logical_key = x_keysym_to_driver_key(logical_key),
                        .character = len == 1 ? buf[0] : '\0',

                        .key_down = key_down,

                        .mod_lctrl = self->mod_lctrl,
                        .mod_lshift = self->mod_lshift,
                        .mod_lalt = self->mod_lalt,
                        .mod_rctrl = self->mod_rctrl,
                        .mod_rshift = self->mod_rshift,
                        .mod_ralt = self->mod_ralt,
                        .mod_caps_lock = event.xkey.state & LockMask,
                    },
            };
            return true;
        } break;
        }
    }

    return false;
}

static void driver_set_title(ParrotWindowDriverWindow *self, const char *title) {
    Atom netWmName = XInternAtom(self->display, "_NET_WM_NAME", False);
    Atom utf8 = XInternAtom(self->display, "UTF8_STRING", False);
    XChangeProperty(self->display,
                    self->window,
                    netWmName,
                    utf8,
                    8,
                    PropModeReplace,
                    (const unsigned char *)title,
                    strlen(title) * sizeof(char));
}

static void driver_set_size(ParrotWindowDriverWindow *self, int width, int height) {
    XResizeWindow(self->display, self->window, width, height);
}

static int driver_get_width(ParrotWindowDriverWindow *self) {
    return self->width;
}

static int driver_get_height(ParrotWindowDriverWindow *self) {
    return self->height;
}

static void driver_set_image(ParrotWindowDriverWindow *self, const uint32_t *rgbx8888) {
    PARROT_FAIL_NULL(self);

    XWindowAttributes attrs;
    XGetWindowAttributes(self->display, self->window, &attrs);

    for (int y = 0; y < attrs.height; y++) {
        for (int x = 0; x < attrs.width; x++) {
            uint8_t r = rgbx8888[y * attrs.width + x] & 0xFF;
            uint8_t g = (rgbx8888[y * attrs.width + x] >> 8) & 0xFF;
            uint8_t b = (rgbx8888[y * attrs.width + x] >> 16) & 0xFF;
            self->image_data[y * attrs.width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
    XPutImage(self->display, self->back_buffer, self->gc, self->image, 0, 0, 0, 0, attrs.width, attrs.height);

    XdbeSwapInfo swap_info = {self->window, XdbeBackground};
    XdbeSwapBuffers(self->display, &swap_info, 1);
}

static ParrotWindowDriverEventKey x_keysym_to_driver_key(KeySym keysym) {
    switch (keysym) {
    case XK_Escape:
        return ParrotWindowDriverEventKey_ESCAPE;
    case XK_BackSpace:
        return ParrotWindowDriverEventKey_BACKSPACE;
    case XK_Tab:
        return ParrotWindowDriverEventKey_TAB;
    case XK_Clear:
        return ParrotWindowDriverEventKey_CLEAR;
    case XK_Return:
        return ParrotWindowDriverEventKey_RETURN;
    case XK_Pause:
        return ParrotWindowDriverEventKey_PAUSE;
    case XK_Scroll_Lock:
        return ParrotWindowDriverEventKey_SCROLLLOCK;
    case XK_Delete:
        return ParrotWindowDriverEventKey_DELETE;
    case XK_Home:
        return ParrotWindowDriverEventKey_HOME;
    case XK_Up:
        return ParrotWindowDriverEventKey_UP;
    case XK_Right:
        return ParrotWindowDriverEventKey_RIGHT;
    case XK_Down:
        return ParrotWindowDriverEventKey_DOWN;
    case XK_Prior: /* same value as XK_Page_Up */
        return ParrotWindowDriverEventKey_PRIOR;
    case XK_Page_Down:
        return ParrotWindowDriverEventKey_PAGEDOWN;
    case XK_End:
        return ParrotWindowDriverEventKey_END;
    case XK_Select:
        return ParrotWindowDriverEventKey_SELECT;
    case XK_Print:
        return ParrotWindowDriverEventKey_PRINTSCREEN;
    case XK_Insert:
        return ParrotWindowDriverEventKey_INSERT;
    case XK_Undo:
        return ParrotWindowDriverEventKey_UNDO;
    case XK_Menu:
        return ParrotWindowDriverEventKey_MENU;
    case XK_Cancel:
        return ParrotWindowDriverEventKey_CANCEL;
    case XK_Help:
        return ParrotWindowDriverEventKey_HELP;
    case XK_Num_Lock:
        return ParrotWindowDriverEventKey_NUMLOCKCLEAR;
    case XK_KP_Space:
        return ParrotWindowDriverEventKey_KP_SPACE;
    case XK_KP_Tab:
        return ParrotWindowDriverEventKey_KP_TAB;
    case XK_KP_Enter:
        return ParrotWindowDriverEventKey_KP_ENTER;
    case XK_KP_Multiply:
        return ParrotWindowDriverEventKey_KP_MULTIPLY;
    case XK_KP_Add:
        return ParrotWindowDriverEventKey_KP_PLUS;
    case XK_KP_Subtract:
        return ParrotWindowDriverEventKey_KP_MINUS;
    case XK_KP_Decimal:
        return ParrotWindowDriverEventKey_KP_DECIMAL;
    case XK_KP_Divide:
        return ParrotWindowDriverEventKey_KP_DIVIDE;
    case XK_KP_0:
        return ParrotWindowDriverEventKey_KP_0;
    case XK_KP_1:
        return ParrotWindowDriverEventKey_KP_1;
    case XK_KP_2:
        return ParrotWindowDriverEventKey_KP_2;
    case XK_KP_3:
        return ParrotWindowDriverEventKey_KP_3;
    case XK_KP_4:
        return ParrotWindowDriverEventKey_KP_4;
    case XK_KP_5:
        return ParrotWindowDriverEventKey_KP_5;
    case XK_KP_6:
        return ParrotWindowDriverEventKey_KP_6;
    case XK_KP_7:
        return ParrotWindowDriverEventKey_KP_7;
    case XK_KP_8:
        return ParrotWindowDriverEventKey_KP_8;
    case XK_KP_9:
        return ParrotWindowDriverEventKey_KP_9;
    case XK_F1:
        return ParrotWindowDriverEventKey_F1;
    case XK_F12:
        return ParrotWindowDriverEventKey_F12;
    case XK_F13:
        return ParrotWindowDriverEventKey_F13;
    case XK_F14:
        return ParrotWindowDriverEventKey_F14;
    case XK_F15:
        return ParrotWindowDriverEventKey_F15;
    case XK_F16:
        return ParrotWindowDriverEventKey_F16;
    case XK_F17:
        return ParrotWindowDriverEventKey_F17;
    case XK_F18:
        return ParrotWindowDriverEventKey_F18;
    case XK_F19:
        return ParrotWindowDriverEventKey_F19;
    case XK_F20:
        return ParrotWindowDriverEventKey_F20;
    case XK_F21:
        return ParrotWindowDriverEventKey_F21;
    case XK_F22:
        return ParrotWindowDriverEventKey_F22;
    case XK_F23:
        return ParrotWindowDriverEventKey_F23;
    case XK_F24:
        return ParrotWindowDriverEventKey_F24;
    case XK_Shift_L:
        return ParrotWindowDriverEventKey_LSHIFT;
    case XK_Shift_R:
        return ParrotWindowDriverEventKey_RSHIFT;
    case XK_Control_L:
        return ParrotWindowDriverEventKey_LCTRL;
    case XK_Control_R:
        return ParrotWindowDriverEventKey_RCTRL;
    case XK_Caps_Lock:
        return ParrotWindowDriverEventKey_CAPSLOCK;
    case XK_Alt_L:
        return ParrotWindowDriverEventKey_LALT;
    case XK_Alt_R:
        return ParrotWindowDriverEventKey_RALT;
    case XK_space:
        return ParrotWindowDriverEventKey_SPACE;
    case XK_apostrophe: /* XK_quoteright has the same value (0x0027) */
        return ParrotWindowDriverEventKey_APOSTROPHE;
    case XK_comma:
        return ParrotWindowDriverEventKey_COMMA;
    case XK_minus:
        return ParrotWindowDriverEventKey_MINUS;
    case XK_period:
        return ParrotWindowDriverEventKey_PERIOD;
    case XK_slash:
        return ParrotWindowDriverEventKey_SLASH;
    case XK_0:
        return ParrotWindowDriverEventKey_0;
    case XK_1:
        return ParrotWindowDriverEventKey_1;
    case XK_2:
        return ParrotWindowDriverEventKey_2;
    case XK_3:
        return ParrotWindowDriverEventKey_3;
    case XK_4:
        return ParrotWindowDriverEventKey_4;
    case XK_5:
        return ParrotWindowDriverEventKey_5;
    case XK_6:
        return ParrotWindowDriverEventKey_6;
    case XK_7:
        return ParrotWindowDriverEventKey_7;
    case XK_8:
        return ParrotWindowDriverEventKey_8;
    case XK_9:
        return ParrotWindowDriverEventKey_9;
    case XK_semicolon:
        return ParrotWindowDriverEventKey_SEMICOLON;
    case XK_equal:
        return ParrotWindowDriverEventKey_EQUALS;
    case XK_A:
    case XK_a:
        return ParrotWindowDriverEventKey_A;
    case XK_B:
    case XK_b:
        return ParrotWindowDriverEventKey_B;
    case XK_C:
    case XK_c:
        return ParrotWindowDriverEventKey_C;
    case XK_D:
    case XK_d:
        return ParrotWindowDriverEventKey_D;
    case XK_E:
    case XK_e:
        return ParrotWindowDriverEventKey_E;
    case XK_F:
    case XK_f:
        return ParrotWindowDriverEventKey_F;
    case XK_G:
    case XK_g:
        return ParrotWindowDriverEventKey_G;
    case XK_H:
    case XK_h:
        return ParrotWindowDriverEventKey_H;
    case XK_I:
    case XK_i:
        return ParrotWindowDriverEventKey_I;
    case XK_J:
    case XK_j:
        return ParrotWindowDriverEventKey_J;
    case XK_K:
    case XK_k:
        return ParrotWindowDriverEventKey_K;
    case XK_L:
    case XK_l:
        return ParrotWindowDriverEventKey_L;
    case XK_M:
    case XK_m:
        return ParrotWindowDriverEventKey_M;
    case XK_N:
    case XK_n:
        return ParrotWindowDriverEventKey_N;
    case XK_O:
    case XK_o:
        return ParrotWindowDriverEventKey_O;
    case XK_P:
    case XK_p:
        return ParrotWindowDriverEventKey_P;
    case XK_Q:
    case XK_q:
        return ParrotWindowDriverEventKey_Q;
    case XK_R:
    case XK_r:
        return ParrotWindowDriverEventKey_R;
    case XK_S:
    case XK_s:
        return ParrotWindowDriverEventKey_S;
    case XK_T:
    case XK_t:
        return ParrotWindowDriverEventKey_T;
    case XK_U:
    case XK_u:
        return ParrotWindowDriverEventKey_U;
    case XK_V:
    case XK_v:
        return ParrotWindowDriverEventKey_V;
    case XK_W:
    case XK_w:
        return ParrotWindowDriverEventKey_W;
    case XK_X:
    case XK_x:
        return ParrotWindowDriverEventKey_X;
    case XK_Y:
    case XK_y:
        return ParrotWindowDriverEventKey_Y;
    case XK_Z:
    case XK_z:
        return ParrotWindowDriverEventKey_Z;
    case XK_bracketleft:
        return ParrotWindowDriverEventKey_LEFTBRACKET;
    case XK_backslash:
        return ParrotWindowDriverEventKey_BACKSLASH;
    case XK_bracketright:
        return ParrotWindowDriverEventKey_RIGHTBRACKET;
    case XK_grave: /* XK_quoteleft has the same value (0x0060) */
        return ParrotWindowDriverEventKey_GRAVE;
    default:
        return ParrotWindowDriverEventKey_UNKNOWN;
    }
}

const ParrotWindowDriver Parrot_x11_window_driver = {
    .init = driver_init,
    .shutdown = driver_shutdown,

    .create_window = driver_create_window,
    .delete_window = driver_delete_window,

    .poll_events = driver_poll_events,

    .set_title = driver_set_title,

    .set_size = driver_set_size,
    .get_width = driver_get_width,
    .get_height = driver_get_height,

    .set_image = driver_set_image,
};

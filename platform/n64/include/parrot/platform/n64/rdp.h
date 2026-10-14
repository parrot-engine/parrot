#ifndef PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RDP_H_
#define PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RDP_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t ParrotRDPCommand;

#define ParrotRDPCommand_id(cmd) ((ParrotRDPCommand)(cmd) << 56)

#define ParrotRDPCommand_nop() ParrotRDPCommand_id(0x00)

#define ParrotRDPCommand_triangle(shade, texture, zbuffer, lmajor, level, tile, yl, ym, yh)                             \
    (ParrotRDPCommand_id(0x08 | (((shade) & 1) << 2) | (((texture) & 1) << 1) | (((zbuffer) & 1))) |                    \
     ((ParrotRDPCommand)((lmajor) & 0x1) << 55) | ((ParrotRDPCommand)((level) & 0x7) << 51) |                           \
     ((ParrotRDPCommand)((tile) & 0x7) << 48) | ((ParrotRDPCommand)((yl) & 0x3FFF) << 32) |                             \
     ((ParrotRDPCommand)((ym) & 0x3FFF) << 16) | ((ParrotRDPCommand)((yh) & 0x3FFF)))
#define ParrotRDPCommand_triangle_xl(xl_i, xl_f, dxldy_i, dxldy_f)                                                      \
    (((ParrotRDPCommand)((xl_i) & 0x0FFF) << 48) | ((ParrotRDPCommand)((xl_f) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dxldy_i) & 0x3FFF) << 16) | ((ParrotRDPCommand)((dxldy_f) & 0xFFFF)))
#define ParrotRDPCommand_triangle_xh(xh_i, xh_f, dxhdy_i, dxhdy_f)                                                      \
    (((ParrotRDPCommand)((xh_i) & 0x0FFF) << 48) | ((ParrotRDPCommand)((xh_f) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dxhdy_i) & 0x3FFF) << 16) | ((ParrotRDPCommand)((dxhdy_f) & 0xFFFF)))
#define ParrotRDPCommand_triangle_xm(xm_i, xm_f, dxmdy_i, dxmdy_f)                                                      \
    (((ParrotRDPCommand)((xm_i) & 0x0FFF) << 48) | ((ParrotRDPCommand)((xm_f) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dxmdy_i) & 0x3FFF) << 16) | ((ParrotRDPCommand)((dxmdy_f) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_rgba_i(r, g, b, a)                                                              \
    (((ParrotRDPCommand)((r) & 0x1FF) << 48) | ((ParrotRDPCommand)((g) & 0x1FF) << 32) |                                \
     ((ParrotRDPCommand)((b) & 0x1FF) << 16) | ((ParrotRDPCommand)((a) & 0x1FF)))
#define ParrotRDPCommand_triangle_shade_drgbadx_i(drdx, dgdx, dbdx, dadx)                                               \
    (((ParrotRDPCommand)((drdx) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgdx) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbdx) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dadx) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_rgba_f(r, g, b, a)                                                              \
    (((ParrotRDPCommand)((r) & 0xFFFF) << 48) | ((ParrotRDPCommand)((g) & 0xFFFF) << 32) |                              \
     ((ParrotRDPCommand)((b) & 0xFFFF) << 16) | ((ParrotRDPCommand)((a) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_drgbadx_f(drdx, dgdx, dbdx, dadx)                                               \
    (((ParrotRDPCommand)((drdx) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgdx) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbdx) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dadx) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_drgbade_i(drde, dgde, dbde, dade)                                               \
    (((ParrotRDPCommand)((drde) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgde) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbde) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dade) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_drgbady_i(drdy, dgdy, dbdy, dady)                                               \
    (((ParrotRDPCommand)((drdy) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgdy) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbdy) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dady) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_drgbade_f(drde, dgde, dbde, dade)                                               \
    (((ParrotRDPCommand)((drde) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgde) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbde) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dade) & 0xFFFF)))
#define ParrotRDPCommand_triangle_shade_drgbady_f(drdy, dgdy, dbdy, dady)                                               \
    (((ParrotRDPCommand)((drdy) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dgdy) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dbdy) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dady) & 0xFFFF)))

#define ParrotRDPCommand_triangle_tex_stw_i(s, t, w)                                                                    \
    (((ParrotRDPCommand)((s) & 0xFFFF) << 48) | ((ParrotRDPCommand)((t) & 0xFFFF) << 32) |                              \
     ((ParrotRDPCommand)((w) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_dx_i(dsdx, dtdx, dwdx)                                                       \
    (((ParrotRDPCommand)((dsdx) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtdx) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwdx) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_stw_f(s, t, w)                                                                    \
    (((ParrotRDPCommand)((s) & 0xFFFF) << 48) | ((ParrotRDPCommand)((t) & 0xFFFF) << 32) |                              \
     ((ParrotRDPCommand)((w) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_dx_f(dsdx, dtdx, dwdx)                                                       \
    (((ParrotRDPCommand)((dsdx) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtdx) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwdx) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_de_i(dsde, dtde, dwde)                                                       \
    (((ParrotRDPCommand)((dsde) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtde) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwde) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_dy_i(dsdy, dtdy, dwdy)                                                       \
    (((ParrotRDPCommand)((dsdy) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtdy) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwdy) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_de_f(dsde, dtde, dwde)                                                       \
    (((ParrotRDPCommand)((dsde) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtde) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwde) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_tex_dstw_dy_f(dsdy, dtdy, dwdy)                                                       \
    (((ParrotRDPCommand)((dsdy) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dtdy) & 0xFFFF) << 32) |                        \
     ((ParrotRDPCommand)((dwdy) & 0xFFFF) << 16))
#define ParrotRDPCommand_triangle_zbuf_z(z_i, z_f, dzdx_i, dzdx_f)                                                      \
    (((ParrotRDPCommand)((z_i) & 0xFFFF) << 48) | ((ParrotRDPCommand)((z_f) & 0xFFFF) << 32) |                          \
     ((ParrotRDPCommand)((dzdx_i) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dzdx_f) & 0xFFFF)))
#define ParrotRDPCommand_triangle_zbuf_dz(dzde_i, dzde_f, dzdy_i, dzdy_f)                                               \
    (((ParrotRDPCommand)((dzde_i) & 0xFFFF) << 48) | ((ParrotRDPCommand)((dzde_f) & 0xFFFF) << 32) |                    \
     ((ParrotRDPCommand)((dzdy_i) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dzdy_f) & 0xFFFF)))

#define ParrotRDPCommand_texture_rect(lrx, lry, tile, ulx, uly)                                                         \
    (ParrotRDPCommand_id(0x24) | ((ParrotRDPCommand)((lrx) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((lry) & 0xFFF) << 32) | ((ParrotRDPCommand)((tile) & 0x7) << 24) |                             \
     ((ParrotRDPCommand)((ulx) & 0xFFF) << 12) | ((ParrotRDPCommand)((uly) & 0xFFF)))
#define ParrotRDPCommand_texture_rect_flip(lrx, lry, tile, ulx, uly)                                                    \
    (ParrotRDPCommand_id(0x25) | ((ParrotRDPCommand)((lrx) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((lry) & 0xFFF) << 32) | ((ParrotRDPCommand)((tile) & 0x7) << 24) |                             \
     ((ParrotRDPCommand)((ulx) & 0xFFF) << 12) | ((ParrotRDPCommand)((uly) & 0xFFF)))
#define ParrotRDPCommand_texture_rect_st(s, t, dsdx, dtdy)                                                              \
    (((ParrotRDPCommand)((s) & 0xFFFF) << 48) | ((ParrotRDPCommand)((t) & 0xFFFF) << 32) |                              \
     ((ParrotRDPCommand)((dsdx) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dtdy) & 0xFFFF)))

#define ParrotRDPCommand_sync_load() ParrotRDPCommand_id(0x26)
#define ParrotRDPCommand_pipe_sync() ParrotRDPCommand_id(0x27)
#define ParrotRDPCommand_pipe_tile_sync() ParrotRDPCommand_id(0x28)
#define ParrotRDPCommand_sync_full() ParrotRDPCommand_id(0x29)

#define ParrotRDPCommand_set_key_gb(width_g, center_g, scale_g, width_b, center_b, scale_b)                             \
    (ParrotRDPCommand_id(0x2A) | ((ParrotRDPCommand)((width_g) & 0xFFF) << 44) |                                        \
     ((ParrotRDPCommand)((width_b) & 0xFFF) << 32) | ((ParrotRDPCommand)((center_g) & 0xFF) << 24) |                    \
     ((ParrotRDPCommand)((scale_g) & 0xFF) << 16) | ((ParrotRDPCommand)((center_b) & 0xFF) << 8) |                      \
     ((ParrotRDPCommand)((scale_b) & 0xFF)))
#define ParrotRDPCommand_set_key_r(width_r, center_r, scale_r)                                                          \
    (ParrotRDPCommand_id(0x2B) | ((ParrotRDPCommand)((width_r) & 0xFFF) << 16) |                                        \
     ((ParrotRDPCommand)((center_r) & 0xFF) << 8) | ((ParrotRDPCommand)((scale_r) & 0xFF)))
#define ParrotRDPCommand_set_convert(k0, k1, k2, k3, k4, k5)                                                            \
    (ParrotRDPCommand_id(0x2C) | ((ParrotRDPCommand)((k0) & 0x1FF) << 45) | ((ParrotRDPCommand)((k1) & 0x1FF) << 36) |  \
     ((ParrotRDPCommand)((k2) & 0x1FF) << 27) | ((ParrotRDPCommand)((k3) & 0x1FF) << 18) |                              \
     ((ParrotRDPCommand)((k4) & 0x1FF) << 9) | ((ParrotRDPCommand)((k5) & 0x1FF)))

#define ParrotRDPCommand_set_scissor(ulx, uly, field, odd, lrx, lry)                                                    \
    (ParrotRDPCommand_id(0x2D) | ((ParrotRDPCommand)((ulx) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((uly) & 0xFFF) << 32) | ((ParrotRDPCommand)((field) & 0x1) << 25) |                            \
     ((ParrotRDPCommand)((odd) & 0x1) << 24) | ((ParrotRDPCommand)((lrx) & 0xFFF) << 12) |                              \
     ((ParrotRDPCommand)((lry) & 0xFFF)))

#define ParrotRDPCommand_set_prim_depth(z, dz)                                                                          \
    (ParrotRDPCommand_id(0x2E) | ((ParrotRDPCommand)((z) & 0xFFFF) << 16) | ((ParrotRDPCommand)((dz) & 0xFFFF)))

#define ParrotRDPCommand_set_other_modes(flags) (ParrotRDPCommand_id(0x2F) | ((ParrotRDPCommand)(flags)))

#define ParrotRDPCommand_CYCLE_TYPE_1CYCLE 0
#define ParrotRDPCommand_CYCLE_TYPE_2CYCLE 1
#define ParrotRDPCommand_CYCLE_TYPE_COPY 2
#define ParrotRDPCommand_CYCLE_TYPE_FILL 3

#define ParrotRDPCommand_RGB_DITHER_MAGIC_SQUARE 0
#define ParrotRDPCommand_RGB_DITHER_BAYER 1
#define ParrotRDPCommand_RGB_DITHER_NOISE 2
#define ParrotRDPCommand_RGB_DITHER_DISABLED 3

#define ParrotRDPCommand_ALPHA_DITHER_SAME 0
#define ParrotRDPCommand_ALPHA_DITHER_INVERSE 1
#define ParrotRDPCommand_ALPHA_DITHER_NOISE 2
#define ParrotRDPCommand_ALPHA_DITHER_DISABLED 3

#define ParrotRDPCommand_Z_MODE_OPAQUE 0
#define ParrotRDPCommand_Z_MODE_INTERPENETRATE 1
#define ParrotRDPCommand_Z_MODE_TRANSPARENT 2
#define ParrotRDPCommand_Z_MODE_DECAL 3

#define ParrotRDPCommand_CVG_DEST_CLAMP 0
#define ParrotRDPCommand_CVG_DEST_WRAP 1
#define ParrotRDPCommand_CVG_DEST_FULL 2
#define ParrotRDPCommand_CVG_DEST_SAVE 3

#define ParrotRDPCommand_BL_PM_CC_COLOR 0
#define ParrotRDPCommand_BL_PM_MEM_COLOR 1
#define ParrotRDPCommand_BL_PM_BLEND_COLOR 2
#define ParrotRDPCommand_BL_PM_FOG_COLOR 3

#define ParrotRDPCommand_BL_A_CC_ALPHA 0
#define ParrotRDPCommand_BL_A_FOG_ALPHA 1
#define ParrotRDPCommand_BL_A_SHADE_ALPHA 2
#define ParrotRDPCommand_BL_A_ZERO 3

#define ParrotRDPCommand_BL_B_ONE_MINUS_A 0
#define ParrotRDPCommand_BL_B_MEM_COVERAGE 1
#define ParrotRDPCommand_BL_B_ONE 2
#define ParrotRDPCommand_BL_B_ZERO 3

#define ParrotRDPCommand_OM_ATOMIC_PRIM ((ParrotRDPCommand)1 << 55)
#define ParrotRDPCommand_OM_CYCLE_TYPE(x) ((ParrotRDPCommand)((x) & 0x3) << 52)
#define ParrotRDPCommand_OM_PERSP_TEX_EN ((ParrotRDPCommand)1 << 51)
#define ParrotRDPCommand_OM_DETAIL_TEX_EN ((ParrotRDPCommand)1 << 50)
#define ParrotRDPCommand_OM_SHARPEN_TEX_EN ((ParrotRDPCommand)1 << 49)
#define ParrotRDPCommand_OM_TEX_LOD_EN ((ParrotRDPCommand)1 << 48)
#define ParrotRDPCommand_OM_TLUT_EN ((ParrotRDPCommand)1 << 47)
#define ParrotRDPCommand_OM_TLUT_TYPE_IA16 ((ParrotRDPCommand)1 << 46)
#define ParrotRDPCommand_OM_SAMPLE_BILINEAR ((ParrotRDPCommand)1 << 45)
#define ParrotRDPCommand_OM_MID_TEXEL ((ParrotRDPCommand)1 << 44)
#define ParrotRDPCommand_OM_BI_LERP_0 ((ParrotRDPCommand)1 << 43)
#define ParrotRDPCommand_OM_BI_LERP_1 ((ParrotRDPCommand)1 << 42)
#define ParrotRDPCommand_OM_CONVERT_ONE ((ParrotRDPCommand)1 << 41)
#define ParrotRDPCommand_OM_KEY_EN ((ParrotRDPCommand)1 << 40)
#define ParrotRDPCommand_OM_RGB_DITHER(x) ((ParrotRDPCommand)((x) & 0x3) << 38)
#define ParrotRDPCommand_OM_ALPHA_DITHER(x) ((ParrotRDPCommand)((x) & 0x3) << 36)
#define ParrotRDPCommand_OM_BL_M1A_0(x) ((ParrotRDPCommand)((x) & 0x3) << 30)
#define ParrotRDPCommand_OM_BL_M1A_1(x) ((ParrotRDPCommand)((x) & 0x3) << 28)
#define ParrotRDPCommand_OM_BL_M1B_0(x) ((ParrotRDPCommand)((x) & 0x3) << 26)
#define ParrotRDPCommand_OM_BL_M1B_1(x) ((ParrotRDPCommand)((x) & 0x3) << 24)
#define ParrotRDPCommand_OM_BL_M2A_0(x) ((ParrotRDPCommand)((x) & 0x3) << 22)
#define ParrotRDPCommand_OM_BL_M2A_1(x) ((ParrotRDPCommand)((x) & 0x3) << 20)
#define ParrotRDPCommand_OM_BL_M2B_0(x) ((ParrotRDPCommand)((x) & 0x3) << 18)
#define ParrotRDPCommand_OM_BL_M2B_1(x) ((ParrotRDPCommand)((x) & 0x3) << 16)
#define ParrotRDPCommand_OM_FORCE_BLEND ((ParrotRDPCommand)1 << 14)
#define ParrotRDPCommand_OM_ALPHA_CVG_SELECT ((ParrotRDPCommand)1 << 13)
#define ParrotRDPCommand_OM_CVG_X_ALPHA ((ParrotRDPCommand)1 << 12)
#define ParrotRDPCommand_OM_Z_MODE(x) ((ParrotRDPCommand)((x) & 0x3) << 10)
#define ParrotRDPCommand_OM_CVG_DEST(x) ((ParrotRDPCommand)((x) & 0x3) << 8)
#define ParrotRDPCommand_OM_COLOR_ON_CVG ((ParrotRDPCommand)1 << 7)
#define ParrotRDPCommand_OM_IMAGE_READ_EN ((ParrotRDPCommand)1 << 6)
#define ParrotRDPCommand_OM_Z_UPDATE_EN ((ParrotRDPCommand)1 << 5)
#define ParrotRDPCommand_OM_Z_COMPARE_EN ((ParrotRDPCommand)1 << 4)
#define ParrotRDPCommand_OM_ANTIALIAS_EN ((ParrotRDPCommand)1 << 3)
#define ParrotRDPCommand_OM_Z_SOURCE_PRIMITIVE ((ParrotRDPCommand)1 << 2) /* 0 = per-pixel */
#define ParrotRDPCommand_OM_DITHER_ALPHA_EN ((ParrotRDPCommand)1 << 1)
#define ParrotRDPCommand_OM_ALPHA_COMPARE_EN ((ParrotRDPCommand)1 << 0)

#define ParrotRDPCommand_load_tlut(uls, ult, tile, lrs, lrt)                                                            \
    (ParrotRDPCommand_id(0x30) | ((ParrotRDPCommand)((uls) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((ult) & 0xFFF) << 32) | ((ParrotRDPCommand)((tile) & 0x7) << 24) |                             \
     ((ParrotRDPCommand)((lrs) & 0xFFF) << 12) | ((ParrotRDPCommand)((lrt) & 0xFFF)))
#define ParrotRDPCommand_set_tile_size(uls, ult, index, lrs, lrt)                                                       \
    (ParrotRDPCommand_id(0x32) | ((ParrotRDPCommand)((uls) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((ult) & 0xFFF) << 32) | ((ParrotRDPCommand)((index) & 0x7) << 24) |                            \
     ((ParrotRDPCommand)((lrs) & 0xFFF) << 12) | ((ParrotRDPCommand)((lrt) & 0xFFF)))
#define ParrotRDPCommand_load_block(uls, ult, tile, lrs, dxt)                                                           \
    (ParrotRDPCommand_id(0x33) | ((ParrotRDPCommand)((uls) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((ult) & 0xFFF) << 32) | ((ParrotRDPCommand)((tile) & 0x7) << 24) |                             \
     ((ParrotRDPCommand)((lrs) & 0xFFF) << 12) | ((ParrotRDPCommand)((dxt) & 0xFFF)))
#define ParrotRDPCommand_load_tile(uls, ult, tile, lrs, lrt)                                                            \
    (ParrotRDPCommand_id(0x34) | ((ParrotRDPCommand)((uls) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((ult) & 0xFFF) << 32) | ((ParrotRDPCommand)((tile) & 0x7) << 24) |                             \
     ((ParrotRDPCommand)((lrs) & 0xFFF) << 12) | ((ParrotRDPCommand)((lrt) & 0xFFF)))

#define ParrotRDPCommand_set_tile(format,                                                                               \
                                  size,                                                                                 \
                                  line,                                                                                 \
                                  address,                                                                              \
                                  index,                                                                                \
                                  palette,                                                                              \
                                  clamp_t,                                                                              \
                                  mirror_t,                                                                             \
                                  mask_t,                                                                               \
                                  shift_t,                                                                              \
                                  clamp_s,                                                                              \
                                  mirror_s,                                                                             \
                                  mask_s,                                                                               \
                                  shift_s)                                                                              \
    (ParrotRDPCommand_id(0x35) | ((ParrotRDPCommand)((format) & 0x7) << 53) |                                           \
     ((ParrotRDPCommand)((size) & 0x3) << 51) | ((ParrotRDPCommand)((line) & 0x1FF) << 41) |                            \
     ((ParrotRDPCommand)((address) & 0x1FF) << 32) | ((ParrotRDPCommand)((index) & 0x7) << 24) |                        \
     ((ParrotRDPCommand)((palette) & 0xF) << 20) | ((ParrotRDPCommand)((clamp_t) & 0x1) << 19) |                        \
     ((ParrotRDPCommand)((mirror_t) & 0x1) << 18) | ((ParrotRDPCommand)((mask_t) & 0xF) << 14) |                        \
     ((ParrotRDPCommand)((shift_t) & 0xF) << 10) | ((ParrotRDPCommand)((clamp_s) & 0x1) << 9) |                         \
     ((ParrotRDPCommand)((mirror_s) & 0x1) << 8) | ((ParrotRDPCommand)((mask_s) & 0xF) << 4) |                          \
     ((ParrotRDPCommand)((shift_s) & 0xF)))

#define ParrotRDPCommand_FORMAT_RGBA 0
#define ParrotRDPCommand_FORMAT_YUV 1
#define ParrotRDPCommand_FORMAT_CI 2
#define ParrotRDPCommand_FORMAT_IA 3
#define ParrotRDPCommand_FORMAT_I 4

#define ParrotRDPCommand_SIZE_4BPP 0
#define ParrotRDPCommand_SIZE_8BPP 1
#define ParrotRDPCommand_SIZE_16BPP 2
#define ParrotRDPCommand_SIZE_32BPP 3

#define ParrotRDPCommand_fill_rect(lrx, lry, ulx, uly)                                                                  \
    (ParrotRDPCommand_id(0x36) | ((ParrotRDPCommand)((lrx) & 0xFFF) << 44) |                                            \
     ((ParrotRDPCommand)((lry) & 0xFFF) << 32) | ((ParrotRDPCommand)((ulx) & 0xFFF) << 12) |                            \
     ((ParrotRDPCommand)((uly) & 0xFFF)))
#define ParrotRDPCommand_set_fill_color(color) (ParrotRDPCommand_id(0x37) | ((ParrotRDPCommand)((color) & 0xFFFFFFFF)))
#define ParrotRDPCommand_set_fog_color(r, g, b, a)                                                                      \
    (ParrotRDPCommand_id(0x38) | ((ParrotRDPCommand)((r) & 0xFF) << 24) | ((ParrotRDPCommand)((g) & 0xFF) << 16) |      \
     ((ParrotRDPCommand)((b) & 0xFF) << 8) | ((ParrotRDPCommand)((a) & 0xFF)))
#define ParrotRDPCommand_set_blend_color(r, g, b, a)                                                                    \
    (ParrotRDPCommand_id(0x39) | ((ParrotRDPCommand)((r) & 0xFF) << 24) | ((ParrotRDPCommand)((g) & 0xFF) << 16) |      \
     ((ParrotRDPCommand)((b) & 0xFF) << 8) | ((ParrotRDPCommand)((a) & 0xFF)))
#define ParrotRDPCommand_set_prim_color(min_level, prim_lod_frac, r, g, b, a)                                           \
    (ParrotRDPCommand_id(0x3A) | ((ParrotRDPCommand)((min_level) & 0xFF) << 40) |                                       \
     ((ParrotRDPCommand)((prim_lod_frac) & 0xFF) << 32) | ((ParrotRDPCommand)((r) & 0xFF) << 24) |                      \
     ((ParrotRDPCommand)((g) & 0xFF) << 16) | ((ParrotRDPCommand)((b) & 0xFF) << 8) | ((ParrotRDPCommand)((a) & 0xFF)))
#define ParrotRDPCommand_set_env_color(r, g, b, a)                                                                      \
    (ParrotRDPCommand_id(0x3B) | ((ParrotRDPCommand)((r) & 0xFF) << 24) | ((ParrotRDPCommand)((g) & 0xFF) << 16) |      \
     ((ParrotRDPCommand)((b) & 0xFF) << 8) | ((ParrotRDPCommand)((a) & 0xFF)))
#define ParrotRDPCommand_set_combine_mode(rgb_a_0,                                                                      \
                                          rgb_b_0,                                                                      \
                                          rgb_c_0,                                                                      \
                                          rgb_d_0,                                                                      \
                                          a_a_0,                                                                        \
                                          a_b_0,                                                                        \
                                          a_c_0,                                                                        \
                                          a_d_0,                                                                        \
                                          rgb_a_1,                                                                      \
                                          rgb_b_1,                                                                      \
                                          rgb_c_1,                                                                      \
                                          rgb_d_1,                                                                      \
                                          a_a_1,                                                                        \
                                          a_b_1,                                                                        \
                                          a_c_1,                                                                        \
                                          a_d_1)                                                                        \
    (ParrotRDPCommand_id(0x3C) | ((ParrotRDPCommand)((rgb_a_0) & 0xF) << 52) |                                          \
     ((ParrotRDPCommand)((rgb_c_0) & 0x1F) << 47) | ((ParrotRDPCommand)((a_a_0) & 0x7) << 44) |                         \
     ((ParrotRDPCommand)((a_c_0) & 0x7) << 41) | ((ParrotRDPCommand)((rgb_a_1) & 0xF) << 37) |                          \
     ((ParrotRDPCommand)((rgb_c_1) & 0x1F) << 32) | ((ParrotRDPCommand)((rgb_b_0) & 0xF) << 28) |                       \
     ((ParrotRDPCommand)((rgb_b_1) & 0xF) << 24) | ((ParrotRDPCommand)((a_a_1) & 0x7) << 21) |                          \
     ((ParrotRDPCommand)((a_c_1) & 0x7) << 18) | ((ParrotRDPCommand)((rgb_d_0) & 0x7) << 15) |                          \
     ((ParrotRDPCommand)((a_b_0) & 0x7) << 12) | ((ParrotRDPCommand)((a_d_0) & 0x7) << 9) |                             \
     ((ParrotRDPCommand)((rgb_d_1) & 0x7) << 6) | ((ParrotRDPCommand)((a_b_1) & 0x7) << 3) |                            \
     ((ParrotRDPCommand)((a_d_1) & 0x7)))

#define ParrotRDPCommand_CC_RGB_COMBINED 0
#define ParrotRDPCommand_CC_RGB_TEX0 1
#define ParrotRDPCommand_CC_RGB_TEX1 2
#define ParrotRDPCommand_CC_RGB_PRIMITIVE 3
#define ParrotRDPCommand_CC_RGB_SHADE 4
#define ParrotRDPCommand_CC_RGB_ENVIRONMENT 5
#define ParrotRDPCommand_CC_RGBA_ONE 6
#define ParrotRDPCommand_CC_RGBA_NOISE 7
#define ParrotRDPCommand_CC_RGBA_ZERO 8
#define ParrotRDPCommand_CC_RGBD_ONE 6
#define ParrotRDPCommand_CC_RGBD_ZERO 7

#define ParrotRDPCommand_CC_RGBB_CENTER 6
#define ParrotRDPCommand_CC_RGBB_K4 7
#define ParrotRDPCommand_CC_RGBB_ZERO 8

#define ParrotRDPCommand_CC_RGBC_COMBINED 0
#define ParrotRDPCommand_CC_RGBC_TEX0 1
#define ParrotRDPCommand_CC_RGBC_TEX1 2
#define ParrotRDPCommand_CC_RGBC_PRIMITIVE 3
#define ParrotRDPCommand_CC_RGBC_SHADE 4
#define ParrotRDPCommand_CC_RGBC_ENVIRONMENT 5
#define ParrotRDPCommand_CC_RGBC_SCALE 6
#define ParrotRDPCommand_CC_RGBC_COMBINED_ALPHA 7
#define ParrotRDPCommand_CC_RGBC_TEX0_ALPHA 8
#define ParrotRDPCommand_CC_RGBC_TEX1_ALPHA 9
#define ParrotRDPCommand_CC_RGBC_PRIM_ALPHA 10
#define ParrotRDPCommand_CC_RGBC_SHADE_ALPHA 11
#define ParrotRDPCommand_CC_RGBC_ENV_ALPHA 12
#define ParrotRDPCommand_CC_RGBC_LOD_FRACTION 13
#define ParrotRDPCommand_CC_RGBC_PRIM_LOD_FRAC 14
#define ParrotRDPCommand_CC_RGBC_K5 15
#define ParrotRDPCommand_CC_RGBC_ZERO 16

#define ParrotRDPCommand_CC_A_COMBINED 0
#define ParrotRDPCommand_CC_A_TEX0 1
#define ParrotRDPCommand_CC_A_TEX1 2
#define ParrotRDPCommand_CC_A_PRIMITIVE 3
#define ParrotRDPCommand_CC_A_SHADE 4
#define ParrotRDPCommand_CC_A_ENVIRONMENT 5
#define ParrotRDPCommand_CC_A_ONE 6
#define ParrotRDPCommand_CC_A_ZERO 7

#define ParrotRDPCommand_CC_AC_LOD_FRACTION 0
#define ParrotRDPCommand_CC_AC_TEX0 1
#define ParrotRDPCommand_CC_AC_TEX1 2
#define ParrotRDPCommand_CC_AC_PRIMITIVE 3
#define ParrotRDPCommand_CC_AC_SHADE 4
#define ParrotRDPCommand_CC_AC_ENVIRONMENT 5
#define ParrotRDPCommand_CC_AC_PRIM_LOD_FRAC 6
#define ParrotRDPCommand_CC_AC_ZERO 7

#define ParrotRDPCommand_set_texture_image(format, size, width, addr)                                                   \
    (ParrotRDPCommand_id(0x3D) | ((ParrotRDPCommand)((format) & 0x7) << 53) |                                           \
     ((ParrotRDPCommand)((size) & 0x3) << 51) | ((ParrotRDPCommand)((width) & 0x3FF) << 32) |                           \
     ((ParrotRDPCommand)((addr) & 0xFFFFFF)))
#define ParrotRDPCommand_set_depth_image(addr) (ParrotRDPCommand_id(0x3E) | ((ParrotRDPCommand)((addr) & 0xFFFFFF)))
#define ParrotRDPCommand_set_color_image(format, size, width, addr)                                                     \
    (ParrotRDPCommand_id(0x3F) | ((ParrotRDPCommand)((format) & 0x7) << 53) |                                           \
     ((ParrotRDPCommand)((size) & 0x3) << 51) | ((ParrotRDPCommand)((width) & 0x3FF) << 32) |                           \
     ((ParrotRDPCommand)((addr) & 0xFFFFFF)))

PARROT_API bool ParrotRDP_send_dl(const ParrotRDPCommand *dl, size_t count);

#endif // PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RDP_H_

#ifndef PARROT_PARROT_SRC_CORE_BUFFER_H_
#define PARROT_PARROT_SRC_CORE_BUFFER_H_

#include "parrot/core/buffer.h"

#define ParrotBuffer_new_stbds_array(p_arr_data) ParrotBuffer_new_stbds_array_raw(&(p_arr_data));
ParrotBuffer *ParrotBuffer_new_stbds_array_raw(uint8_t **p_arr_data);

#endif // PARROT_PARROT_SRC_CORE_BUFFER_H_

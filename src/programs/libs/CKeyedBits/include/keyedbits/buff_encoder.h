#ifndef __KEYEDBITS_BUFF_ENCODER_H__
#define __KEYEDBITS_BUFF_ENCODER_H__

#include <keyedbits/types.h>

/**
 * Initialize a KeyedBits encode buffer stream.
 */
void kb_buff_initialize_encode(kb_buff_t * kb, void * buff, uint64_t len);

/**
 * Write a number to the buffer. If the number exceeds the 32-bit limits, it
 * will automatically be encoded as 64-bits.
 * @return false on buffer overflow.
 */
bool kb_buff_write_int(kb_buff_t * kb, int64_t number);

/**
 * Write a UTF8 NULL-terminated string.
 * @return false on buffer overflow.
 */
bool kb_buff_write_string(kb_buff_t * kb, const char * str);

/**
 * Write a double which conforms to the KeyedBits version 1 specification.
 * This is what you should use if you are encoding a double for decoders which
 * may not support KB v2.
 * @return false on buffer overflow.
 */
bool kb_buff_write_double_v1(kb_buff_t * kb, double d);

/**
 * Writes a double conforming to the KeyedBits version 2 specification. This
 * means that the output may be in scientific notation if it reduces the
 * length of the string.
 * @return false on buffer overflow.
 */
bool kb_buff_write_double_v2(kb_buff_t * kb, double d);

/**
 * Writes a piece of binary data to the stream. This function automatically
 * figures out which length length field to use.
 * @return false on buffer overflow.
 */
bool kb_buff_write_data(kb_buff_t * kb, const void * buff, uint32_t len);

/**
 * Writes an ASCII dictionary key to the buffer.
 * @return false on buffer overflow; false if the key is not valid ASCII.
 */
bool kb_buff_write_key(kb_buff_t * kb, const char * key);

/**
 * Write a single-byte `null` object.
 * @return false on buffer overflow.
 */
bool kb_buffer_write_null(kb_buff_t * kb);

/**
 * Write the single-byte array header to the buffer.
 * @return false on buffer overflow.
 */
bool kb_buff_write_array(kb_buff_t * kb);

/**
 * Write the single-byte dictionary header to the buffer.
 * @return false on buffer overflow.
 */
bool kb_buff_write_dict(kb_buff_t * kb);

/**
 * Write a single 0 byte to the buffer.
 * @return false on buffer overflow.
 */
bool kb_buff_write_terminator(kb_buff_t * kb);

#endif
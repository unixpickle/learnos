# Abstract

This is going to be an easy-to-use C-based [KeyedBits](https://github.com/unixpickle/KeyedBits) transcoder. It will provide encoding and decoding from event-driven streams. I plan to use this for messaging in my operating system.

# Packet (a.k.a. "buff") encoders

The two headers, `<keyedbits/buff_encoder.h>` and `<keyedbits/buff_decoder.h>` define an interface for reading and writing KeyedBits objects to and from fixed-size data buffers. This is useful when dynamic memory allocation is not available. The obvious drawback is that this API forces you to keep your objects below a certain maximum size in order for the encode/decode to succeed.

## Encoding objects

Let `buffer` be your pre-allocated buffer for KeyedBits data, and let `len` be the maximum number of bits which may be stored in it. Setup an encoder as follows:

    kb_buff_t kb;
    kb_buff_initialize_decode(&kb, buffer, len);

Now, encode basic datatypes with the following functions, all of which return `true` on success or `false` on buffer overflow:

 * `bool kb_buff_write_int(kb_buff_t * kb, int64_t number)`
 * `bool kb_buff_write_string(kb_buff_t * kb, const char * str)`
 * `bool kb_buff_write_double_v2(kb_buff_t * kb, double d)`
 * `bool kb_buff_write_data(kb_buff_t * kb, const void * buff, uint32_t len)`
 * `bool kb_buffer_write_null(kb_buff_t * kb)`

To begin an array, use `bool kb_buff_write_array(kb_buff_t * kb)`. Likewise, to begin a dictionary, use `bool kb_buff_write_dict(kb_buff_t * kb)`. To terminate an array or a dictionary, use `bool kb_buff_write_terminator(kb_buff_t * kb)`. To write the next key in a dictionary, use `bool kb_buff_write_key(kb_buff_t * kb, const char * key)`.

## Decoding objects

Let `buffer` be your input buffer of KeyedBits data, and let `len` be its length. Setup a decoder as follows:

    kb_buff_t kb;
    kb_buff_initialize_decode(&kb, buffer, len);

All of the decoding functions from here on out return a `bool` which will be `true` if the requested data was successfully decoded. First, to read an object header, do the following:

    kb_header_t head;
    bool success = kb_buff_read_header(&kb, &head);
    if (!success) { // fail... }
    bool valid = kb_validate_header(&head); // from validation.h
    if (!valid) { // fail... }

Next, you need to call one of the following functions depending on what kind of data you got. Here's a list:

 * `bool kb_buff_read_string(kb_buff_t * kb, const char ** out, uint64_t * len)`
 * `bool kb_buff_read_double(kb_buff_t * kb, double * out)`
 * `bool kb_buff_read_int(kb_buff_t * kb, uint8_t lenLen, int64_t * out)`
 * `bool kb_buff_read_data(kb_buff_t * kb, uint8_t lenLen, const void ** start, uint64_t * len)`
 * `bool kb_buff_read_key(kb_buff_t * kb, char * out, uint64_t max)`

If you read an array header, you should follow it with a loop in which you call `kb_buff_read_header` again and again, validate the header each time, read each value recursively, and stop when you see a terminator header.

If you read a dictionary header, you should do a similar process, except before each object you read, call `kb_buff_read_key` and check for an empty-string key to terminate the dictionary.

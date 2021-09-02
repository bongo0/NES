#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdlib.h>


typedef struct ring_buffer {

  uint8_t *buf;
  uint8_t *read;  // read head position
  uint8_t *write; // write tail position
  size_t free_bytes;
  size_t size; // in bytes
} ring_buffer;

ring_buffer * ring_buffer_new(size_t n_bytes);
void ring_buffer_reset(ring_buffer *buf);

size_t ring_buffer_free_bytes(ring_buffer *b);

uint8_t ring_buffer_memcpy_to(ring_buffer *dest, const void *src, size_t n_bytes );
size_t ring_buffer_memcpy_from(ring_buffer *src, void *dest, size_t n_bytes);
size_t ring_buffer_memcpy_n_from(ring_buffer *src, void *dest_, size_t n_bytes);

uint8_t ring_buffer_push(ring_buffer *b, uint8_t byte);
uint8_t ring_buffer_pop(ring_buffer *b, uint8_t *output);

void ring_buffer_free(ring_buffer *buf);


#endif // RING_BUFFER_H
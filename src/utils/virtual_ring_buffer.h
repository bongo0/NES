#ifndef VIRTUAL_RING_BUFFER_H
#define VIRTUAL_RING_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  size_t read;
  size_t write;
  size_t size;

  uint8_t *v_buf;
  size_t order;
  size_t flags;
} vr_buf;

vr_buf *vr_buf_new(size_t order);
void vr_buf_free(vr_buf *vb);

size_t vr_buf_memcpy_to(vr_buf *dest, const void *src, size_t n_bytes);
size_t vr_buf_memcpy_from(void *dest, vr_buf *src, size_t n_bytes);



#endif // VIRTUAL_RING_BUFFER_H
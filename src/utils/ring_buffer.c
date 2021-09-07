#include "ring_buffer.h"

#include <memory.h>

ring_buffer *ring_buffer_new(size_t n_bytes) {
  ring_buffer *buf = malloc(sizeof(ring_buffer));

  if (buf == NULL)
    return NULL;

  buf->size = n_bytes;
  buf->free_bytes = n_bytes;
  buf->buf = malloc(buf->size);

  if (!buf->buf) {
    free(buf);
    return NULL;
  }

  ring_buffer_reset(buf);

  return buf;
}

void ring_buffer_reset(ring_buffer *buf) {
  buf->read = buf->buf;
  buf->write = buf->buf;
  buf->free_bytes = buf->size;
}

size_t ring_buffer_free_bytes(ring_buffer *b) { return b->free_bytes; }

/*
 push one byte to the buffer,
 return 1 if overflow (overwrote data)
 return 0 otherwise
*/
uint8_t ring_buffer_push(ring_buffer *b, uint8_t byte) {
  *b->write = byte;
  if (b->free_bytes)
    b->free_bytes--;
  b->write++;

  // wrap aroud
  if (b->write >= b->buf + b->size) {
    b->write = b->buf;
  }

  // overflow
  if (b->read == b->write - 1 && b->free_bytes == 0) {
    b->read++;
    return 1;
  }
  return 0;
}

/*
 pop one byte from the buffer into *output,
 return 0 if buffer empty and no output is set
 return 1 on success
*/
uint8_t ring_buffer_pop(ring_buffer *b, uint8_t *output) {
  if (b->free_bytes == b->size)
    return 0; // buffer is empty
  // wrap around
  if (b->read >= b->buf + b->size) {
    b->read = b->buf;
  }

  *output = *b->read;
  b->free_bytes++;
  b->read++;

  return 1;
}

/*
 memcpy n_bytes from src to dest ring buffer,
 If n_bytes is bigger than dest buffer
 the last dest.size amount of bytes are copied to the buffer.
 return 1 if overflow (overwrote data)
 return 0 otherwise
*/
uint8_t ring_buffer_memcpy_to(ring_buffer *dest, const void *src_,
                              size_t n_bytes) {
  const uint8_t *src = src_;
  uint8_t overflow_flag = 0;

  if (dest->size < n_bytes) {
    src = src + n_bytes - dest->size;
    n_bytes = dest->size;
  }

  size_t n = (dest->buf + dest->size) - dest->write;
  // needs wrapping
  if (n < n_bytes) {
    // first copy
    memcpy(dest->write, src, n);
    // wrap around copy
    memcpy(dest->buf, src + n, n_bytes - n);

    if (dest->free_bytes < n_bytes)
      dest->free_bytes = 0;
    else
      dest->free_bytes -= n_bytes;

    // overflow handle read pointer
    uint8_t *new_write = dest->buf + (n_bytes - n);
    if ((dest->read >= dest->write) ||
        (dest->read < new_write && dest->free_bytes == 0)) {
      dest->read = new_write;
      overflow_flag = 1;
    }
    dest->write = new_write;

  } else {

    memcpy(dest->write, src, n_bytes);
    dest->write += n_bytes;

    if (dest->free_bytes < n_bytes)
      dest->free_bytes = 0;
    else
      dest->free_bytes -= n_bytes;

    // wrap around
    if (dest->write >= dest->buf + dest->size)
      dest->write = dest->buf;

    // overflow
    if (dest->read < dest->write && dest->free_bytes == 0) {
      dest->read = dest->write;
      overflow_flag = 1;
    }
  }
  return overflow_flag;
}

/*
 attempt to memcpy n_bytes from ring_buffer src to dest,

 returns how many bytes copied
*/
size_t ring_buffer_memcpy_from(ring_buffer *src, void *dest_, size_t n_bytes) {
  uint8_t *dest = dest_;
  size_t bytes_used = (src->size - src->free_bytes);
  uint8_t underflow_flag = n_bytes > bytes_used;
  if (underflow_flag)
    return 0;
  // read only as many bytes as is used
  size_t read_n = underflow_flag ? bytes_used : n_bytes;
  // needs wrapping
  size_t n = (src->buf + src->size) - src->read;
  if (n < read_n) {
    // first read
    memcpy(dest, src->read, n);
    // read rest
    memcpy(dest + n, src->buf, read_n - n);
    src->read = src->buf + read_n - n;
  } else {
    memcpy(dest, src->read, read_n);
    src->read += read_n;
    // wrap around
    if (src->read >= src->buf + src->size)
      src->read = src->buf;
  }
  src->free_bytes += read_n;
  return read_n;
}

/*
 attempt to memcpy n_bytes from ring_buffer src to dest,
 if no n_bytes available dont do anything
 returns how many bytes copied
*/
size_t ring_buffer_memcpy_n_from(ring_buffer *src, void *dest_,
                                 size_t n_bytes) {
  uint8_t *dest = dest_;
  if (n_bytes > (src->size - src->free_bytes))
    return 0;

  size_t written = 0;
  while (n_bytes != written) {
    size_t wn = (size_t)((src->buf + src->size) - src->read) < (size_t)(n_bytes - written)
                    ? (size_t)((src->buf + src->size) - src->read)
                    : (size_t)(n_bytes - written);
    memcpy(dest + written,src->read, wn);
    src->read+=wn;
    written+=wn;

    // wrap around
    if (src->read >= src->buf + src->size)
      src->read = src->buf;
  }
  /* size_t n = (src->buf + src->size) - src->read;
  // needs wrapping
  if (n < n_bytes) {
    // first read
    memcpy(dest, src->read, n);
    // read rest
    memcpy(dest + n, src->buf, n_bytes - n);
    src->read = src->buf + n_bytes - n;
  } else {
    memcpy(dest, src->read, n_bytes);
    src->read += n_bytes;
    // wrap around
    if (src->read >= src->buf + src->size)
      src->read = src->buf;
  } */
  src->free_bytes += n_bytes;
  return n_bytes;
}

void ring_buffer_free(ring_buffer *buf) {
  free(buf->buf);
  free(buf);
}
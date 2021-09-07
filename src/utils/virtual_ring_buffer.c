#include "virtual_ring_buffer.h"

#include <stdlib.h>
#include <stdio.h>

#define __USE_MISC
#include <sys/mman.h>
#include <sys/shm.h>

#include <unistd.h> // getpagesize
#include <memory.h>

#include "../logger.h"

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

vr_buf *vr_buf_new(size_t order) {
  vr_buf *vb = malloc(sizeof(vr_buf));
  if (vb == NULL)
    return NULL;

  size_t page_size;
#ifdef _SC_PAGE_SIZE
  page_size = sysconf(_SC_PAGE_SIZE);
#else
  page_size = getpagesize();
#endif

  vb->order = order;
  vb->size = (page_size) * (1 << order);
  vb->read = 0;
  vb->write = 0;

  //                     2 times the size
  vb->v_buf = mmap(NULL, vb->size *2, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    if(vb->v_buf == MAP_FAILED){
        free(vb);
        LOG_ERROR("vr_buf mmap fail\n");
        return NULL;
    }

    int shm_id = shmget(IPC_PRIVATE, vb->size, IPC_CREAT|0700);
    if(shm_id<0){
        free(vb);
        munmap(vb->v_buf, vb->size *2);
        LOG_ERROR("vr_buf shmget fail\n");
        return NULL;
    }

    munmap(vb->v_buf, vb->size*2);
    // map 1st half
    if(vb->v_buf != shmat(shm_id,vb->v_buf,0)){
        shmctl(shm_id,IPC_RMID,NULL);
        free(vb);
        LOG_ERROR("vr_buf 1st shmat fail\n");
        return NULL;
    }

    // map 2nd half
    if((vb->v_buf+vb->size)!= shmat(shm_id,vb->v_buf+vb->size,0)){
        shmdt(vb->v_buf);
        shmctl(shm_id,IPC_RMID,NULL);
        free(vb);
        LOG_ERROR("vr_buf 2st shmat fail\n");
        return NULL;
    }

    if(shmctl(shm_id,IPC_RMID,NULL)<0){
        shmdt(vb->v_buf+vb->size);
        shmdt(vb->v_buf);
        shmctl(shm_id,IPC_RMID,NULL);
        free(vb);
        LOG_ERROR("vr_buf shmctl fail\n");
        return NULL;
    }


    // we made it
    return vb;
}

void vr_buf_free(vr_buf *vb) {
    if(vb==NULL)return;
    shmdt(vb->v_buf);
    shmdt(vb->v_buf+vb->size);
    free(vb);
}

size_t vr_buf_memcpy_to(vr_buf *dest, const void *src, size_t n_bytes) {
    size_t n_to_write = min(  (dest->size-dest->write+dest->read-1)%dest->size, n_bytes );
    if(n_to_write==0)return 0;
    memcpy(dest->v_buf+dest->write, src, n_to_write);
    dest->write = (dest->write+n_to_write)%dest->size;
    return n_to_write;
}

size_t vr_buf_memcpy_from(void *dest, vr_buf *src, size_t n_bytes) {
    size_t n_to_read = min(  (src->size-src->read+src->write)%src->size, n_bytes );
    if(n_to_read==0)return 0;
    memcpy(dest,src->v_buf + src->read, n_to_read);
    src->read = (src->read + n_to_read)%src->size;
    return n_to_read;
}

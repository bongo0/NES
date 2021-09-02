#include <stdio.h>

#include "../src/ring_buffer.h"

void print_buf(ring_buffer *b){
    
    for(int i=0; i<b->size;++i){
        if(b->read-b->buf==i)printf("v  ");
        else printf("   ");
    }
    printf("      :read\n");
    for(int i=0; i<b->size;++i){
        printf("%02X ", b->buf[i]);
    }
    printf("free:%lu", b->free_bytes);
    printf("\n");
    for(int i=0; i<b->size;++i){
        if(b->write-b->buf==i)printf("^  ");
        else printf("   ");
    }
    printf("      :write\n");
}

void print_buf_ch(ring_buffer *b){
    printf("buffer: ");
    for(int i=0; i<b->size;++i){
        if(b->read-b->buf==i && b->write-b->buf==i)printf("\033[33m");
        else if(b->read-b->buf==i)printf("\033[31m");
        else if(b->write-b->buf==i)printf("\033[32m");
        else printf("\033[0m");
        printf("%c",b->buf[i]);
    }
    printf("\n");
}

int main(){
printf("\n\n\n rw:y r:r w:g\n");
  ring_buffer *b = ring_buffer_new(30);

  char *a = "_HELLO_BUFFER";
  char *r = malloc(14);r[13]=0;
uint8_t u;

  ring_buffer_memcpy_to(b,a,13);print_buf_ch(b);

  ring_buffer_memcpy_to(b,a,13);print_buf_ch(b);
  ring_buffer_memcpy_to(b,a,13);print_buf_ch(b);
  ring_buffer_memcpy_to(b,a,13);print_buf_ch(b);

  r[0]=0;u=ring_buffer_memcpy_from(b,r,13);print_buf_ch(b);printf("%d read: %s\n",u,r);
  r[0]=0;u=ring_buffer_memcpy_from(b,r,13);print_buf_ch(b);printf("%d read: %s\n",u,r);
  r[0]=0;u=ring_buffer_memcpy_from(b,r,13);print_buf_ch(b);printf("%d read: %s\n",u,r);
  r[0]=0;u=ring_buffer_memcpy_from(b,r,13);print_buf_ch(b);printf("%d read: %s\n",u,r);


/*   print_buf(b);
  ring_buffer_push(b,1);
  print_buf(b);
  ring_buffer_push(b,2);
  print_buf(b);
  ring_buffer_push(b,3);
  print_buf(b);
  ring_buffer_push(b,4);
  print_buf(b);
  ring_buffer_push(b,5);
  print_buf(b);
  ring_buffer_push(b,6);
  print_buf(b);
  ring_buffer_push(b,7);
  print_buf(b);
  ring_buffer_push(b,8);
  print_buf(b);
  ring_buffer_push(b,9);
  print_buf(b);
  ring_buffer_push(b,10);
  print_buf(b);
  ring_buffer_push(b,11);
  print_buf(b);
    ring_buffer_push(b,12);
  print_buf(b);
    ring_buffer_push(b,13);
  print_buf(b);
    ring_buffer_push(b,14);
  print_buf(b);
printf("##########################\n");
uint8_t out=-1;
uint8_t ret;
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
  ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);

ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);
ret=ring_buffer_pop(b,&out);print_buf(b);printf("ret:%d out: %X\n",ret,out);

printf("\n\n\n\n\n");

ring_buffer *b2 = ring_buffer_new(10);

uint8_t src[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

ring_buffer_memcpy_to(b2,src,1);
print_buf(b2);
ring_buffer_memcpy_to(b2,src,20);
print_buf(b2);

uint8_t *buf = malloc(100);

ring_buffer_memcpy_from(b2,buf,5);
print_buf(b2);
for(int i =0;i<10;i++)printf("%02X ",buf[i]);
printf("\n"); */

}



/*



src 25, dest 10,   src + 25 - 10
0 1 2 3 4 5 6 7 8 9   0 1 2 3 4 5 6 7 8 9 A B C D E F
                                  0 1 2 3 4 5 6 7 8 9




read :v
tail |0 1 2 3 4 5 6 7 8 9 F
     |0 0 0 0 0 0 0 0 0 0
write:              ^
head

write 4 bytes:

put data beginning at write and advance write+=4

read :v
     |0 1 2 3 4 5 6 7 8 9 F
     |1 1 1 1 1 0 0 0 0 0
write:        ^


full:
read :v
     |0 1 2 3 4 5 6 7 8 9 F
     |1 1 1 1 1 1 1 1 1 1
write:                    ^

overflow?


capacity=  size-(read-write)
read :          v
     |0 1 2 3 4 5 6 7 8 9 F
     |1 1 1 1 1 1 1 1 1 1
write:        ^

read :v
     |0 1 2 3 4 5 6 7 8 9 F
     |1 1 1 1 1 1 1 1 1 1
write:^
*/
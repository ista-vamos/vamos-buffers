#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include "../gen/shamon.h"
#include "../gen/mmlib.h"
typedef enum __MM_STREAMCONST_ENUM { __MM_STREAMCONST_ENUM_Left, __MM_STREAMCONST_ENUM_Right } _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM { __MM_EVENTCONST_NOTHING, __MM_EVENTCONST_ENUM_hole, __MM_EVENTCONST_ENUM_Prime, __MM_EVENTCONST_ENUM_LPrime, __MM_EVENTCONST_ENUM_RPrime, __MM_EVENTCONST_ENUM_LSkip, __MM_EVENTCONST_ENUM_RSkip, __MM_EVENTCONST_ENUM_EFinal } _MM_EVENTCONST_ENUM;
typedef struct source_control _mm_source_control;
void _mm_print_streams( );
typedef struct __MMEV_Prime _MMEV_Prime;
typedef struct __MMEV_LPrime _MMEV_LPrime;
typedef struct __MMEV_RPrime _MMEV_RPrime;
typedef struct __MMEV_LSkip _MMEV_LSkip;
typedef struct __MMEV_RSkip _MMEV_RSkip;
typedef struct __MMEV_EFinal _MMEV_EFinal;
size_t __mma_strm_ilen_Left = 0 ;
size_t __mma_strm_blen_Left = 0 ;
size_t __mma_strm_tlen_Left = 0 ;
size_t __mma_strm_flen_Left = 0 ;
atomic_int __mm_strm_done_Left;
thrd_t __mm_strm_thread_Left;
shm_arbiter_buffer * __mma_strm_buf_Left;
typedef struct __mm_strm_in_Left _mm_strm_in_Left;
typedef struct __mm_strm_out_Left _mm_strm_out_Left;
typedef struct __mm_strm_hole_Left _mm_strm_hole_Left;
_mm_strm_out_Left * __mma_strm_istrt_Left = 0 ;
_mm_strm_out_Left * __mma_strm_bstrt_Left = 0 ;
void _mm_print_strm_Left( );
size_t __mma_strm_ilen_Right = 0 ;
size_t __mma_strm_blen_Right = 0 ;
size_t __mma_strm_tlen_Right = 0 ;
size_t __mma_strm_flen_Right = 0 ;
atomic_int __mm_strm_done_Right;
thrd_t __mm_strm_thread_Right;
shm_arbiter_buffer * __mma_strm_buf_Right;
typedef struct __mm_strm_in_Right _mm_strm_in_Right;
typedef struct __mm_strm_out_Right _mm_strm_out_Right;
typedef struct __mm_strm_hole_Right _mm_strm_hole_Right;
_mm_strm_out_Right * __mma_strm_istrt_Right = 0 ;
_mm_strm_out_Right * __mma_strm_bstrt_Right = 0 ;

struct __MMEV_Prime {
  int n ;
  int p ;
};
struct __MMEV_LPrime {
  int n ;
};
struct __MMEV_RPrime {
  int n ;
};
struct __MMEV_LSkip {
  int x ;
};
struct __MMEV_RSkip {
  int x ;
};
struct __MMEV_EFinal {
};
struct __mm_strm_hole_Left {
  int n ;
};
struct __mm_strm_in_Left {
  shm_event head ;
  union {
    _MMEV_Prime Prime ;
  } cases;
};

struct event {
        shm_event base;
        int n;
        int p;
};

int _mm_strm_fun_Left(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_Left ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_Left * inevent ;
  _mm_strm_out_Left * outevent ;
  while((! shm_arbiter_buffer_active ( buffer )))
  {
    sleep_ns ( 10 ) ;
  }
  void *last_addr = 0;
  while(1)
  {
    inevent = stream_fetch ( stream,buffer ) ;
    if((inevent == 0))
    {
      break;
    }
    else
    {
       /*
       printf("(L) %lu, %lu: %p (%lu)\n",
              shm_event_kind((shm_event*)inevent),
              shm_event_id((shm_event*)inevent),
              ((struct event*)inevent)->p,
              ((struct event*)inevent)->p - last_addr
              );
       last_addr = ((struct event*)inevent)->p;
       */
    }

    outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
    assert(outevent && "Buffer is full");
    memcpy ( outevent,inevent,sizeof ( _mm_strm_in_Left ) ) ;
    shm_arbiter_buffer_write_finish ( buffer ) ;
    shm_stream_consume ( stream,1 ) ;
  }
  atomic_store ( (&__mm_strm_done_Left),1 ) ;
  return 0 ;
}
struct __mm_strm_hole_Right {
  int n ;
};
struct __mm_strm_in_Right {
  shm_event head ;
  union {
    _MMEV_Prime Prime ;
  } cases;
};

int _mm_strm_fun_Right(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_Right ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_Right * inevent ;
  _mm_strm_out_Right * outevent ;
  while((! shm_arbiter_buffer_active ( buffer )))
  {
    sleep_ns ( 10 ) ;
  }
  void *last_addr = 0;
  while(1)
  {
    inevent = stream_fetch ( stream,buffer ) ;
    if((inevent == 0))
    {
      break;
    }
    else
    {
       /*
       printf("(R) %lu, %lu: %p (%lu)\n",
              shm_event_kind((shm_event*)inevent),
              shm_event_id((shm_event*)inevent),
              ((struct event*)inevent)->p,
              ((struct event*)inevent)->p - last_addr
              );
       last_addr = ((struct event*)inevent)->p;
       */
    }

    outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
    assert(outevent && "Buffer is full");
    memcpy ( outevent,inevent,sizeof ( _mm_strm_in_Right ) ) ;
    shm_arbiter_buffer_write_finish ( buffer ) ;
    shm_stream_consume ( stream,1 ) ;
  }
  atomic_store ( (&__mm_strm_done_Right),1 ) ;
  return 0 ;
}
int __mm_monitor_running = 1 ;

int arbiterMonitor( ) {
  size_t skipped_l = 0;
  size_t skipped_r = 0;
  size_t holes_l = 0;
  size_t holes_r = 0;
  size_t dropped_l = 0;
  size_t dropped_r = 0;
  size_t compared = 0;

  struct __mm_strm_in_Left *evL = 0;
  struct __mm_strm_in_Right *evR = 0;
  size_t left_next_id = 0;
  size_t right_next_id = 0;
  int skip_1 = 0, skip_2 = 0;
  shm_event *ev, *ev2;
  while(__mm_monitor_running)
  {
        evL = 0;
        evR = 0;

        if (ev = shm_arbiter_buffer_top(__mma_strm_buf_Left)) {
            if (ev->kind == 1) {
                skip_2 += ((shm_event_dropped*)ev)->n;
                dropped_l +=  ((shm_event_dropped*)ev)->n;
                ++holes_l;
                shm_arbiter_buffer_drop(__mma_strm_buf_Left, 1);
            } else if (skip_1 > 0) {
                --skip_1;
                ++skipped_l;
                shm_arbiter_buffer_drop(__mma_strm_buf_Left, 1);
           } else {
                evL = (struct __mm_strm_in_Left*) ev;
           }
        }

        if (ev = shm_arbiter_buffer_top(__mma_strm_buf_Right)) {
            if (ev->kind == 1) {
                skip_1 += ((shm_event_dropped*)ev)->n;
                dropped_r +=  ((shm_event_dropped*)ev)->n;
                ++holes_r;
                shm_arbiter_buffer_drop(__mma_strm_buf_Right, 1);
            } else if (skip_2 > 0) {
                --skip_2;
                ++skipped_r;
                shm_arbiter_buffer_drop(__mma_strm_buf_Right, 1);
           } else {
                evR = (struct __mm_strm_in_Right*) ev;
           }
        }

        /* compare as much elements as you can at once */
        while (evL && evR) {
            assert(skip_1 == 0 && skip_2 == 0);
            ++compared;
            if (evR->cases.Prime.n != evL->cases.Prime.n ||
                evR->cases.Prime.p != evL->cases.Prime.p) {
                 printf("ERROR: {%d, %d} != {%d, %d}\n",
                         evL->cases.Prime.n,
                         evL->cases.Prime.p,
                         evR->cases.Prime.n,
                         evR->cases.Prime.p);
            } /* else {
                    printf("OK: {%d, %d} == {%d, %d}\n",
                         evL->cases.Prime.n,
                         evL->cases.Prime.p,
                         evR->cases.Prime.n,
                         evR->cases.Prime.p);
            }
            */
            shm_arbiter_buffer_drop(__mma_strm_buf_Left, 1);
            shm_arbiter_buffer_drop(__mma_strm_buf_Right, 1);

            if (evL = shm_arbiter_buffer_top(__mma_strm_buf_Left)) {
                if (evL->head.kind == 1) {
                        break;
                } else if (evR = shm_arbiter_buffer_top(__mma_strm_buf_Right)) {
                    if (evR->head.kind == 1) {
                            break;
                    }
                }
            }
        }

        __mm_monitor_running
                = !(__mm_strm_done_Right && __mm_strm_done_Left &&
                    shm_arbiter_buffer_size(__mma_strm_buf_Left) == 0 &&
                    shm_arbiter_buffer_size(__mma_strm_buf_Right) == 0);
  }
  printf("LEFT: processed %lu events "
         "(%lu compared, %lu dropped (in %lu holes), %lu skipped)\n",
        compared + skipped_l + dropped_l,
        compared, dropped_l, holes_l, skipped_l);
  printf("RIGHT: processed %lu events "
         "(%lu compared, %lu dropped (in %lu holes), %lu skipped)\n",
        compared + skipped_r + dropped_r,
        compared, dropped_r, holes_r, skipped_r);
  return 1 ;
}
int main(int argc,char * * argv) {
  fprintf(stderr, "Reading and dropping events (empty monitor)\n");
  initialize_events ( ) ;
  _mm_source_control * __mm_strm_sourcecontrol_Left ;
  atomic_init ( (&__mm_strm_done_Left),0 ) ;
  shm_stream * __mma_strm_strm_Left = shm_stream_create ( "Left",(&__mm_strm_sourcecontrol_Left),argc,argv ) ;
  __mma_strm_buf_Left = shm_arbiter_buffer_create ( __mma_strm_strm_Left,sizeof ( _mm_strm_in_Left ), SHMBUF_ARBITER_BUFSIZE) ;
  thrd_create ( (&__mm_strm_thread_Left),(&_mm_strm_fun_Left),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_Left,1 ) ;
  _mm_source_control * __mm_strm_sourcecontrol_Right ;
  atomic_init ( (&__mm_strm_done_Right),0 ) ;
  shm_stream * __mma_strm_strm_Right = shm_stream_create ( "Right",(&__mm_strm_sourcecontrol_Right),argc,argv ) ;
  __mma_strm_buf_Right = shm_arbiter_buffer_create ( __mma_strm_strm_Right,sizeof ( _mm_strm_in_Right ), SHMBUF_ARBITER_BUFSIZE) ;
  thrd_create ( (&__mm_strm_thread_Right),(&_mm_strm_fun_Right),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_Right,1 ) ;
  arbiterMonitor ( ) ;

  /*
  shm_arbiter_buffer_dump_stats ( __mma_strm_buf_Left) ;
  shm_arbiter_buffer_dump_stats ( __mma_strm_buf_Right) ;
  */

  shm_stream_destroy ( __mma_strm_strm_Left ) ;
  shm_stream_destroy ( __mma_strm_strm_Right ) ;
  deinitialize_events ( ) ;
  return 0 ;
}

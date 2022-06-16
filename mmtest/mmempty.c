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

static size_t __mm_label_arbmon_R_ArBmOn_EQ_num;
static size_t __mm_label_arbmon_L_ArBmOn_Right3_num;
static size_t __mm_label_arbmon_L_ArBmOn_Right2_num;
static size_t __mm_label_arbmon_L_ArBmOn_Right1_num;
static size_t __mm_label_arbmon_L_ArBmOn_Left3_num;
static size_t __mm_label_arbmon_L_ArBmOn_Left2_num;
static size_t __mm_label_arbmon_L_ArBmOn_Left1_num;
static size_t __mm_label_arbmon_R_ArBmOn_Left1_num;
static size_t __mm_label_arbmon_R_ArBmOn_Left2_num;
static size_t __mm_label_arbmon_R_ArBmOn_Left3_num;
static size_t __mm_label_arbmon_R_ArBmOn_Right1_num;
static size_t __mm_label_arbmon_R_ArBmOn_Right2_num;
static size_t __mm_label_arbmon_R_ArBmOn_Right3_num;
static size_t __mm_label_arbmon_L_ArBmOn_EQ_num;


void _mm_print_strm_Right( );
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
struct __mm_strm_out_Left {
  shm_event head ;
  union {
    _mm_strm_hole_Left hole ;
    _MMEV_LPrime LPrime ;
  } cases;
};
void _mm_print_event_Left(const _mm_strm_out_Left * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_hole:
    {
      printf ( "hole(%i)\n",(((ev->cases).hole).n) ) ;
      break;
    }
    case __MM_EVENTCONST_ENUM_LPrime:
    {
      printf ( "LPrime(" ) ;
      printf ( "%i",(((ev->cases).LPrime).n) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_inevent_Left(const _mm_strm_in_Left * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_Prime:
    {
      printf ( "Prime(" ) ;
      printf ( "%i",(((ev->cases).Prime).n) ) ;
      printf ( "%i",(((ev->cases).Prime).p) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_strm_Left( ) {
  _mm_strm_out_Left * cur = __mma_strm_istrt_Left ;
  printf ( "\nSTREAM Left:\n" ) ;
  while((cur < (__mma_strm_istrt_Left + __mma_strm_ilen_Left)))
  {
    _mm_print_event_Left ( cur ) ;
    cur = (cur + 1) ;
  }
  cur = __mma_strm_bstrt_Left ;
  while((cur < (__mma_strm_bstrt_Left + __mma_strm_blen_Left)))
  {
    _mm_print_event_Left ( cur ) ;
    cur = (cur + 1) ;
  }
}
int _mm_strm_fun_Left(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_Left ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_Left * inevent ;
  _mm_strm_out_Left * outevent ;
  while((! shm_arbiter_buffer_active ( buffer )))
  {
    sleep_ns ( 10 ) ;
  }
  while(1)
  {
    inevent = stream_fetch ( stream,buffer ) ;
    if((inevent == 0))
    {
      break;
    }
    else
    {
    }
    
    switch (((inevent->head).kind)) {
      case __MM_EVENTCONST_ENUM_Prime:
      {
        int _mm_uv_mvar_n_0 = (((inevent->cases).Prime).n) ;
        int _mm_uv_mvar_p_1 = (((inevent->cases).Prime).p) ;
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        /* NOTE: this copies also 'kind' which is then overwritten */
        memcpy ( outevent,inevent,sizeof ( shm_event ) ) ;
        ((outevent->head).kind) = __MM_EVENTCONST_ENUM_LPrime ;
        (((outevent->cases).LPrime).n) = _mm_uv_mvar_p_1 ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
        continue;
      }
      default:
      {
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( _mm_strm_in_Left ) ) ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
      }
    }
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
struct __mm_strm_out_Right {
  shm_event head ;
  union {
    _mm_strm_hole_Right hole ;
    _MMEV_RPrime RPrime ;
  } cases;
};
void _mm_print_event_Right(const _mm_strm_out_Right * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_hole:
    {
      printf ( "hole(%i)\n",(((ev->cases).hole).n) ) ;
      break;
    }
    case __MM_EVENTCONST_ENUM_RPrime:
    {
      printf ( "RPrime(" ) ;
      printf ( "%i",(((ev->cases).RPrime).n) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_inevent_Right(const _mm_strm_in_Right * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_Prime:
    {
      printf ( "Prime(" ) ;
      printf ( "%i",(((ev->cases).Prime).n) ) ;
      printf ( "%i",(((ev->cases).Prime).p) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_strm_Right( ) {
  _mm_strm_out_Right * cur = __mma_strm_istrt_Right ;
  printf ( "\nSTREAM Right:\n" ) ;
  while((cur < (__mma_strm_istrt_Right + __mma_strm_ilen_Right)))
  {
    _mm_print_event_Right ( cur ) ;
    cur = (cur + 1) ;
  }
  cur = __mma_strm_bstrt_Right ;
  while((cur < (__mma_strm_bstrt_Right + __mma_strm_blen_Right)))
  {
    _mm_print_event_Right ( cur ) ;
    cur = (cur + 1) ;
  }
}
int _mm_strm_fun_Right(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_Right ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_Right * inevent ;
  _mm_strm_out_Right * outevent ;
  while((! shm_arbiter_buffer_active ( buffer )))
  {
    sleep_ns ( 10 ) ;
  }
  while(1)
  {
    inevent = stream_fetch ( stream,buffer ) ;
    if((inevent == 0))
    {
      break;
    }
    else
    {
    }
    
    switch (((inevent->head).kind)) {
      case __MM_EVENTCONST_ENUM_Prime:
      {
        int _mm_uv_mvar_n_2 = (((inevent->cases).Prime).n) ;
        int _mm_uv_mvar_p_3 = (((inevent->cases).Prime).p) ;
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( shm_event ) ) ;
        ((outevent->head).kind) = __MM_EVENTCONST_ENUM_RPrime ;
        (((outevent->cases).RPrime).n) = _mm_uv_mvar_p_3 ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
        continue;
      }
      default:
      {
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( _mm_strm_in_Right ) ) ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
      }
    }
  }
  atomic_store ( (&__mm_strm_done_Right),1 ) ;
  return 0 ;
}
int __mm_monitor_running = 1 ;
typedef struct __MMARBTP _MMARBTP;
typedef struct __MMMONTP _MMMONTP;
struct __MMARBTP {
  int pre ;
  int seen ;
  int post ;
  int sl ;
  int sr ;
  int dl ;
  int dr ;
  int il ;
  int ir ;
};
struct __MMMONTP {
  int * pbuf ;
  int pl ;
  int pr ;
};
void _mm_print_state(_MMARBTP * arbiter,_MMMONTP * monitor,char * arbstate,char * monstate) {
  printf ( "Arbiter (%s):\n",arbstate ) ;
  printf ( "pre: %i\n",(arbiter->pre) ) ;
  printf ( "seen: %i\n",(arbiter->seen) ) ;
  printf ( "post: %i\n",(arbiter->post) ) ;
  printf ( "sl: %i\n",(arbiter->sl) ) ;
  printf ( "sr: %i\n",(arbiter->sr) ) ;
  printf ( "dl: %i\n",(arbiter->dl) ) ;
  printf ( "dr: %i\n",(arbiter->dr) ) ;
  printf ( "il: %i\n",(arbiter->il) ) ;
  printf ( "ir: %i\n",(arbiter->ir) ) ;
  printf ( "Monitor (%s):\n",monstate ) ;
  printf ( "pl: %i\n",(monitor->pl) ) ;
  printf ( "pr: %i\n",(monitor->pr) ) ;
  _mm_print_streams ( ) ;
}
int arbiterMonitor( ) {
  uint64_t tmp1, tmp2;
  while(__mm_monitor_running)
  {
          tmp1 = shm_arbiter_buffer_size(__mma_strm_buf_Left);
          if (tmp1 > 0) {
                  shm_arbiter_buffer_drop(__mma_strm_buf_Left, tmp1);
          }
          tmp2 = shm_arbiter_buffer_size(__mma_strm_buf_Right);
          if (tmp2 > 0) {
                  shm_arbiter_buffer_drop(__mma_strm_buf_Right, tmp2);
          }
          __mm_monitor_running
                = !(tmp1 == 0 && tmp2 == 0 && atomic_load_explicit(&__mm_strm_done_Right, memory_order_acquire) && atomic_load_explicit(&__mm_strm_done_Left, memory_order_acquire));
  }
  return 1 ;
}
int main(int argc,char * * argv) {
  fprintf(stderr, "Reading and dropping events (empty monitor)\n");
  initialize_events ( ) ;
  _mm_source_control * __mm_strm_sourcecontrol_Left ;
  atomic_init ( (&__mm_strm_done_Left),0 ) ;
  shm_stream * __mma_strm_strm_Left = shm_stream_create ( "Left",(&__mm_strm_sourcecontrol_Left),argc,argv ) ;
  __mma_strm_buf_Left = shm_arbiter_buffer_create ( __mma_strm_strm_Left,sizeof ( _mm_strm_out_Left ), SHMBUF_ARBITER_BUFSIZE) ;
  thrd_create ( (&__mm_strm_thread_Left),(&_mm_strm_fun_Left),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_Left,1 ) ;
  _mm_source_control * __mm_strm_sourcecontrol_Right ;
  atomic_init ( (&__mm_strm_done_Right),0 ) ;
  shm_stream * __mma_strm_strm_Right = shm_stream_create ( "Right",(&__mm_strm_sourcecontrol_Right),argc,argv ) ;
  __mma_strm_buf_Right = shm_arbiter_buffer_create ( __mma_strm_strm_Right,sizeof ( _mm_strm_out_Right ), SHMBUF_ARBITER_BUFSIZE) ;
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
void _mm_print_streams( ) {
  _mm_print_strm_Left ( ) ;
  _mm_print_strm_Right ( ) ;
}

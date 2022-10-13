#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include "../gen/shamon.h"
#include "../gen/mmlib.h"
typedef enum __MM_STREAMCONST_ENUM { __MM_STREAMCONST_ENUM_S } _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM { __MM_EVENTCONST_NOTHING, __MM_EVENTCONST_ENUM_hole, __MM_EVENTCONST_ENUM_E, __MM_EVENTCONST_ENUM_M } _MM_EVENTCONST_ENUM;
typedef struct source_control _mm_source_control;
void _mm_print_streams( );
typedef struct __MMEV_E _MMEV_E;
typedef struct __MMEV_M _MMEV_M;
size_t __mma_strm_ilen_S = 0 ;
size_t __mma_strm_blen_S = 0 ;
size_t __mma_strm_tlen_S = 0 ;
size_t __mma_strm_flen_S = 0 ;
atomic_int __mm_strm_done_S;
thrd_t __mm_strm_thread_S;
shm_arbiter_buffer * __mma_strm_buf_S;
typedef struct __mm_strm_in_S _mm_strm_in_S;
typedef struct __mm_strm_out_S _mm_strm_out_S;
typedef struct __mm_strm_hole_S _mm_strm_hole_S;
_mm_strm_out_S * __mma_strm_istrt_S = 0 ;
_mm_strm_out_S * __mma_strm_bstrt_S = 0 ;
void _mm_print_strm_S( );
struct __MMEV_E {
  int n ;
};
struct __MMEV_M {
};
struct __mm_strm_hole_S {
  int n ;
};
struct __mm_strm_in_S {
  shm_event head ;
  union {
    _MMEV_E E ;
  } cases;
};
struct __mm_strm_out_S {
  shm_event head ;
  union {
    _mm_strm_hole_S hole ;
    _MMEV_E E ;
  } cases;
};
void _mm_print_event_S(const _mm_strm_out_S * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_hole:
    {
      printf ( "hole(%i)\n",(((ev->cases).hole).n) ) ;
      break;
    }
    case __MM_EVENTCONST_ENUM_E:
    {
      printf ( "E(" ) ;
      printf ( "%i",(((ev->cases).E).n) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_inevent_S(const _mm_strm_in_S * ev) {
  switch (((ev->head).kind)) {
    case __MM_EVENTCONST_ENUM_E:
    {
      printf ( "E(" ) ;
      printf ( "%i",(((ev->cases).E).n) ) ;
      printf ( ")\n" ) ;
      break;
    }
  }
}
void _mm_print_strm_S( ) {
  _mm_strm_out_S * cur = __mma_strm_istrt_S ;
  printf ( "\nSTREAM S:\n" ) ;
  while((cur < (__mma_strm_istrt_S + __mma_strm_ilen_S)))
  {
    _mm_print_event_S ( cur ) ;
    cur = (cur + 1) ;
  }
  cur = __mma_strm_bstrt_S ;
  while((cur < (__mma_strm_bstrt_S + __mma_strm_blen_S)))
  {
    _mm_print_event_S ( cur ) ;
    cur = (cur + 1) ;
  }
}
int _mm_strm_fun_S(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_S ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_S * inevent ;
  _mm_strm_out_S * outevent ;
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
      default:
      {
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( _mm_strm_in_S ) ) ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
      }
    }
  }
  atomic_store ( (&__mm_strm_done_S),1 ) ;
  return 0 ;
}
int __mm_monitor_running = 1 ;
typedef struct __MMARBTP _MMARBTP;
typedef struct __MMMONTP _MMMONTP;
struct __MMARBTP {
};
struct __MMMONTP {
  int l ;
};
void _mm_print_state(_MMARBTP * arbiter,_MMMONTP * monitor,char * arbstate,char * monstate) {
  printf ( "Arbiter (%s):\n",arbstate ) ;
  printf ( "Monitor (%s):\n",monstate ) ;
  printf ( "l: %i\n",(monitor->l) ) ;
  _mm_print_streams ( ) ;
}
int arbiterMonitor( ) {
  _MMARBTP _mm_arbiter ;
  _MMMONTP _mm_monitor ;
  (_mm_monitor.l) = 0 ;
  while(__mm_monitor_running)
  {
    goto __mm_label_arbmon_X_ArBmOn_X;
    __mm_label_arbmon_X_ArBmOn_X:
    {
      __mma_strm_tlen_S = shm_arbiter_buffer_peek ( __mma_strm_buf_S,1,((void * *)(&__mma_strm_istrt_S)),(&__mma_strm_ilen_S),((void * *)(&__mma_strm_bstrt_S)),(&__mma_strm_blen_S) ) ;
      __mma_strm_flen_S = (__mma_strm_ilen_S + __mma_strm_blen_S) ;
      if((__mma_strm_flen_S >= 1))
      {
        _mm_strm_out_S * __mm_evref_S_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_bstrt_S ) ;
        if((((__mm_evref_S_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S_0_n = (((__mm_evref_S_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S,1,__mma_strm_tlen_S,__mma_strm_flen_S,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_blen_S,__mma_strm_bstrt_S ) ;
          {
            {
              int _mm_uv_mvar_n_2 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_2 == __mm_arbiter_yieldvar_n))
              {
                if((_mm_uv_mvar_n_2 == ((_mm_monitor.l) + 1)))
                {
                  (_mm_monitor.l) = _mm_uv_mvar_n_2 ;
                  goto __mm_label_arbmon_X_ArBmOn_X;
                }
                else
                {
                  int _mm_uv_mvar_n_3 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_3 == __mm_arbiter_yieldvar_n))
                  {
                    _mm_gco _mm_uv_gvar_tmpstrng_5 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_3 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l) ) ),_mm_lib_make_string ( "!" ) ) ;
                    printf ( "%s\n",(_mm_uv_gvar_tmpstrng_5->data) ) ;
                    _mm_decref ( _mm_uv_gvar_tmpstrng_5 ) ;
                    (_mm_monitor.l) = _mm_uv_mvar_n_3 ;
                    goto __mm_label_arbmon_X_ArBmOn_X;
                  }
                  else
                  {
                  }
                  
                }
                
              }
              else
              {
                int _mm_uv_mvar_n_3 = __mm_arbiter_yieldvar_n ;
                if((_mm_uv_mvar_n_3 == __mm_arbiter_yieldvar_n))
                {
                  _mm_gco _mm_uv_gvar_tmpstrng_6 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_3 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l) ) ),_mm_lib_make_string ( "!" ) ) ;
                  printf ( "%s\n",(_mm_uv_gvar_tmpstrng_6->data) ) ;
                  _mm_decref ( _mm_uv_gvar_tmpstrng_6 ) ;
                  (_mm_monitor.l) = _mm_uv_mvar_n_3 ;
                  goto __mm_label_arbmon_X_ArBmOn_X;
                }
                else
                {
                }
                
              }
              
            }
          }
        }
        else
        {
        }
        
        if((((__mm_evref_S_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S_0_n = (((__mm_evref_S_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S,1,__mma_strm_tlen_S,__mma_strm_flen_S,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_blen_S,__mma_strm_bstrt_S ) ;
          {
            {
              goto __mm_label_arbmon_X_ArBmOn_M;
            }
          }
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S == 0) && (atomic_load_explicit ( (&__mm_strm_done_S),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S,0,((void * *)(&__mma_strm_istrt_S)),(&__mma_strm_ilen_S),((void * *)(&__mma_strm_bstrt_S)),(&__mma_strm_blen_S) ) == 0))))
      {
        return 0 ;
      }
      else
      {
      }
      
      if((__mma_strm_tlen_S == 0))
      {
        goto __mm_label_arbmon_X_ArBmOn_X;
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X/X\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X","X" ) ;
    break;
    __mm_label_arbmon_X_ArBmOn_M:
    {
      __mma_strm_tlen_S = shm_arbiter_buffer_peek ( __mma_strm_buf_S,1,((void * *)(&__mma_strm_istrt_S)),(&__mma_strm_ilen_S),((void * *)(&__mma_strm_bstrt_S)),(&__mma_strm_blen_S) ) ;
      __mma_strm_flen_S = (__mma_strm_ilen_S + __mma_strm_blen_S) ;
      if((__mma_strm_flen_S >= 1))
      {
        _mm_strm_out_S * __mm_evref_S_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_bstrt_S ) ;
        if((((__mm_evref_S_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S_0_n = (((__mm_evref_S_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S,1,__mma_strm_tlen_S,__mma_strm_flen_S,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_blen_S,__mma_strm_bstrt_S ) ;
          {
            {
              int _mm_uv_mvar_n_4 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_4 == __mm_arbiter_yieldvar_n))
              {
                (_mm_monitor.l) = _mm_uv_mvar_n_4 ;
                goto __mm_label_arbmon_X_ArBmOn_X;
              }
              else
              {
              }
              
            }
          }
        }
        else
        {
        }
        
        if((((__mm_evref_S_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S_0_n = (((__mm_evref_S_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S,1,__mma_strm_tlen_S,__mma_strm_flen_S,__mma_strm_ilen_S,__mma_strm_istrt_S,__mma_strm_blen_S,__mma_strm_bstrt_S ) ;
          {
            {
              goto __mm_label_arbmon_X_ArBmOn_M;
            }
          }
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S == 0) && (atomic_load_explicit ( (&__mm_strm_done_S),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S,0,((void * *)(&__mma_strm_istrt_S)),(&__mma_strm_ilen_S),((void * *)(&__mma_strm_bstrt_S)),(&__mma_strm_blen_S) ) == 0))))
      {
        return 0 ;
      }
      else
      {
      }
      
      if((__mma_strm_tlen_S == 0))
      {
        goto __mm_label_arbmon_X_ArBmOn_M;
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X/M\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X","M" ) ;
    break;
  }
  return 1 ;
}
int main(int argc,char * * argv) {
  initialize_events ( ) ;
  _mm_source_control * __mm_strm_sourcecontrol_S ;
  atomic_init ( (&__mm_strm_done_S),0 ) ;
  shm_stream * __mma_strm_strm_S = shm_stream_create ( "S",(&__mm_strm_sourcecontrol_S),argc,argv ) ;
  __mma_strm_buf_S = shm_arbiter_buffer_create ( __mma_strm_strm_S,sizeof ( _mm_strm_out_S ),120 ) ;
  thrd_create ( (&__mm_strm_thread_S),(&_mm_strm_fun_S),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_S,1 ) ;
  arbiterMonitor ( ) ;
  shm_arbiter_buffer_dump_stats ( __mma_strm_buf_S ) ;
  shm_stream_destroy ( __mma_strm_strm_S ) ;
  deinitialize_events ( ) ;
  return 0 ;
}
void _mm_print_streams( ) {
  _mm_print_strm_S ( ) ;
}

#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include "../gen/shamon.h"
#include "../gen/mmlib.h"
typedef enum __MM_STREAMCONST_ENUM { __MM_STREAMCONST_ENUM_S1, __MM_STREAMCONST_ENUM_S2 } _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM { __MM_EVENTCONST_NOTHING, __MM_EVENTCONST_ENUM_hole, __MM_EVENTCONST_ENUM_E, __MM_EVENTCONST_ENUM_E1, __MM_EVENTCONST_ENUM_E2, __MM_EVENTCONST_ENUM_M1, __MM_EVENTCONST_ENUM_M2 } _MM_EVENTCONST_ENUM;
typedef struct source_control _mm_source_control;
void _mm_print_streams( );
typedef struct __MMEV_E _MMEV_E;
typedef struct __MMEV_E1 _MMEV_E1;
typedef struct __MMEV_E2 _MMEV_E2;
typedef struct __MMEV_M1 _MMEV_M1;
typedef struct __MMEV_M2 _MMEV_M2;
size_t __mma_strm_ilen_S1 = 0 ;
size_t __mma_strm_blen_S1 = 0 ;
size_t __mma_strm_tlen_S1 = 0 ;
size_t __mma_strm_flen_S1 = 0 ;
atomic_int __mm_strm_done_S1;
thrd_t __mm_strm_thread_S1;
shm_arbiter_buffer * __mma_strm_buf_S1;
typedef struct __mm_strm_in_S1 _mm_strm_in_S1;
typedef struct __mm_strm_out_S1 _mm_strm_out_S1;
typedef struct __mm_strm_hole_S1 _mm_strm_hole_S1;
_mm_strm_out_S1 * __mma_strm_istrt_S1 = 0 ;
_mm_strm_out_S1 * __mma_strm_bstrt_S1 = 0 ;
void _mm_print_strm_S1( );
size_t __mma_strm_ilen_S2 = 0 ;
size_t __mma_strm_blen_S2 = 0 ;
size_t __mma_strm_tlen_S2 = 0 ;
size_t __mma_strm_flen_S2 = 0 ;
atomic_int __mm_strm_done_S2;
thrd_t __mm_strm_thread_S2;
shm_arbiter_buffer * __mma_strm_buf_S2;
typedef struct __mm_strm_in_S2 _mm_strm_in_S2;
typedef struct __mm_strm_out_S2 _mm_strm_out_S2;
typedef struct __mm_strm_hole_S2 _mm_strm_hole_S2;
_mm_strm_out_S2 * __mma_strm_istrt_S2 = 0 ;
_mm_strm_out_S2 * __mma_strm_bstrt_S2 = 0 ;
void _mm_print_strm_S2( );
struct __MMEV_E {
  int n ;
};
struct __MMEV_E1 {
  int n ;
};
struct __MMEV_E2 {
  int n ;
};
struct __MMEV_M1 {
};
struct __MMEV_M2 {
};
struct __mm_strm_hole_S1 {
  int n ;
};
struct __mm_strm_in_S1 {
  shm_event head ;
  union {
    _MMEV_E E ;
  } cases;
};
struct __mm_strm_out_S1 {
  shm_event head ;
  union {
    _mm_strm_hole_S1 hole ;
    _MMEV_E E ;
  } cases;
};
void _mm_print_event_S1(const _mm_strm_out_S1 * ev) {
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
void _mm_print_inevent_S1(const _mm_strm_in_S1 * ev) {
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
void _mm_print_strm_S1( ) {
  _mm_strm_out_S1 * cur = __mma_strm_istrt_S1 ;
  printf ( "\nSTREAM S1:\n" ) ;
  while((cur < (__mma_strm_istrt_S1 + __mma_strm_ilen_S1)))
  {
    _mm_print_event_S1 ( cur ) ;
    cur = (cur + 1) ;
  }
  cur = __mma_strm_bstrt_S1 ;
  while((cur < (__mma_strm_bstrt_S1 + __mma_strm_blen_S1)))
  {
    _mm_print_event_S1 ( cur ) ;
    cur = (cur + 1) ;
  }
}
int _mm_strm_fun_S1(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_S1 ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_S1 * inevent ;
  _mm_strm_out_S1 * outevent ;
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
	    fprintf(stderr, "S1 fetched {kind = %lu, id = %lu}\n",
	            shm_event_kind(inevent), shm_event_id(inevent));
    }
    
    switch (((inevent->head).kind)) {
      default:
      {
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( _mm_strm_in_S1 ) ) ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
      }
    }
  }
  atomic_store ( (&__mm_strm_done_S1),1 ) ;
  return 0 ;
}
struct __mm_strm_hole_S2 {
  int n ;
};
struct __mm_strm_in_S2 {
  shm_event head ;
  union {
    _MMEV_E E ;
  } cases;
};
struct __mm_strm_out_S2 {
  shm_event head ;
  union {
    _mm_strm_hole_S2 hole ;
    _MMEV_E E ;
  } cases;
};
void _mm_print_event_S2(const _mm_strm_out_S2 * ev) {
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
void _mm_print_inevent_S2(const _mm_strm_in_S2 * ev) {
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
void _mm_print_strm_S2( ) {
  _mm_strm_out_S2 * cur = __mma_strm_istrt_S2 ;
  printf ( "\nSTREAM S2:\n" ) ;
  while((cur < (__mma_strm_istrt_S2 + __mma_strm_ilen_S2)))
  {
    _mm_print_event_S2 ( cur ) ;
    cur = (cur + 1) ;
  }
  cur = __mma_strm_bstrt_S2 ;
  while((cur < (__mma_strm_bstrt_S2 + __mma_strm_blen_S2)))
  {
    _mm_print_event_S2 ( cur ) ;
    cur = (cur + 1) ;
  }
}
int _mm_strm_fun_S2(void * arg) {
  shm_arbiter_buffer * buffer = __mma_strm_buf_S2 ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_S2 * inevent ;
  _mm_strm_out_S2 * outevent ;
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
	    fprintf(stderr, "S2 fetched {kind = %lu, id = %lu}\n",
	            shm_event_kind(inevent), shm_event_id(inevent));
    }
    
    switch (((inevent->head).kind)) {
      default:
      {
        outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
        memcpy ( outevent,inevent,sizeof ( _mm_strm_in_S2 ) ) ;
        shm_arbiter_buffer_write_finish ( buffer ) ;
        shm_stream_consume ( stream,1 ) ;
      }
    }
  }
  atomic_store ( (&__mm_strm_done_S2),1 ) ;
  return 0 ;
}
int __mm_monitor_running = 1 ;
typedef struct __MMARBTP _MMARBTP;
typedef struct __MMMONTP _MMMONTP;
struct __MMARBTP {
};
struct __MMMONTP {
  int l1 ;
  int l2 ;
};
void _mm_print_state(_MMARBTP * arbiter,_MMMONTP * monitor,char * arbstate,char * monstate) {
  printf ( "Arbiter (%s):\n",arbstate ) ;
  printf ( "Monitor (%s):\n",monstate ) ;
  printf ( "l1: %i\n",(monitor->l1) ) ;
  printf ( "l2: %i\n",(monitor->l2) ) ;
  _mm_print_streams ( ) ;
}
int arbiterMonitor( ) {
  _MMARBTP _mm_arbiter ;
  _MMMONTP _mm_monitor ;
  (_mm_monitor.l1) = 0 ;
  (_mm_monitor.l2) = 0 ;
  while(__mm_monitor_running)
  {
    goto __mm_label_arbmon_X1_ArBmOn_X;
    __mm_label_arbmon_X1_ArBmOn_X:
    {
      __mma_strm_tlen_S1 = shm_arbiter_buffer_peek ( __mma_strm_buf_S1,1,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) ;
      __mma_strm_flen_S1 = (__mma_strm_ilen_S1 + __mma_strm_blen_S1) ;
      if((__mma_strm_flen_S1 >= 1))
      {
        _mm_strm_out_S1 * __mm_evref_S1_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_bstrt_S1 ) ;
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S1_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              int _mm_uv_mvar_n_4 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_4 == __mm_arbiter_yieldvar_n))
              {
                if((_mm_uv_mvar_n_4 == ((_mm_monitor.l1) + 1)))
                {
                  (_mm_monitor.l1) = _mm_uv_mvar_n_4 ;
                  goto __mm_label_arbmon_X2_ArBmOn_X;
                }
                else
                {
                  int _mm_uv_mvar_n_5 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_5 == __mm_arbiter_yieldvar_n))
                  {
                    _mm_gco _mm_uv_gvar_tmpstrng_16 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_5 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l1) ) ),_mm_lib_make_string ( "!" ) ) ;
                    printf ( "%s\n",(_mm_uv_gvar_tmpstrng_16->data) ) ;
                    _mm_decref ( _mm_uv_gvar_tmpstrng_16 ) ;
                    (_mm_monitor.l1) = _mm_uv_mvar_n_5 ;
                    goto __mm_label_arbmon_X2_ArBmOn_X;
                  }
                  else
                  {
                  }
                  
                }
                
              }
              else
              {
                int _mm_uv_mvar_n_5 = __mm_arbiter_yieldvar_n ;
                if((_mm_uv_mvar_n_5 == __mm_arbiter_yieldvar_n))
                {
                  _mm_gco _mm_uv_gvar_tmpstrng_17 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_5 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l1) ) ),_mm_lib_make_string ( "!" ) ) ;
                  printf ( "%s\n",(_mm_uv_gvar_tmpstrng_17->data) ) ;
                  _mm_decref ( _mm_uv_gvar_tmpstrng_17 ) ;
                  (_mm_monitor.l1) = _mm_uv_mvar_n_5 ;
                  goto __mm_label_arbmon_X2_ArBmOn_X;
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
        
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S1_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              goto __mm_label_arbmon_X2_ArBmOn_M1;
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
      
      if((__mma_strm_tlen_S1 == 0))
      {
	if (atomic_load_explicit (&__mm_strm_done_S1,memory_order_acquire) &&
	    atomic_load_explicit (&__mm_strm_done_S2,memory_order_acquire)) {
		break;
	}
        goto __mm_label_arbmon_X2_ArBmOn_X;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X1/X\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X1","X" ) ;
    break;
    __mm_label_arbmon_X1_ArBmOn_M1:
    {
      __mma_strm_tlen_S1 = shm_arbiter_buffer_peek ( __mma_strm_buf_S1,1,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) ;
      __mma_strm_flen_S1 = (__mma_strm_ilen_S1 + __mma_strm_blen_S1) ;
      if((__mma_strm_flen_S1 >= 1))
      {
        _mm_strm_out_S1 * __mm_evref_S1_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_bstrt_S1 ) ;
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S1_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              int _mm_uv_mvar_n_8 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_8 == __mm_arbiter_yieldvar_n))
              {
                (_mm_monitor.l1) = _mm_uv_mvar_n_8 ;
                goto __mm_label_arbmon_X2_ArBmOn_X;
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
        
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S1_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              goto __mm_label_arbmon_X2_ArBmOn_M1;
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
      
      if((__mma_strm_tlen_S1 == 0))
      {
        goto __mm_label_arbmon_X2_ArBmOn_M1;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X1/M1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X1","M1" ) ;
    break;
    __mm_label_arbmon_X1_ArBmOn_M2:
    {
      __mma_strm_tlen_S1 = shm_arbiter_buffer_peek ( __mma_strm_buf_S1,1,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) ;
      __mma_strm_flen_S1 = (__mma_strm_ilen_S1 + __mma_strm_blen_S1) ;
      if((__mma_strm_flen_S1 >= 1))
      {
        _mm_strm_out_S1 * __mm_evref_S1_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_bstrt_S1 ) ;
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S1_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              int _mm_uv_mvar_n_12 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_12 == __mm_arbiter_yieldvar_n))
              {
                if((_mm_uv_mvar_n_12 == ((_mm_monitor.l1) + 1)))
                {
                  (_mm_monitor.l1) = _mm_uv_mvar_n_12 ;
                  goto __mm_label_arbmon_X2_ArBmOn_X;
                }
                else
                {
                  int _mm_uv_mvar_n_13 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_13 == __mm_arbiter_yieldvar_n))
                  {
                    _mm_gco _mm_uv_gvar_tmpstrng_18 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_13 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l1) ) ),_mm_lib_make_string ( "!" ) ) ;
                    printf ( "%s\n",(_mm_uv_gvar_tmpstrng_18->data) ) ;
                    _mm_decref ( _mm_uv_gvar_tmpstrng_18 ) ;
                    (_mm_monitor.l1) = _mm_uv_mvar_n_13 ;
                    goto __mm_label_arbmon_X2_ArBmOn_X;
                  }
                  else
                  {
                  }
                  
                }
                
              }
              else
              {
                int _mm_uv_mvar_n_13 = __mm_arbiter_yieldvar_n ;
                if((_mm_uv_mvar_n_13 == __mm_arbiter_yieldvar_n))
                {
                  _mm_gco _mm_uv_gvar_tmpstrng_19 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_13 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l1) ) ),_mm_lib_make_string ( "!" ) ) ;
                  printf ( "%s\n",(_mm_uv_gvar_tmpstrng_19->data) ) ;
                  _mm_decref ( _mm_uv_gvar_tmpstrng_19 ) ;
                  (_mm_monitor.l1) = _mm_uv_mvar_n_13 ;
                  goto __mm_label_arbmon_X2_ArBmOn_X;
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
        
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S1_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              goto __mm_label_arbmon_X2_ArBmOn_MB;
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
      
      if((__mma_strm_tlen_S1 == 0))
      {
        goto __mm_label_arbmon_X2_ArBmOn_M2;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X1/M2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X1","M2" ) ;
    break;
    __mm_label_arbmon_X1_ArBmOn_MB:
    {
      __mma_strm_tlen_S1 = shm_arbiter_buffer_peek ( __mma_strm_buf_S1,1,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) ;
      __mma_strm_flen_S1 = (__mma_strm_ilen_S1 + __mma_strm_blen_S1) ;
      if((__mma_strm_flen_S1 >= 1))
      {
        _mm_strm_out_S1 * __mm_evref_S1_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_bstrt_S1 ) ;
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).E).n) ;
          int _mm_uv_mvar_n_0 = __mm_evfref_S1_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_0 ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              int _mm_uv_mvar_n_14 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_14 == __mm_arbiter_yieldvar_n))
              {
                (_mm_monitor.l1) = _mm_uv_mvar_n_14 ;
                goto __mm_label_arbmon_X2_ArBmOn_M2;
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
        
        if((((__mm_evref_S1_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S1_0_n = (((__mm_evref_S1_0->cases).hole).n) ;
          int _mm_uv_mvar_n_1 = __mm_evfref_S1_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S1,1,__mma_strm_tlen_S1,__mma_strm_flen_S1,__mma_strm_ilen_S1,__mma_strm_istrt_S1,__mma_strm_blen_S1,__mma_strm_bstrt_S1 ) ;
          {
            {
              goto __mm_label_arbmon_X2_ArBmOn_MB;
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
      
      if((__mma_strm_tlen_S1 == 0))
      {
        goto __mm_label_arbmon_X2_ArBmOn_MB;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X1/MB\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X1","MB" ) ;
    break;
    __mm_label_arbmon_X2_ArBmOn_X:
    {
      __mma_strm_tlen_S2 = shm_arbiter_buffer_peek ( __mma_strm_buf_S2,1,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) ;
      __mma_strm_flen_S2 = (__mma_strm_ilen_S2 + __mma_strm_blen_S2) ;
      if((__mma_strm_flen_S2 >= 1))
      {
        _mm_strm_out_S2 * __mm_evref_S2_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_bstrt_S2 ) ;
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).E).n) ;
          int _mm_uv_mvar_n_2 = __mm_evfref_S2_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_2 ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              int _mm_uv_mvar_n_6 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_6 == __mm_arbiter_yieldvar_n))
              {
                if((_mm_uv_mvar_n_6 == ((_mm_monitor.l2) + 1)))
                {
                  (_mm_monitor.l2) = _mm_uv_mvar_n_6 ;
                  goto __mm_label_arbmon_X1_ArBmOn_X;
                }
                else
                {
                  int _mm_uv_mvar_n_7 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_7 == __mm_arbiter_yieldvar_n))
                  {
                    _mm_gco _mm_uv_gvar_tmpstrng_20 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_7 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l2) ) ),_mm_lib_make_string ( "!" ) ) ;
                    printf ( "%s\n",(_mm_uv_gvar_tmpstrng_20->data) ) ;
                    _mm_decref ( _mm_uv_gvar_tmpstrng_20 ) ;
                    (_mm_monitor.l2) = _mm_uv_mvar_n_7 ;
                    goto __mm_label_arbmon_X1_ArBmOn_X;
                  }
                  else
                  {
                  }
                  
                }
                
              }
              else
              {
                int _mm_uv_mvar_n_7 = __mm_arbiter_yieldvar_n ;
                if((_mm_uv_mvar_n_7 == __mm_arbiter_yieldvar_n))
                {
                  _mm_gco _mm_uv_gvar_tmpstrng_21 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_7 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l2) ) ),_mm_lib_make_string ( "!" ) ) ;
                  printf ( "%s\n",(_mm_uv_gvar_tmpstrng_21->data) ) ;
                  _mm_decref ( _mm_uv_gvar_tmpstrng_21 ) ;
                  (_mm_monitor.l2) = _mm_uv_mvar_n_7 ;
                  goto __mm_label_arbmon_X1_ArBmOn_X;
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
        
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).hole).n) ;
          int _mm_uv_mvar_n_3 = __mm_evfref_S2_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              goto __mm_label_arbmon_X1_ArBmOn_M2;
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
      
      if((__mma_strm_tlen_S2 == 0))
      {
        goto __mm_label_arbmon_X1_ArBmOn_X;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X2/X\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X2","X" ) ;
    break;
    __mm_label_arbmon_X2_ArBmOn_M1:
    {
      __mma_strm_tlen_S2 = shm_arbiter_buffer_peek ( __mma_strm_buf_S2,1,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) ;
      __mma_strm_flen_S2 = (__mma_strm_ilen_S2 + __mma_strm_blen_S2) ;
      if((__mma_strm_flen_S2 >= 1))
      {
        _mm_strm_out_S2 * __mm_evref_S2_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_bstrt_S2 ) ;
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).E).n) ;
          int _mm_uv_mvar_n_2 = __mm_evfref_S2_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_2 ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              int _mm_uv_mvar_n_9 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_9 == __mm_arbiter_yieldvar_n))
              {
                if((_mm_uv_mvar_n_9 == ((_mm_monitor.l2) + 1)))
                {
                  (_mm_monitor.l2) = _mm_uv_mvar_n_9 ;
                  goto __mm_label_arbmon_X1_ArBmOn_X;
                }
                else
                {
                  int _mm_uv_mvar_n_10 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_10 == __mm_arbiter_yieldvar_n))
                  {
                    _mm_gco _mm_uv_gvar_tmpstrng_22 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_10 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l2) ) ),_mm_lib_make_string ( "!" ) ) ;
                    printf ( "%s\n",(_mm_uv_gvar_tmpstrng_22->data) ) ;
                    _mm_decref ( _mm_uv_gvar_tmpstrng_22 ) ;
                    (_mm_monitor.l2) = _mm_uv_mvar_n_10 ;
                    goto __mm_label_arbmon_X1_ArBmOn_X;
                  }
                  else
                  {
                  }
                  
                }
                
              }
              else
              {
                int _mm_uv_mvar_n_10 = __mm_arbiter_yieldvar_n ;
                if((_mm_uv_mvar_n_10 == __mm_arbiter_yieldvar_n))
                {
                  _mm_gco _mm_uv_gvar_tmpstrng_23 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_int_to_string ( _mm_uv_mvar_n_10 ),_mm_lib_make_string ( " is not the direct successor of " ) ),_mm_lib_int_to_string ( (_mm_monitor.l2) ) ),_mm_lib_make_string ( "!" ) ) ;
                  printf ( "%s\n",(_mm_uv_gvar_tmpstrng_23->data) ) ;
                  _mm_decref ( _mm_uv_gvar_tmpstrng_23 ) ;
                  (_mm_monitor.l2) = _mm_uv_mvar_n_10 ;
                  goto __mm_label_arbmon_X1_ArBmOn_X;
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
        
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).hole).n) ;
          int _mm_uv_mvar_n_3 = __mm_evfref_S2_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              goto __mm_label_arbmon_X1_ArBmOn_MB;
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
      
      if((__mma_strm_tlen_S2 == 0))
      {
        goto __mm_label_arbmon_X1_ArBmOn_M1;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X2/M1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X2","M1" ) ;
    break;
    __mm_label_arbmon_X2_ArBmOn_M2:
    {
      __mma_strm_tlen_S2 = shm_arbiter_buffer_peek ( __mma_strm_buf_S2,1,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) ;
      __mma_strm_flen_S2 = (__mma_strm_ilen_S2 + __mma_strm_blen_S2) ;
      if((__mma_strm_flen_S2 >= 1))
      {
        _mm_strm_out_S2 * __mm_evref_S2_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_bstrt_S2 ) ;
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).E).n) ;
          int _mm_uv_mvar_n_2 = __mm_evfref_S2_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_2 ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              int _mm_uv_mvar_n_11 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_11 == __mm_arbiter_yieldvar_n))
              {
                (_mm_monitor.l2) = _mm_uv_mvar_n_11 ;
                goto __mm_label_arbmon_X1_ArBmOn_X;
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
        
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).hole).n) ;
          int _mm_uv_mvar_n_3 = __mm_evfref_S2_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              goto __mm_label_arbmon_X1_ArBmOn_M2;
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
      
      if((__mma_strm_tlen_S2 == 0))
      {
        goto __mm_label_arbmon_X1_ArBmOn_M2;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X2/M2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X2","M2" ) ;
    break;
    __mm_label_arbmon_X2_ArBmOn_MB:
    {
      __mma_strm_tlen_S2 = shm_arbiter_buffer_peek ( __mma_strm_buf_S2,1,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) ;
      __mma_strm_flen_S2 = (__mma_strm_ilen_S2 + __mma_strm_blen_S2) ;
      if((__mma_strm_flen_S2 >= 1))
      {
        _mm_strm_out_S2 * __mm_evref_S2_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_bstrt_S2 ) ;
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).E).n) ;
          int _mm_uv_mvar_n_2 = __mm_evfref_S2_0_n ;
          int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_2 ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              int _mm_uv_mvar_n_15 = __mm_arbiter_yieldvar_n ;
              if((_mm_uv_mvar_n_15 == __mm_arbiter_yieldvar_n))
              {
                (_mm_monitor.l2) = _mm_uv_mvar_n_15 ;
                goto __mm_label_arbmon_X1_ArBmOn_M1;
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
        
        if((((__mm_evref_S2_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_S2_0_n = (((__mm_evref_S2_0->cases).hole).n) ;
          int _mm_uv_mvar_n_3 = __mm_evfref_S2_0_n ;
          __MM_BUFDROP ( __mma_strm_buf_S2,1,__mma_strm_tlen_S2,__mma_strm_flen_S2,__mma_strm_ilen_S2,__mma_strm_istrt_S2,__mma_strm_blen_S2,__mma_strm_bstrt_S2 ) ;
          {
            {
              goto __mm_label_arbmon_X1_ArBmOn_MB;
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
      
      if((__mma_strm_tlen_S2 == 0))
      {
        goto __mm_label_arbmon_X1_ArBmOn_MB;
      }
      else
      {
      }
      
      if(((__mma_strm_tlen_S1 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S1),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S1,0,((void * *)(&__mma_strm_istrt_S1)),(&__mma_strm_ilen_S1),((void * *)(&__mma_strm_bstrt_S1)),(&__mma_strm_blen_S1) ) == 0))))
      {
        if(((__mma_strm_tlen_S2 == 0) && (atomic_load_explicit ( (&__mm_strm_done_S2),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_S2,0,((void * *)(&__mma_strm_istrt_S2)),(&__mma_strm_ilen_S2),((void * *)(&__mma_strm_bstrt_S2)),(&__mma_strm_blen_S2) ) == 0))))
        {
          return 0 ;
        }
        else
        {
        }
        
      }
      else
      {
      }
      
    }
    printf ( "ERROR: Monitor could not match rule in state X2/MB\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"X2","MB" ) ;
    break;
  }
  return 1 ;
}
int main(int argc,char * * argv) {
  initialize_events ( ) ;
  _mm_source_control * __mm_strm_sourcecontrol_S1 ;
  atomic_init ( (&__mm_strm_done_S1),0 ) ;
  shm_stream * __mma_strm_strm_S1 = shm_stream_create ( "S1",(&__mm_strm_sourcecontrol_S1),argc,argv ) ;
  __mma_strm_buf_S1 = shm_arbiter_buffer_create ( __mma_strm_strm_S1,sizeof ( _mm_strm_out_S1 ),120 ) ;
  thrd_create ( (&__mm_strm_thread_S1),(&_mm_strm_fun_S1),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_S1,1 ) ;
  _mm_source_control * __mm_strm_sourcecontrol_S2 ;
  atomic_init ( (&__mm_strm_done_S2),0 ) ;
  shm_stream * __mma_strm_strm_S2 = shm_stream_create ( "S2",(&__mm_strm_sourcecontrol_S2),argc,argv ) ;
  __mma_strm_buf_S2 = shm_arbiter_buffer_create ( __mma_strm_strm_S2,sizeof ( _mm_strm_out_S2 ),120 ) ;
  thrd_create ( (&__mm_strm_thread_S2),(&_mm_strm_fun_S2),0 ) ;
  shm_arbiter_buffer_set_active ( __mma_strm_buf_S2,1 ) ;
  arbiterMonitor ( ) ;
  shm_arbiter_buffer_dump_stats(__mma_strm_buf_S1);
  shm_arbiter_buffer_dump_stats(__mma_strm_buf_S2);
  shm_stream_destroy ( __mma_strm_strm_S1 ) ;
  shm_stream_destroy ( __mma_strm_strm_S2 ) ;
  deinitialize_events ( ) ;
  return 0 ;
}
void _mm_print_streams( ) {
  _mm_print_strm_S1 ( ) ;
  _mm_print_strm_S2 ( ) ;
}

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
        assert(outevent && "Arbiter buffer is full");
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
        assert(outevent && "Arbiter buffer is full");
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
        assert(outevent && "Arbiter buffer is full");
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
        assert(outevent && "Arbiter buffer is full");
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
  _MMARBTP _mm_arbiter ;
  _MMMONTP _mm_monitor ;
  (_mm_arbiter.pre) = 0 ;
  (_mm_arbiter.seen) = 0 ;
  (_mm_arbiter.post) = 0 ;
  (_mm_arbiter.sl) = 0 ;
  (_mm_arbiter.sr) = 0 ;
  (_mm_arbiter.dl) = 0 ;
  (_mm_arbiter.dr) = 0 ;
  (_mm_arbiter.il) = 0 ;
  (_mm_arbiter.ir) = 0 ;
  (_mm_monitor.pbuf) = (int *) malloc(sizeof(int) * 3) ;
  (_mm_monitor.pl) = 0 ;
  (_mm_monitor.pr) = 0 ;
  while(__mm_monitor_running)
  {
    goto __mm_label_arbmon_L_ArBmOn_EQ;
    __mm_label_arbmon_L_ArBmOn_ERROR:
    printf ( "ERROR: Monitor moved to error state\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","ERROR" ) ;
    exit ( 1 ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_EQ:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_44 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_44->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_44 ) ;
              goto __mm_label_arbmon_Finished_ArBmOn_EQ;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_EQ;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_EQ;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_EQ;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_EQ;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_EQ;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_27 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_27 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_27 ;
                    goto __mm_label_arbmon_L_ArBmOn_Right1;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 0))
                {
                  goto __mm_label_arbmon_R_ArBmOn_EQ;
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
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_26 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_26 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_26 ;
                    goto __mm_label_arbmon_L_ArBmOn_Left1;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_26 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_26 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_26 ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_EQ;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_EQ;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_EQ;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_EQ;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_EQ;
    }
    printf ( "ERROR: Monitor could not match rule in state L/EQ\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","EQ" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Left1:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_45 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_45->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_45 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 1) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left1;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Left1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Left1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left1;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_29 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_29 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_29 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_EQ;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_46 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_46->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_46 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_47 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_47->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_47 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_EQ;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 1))
                {
                  goto __mm_label_arbmon_R_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_R_ArBmOn_Left1;
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
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 1))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 1))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_28 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_28 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_28 ;
                    goto __mm_label_arbmon_L_ArBmOn_Left2;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_28 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_28 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_28 ;
                      goto __mm_label_arbmon_L_ArBmOn_Left2;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Left1;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Left1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Left1" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Left2:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_48 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_48->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_48 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 2) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left2;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Left2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Left2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left2;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_32 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_32 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_32 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_49 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_49->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_49 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Left1;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_50 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_50->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_50 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Left1;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 2))
                {
                  goto __mm_label_arbmon_R_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 1))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                    ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                    goto __mm_label_arbmon_R_ArBmOn_Left1;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_R_ArBmOn_Left2;
                    }
                    else
                    {
                    }
                    
                  }
                  
                }
                
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 1))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 0))
                      {
                        goto __mm_label_arbmon_L_ArBmOn_Left2;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
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
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 1))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 0))
                      {
                        goto __mm_label_arbmon_L_ArBmOn_Left2;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_31 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_31 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_31 ;
                    goto __mm_label_arbmon_L_ArBmOn_Left3;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_31 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_31 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_31 ;
                      goto __mm_label_arbmon_L_ArBmOn_Left3;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Left2;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Left2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Left2" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Left3:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_51 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_51->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_51 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 3) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left3;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Left3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Left3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Left3;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_34 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_34 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_34 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left2;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_52 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_52->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_52 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Left2;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_53 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_53->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_53 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Left2;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 3))
                {
                  goto __mm_label_arbmon_R_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                    ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                    ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                    goto __mm_label_arbmon_R_ArBmOn_Left1;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 1))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_R_ArBmOn_Left2;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 0))
                      {
                        goto __mm_label_arbmon_R_ArBmOn_Left3;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
                }
                
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 3))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 2))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                        goto __mm_label_arbmon_L_ArBmOn_Left2;
                      }
                      else
                      {
                        if((__mm_arbiter_yieldvar_x == 0))
                        {
                          goto __mm_label_arbmon_L_ArBmOn_Left3;
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    
                  }
                  
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
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 3))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 2))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_L_ArBmOn_Left1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                        goto __mm_label_arbmon_L_ArBmOn_Left2;
                      }
                      else
                      {
                        if((__mm_arbiter_yieldvar_x == 0))
                        {
                          goto __mm_label_arbmon_L_ArBmOn_Left3;
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    
                  }
                  
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Left3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Left3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Left3;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Left3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Left3" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Right1:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_54 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_54->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_54 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 1) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right1;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Right1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Right1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right1;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_36 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_36 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_36 ;
                    goto __mm_label_arbmon_L_ArBmOn_Right2;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_37 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_37 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_37 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_EQ;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_55 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_55->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_55 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_56 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_56->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_56 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_EQ;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_37 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_37 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_37 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_EQ;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_57 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_57->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_57 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            goto __mm_label_arbmon_L_ArBmOn_EQ;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_58 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_58->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_58 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Right1;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Right1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Right1" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Right2:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_59 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_59->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_59 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 2) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right2;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Right2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Right2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right2;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_39 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_39 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_39 ;
                    goto __mm_label_arbmon_L_ArBmOn_Right3;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_40 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_40 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_40 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Right1;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_60 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_60->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_60 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Right1;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_61 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_61->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_61 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right1;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_40 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_40 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_40 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right1;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_62 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_62->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_62 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            goto __mm_label_arbmon_L_ArBmOn_Right1;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_63 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_63->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_63 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Right1;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Right2;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Right2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Right2" ) ;
    break;
    __mm_label_arbmon_L_ArBmOn_Right3:
    {
      if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
      {
        if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_64 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_64->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_64 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 3) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right3;
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
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_tlen_Left == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_R_ArBmOn_Right3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_L_ArBmOn_Right3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_4 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_4) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_4) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_5 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_5 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_5) ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_5) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_6 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) >= 1))
          {
            (_mm_arbiter.ir) = ((_mm_arbiter.ir) + 1) ;
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            goto __mm_label_arbmon_L_ArBmOn_Right3;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_7 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_7 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_8 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_8 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_8 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_8) ;
            __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_9 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_9 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_9 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_9) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_9) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_10 = __mm_evfref_Right_0_n ;
          if((_mm_uv_mvar_n_10 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_10 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_10 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + (_mm_uv_mvar_n_10 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.pre) = 0 ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_10) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_11 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_11 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_42 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_42 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_42 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Right2;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_65 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_65->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_65 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Right2;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_66 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_66->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_66 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right2;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_12 = __mm_evfref_Left_0_n ;
          if((__mma_strm_tlen_Right == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_12 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                {
                  {
                    int _mm_uv_mvar_n_42 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_42 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_42 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right2;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_67 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_67->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_67 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                            ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            goto __mm_label_arbmon_L_ArBmOn_Right2;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_68 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_68->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_68 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_L_ArBmOn_Right2;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_13 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_14 = __mm_evfref_Left_0_n ;
          if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              goto __mm_label_arbmon_L_ArBmOn_Right3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_14) ;
                (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_14) ;
                __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
                goto __mm_label_arbmon_L_ArBmOn_Right3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_L_ArBmOn_Right3;
    }
    printf ( "ERROR: Monitor could not match rule in state L/Right3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"L","Right3" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_ERROR:
    printf ( "ERROR: Monitor moved to error state\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","ERROR" ) ;
    exit ( 1 ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_EQ:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_69 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_69->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_69 ) ;
              goto __mm_label_arbmon_Finished_ArBmOn_EQ;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_EQ;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_EQ;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_EQ;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_EQ;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_EQ;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_26 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_26 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_26 ;
                    goto __mm_label_arbmon_R_ArBmOn_Left1;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 0))
                {
                  goto __mm_label_arbmon_L_ArBmOn_EQ;
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
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_R_ArBmOn_EQ;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_27 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_27 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_27 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right1;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_27 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_27 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_27 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right1;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_27 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_27 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_27 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right1;
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_27 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_27 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = _mm_uv_mvar_n_27 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right1;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_EQ;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_EQ;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_EQ;
    }
    printf ( "ERROR: Monitor could not match rule in state R/EQ\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","EQ" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Left1:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_70 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_70->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_70 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 1) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left1;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Left1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Left1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left1;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_28 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_28 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_28 ;
                    goto __mm_label_arbmon_R_ArBmOn_Left2;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_29 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_29 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_29 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_EQ;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_71 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_71->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_71 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_72 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_72->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_72 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_EQ;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_29 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_29 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_29 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_EQ;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_73 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_73->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_73 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_EQ;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_74 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_74->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_74 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_29 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_29 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_29 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_EQ;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_75 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_75->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_75 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_76 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_76->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_76 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_EQ;
                      }
                      else
                      {
                      }
                      
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_29 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_29 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_29 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_EQ;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_77 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_77->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_77 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_EQ;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_30 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_30 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_30 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_78 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_30 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_78->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_78 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Left1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Left1;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Left1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Left1" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Left2:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_79 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_79->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_79 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 2) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left2;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Left2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Left2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left2;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_31 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_31 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_31 ;
                    goto __mm_label_arbmon_R_ArBmOn_Left3;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_32 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_32 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_32 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Left1;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_80 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_80->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_80 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left1;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_81 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_81->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_81 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left1;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_32 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_32 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_32 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left1;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_82 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_82->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_82 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_Left1;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_83 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_83->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_83 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left1;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_32 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_32 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_32 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Left1;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_84 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_84->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_84 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left1;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_85 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_85->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_85 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left1;
                      }
                      else
                      {
                      }
                      
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_32 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_32 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_32 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left1;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_86 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_86->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_86 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_Left1;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_33 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_33 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_33 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_87 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_33 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_87->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_87 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left1;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Left2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Left2;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Left2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Left2" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Left3:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_88 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_88->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_88 ) ;
              (_mm_arbiter.sl) = ((_mm_arbiter.sl) + 3) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Left3;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Left3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Left3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Left3;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_34 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_34 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_34 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Left2;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_89 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_89->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_89 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left2;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_90 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_90->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_90 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left2;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_34 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_34 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_34 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left2;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_91 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_91->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_91 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                            ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_Left2;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_92 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_92->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_92 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left2;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_34 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_34 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_34 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Left2;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_93 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_93->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_93 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left2;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_94 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_94->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_94 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left2;
                      }
                      else
                      {
                      }
                      
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_34 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_34 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_34 == ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Left2;
                      }
                      else
                      {
                        int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                        if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                        {
                          if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                          {
                            _mm_gco _mm_uv_gvar_tmpstrng_95 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                            printf ( "%s\n",(_mm_uv_gvar_tmpstrng_95->data) ) ;
                            _mm_decref ( _mm_uv_gvar_tmpstrng_95 ) ;
                            ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                            ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                            ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                            (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                            (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                            goto __mm_label_arbmon_R_ArBmOn_Left2;
                          }
                          else
                          {
                          }
                          
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    else
                    {
                      int _mm_uv_mvar_n_35 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_35 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_35 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_96 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_35 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_96->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_96 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Left2;
                        }
                        else
                        {
                        }
                        
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Left3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Left3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Left3;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Left3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Left3" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Right1:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_97 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_97->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_97 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 1) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right1;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Right1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Right1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right1;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right1;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right1;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_37 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_37 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_37 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_EQ;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_98 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_98->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_98 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_EQ;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_38 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_38 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_38 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_99 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_38 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_99->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_99 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_EQ;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 1))
                {
                  goto __mm_label_arbmon_L_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_Right1;
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
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 1))
                  {
                    goto __mm_label_arbmon_R_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_R_ArBmOn_Right1;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 1))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_L_ArBmOn_Right1;
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_36 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_36 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_36 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right2;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_36 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_36 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_36 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right2;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_36 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_36 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_36 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right2;
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_36 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_36 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 1 ]) = _mm_uv_mvar_n_36 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right2;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right1;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Right1;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Right1;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Right1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Right1" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Right2:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_100 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_100->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_100 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 2) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right2;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Right2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Right2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right2;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right2;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right2;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_40 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_40 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_40 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Right1;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_101 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_101->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_101 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Right1;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_41 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_41 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_41 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_102 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_41 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_102->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_102 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Right1;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 2))
                {
                  goto __mm_label_arbmon_L_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 0))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_Right2;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 1))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      goto __mm_label_arbmon_L_ArBmOn_Right1;
                    }
                    else
                    {
                    }
                    
                  }
                  
                }
                
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    goto __mm_label_arbmon_R_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_R_ArBmOn_Right2;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Right1;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
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
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 0))
                    {
                      goto __mm_label_arbmon_L_ArBmOn_Right2;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right1;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_39 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_39 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_39 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right3;
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_39 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_39 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_39 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right3;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                  int _mm_uv_mvar_n_39 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_39 == __mm_arbiter_yieldvar_n))
                  {
                    ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_39 ;
                    goto __mm_label_arbmon_R_ArBmOn_Right3;
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
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
                    int _mm_uv_mvar_n_39 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_39 == __mm_arbiter_yieldvar_n))
                    {
                      ((_mm_monitor.pbuf) [ 2 ]) = _mm_uv_mvar_n_39 ;
                      goto __mm_label_arbmon_R_ArBmOn_Right3;
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
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right2;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Right2;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Right2;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Right2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Right2" ) ;
    break;
    __mm_label_arbmon_R_ArBmOn_Right3:
    {
      if(((__mma_strm_tlen_Right == 0) && (atomic_load_explicit ( (&__mm_strm_done_Right),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Right,0,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) == 0))))
      {
        if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
        {
          {
            {
              _mm_gco _mm_uv_gvar_tmpstrng_103 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "Done! Dropped " ),_mm_lib_int_to_string ( (_mm_arbiter.dl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.dr) ) ),_mm_lib_make_string ( " (Right); skipped " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.sr) ) ),_mm_lib_make_string ( " (Right); ignored " ) ),_mm_lib_int_to_string ( (_mm_arbiter.il) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_arbiter.ir) ) ),_mm_lib_make_string ( " (Right); processed " ) ),_mm_lib_int_to_string ( (_mm_monitor.pl) ) ),_mm_lib_make_string ( " (Left) and " ) ),_mm_lib_int_to_string ( (_mm_monitor.pr) ) ),_mm_lib_make_string ( " (Right)" ) ) ;
              printf ( "%s\n",(_mm_uv_gvar_tmpstrng_103->data) ) ;
              _mm_decref ( _mm_uv_gvar_tmpstrng_103 ) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + 3) ;
              goto __mm_label_arbmon_Finished_ArBmOn_Right3;
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
      
      __mma_strm_tlen_Right = shm_arbiter_buffer_peek ( __mma_strm_buf_Right,1,((void * *)(&__mma_strm_istrt_Right)),(&__mma_strm_ilen_Right),((void * *)(&__mma_strm_bstrt_Right)),(&__mma_strm_blen_Right) ) ;
      __mma_strm_flen_Right = (__mma_strm_ilen_Right + __mma_strm_blen_Right) ;
      if((__mma_strm_tlen_Right == 0))
      {
        if(((_mm_arbiter.pre) == 0))
        {
          if(((_mm_arbiter.post) == 0))
          {
            if(((_mm_arbiter.seen) == 0))
            {
              goto __mm_label_arbmon_L_ArBmOn_Right3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if(((_mm_arbiter.pre) == 0))
      {
        if(((_mm_arbiter.post) > 0))
        {
          if(((_mm_arbiter.seen) == 0))
          {
            (_mm_arbiter.pre) = (_mm_arbiter.post) ;
            (_mm_arbiter.post) = 0 ;
            goto __mm_label_arbmon_R_ArBmOn_Right3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_15 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.post) == 0))
            {
              (_mm_arbiter.pre) = ((_mm_arbiter.pre) + _mm_uv_mvar_n_15) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_15) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right3;
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      __mma_strm_tlen_Left = shm_arbiter_buffer_peek ( __mma_strm_buf_Left,1,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) ;
      __mma_strm_flen_Left = (__mma_strm_ilen_Left + __mma_strm_blen_Left) ;
      if((__mma_strm_flen_Left >= 1))
      {
        _mm_strm_out_Left * __mm_evref_Left_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_bstrt_Left ) ;
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_16 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_16 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - _mm_uv_mvar_n_16) ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_16) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right3;
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_LPrime))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).LPrime).n) ;
          int _mm_uv_mvar_n_17 = __mm_evfref_Left_0_n ;
          if((1 <= (_mm_arbiter.pre)))
          {
            (_mm_arbiter.pre) = ((_mm_arbiter.pre) - 1) ;
            (_mm_arbiter.il) = ((_mm_arbiter.il) + 1) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            goto __mm_label_arbmon_R_ArBmOn_Right3;
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_18 = __mm_evfref_Left_0_n ;
          if(((_mm_arbiter.pre) == 0))
          {
            if(((_mm_arbiter.seen) >= 1))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_18 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  int _mm_uv_mvar_n_42 = __mm_arbiter_yieldvar_n ;
                  if((_mm_uv_mvar_n_42 == __mm_arbiter_yieldvar_n))
                  {
                    if((_mm_uv_mvar_n_42 == ((_mm_monitor.pbuf) [ 0 ])))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                      (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                      goto __mm_label_arbmon_R_ArBmOn_Right2;
                    }
                    else
                    {
                      int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                      if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                      {
                        if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                        {
                          _mm_gco _mm_uv_gvar_tmpstrng_104 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                          printf ( "%s\n",(_mm_uv_gvar_tmpstrng_104->data) ) ;
                          _mm_decref ( _mm_uv_gvar_tmpstrng_104 ) ;
                          ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                          ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                          ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                          (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                          (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                          goto __mm_label_arbmon_R_ArBmOn_Right2;
                        }
                        else
                        {
                        }
                        
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  else
                  {
                    int _mm_uv_mvar_n_43 = __mm_arbiter_yieldvar_n ;
                    if((_mm_uv_mvar_n_43 == __mm_arbiter_yieldvar_n))
                    {
                      if((_mm_uv_mvar_n_43 != ((_mm_monitor.pbuf) [ 0 ])))
                      {
                        _mm_gco _mm_uv_gvar_tmpstrng_105 = _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_string_concat ( _mm_lib_make_string ( "ERROR! " ),_mm_lib_int_to_string ( _mm_uv_mvar_n_43 ) ),_mm_lib_make_string ( " is not equal to " ) ),_mm_lib_int_to_string ( ((_mm_monitor.pbuf) [ 0 ]) ) ) ;
                        printf ( "%s\n",(_mm_uv_gvar_tmpstrng_105->data) ) ;
                        _mm_decref ( _mm_uv_gvar_tmpstrng_105 ) ;
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 1) ;
                        (_mm_monitor.pl) = ((_mm_monitor.pl) + 1) ;
                        (_mm_monitor.pr) = ((_mm_monitor.pr) + 1) ;
                        goto __mm_label_arbmon_R_ArBmOn_Right2;
                      }
                      else
                      {
                      }
                      
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Left_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Left_0_n = (((__mm_evref_Left_0->cases).hole).n) ;
          int _mm_uv_mvar_n_19 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_19 >= (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
          {
            int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
            (_mm_arbiter.pre) = (_mm_uv_mvar_n_19 - (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))) ;
            (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
            (_mm_arbiter.post) = 0 ;
            (_mm_arbiter.seen) = 0 ;
            (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_19) ;
            __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
            {
              {
                if((__mm_arbiter_yieldvar_x == 3))
                {
                  goto __mm_label_arbmon_L_ArBmOn_EQ;
                }
                else
                {
                  if((__mm_arbiter_yieldvar_x == 2))
                  {
                    ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                    ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                    ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                    goto __mm_label_arbmon_L_ArBmOn_Right1;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 1))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_L_ArBmOn_Right2;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 0))
                      {
                        goto __mm_label_arbmon_L_ArBmOn_Right3;
                      }
                      else
                      {
                      }
                      
                    }
                    
                  }
                  
                }
                
              }
            }
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_20 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_20 >= ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
          {
            if((_mm_uv_mvar_n_20 < (((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_arbiter.seen) ;
              (_mm_arbiter.pre) = ((((_mm_arbiter.pre) + (_mm_arbiter.seen)) + (_mm_arbiter.post)) - _mm_uv_mvar_n_20) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_arbiter.seen)) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.seen) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_20) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 3))
                  {
                    goto __mm_label_arbmon_R_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 2))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_R_ArBmOn_Right1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                        goto __mm_label_arbmon_R_ArBmOn_Right2;
                      }
                      else
                      {
                        if((__mm_arbiter_yieldvar_x == 0))
                        {
                          goto __mm_label_arbmon_R_ArBmOn_Right3;
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    
                  }
                  
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
          
          int _mm_uv_mvar_n_21 = __mm_evfref_Left_0_n ;
          if((_mm_uv_mvar_n_21 >= (_mm_arbiter.pre)))
          {
            if((_mm_uv_mvar_n_21 < ((_mm_arbiter.pre) + (_mm_arbiter.seen))))
            {
              int __mm_arbiter_yieldvar_x = (_mm_uv_mvar_n_21 - (_mm_arbiter.pre)) ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) - (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.sr) = ((_mm_arbiter.sr) + (_mm_uv_mvar_n_21 - (_mm_arbiter.pre))) ;
              (_mm_arbiter.post) = 0 ;
              (_mm_arbiter.dl) = ((_mm_arbiter.dl) + _mm_uv_mvar_n_21) ;
              __MM_BUFDROP ( __mma_strm_buf_Left,1,__mma_strm_tlen_Left,__mma_strm_flen_Left,__mma_strm_ilen_Left,__mma_strm_istrt_Left,__mma_strm_blen_Left,__mma_strm_bstrt_Left ) ;
              {
                {
                  if((__mm_arbiter_yieldvar_x == 3))
                  {
                    goto __mm_label_arbmon_L_ArBmOn_EQ;
                  }
                  else
                  {
                    if((__mm_arbiter_yieldvar_x == 2))
                    {
                      ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                      ((_mm_monitor.pbuf) [ 1 ]) = (- 1) ;
                      ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                      goto __mm_label_arbmon_L_ArBmOn_Right1;
                    }
                    else
                    {
                      if((__mm_arbiter_yieldvar_x == 1))
                      {
                        ((_mm_monitor.pbuf) [ 0 ]) = ((_mm_monitor.pbuf) [ 1 ]) ;
                        ((_mm_monitor.pbuf) [ 1 ]) = ((_mm_monitor.pbuf) [ 2 ]) ;
                        ((_mm_monitor.pbuf) [ 2 ]) = (- 2) ;
                        goto __mm_label_arbmon_L_ArBmOn_Right2;
                      }
                      else
                      {
                        if((__mm_arbiter_yieldvar_x == 0))
                        {
                          goto __mm_label_arbmon_L_ArBmOn_Right3;
                        }
                        else
                        {
                        }
                        
                      }
                      
                    }
                    
                  }
                  
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
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      if((__mma_strm_flen_Right >= 1))
      {
        _mm_strm_out_Right * __mm_evref_Right_0 = _MM_EVACCESS ( 0,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_bstrt_Right ) ;
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_RPrime))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).RPrime).n) ;
          int _mm_uv_mvar_n_22 = __mm_evfref_Right_0_n ;
          if(((_mm_arbiter.seen) == 0))
          {
            if(((_mm_arbiter.pre) == 0))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_22 ;
              (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
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
          
          int _mm_uv_mvar_n_23 = __mm_evfref_Right_0_n ;
          if((__mma_strm_tlen_Left == 0))
          {
            if(((_mm_arbiter.seen) < 3))
            {
              if(((_mm_arbiter.post) == 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_23 ;
                (_mm_arbiter.seen) = ((_mm_arbiter.seen) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
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
            
          }
          else
          {
          }
          
          int _mm_uv_mvar_n_24 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) >= 3))
            {
              int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
              (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              {
                {
                }
              }
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                int __mm_arbiter_yieldvar_n = _mm_uv_mvar_n_24 ;
                (_mm_arbiter.post) = ((_mm_arbiter.post) + 1) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                {
                  {
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
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
        if((((__mm_evref_Right_0->head).kind) == __MM_EVENTCONST_ENUM_hole))
        {
          int __mm_evfref_Right_0_n = (((__mm_evref_Right_0->cases).hole).n) ;
          int _mm_uv_mvar_n_25 = __mm_evfref_Right_0_n ;
          if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
          {
            if(((_mm_arbiter.seen) > 0))
            {
              (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
              (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
              __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
              goto __mm_label_arbmon_R_ArBmOn_Right3;
            }
            else
            {
            }
            
            if(((__mma_strm_tlen_Left == 0) && (atomic_load_explicit ( (&__mm_strm_done_Left),memory_order_acquire ) && (shm_arbiter_buffer_peek ( __mma_strm_buf_Left,0,((void * *)(&__mma_strm_istrt_Left)),(&__mma_strm_ilen_Left),((void * *)(&__mma_strm_bstrt_Left)),(&__mma_strm_blen_Left) ) == 0))))
            {
              if(((_mm_arbiter.post) > 0))
              {
                (_mm_arbiter.post) = ((_mm_arbiter.post) + _mm_uv_mvar_n_25) ;
                (_mm_arbiter.dr) = ((_mm_arbiter.dr) + _mm_uv_mvar_n_25) ;
                __MM_BUFDROP ( __mma_strm_buf_Right,1,__mma_strm_tlen_Right,__mma_strm_flen_Right,__mma_strm_ilen_Right,__mma_strm_istrt_Right,__mma_strm_blen_Right,__mma_strm_bstrt_Right ) ;
                goto __mm_label_arbmon_R_ArBmOn_Right3;
              }
              else
              {
              }
              
            }
            else
            {
            }
            
          }
          else
          {
          }
          
        }
        else
        {
        }
        
      }
      else
      {
      }
      
      goto __mm_label_arbmon_R_ArBmOn_Right3;
    }
    printf ( "ERROR: Monitor could not match rule in state R/Right3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"R","Right3" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_ERROR:
    printf ( "ERROR: Monitor moved to error state\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","ERROR" ) ;
    exit ( 1 ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_EQ:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/EQ\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","EQ" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Left1:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Left1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Left1" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Left2:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Left2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Left2" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Left3:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Left3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Left3" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Right1:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Right1\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Right1" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Right2:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Right2\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Right2" ) ;
    break;
    __mm_label_arbmon_Finished_ArBmOn_Right3:
    {
      return 0 ;
    }
    printf ( "ERROR: Monitor could not match rule in state Finished/Right3\n" ) ;
    _mm_print_state ( (&_mm_arbiter),(&_mm_monitor),"Finished","Right3" ) ;
    break;
  }
  return 1 ;
}
int main(int argc,char * * argv) {
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

  shm_arbiter_buffer_dump_stats ( __mma_strm_buf_Left) ;
  shm_arbiter_buffer_dump_stats ( __mma_strm_buf_Right) ;

  shm_stream_destroy ( __mma_strm_strm_Left ) ;
  shm_stream_destroy ( __mma_strm_strm_Right ) ;
  deinitialize_events ( ) ;
  return 0 ;
}
void _mm_print_streams( ) {
  _mm_print_strm_Left ( ) ;
  _mm_print_strm_Right ( ) ;
}

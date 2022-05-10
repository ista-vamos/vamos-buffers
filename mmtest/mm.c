#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include "../gen/shamon.h"
typedef enum __MM_STREAMCONST_ENUM { __MM_STREAMCONST_ENUM_S } _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM { __MM_EVENTCONST_NOTHING, __MM_EVENTCONST_ENUM_hole, __MM_EVENTCONST_ENUM_E } _MM_EVENTCONST_ENUM;
typedef struct __MMTP_A _MMTP_A;
typedef struct __MMEV_E _MMEV_E;
int F(int _mm_uv_arg_n);
size_t __mm_arbiter_strmvar_initlen_S = 0 ;
size_t __mm_arbiter_strmvar_breaklen_S = 0 ;
size_t __mm_arbiter_strmvar_totallen_S = 0 ;
size_t __mm_arbiter_strmvar_fetchedlen_S = 0 ;
thrd_t __mm_strmvar_thread_S;
shm_arbiter_buffer * __mm_arbiter_strmvar_buffer_S;
typedef struct __mm_strm_in_S _mm_strm_in_S;
typedef struct __mm_strm_out_S _mm_strm_out_S;
typedef struct __mm_strm_hole_S _mm_strm_hole_S;
_mm_strm_out_S * __mm_arbiter_strmvar_initstart_S = 0 ;
_mm_strm_out_S * __mm_arbiter_strmvar_breakstart_S = 0 ;
struct __MMTP_A
{
  _MMTP_A * x ;
  int b ;
};
struct __MMEV_E
{
  int z ;
  int w ;
};
int F(int _mm_uv_arg_n)
{
  return ((5 + 2) + _mm_uv_arg_n) ;
}
struct __mm_strm_hole_S
{
  int n ;
};
struct __mm_strm_in_S
{
  shm_event head ;
  union {
    _MMEV_E E ;
  } cases;
};
struct __mm_strm_out_S
{
  shm_event head ;
  union {
    _mm_strm_hole_S hole ;
    _MMEV_E E ;
  } cases;
};
int _mm_strm_fun_S(void * arg)
{
  shm_arbiter_buffer * buffer = __mm_arbiter_strmvar_buffer_S ;
  shm_stream * stream = shm_arbiter_buffer_stream ( buffer ) ;
  const _mm_strm_in_S * inevent ;
  _mm_strm_out_S * outevent ;
  while ((! shm_arbiter_buffer_active ( buffer )))
  {
    sleep ( 100 ) ;
  }
  while (1)
  {
    inevent = stream_fetch ( stream,buffer ) ;
    if ((inevent == 0))
    {
    }
    else
    {
    }
    switch (((inevent->head).kind))
    {
      case __MM_EVENTCONST_ENUM_E:
          {
            int _mm_uv_mvar_x = (((inevent->cases).E).z) ;
            int _mm_uv_mvar_y = (((inevent->cases).E).w) ;
            if ((_mm_uv_mvar_x > _mm_uv_mvar_y))
            {
              if ((((&((inevent->cases).E))->z) == 1))
              {
                int _mm_uv_mvar_y = (((inevent->cases).E).w) ;
                if ((_mm_uv_mvar_y > 5))
                {
                  outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
                  ((outevent->head).kind) = __MM_EVENTCONST_ENUM_E ;
                  (((outevent->cases).E).z) = 2 ;
                  (((outevent->cases).E).w) = _mm_uv_mvar_y ;
                  shm_arbiter_buffer_write_finish ( buffer ) ;
                }
                else
                {
                  outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
                  memcpy ( outevent,inevent,sizeof ( _MMEV_E ) ) ;
                  shm_arbiter_buffer_write_finish ( buffer ) ;
                }
              }
              else
              {
                outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
                memcpy ( outevent,inevent,sizeof ( _MMEV_E ) ) ;
                shm_arbiter_buffer_write_finish ( buffer ) ;
              }
            }
            else
            {
            }
            shm_stream_consume ( stream,1 ) ;
            continue;
          }
      default:
        {
          outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
          memcpy ( outevent,inevent,sizeof ( _mm_strm_in_S ) ) ;
          shm_arbiter_buffer_write_finish ( buffer ) ;
          shm_stream_consume ( stream,1 ) ;
        }
    }
  }
}
int __mm_monitor_running = 1 ;
typedef struct __MMARBTP _MMARBTP;
typedef struct __MMMONTP _MMMONTP;
struct __MMARBTP
{
  int y ;
  int * z ;
};
struct __MMMONTP
{
};
int arbiterMonitor( )
{
  _MMARBTP _mm_arbiter ;
  _MMMONTP _mm_monitor ;
  (_mm_arbiter.y) = (5 + 3) ;
  (_mm_arbiter.z) = (int *) malloc(sizeof(int) * 2) ;
  while (__mm_monitor_running)
  {
    goto __mm_label_arbmon_AS_ArBmOn_MS;
    __mm_label_arbmon_AS_ArBmOn_MS:
    {
      __mm_arbiter_strmvar_totallen_S = shm_arbiter_buffer_peek ( __mm_arbiter_strmvar_buffer_S,1,((void * *)(&__mm_arbiter_strmvar_initstart_S)),(&__mm_arbiter_strmvar_initlen_S),((void * *)(&__mm_arbiter_strmvar_breakstart_S)),(&__mm_arbiter_strmvar_breaklen_S) ) ;
      __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_initlen_S + __mm_arbiter_strmvar_breaklen_S) ;
      if ((__mm_arbiter_strmvar_fetchedlen_S >= 1))
      {
        _mm_strm_out_S * __mm_evref_S_0 = (((__mm_arbiter_strmvar_initlen_S > 0))?((__mm_arbiter_strmvar_initstart_S + 0)):((__mm_arbiter_strmvar_breakstart_S + (0 - __mm_arbiter_strmvar_initlen_S)))) ;
        if ((((__mm_evref_S_0->head).kind) == __MM_EVENTCONST_ENUM_E))
        {
          int __mm_evfref_S_0_z = (((__mm_evref_S_0->cases).E).z) ;
          if ((__mm_evfref_S_0_z == (_mm_arbiter.y)))
          {
            int __mm_evfref_S_0_w = (((__mm_evref_S_0->cases).E).w) ;
            int _mm_uv_mvar_x = __mm_evfref_S_0_w ;
            if (((_mm_arbiter.y) == _mm_uv_mvar_x))
            {
              (_mm_arbiter.y) = 2 ;
              shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
              __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
              __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
              if ((__mm_arbiter_strmvar_initlen_S >= 1))
              {
                __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
              }
              else
              {
                __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
              }
              goto __mm_label_arbmon_AS_ArBmOn_MS;
            }
            else
            {
              if ((__mm_evfref_S_0_z == 2))
              {
                if ((__mm_evfref_S_0_w == 1))
                {
                  if (((_mm_arbiter.y) == 2))
                  {
                    (_mm_arbiter.y) = 1 ;
                    shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                    __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                    __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                    if ((__mm_arbiter_strmvar_initlen_S >= 1))
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                    }
                    else
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                    }
                    goto __mm_label_arbmon_AS_ArBmOn_MS;
                  }
                  else
                  {
                  }
                }
                else
                {
                  if ((__mm_evfref_S_0_w == 2))
                  {
                    if (((_mm_arbiter.y) == 2))
                    {
                      (_mm_arbiter.y) = 1 ;
                      shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                      __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                      __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                      if ((__mm_arbiter_strmvar_initlen_S >= 1))
                      {
                        __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                        __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                      }
                      else
                      {
                        __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                        __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                      }
                      goto __mm_label_arbmon_AS_ArBmOn_MS;
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
                if ((__mm_evfref_S_0_z == 1))
                {
                  if ((__mm_evfref_S_0_w == 2))
                  {
                    if (((_mm_arbiter.y) == 1))
                    {
                      (_mm_arbiter.y) = 2 ;
                      shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                      __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                      __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                      if ((__mm_arbiter_strmvar_initlen_S >= 1))
                      {
                        __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                        __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                      }
                      else
                      {
                        __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                        __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                      }
                      goto __mm_label_arbmon_AS_ArBmOn_MS;
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
            }
          }
          else
          {
            if ((__mm_evfref_S_0_z == 2))
            {
              int __mm_evfref_S_0_w = (((__mm_evref_S_0->cases).E).w) ;
              if ((__mm_evfref_S_0_w == 1))
              {
                if (((_mm_arbiter.y) == 2))
                {
                  (_mm_arbiter.y) = 1 ;
                  shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                  __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                  __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                  if ((__mm_arbiter_strmvar_initlen_S >= 1))
                  {
                    __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                    __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                  }
                  else
                  {
                    __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                    __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                  }
                  goto __mm_label_arbmon_AS_ArBmOn_MS;
                }
                else
                {
                }
              }
              else
              {
                if ((__mm_evfref_S_0_w == 2))
                {
                  if (((_mm_arbiter.y) == 2))
                  {
                    (_mm_arbiter.y) = 1 ;
                    shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                    __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                    __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                    if ((__mm_arbiter_strmvar_initlen_S >= 1))
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                    }
                    else
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                    }
                    goto __mm_label_arbmon_AS_ArBmOn_MS;
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
              if ((__mm_evfref_S_0_z == 1))
              {
                int __mm_evfref_S_0_w = (((__mm_evref_S_0->cases).E).w) ;
                if ((__mm_evfref_S_0_w == 2))
                {
                  if (((_mm_arbiter.y) == 1))
                  {
                    (_mm_arbiter.y) = 2 ;
                    shm_arbiter_buffer_drop ( __mm_arbiter_strmvar_buffer_S,1 ) ;
                    __mm_arbiter_strmvar_totallen_S = (__mm_arbiter_strmvar_totallen_S - 1) ;
                    __mm_arbiter_strmvar_fetchedlen_S = (__mm_arbiter_strmvar_fetchedlen_S - 1) ;
                    if ((__mm_arbiter_strmvar_initlen_S >= 1))
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_initlen_S - 1) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_initstart_S + 1) ;
                    }
                    else
                    {
                      __mm_arbiter_strmvar_initlen_S = (__mm_arbiter_strmvar_breaklen_S - (1 - __mm_arbiter_strmvar_initlen_S)) ;
                      __mm_arbiter_strmvar_initstart_S = (__mm_arbiter_strmvar_breakstart_S + (1 - __mm_arbiter_strmvar_initlen_S)) ;
                    }
                    goto __mm_label_arbmon_AS_ArBmOn_MS;
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
    break;
  }
}
int main(int argc,char * * argv)
{
  shm_stream * __mm_arbiter_strmvar_stream_S = shm_stream_create ( "S","some-format-of-stream-signature",argc,argv ) ;
  shm_arbiter_buffer_init ( __mm_arbiter_strmvar_buffer_S,__mm_arbiter_strmvar_stream_S,sizeof ( _mm_strm_in_S ),16 ) ;
  thrd_create ( (&__mm_strmvar_thread_S),(&_mm_strm_fun_S),0 ) ;
  shm_arbiter_buffer_set_active ( __mm_arbiter_strmvar_buffer_S,1 ) ;
  arbiterMonitor ( ) ;
  return 0 ;
}
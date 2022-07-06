#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include<signal.h>
#include "../gen/shamon.h"
#include "../gen/mmlib.h"
#ifndef SHMBUF_ARBITER_BUFSIZE
#define SHMBUF_ARBITER_BUFSIZE 64
#endif
typedef enum __MM_STREAMCONST_ENUM
{
  __MM_STREAMCONST_ENUM_Left,
  __MM_STREAMCONST_ENUM_Right
} _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM
{
  __MM_EVENTCONST_NOTHING,
  __MM_EVENTCONST_ENUM_hole,
  __MM_EVENTCONST_ENUM_Prime
} _MM_EVENTCONST_ENUM;
typedef struct source_control _mm_source_control;
void _mm_print_streams();
typedef struct __MMEV_Prime _MMEV_Prime;
typedef struct __MMEV_LSkip _MMEV_LSkip;
typedef struct __MMEV_RSkip _MMEV_RSkip;
typedef struct __MMEV_EFinal _MMEV_EFinal;
// size_t __mma_strm_ilen_Left = 0 ;
// size_t __mma_strm_blen_Left = 0 ;
// size_t __mma_strm_tlen_Left = 0 ;
// size_t __mma_strm_flen_Left = 0 ;
atomic_int __mm_strm_done_Left;
thrd_t __mm_strm_thread_Left;
shm_arbiter_buffer *__mma_strm_buf_Left;
typedef struct __mm_strm_in_Left _mm_strm_in_Left;
typedef struct __mm_strm_out _mm_strm_out;
typedef struct __mm_strm_hole _mm_strm_hole;
// _mm_strm_out * __mma_strm_istrt_Left = 0 ;
// _mm_strm_out * __mma_strm_bstrt_Left = 0 ;
void _mm_print_strm_Left();
// size_t __mma_strm_ilen_Right = 0 ;
// size_t __mma_strm_blen_Right = 0 ;
// size_t __mma_strm_tlen_Right = 0 ;
// size_t __mma_strm_flen_Right = 0 ;
atomic_int __mm_strm_done_Right;
thrd_t __mm_strm_thread_Right;
shm_arbiter_buffer *__mma_strm_buf_Right;
typedef struct __mm_strm_in_Right _mm_strm_in_Right;
// _mm_strm_out * __mma_strm_istrt_Right = 0 ;
// _mm_strm_out * __mma_strm_bstrt_Right = 0 ;
void _mm_print_strm_Right();
int _mmdbg_enqueued_left = 0;
int _mmdbg_enqueued_right = 0;
struct __MMEV_Prime
{
  int n;
  int p;
};
struct __mm_strm_hole
{
  uint64_t n;
};
struct __mm_strm_in_Left
{
  shm_event head;
  union
  {
    _MMEV_Prime Prime;
  } cases;
};
struct __mm_strm_out
{
  shm_event head;
  union
  {
    _mm_strm_hole hole;
    _MMEV_Prime Prime;
  } cases;
};

typedef struct _strmdata
{
  int ahead;
  shm_arbiter_buffer *buffer;
  int pre;
  int buffered;
  int bufpos;
  int post;
  _mm_strm_out *istrt;
  size_t ilen;
  _mm_strm_out *bstrt;
  size_t blen;
  size_t tlen;
  size_t flen;
  atomic_int *done;
  char *name;
  size_t processed;
  size_t dropped;
  size_t ignored;
  size_t skipped;
  size_t holes;
  int posn;
  int lastholesize;
} strmdata;

strmdata leftstrm;
strmdata rightstrm;

void initStrmdata(strmdata *data, shm_arbiter_buffer *buf, atomic_int *done, char *name)
{
  data->buffer = buf;
  data->ilen = 0;
  data->istrt = 0;
  data->blen = 0;
  data->bstrt = 0;
  data->tlen = 0;
  data->flen = 0;
  data->buffered = 0;
  data->bufpos=0;
  data->pre = 0;
  data->post = 0;
  data->ahead = 0;
  data->done = done;
  data->name = name;
  data->processed = 0;
  data->dropped = 0;
  data->ignored = 0;
  data->skipped = 0;
  data->holes = 0;
  data->posn = 0;
  data->lastholesize=-1;
}

void _mm_print_event_Left(const _mm_strm_out *ev)
{
  switch (((ev->head).kind))
  {
  case __MM_EVENTCONST_ENUM_hole:
  {
    printf("hole(%lu)\n", (((ev->cases).hole).n));
    break;
  }
  case __MM_EVENTCONST_ENUM_Prime:
  {
    printf("LPrime(");
    printf("%i", (((ev->cases).Prime).n));
    printf(")\n");
    break;
  }
  }
}
void _mm_print_inevent_Left(const _mm_strm_in_Left *ev)
{
  switch (((ev->head).kind))
  {
  case __MM_EVENTCONST_ENUM_Prime:
  {
    printf("Prime(");
    printf("%i", (((ev->cases).Prime).n));
    printf("%i", (((ev->cases).Prime).p));
    printf(")\n");
    break;
  }
  }
}
void _mm_print_strm_Left()
{
  _mm_strm_out *cur = leftstrm.istrt;
  printf("\nSTREAM Left:\n");
  while ((cur < (leftstrm.istrt + leftstrm.ilen)))
  {
    _mm_print_event_Left(cur);
    cur = (cur + 1);
  }
  cur = leftstrm.bstrt;
  while ((cur < (leftstrm.bstrt + leftstrm.blen)))
  {
    _mm_print_event_Left(cur);
    cur = (cur + 1);
  }
}
int _mm_strm_fun_Left(void *arg)
{
  shm_arbiter_buffer *buffer = __mma_strm_buf_Left;
  shm_stream *stream = shm_arbiter_buffer_stream(buffer);
  const _mm_strm_in_Left *inevent;
  _mm_strm_out *outevent;
  while ((!shm_arbiter_buffer_active(buffer)))
  {
    sleep_ns(10);
  }
  while (1)
  {
    inevent = stream_fetch(stream, buffer);
    if ((inevent == 0))
    {
      break;
    }
    else
    {
    }

    switch (((inevent->head).kind))
    {
      //   case __MM_EVENTCONST_ENUM_Prime:
      //   {
      //     int _mm_uv_mvar_n_0 = (((inevent->cases).Prime).n) ;
      //     int _mm_uv_mvar_p_1 = (((inevent->cases).Prime).p) ;
      //     outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
      //     /* NOTE: this copies also 'kind' which is then overwritten */
      //     memcpy ( outevent,inevent,sizeof ( shm_event ) ) ;
      //     ((outevent->head).kind) = __MM_EVENTCONST_ENUM_Prime ;
      //     (((outevent->cases).Prime).n) = _mm_uv_mvar_p_1 ;
      //     shm_arbiter_buffer_write_finish ( buffer ) ;
      //     _mmdbg_enqueued_left++;
      //     shm_stream_consume ( stream,1 ) ;
      //     continue;
      //   }
    default:
    {
      outevent = shm_arbiter_buffer_write_ptr(buffer);
      memcpy(outevent, inevent, sizeof(_mm_strm_in_Left));
      shm_arbiter_buffer_write_finish(buffer);
      _mmdbg_enqueued_left++;
      shm_stream_consume(stream, 1);
    }
    }
  }
  atomic_store((&__mm_strm_done_Left), 1);
  return 0;
}

struct __mm_strm_in_Right
{
  shm_event head;
  union
  {
    _MMEV_Prime Prime;
  } cases;
};

void _mm_print_event_Right(const _mm_strm_out *ev)
{
  switch (((ev->head).kind))
  {
  case __MM_EVENTCONST_ENUM_hole:
  {
    printf("hole(%lu)\n", (((ev->cases).hole).n));
    break;
  }
  case __MM_EVENTCONST_ENUM_Prime:
  {
    printf("RPrime(");
    printf("%i", (((ev->cases).Prime).n));
    printf(")\n");
    break;
  }
  }
}
void _mm_print_inevent_Right(const _mm_strm_in_Right *ev)
{
  switch (((ev->head).kind))
  {
  case __MM_EVENTCONST_ENUM_Prime:
  {
    printf("Prime(");
    printf("%i", (((ev->cases).Prime).n));
    printf("%i", (((ev->cases).Prime).p));
    printf(")\n");
    break;
  }
  }
}
void _mm_print_strm_Right()
{
  _mm_strm_out *cur = rightstrm.istrt;
  printf("\nSTREAM Right:\n");
  while ((cur < (rightstrm.istrt + rightstrm.ilen)))
  {
    _mm_print_event_Right(cur);
    cur = (cur + 1);
  }
  cur = rightstrm.bstrt;
  while ((cur < (rightstrm.bstrt + rightstrm.blen)))
  {
    _mm_print_event_Right(cur);
    cur = (cur + 1);
  }
}
int _mm_strm_fun_Right(void *arg)
{
  shm_arbiter_buffer *buffer = __mma_strm_buf_Right;
  shm_stream *stream = shm_arbiter_buffer_stream(buffer);
  const _mm_strm_in_Right *inevent;
  _mm_strm_out *outevent;
  while ((!shm_arbiter_buffer_active(buffer)))
  {
    sleep_ns(10);
  }
  while (1)
  {
    inevent = stream_fetch(stream, buffer);
    if ((inevent == 0))
    {
      break;
    }
    else
    {
    }

    switch (((inevent->head).kind))
    {
      //   case __MM_EVENTCONST_ENUM_Prime:
      //   {
      //     int _mm_uv_mvar_n_2 = (((inevent->cases).Prime).n) ;
      //     int _mm_uv_mvar_p_3 = (((inevent->cases).Prime).p) ;
      //     outevent = shm_arbiter_buffer_write_ptr ( buffer ) ;
      //     memcpy ( outevent,inevent,sizeof ( shm_event ) ) ;
      //     ((outevent->head).kind) = __MM_EVENTCONST_ENUM_Prime ;
      //     (((outevent->cases).Prime).n) = _mm_uv_mvar_p_3 ;
      //     shm_arbiter_buffer_write_finish ( buffer ) ;
      //     _mmdbg_enqueued_right++;
      //     shm_stream_consume ( stream,1 ) ;
      //     continue;
      //   }
    default:
    {
      outevent = shm_arbiter_buffer_write_ptr(buffer);
      memcpy(outevent, inevent, sizeof(_mm_strm_in_Right));
      shm_arbiter_buffer_write_finish(buffer);
      _mmdbg_enqueued_right++;
      shm_stream_consume(stream, 1);
    }
    }
  }
  atomic_store((&__mm_strm_done_Right), 1);
  return 0;
}
int __mm_monitor_running = 1;
typedef struct __MMARBTP _MMARBTP;
typedef struct __MMMONTP _MMMONTP;
struct __MMARBTP
{
  int pre;
  int seen;
  int post;
  int sl;
  int sr;
  int dl;
  int dr;
  int il;
  int ir;
};
struct __MMMONTP
{
  int *pbuf;
  int pl;
  int pr;
};
void _mm_print_state(_MMARBTP *arbiter, _MMMONTP *monitor, char *arbstate, char *monstate)
{
  printf("Arbiter (%s):\n", arbstate);
  printf("pre: %i\n", (arbiter->pre));
  printf("seen: %i\n", (arbiter->seen));
  printf("post: %i\n", (arbiter->post));
  printf("sl: %i\n", (arbiter->sl));
  printf("sr: %i\n", (arbiter->sr));
  printf("dl: %i\n", (arbiter->dl));
  printf("dr: %i\n", (arbiter->dr));
  printf("il: %i\n", (arbiter->il));
  printf("ir: %i\n", (arbiter->ir));
  printf("Monitor (%s):\n", monstate);
  printf("pl: %i\n", (monitor->pl));
  printf("pr: %i\n", (monitor->pr));
  _mm_print_streams();
}

#ifndef MM_MONITORBUFSIZE
#define MM_MONITORBUFSIZE 10
#endif
static FILE* logfile;
int arbiterMonitor()
{
  _MMARBTP _mm_arbiter;
  _MMMONTP _mm_monitor;
  (_mm_arbiter.pre) = 0;
  (_mm_arbiter.seen) = 0;
  (_mm_arbiter.post) = 0;
  (_mm_arbiter.sl) = 0;
  (_mm_arbiter.sr) = 0;
  (_mm_arbiter.dl) = 0;
  (_mm_arbiter.dr) = 0;
  (_mm_arbiter.il) = 0;
  (_mm_arbiter.ir) = 0;
  (_mm_monitor.pbuf) = (int *)malloc(sizeof(int) * 3);
  (_mm_monitor.pl) = 0;
  (_mm_monitor.pr) = 0;
  int monitorbuffer[MM_MONITORBUFSIZE]; // buffered events of faster program
  int fetchsize;

  initStrmdata(&leftstrm, __mma_strm_buf_Left, &__mm_strm_done_Left, "Left");
  initStrmdata(&rightstrm, __mma_strm_buf_Right, &__mm_strm_done_Right, "Right");

  strmdata *cur = &leftstrm;
  strmdata *other = &rightstrm;
  strmdata *swap = 0;

  while (__mm_monitor_running)
  {
    if (other->ahead)
    {
      fetchsize = cur->buffered + MM_MONITORBUFSIZE; // catch up
    }
    else if (cur->buffered == MM_MONITORBUFSIZE || cur->post > 0)
    {
      fetchsize = 1;
    }
    else
    {
      fetchsize = MM_MONITORBUFSIZE - cur->buffered;
    }
    cur->tlen = shm_arbiter_buffer_peek(cur->buffer, fetchsize, ((void **)&cur->istrt), &cur->ilen, ((void **)&cur->bstrt), &cur->blen);
    if (cur->tlen == 0 && atomic_load_explicit(cur->done, memory_order_acquire) && atomic_load_explicit(other->done, memory_order_acquire))
    {
      cur->tlen = shm_arbiter_buffer_peek(cur->buffer, fetchsize, ((void **)&cur->istrt), &cur->ilen, ((void **)&cur->bstrt), &cur->blen);
      other->tlen = shm_arbiter_buffer_peek(other->buffer, 1, ((void **)&other->istrt), &other->ilen, ((void **)&other->bstrt), &other->blen);
      if (cur->tlen == 0)
      {
        if (other->tlen == 0)
        {
          __mm_monitor_running = 0;
          break;
        }
        else
        {
          swap = cur;
          cur = other;
          other = swap;
          continue;
        }
      }
    }
    cur->flen = cur->ilen + cur->blen;
    int consumed = 0;
    if (cur->ilen > 0)
    {
      int icount = cur->ilen;
      int bcount = cur->blen;
      _mm_strm_out *curev = cur->istrt;
      strmdata *fetched = cur;
      while (icount > 0 && fetched == cur) //&&cur->buffered<MONITORBUFSIZE&&cur->post==0
      {
        if (curev->head.kind == __MM_EVENTCONST_ENUM_hole)
        {
          int holesize = curev->cases.hole.n;
          fprintf(logfile, "%s: hole of size %i @ %i\n", cur->name, holesize, (cur->posn+1));
          fflush(logfile);
          cur->lastholesize=holesize;
          cur->posn+=holesize;
          cur->dropped += holesize;
          cur->holes++;
          if (!other->ahead)
          {
            if (!cur->ahead)
            {
              cur->ahead = 1;
              cur->pre += holesize;
            }
            else
            {
              if (cur->buffered > 0)
              {
                cur->post += holesize;
              }
              else
              {
                cur->pre += holesize;
              }
            }
          }
          else
          {
            int opre = other->pre;
            int obuffered = other->buffered;
            int opost = other->post;
            if (holesize >= opre + obuffered + opost)
            {
              cur->ahead = (holesize == opre + obuffered + opost ? 0 : 1);
              other->ahead = 0;
              other->pre = 0;
              other->post = 0;
              other->skipped += other->buffered;
              other->buffered = 0;
              other->bufpos=0;
              cur->pre = holesize - (opre + obuffered + opost);
              consumed++;
              swap = cur;
              cur = other;
              other = swap;
              break;
            }
            else if (holesize >= opre + obuffered)
            {
              other->pre = opost - (holesize - (opre + obuffered));
              other->skipped += other->buffered;
              other->buffered = 0;
              other->bufpos=0;
              other->post = 0;
            }
            else if (holesize >= opre)
            {
              int bufmove = holesize - opre;
              other->skipped += bufmove;
              other->pre = 0;
              other->buffered -= bufmove;
              other->bufpos=(other->bufpos+bufmove)%MM_MONITORBUFSIZE;
            }
            else
            {
              other->pre -= holesize;
            }
          }
        }
        else
        {
          cur->posn++;
          if(cur->posn!=curev->cases.Prime.n)
          {
            fprintf(logfile, "Mismatched position in %s stream: %i @ #%i (expected #%i) after %lu holes (last hole size %i)\n", cur->name, curev->cases.Prime.p, curev->cases.Prime.n, cur->posn, cur->holes,cur->lastholesize);
            fflush(logfile);
            cur->posn=curev->cases.Prime.n;
          }
          cur->lastholesize=-1;
          if (!other->ahead)
          {
            if (cur->post > 0 || cur->buffered >= MM_MONITORBUFSIZE)
            {
              cur->post++;
              cur->ignored++;
            }
            else
            {
              monitorbuffer[(cur->buffered+cur->bufpos)%MM_MONITORBUFSIZE] = curev->cases.Prime.p;
              cur->buffered++;
              cur->ahead = 1;
            }
          }
          else
          {
            if (other->pre > 0)
            {
              other->pre--;
              cur->ignored++;
            }
            else if (other->buffered > 0) // match!
            {
              cur->processed += 1;
              other->processed += 1;
              if (monitorbuffer[other->bufpos] != curev->cases.Prime.p)
              {
                // printf("Found error: expected %i on %s, but found %i\n", monitorbuffer[0], cur->name, curev->cases.Prime.p);
                printf("ERROR! %i is not equal to %i\n", monitorbuffer[other->bufpos], curev->cases.Prime.p);
                fprintf(logfile, "ERROR! %i is not equal to %i\n", monitorbuffer[other->bufpos], curev->cases.Prime.p);
                fflush(logfile);
              }
              other->buffered--;
              other->bufpos=(other->bufpos+1)%MM_MONITORBUFSIZE;
            }
            else if (other->post > 0)
            {
              other->pre = other->post - 1;
              other->post = 0;
              cur->ignored++;
            }
            else
            {
              printf("COUNTING ERROR!\n");
              fprintf(logfile, "COUNTING ERROR!\n");
              exit(-1);
            }
            if (other->pre == 0 && other->buffered == 0)
            {
              if (other->post == 0)
              {
                other->ahead = 0;
              }
              else
              {
                other->pre = other->post;
                other->post = 0;
              }
              other->bufpos=0;
            }
          }
        }
        consumed++;
        icount--;
        curev++;
        if (icount == 0)
        {
          icount = bcount;
          bcount = 0;
          curev = cur->bstrt;
        }
        if (cur==fetched && (cur->post > 0 || cur->buffered == MM_MONITORBUFSIZE))
        {
          swap = cur;
          cur = other;
          other = swap;
          break;
        }
      }
      __MM_BUFDROP(fetched->buffer, consumed, fetched->tlen, fetched->flen, fetched->ilen, fetched->istrt, fetched->blen, fetched->bstrt);
    }
    else
    {
      swap = cur;
      cur = other;
      other = swap;
    }
  }
}
void sig_handler(int signum)
{
  if(logfile)
  {
    fprintf(logfile, "Received signal %i\n", signum);
  }
  exit(-1);
}
int main(int argc, char **argv)
{
  logfile = fopen("mlog.txt","w");
  signal(SIGSEGV, sig_handler);
  signal(SIGILL, sig_handler);
  signal(SIGABRT, sig_handler);
  signal(SIGKILL, sig_handler);
  fprintf(logfile, "Initializing Monitor...\n");
  fflush(logfile);
  initialize_events();
  _mm_source_control *__mm_strm_sourcecontrol_Left;
  atomic_init((&__mm_strm_done_Left), 0);
  fprintf(logfile, "Connecting to left stream...");
  fflush(logfile);
  shm_stream *__mma_strm_strm_Left = mm_stream_create("Left", (&__mm_strm_sourcecontrol_Left), argc, argv, 1000);
  fprintf(logfile, "connected...creating arbiter buffer...");
  fflush(logfile);
  __mma_strm_buf_Left = shm_arbiter_buffer_create(__mma_strm_strm_Left, sizeof(_mm_strm_out), SHMBUF_ARBITER_BUFSIZE);
  fprintf(logfile, "created...starting stream processing thread...");
  fflush(logfile);
  thrd_create((&__mm_strm_thread_Left), (&_mm_strm_fun_Left), 0);
  fprintf(logfile, "started...activating buffer...");
  fflush(logfile);
  shm_arbiter_buffer_set_active(__mma_strm_buf_Left, 1);
  fprintf(logfile, "activated!\n");
  fflush(logfile);
  _mm_source_control *__mm_strm_sourcecontrol_Right;
  atomic_init((&__mm_strm_done_Right), 0);
  fprintf(logfile, "Connecting to right stream...");
  shm_stream *__mma_strm_strm_Right = mm_stream_create("Right", (&__mm_strm_sourcecontrol_Right), argc, argv, 1000);
  fprintf(logfile, "connected...creating arbiter buffer...");
  fflush(logfile);
  __mma_strm_buf_Right = shm_arbiter_buffer_create(__mma_strm_strm_Right, sizeof(_mm_strm_out), SHMBUF_ARBITER_BUFSIZE);
  fprintf(logfile, "created...starting stream processing thread...");
  fflush(logfile);
  thrd_create((&__mm_strm_thread_Right), (&_mm_strm_fun_Right), 0);
  fprintf(logfile, "started...activating buffer...");
  fflush(logfile);
  shm_arbiter_buffer_set_active(__mma_strm_buf_Right, 1);
  fprintf(logfile, "activated!\nStarting monitoring...\n");
  fflush(logfile);
  arbiterMonitor();
  fprintf(logfile, "Monitoring ended!\n");
  fflush(logfile);
#ifdef DUMP_STATS
  shm_arbiter_buffer_dump_stats(__mma_strm_buf_Left);
  shm_arbiter_buffer_dump_stats(__mma_strm_buf_Right);
#endif
  shm_stream_destroy(__mma_strm_strm_Left);
  shm_stream_destroy(__mma_strm_strm_Right);
  deinitialize_events();
  size_t lcomp = leftstrm.processed;
  size_t rcomp = rightstrm.processed;
  size_t ldrop = leftstrm.dropped;
  size_t rdrop = rightstrm.dropped;
  size_t lholes = leftstrm.holes;
  size_t rholes = rightstrm.holes;
  size_t lskip = leftstrm.skipped + leftstrm.ignored;
  size_t rskip = rightstrm.skipped + rightstrm.ignored;
  size_t lall = lcomp + ldrop + lskip;
  size_t rall = rcomp + rdrop + rskip;
  fprintf(logfile,"LEFT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", lall, lcomp, ldrop, lholes, lskip);
  fprintf(logfile,"RIGHT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", rall, rcomp, rdrop, rholes, rskip);
  fflush(logfile);
  fclose(logfile);
  if (_mmdbg_enqueued_right != rightstrm.processed + rightstrm.ignored + rightstrm.skipped || _mmdbg_enqueued_left != leftstrm.processed + leftstrm.ignored + leftstrm.skipped)
  {
    // LEFT : processed 100000 events (21397 compared, 52293 dropped (in 348 holes), 26310 skipped)
    // printf("Done! Dropped %lu (Left) and %lu (Right); skipped %i (Left) and %i (Right); ignored %i (Left) and %i (Right); processed %lu (Left) and %lu (Right)\n", leftstrm.dropped+1, rightstrm.dropped, _mmdbg_enqueued_left, _mmdbg_enqueued_right, 15, 0, leftstrm.processed+23, rightstrm.processed);
    printf("LEFT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", lall, lcomp, ldrop, lholes, lskip);
    printf("RIGHT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", rall, rcomp, rdrop, rholes, rskip);
    FILE *dbgfile = fopen("xdbg.txt", "w");
    fprintf(dbgfile, "PROCESSED COUNT MISMATCH");
    fflush(dbgfile);
    fclose(dbgfile);
    exit(-1);
  }
  if (rall != lall)
  {
    // printf("Done! Dropped %lu (Left) and %lu (Right); skipped %lu (Left) and %lu (Right); ignored %lu (Left) and %lu (Right); processed %lu (Left) and %lu (Right)\n", leftstrm.dropped, rightstrm.dropped, leftstrm.skipped, rightstrm.skipped, leftstrm.ignored, rightstrm.ignored, leftstrm.processed, rightstrm.processed);
    printf("LEFT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", lall, lcomp, ldrop, lholes, lskip);
    printf("RIGHT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", rall, rcomp, rdrop, rholes, rskip);
    FILE *dbgfile = fopen("xdbg.txt", "w");
    fprintf(dbgfile, "PROCESSED SUM MISMATCH");
    fflush(dbgfile);
    fclose(dbgfile);
    exit(-1);
  }
  // printf("Done! Dropped %lu (Left) and %lu (Right); skipped %lu (Left) and %lu (Right); ignored %lu (Left) and %lu (Right); processed %lu (Left) and %lu (Right)\n", leftstrm.dropped, rightstrm.dropped, leftstrm.skipped, rightstrm.skipped, leftstrm.ignored, rightstrm.ignored, leftstrm.processed, rightstrm.processed);
  printf("LEFT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", lall, lcomp, ldrop, lholes, lskip);
  printf("RIGHT: processed %lu events (%lu compared, %lu dropped (in %lu holes), %lu skipped)\n", rall, rcomp, rdrop, rholes, rskip);
  return 0;
}
void _mm_print_streams()
{
  _mm_print_strm_Left();
  _mm_print_strm_Right();
}

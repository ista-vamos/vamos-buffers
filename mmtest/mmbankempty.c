#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <threads.h>
#include "../gen/shamon.h"
#include "../gen/mmlib.h"

long skippedEvents=0;
long processedIn = 0;
long processedOut = 0;

#ifndef SHMBUF_ARBITER_BUFSIZE
#define SHMBUF_ARBITER_BUFSIZE 64
#endif

typedef enum __MM_STREAMCONST_ENUM
{
	__MM_STREAMCONST_ENUM_Out,
	__MM_STREAMCONST_ENUM_In
} _MM_STREAMCONST_ENUM;
typedef enum __MM_EVENTCONST_ENUM_OUT
{
	__MM_EVENTCONST_OUT_NOTHING,
	__MM_EVENTCONST_ENUM_OUT_hole,
	__MM_EVENTCONST_ENUM_OUT_Balance,
	__MM_EVENTCONST_ENUM_OUT_Deposit,
	__MM_EVENTCONST_ENUM_OUT_Withdraw,
	__MM_EVENTCONST_ENUM_OUT_TransferTo,
	__MM_EVENTCONST_ENUM_OUT_TransferAmount,
	__MM_EVENTCONST_ENUM_OUT_DepositSuccess,
	__MM_EVENTCONST_ENUM_OUT_WithdrawSuccess,
	__MM_EVENTCONST_ENUM_OUT_WithdrawFail,
	__MM_EVENTCONST_ENUM_OUT_TransferSuccess,
	__MM_EVENTCONST_ENUM_OUT_TransferFail,
	__MM_EVENTCONST_ENUM_OUT_InvalidAccount,
	__MM_EVENTCONST_ENUM_OUT_Other,
} _MM_EVENTCONST_ENUM_OUT;
typedef enum __MM_EVENTCONST_ENUM_IN
{
	__MM_EVENTCONST_IN_NOTHING,
	__MM_EVENTCONST_ENUM_IN_hole,
	__MM_EVENTCONST_ENUM_IN_Number,
	__MM_EVENTCONST_ENUM_IN_Other
} _MM_EVENTCONST_ENUM_IN;
typedef struct source_control _mm_source_control;
void _mm_print_streams();
typedef struct __MMEV_Prime _MMEV_Prime;
typedef struct __MMEV_LPrime _MMEV_LPrime;
typedef struct __MMEV_RPrime _MMEV_RPrime;
typedef struct __MMEV_LSkip _MMEV_LSkip;
typedef struct __MMEV_RSkip _MMEV_RSkip;
typedef struct __MMEV_EFinal _MMEV_EFinal;
size_t __mma_strm_ilen_Out = 0;
size_t __mma_strm_blen_Out = 0;
size_t __mma_strm_tlen_Out = 0;
size_t __mma_strm_flen_Out = 0;
atomic_int __mm_strm_done_Out;
thrd_t __mm_strm_thread_Out;
shm_arbiter_buffer *__mma_strm_buf_Out;
typedef struct __mm_strm_in_Out _mm_strm_in_Out;
typedef struct __mm_strm_out_Out _mm_strm_out_Out;
typedef struct __mm_strm_hole_Out _mm_strm_hole_Out;
_mm_strm_out_Out *__mma_strm_istrt_Out = 0;
_mm_strm_out_Out *__mma_strm_bstrt_Out = 0;
void _mm_print_strm_Out();
size_t __mma_strm_ilen_In = 0;
size_t __mma_strm_blen_In = 0;
size_t __mma_strm_tlen_In = 0;
size_t __mma_strm_flen_In = 0;
atomic_int __mm_strm_done_In;
thrd_t __mm_strm_thread_In;
shm_arbiter_buffer *__mma_strm_buf_In;
typedef struct __mm_strm_in_In _mm_strm_in_In;
typedef struct __mm_strm_out_In _mm_strm_out_In;
typedef struct __mm_strm_hole_In _mm_strm_hole_In;
_mm_strm_out_In *__mma_strm_istrt_In = 0;
_mm_strm_out_In *__mma_strm_bstrt_In = 0;
void _mm_print_strm_In();
struct __MMEV_Num
{
	int num;
};
struct __MMEV_BalanceOut
{
	int acc;
	int bal;
};
struct __MMEV_DepositPrompt
{
	int acc;
};
struct __MMEV_WithdrawPrompt
{
	int acc;
};
struct __MMEV_TransferToPrompt
{
	int acc;
};
struct __MMEV_TransferAmountPrompt
{
	int acc;
	int target;
};
struct __MMEV_WithdrawFailed
{
};
struct __MMEV_TransferFailed
{
};
struct __MMEV_InvalidAccount
{
};
struct __MMEV_WithdrawSuccess
{
};

struct __MMEV_DepositSuccess
{
};
struct __MMEV_TransferSuccess
{
};
struct __MMEV_OtherOut
{
};
struct __mm_strm_hole
{
	int missed;
};
struct __mm_strm_in_In
{
	shm_event head;
	union
	{
		struct __MMEV_Num number;
	} cases;
};
struct __mm_strm_out_In
{
	shm_event head;
	union
	{
		struct __MMEV_Num number;
		struct __mm_strm_hole hole;
	} cases;
};
struct __mm_strm_in_Out
{
	shm_event head;
	union
	{
		struct __MMEV_Num number;
	} cases;
};
struct __mm_strm_out_Out
{
	shm_event head;
	union
	{
		struct __MMEV_BalanceOut balance;
		struct __MMEV_DepositPrompt deposit;
		struct __MMEV_WithdrawPrompt withdraw;
		struct __MMEV_TransferToPrompt transferTo;
		struct __MMEV_TransferAmountPrompt transferAmount;
		struct __MMEV_DepositSuccess depositSuccess;
		struct __MMEV_WithdrawFailed withdrawFailed;
		struct __MMEV_WithdrawSuccess withdrawSuccess;
		struct __MMEV_TransferFailed transferFailed;
		struct __MMEV_TransferSuccess transferSuccess;
		struct __MMEV_InvalidAccount invalidAccount;
		struct __MMEV_OtherOut other;
		struct __mm_strm_hole hole;
	} cases;
};

int _mm_strm_fun_In(void *arg)
{
	shm_arbiter_buffer *buffer = __mma_strm_buf_In;
	shm_stream *stream = shm_arbiter_buffer_stream(buffer);
	const _mm_strm_in_In *inevent;
	_mm_strm_out_In *outevent;
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
		outevent = shm_arbiter_buffer_write_ptr(buffer);
		memcpy(outevent, inevent, sizeof(_mm_strm_in_In));
		shm_arbiter_buffer_write_finish(buffer);
		shm_stream_consume(stream, 1);
	}
	atomic_store((&__mm_strm_done_In), 1);
	return 0;
}
int _mm_strm_fun_Out(void *arg)
{
	shm_arbiter_buffer *buffer = __mma_strm_buf_Out;
	shm_stream *stream = shm_arbiter_buffer_stream(buffer);
	const _mm_strm_in_Out *inevent;
	_mm_strm_out_Out *outevent;
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

		outevent = shm_arbiter_buffer_write_ptr(buffer);
		memcpy(outevent, inevent, sizeof(_mm_strm_in_In));
		shm_arbiter_buffer_write_finish(buffer);
		shm_stream_consume(stream, 1);
	}
	atomic_store((&__mm_strm_done_Out), 1);
	return 0;
}

int __mm_monitor_running = 1;
void arbiterMonitor()
{
	  uint64_t tmp1, tmp2;
  while(__mm_monitor_running)
  {
          tmp1 = shm_arbiter_buffer_size(__mma_strm_buf_In);
          if (tmp1 > 0) {
                  shm_arbiter_buffer_drop(__mma_strm_buf_In, tmp1);
          }
          tmp2 = shm_arbiter_buffer_size(__mma_strm_buf_Out);
          if (tmp2 > 0) {
                  shm_arbiter_buffer_drop(__mma_strm_buf_Out, tmp2);
          }
		  processedIn+=tmp1;
		  processedOut+=tmp2;
          __mm_monitor_running
                = !(tmp1 == 0 && tmp2 == 0 && atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire));
  }
}

int main(int argc, char **argv)
{
	initialize_events();
	_mm_source_control *__mm_strm_sourcecontrol_Out;
	atomic_init((&__mm_strm_done_Out), 0);
	shm_stream *__mma_strm_strm_Out = shm_stream_create("Out", (&__mm_strm_sourcecontrol_Out), argc, argv);
	__mma_strm_buf_Out = shm_arbiter_buffer_create(__mma_strm_strm_Out, sizeof(_mm_strm_out_Out), SHMBUF_ARBITER_BUFSIZE);
	thrd_create((&__mm_strm_thread_Out), (&_mm_strm_fun_Out), 0);
	shm_arbiter_buffer_set_active(__mma_strm_buf_Out, 1);
	_mm_source_control *__mm_strm_sourcecontrol_In;
	atomic_init((&__mm_strm_done_In), 0);
	shm_stream *__mma_strm_strm_In = shm_stream_create("In", (&__mm_strm_sourcecontrol_In), argc, argv);
	__mma_strm_buf_In = shm_arbiter_buffer_create(__mma_strm_strm_In, sizeof(_mm_strm_out_In), SHMBUF_ARBITER_BUFSIZE);
	thrd_create((&__mm_strm_thread_In), (&_mm_strm_fun_In), 0);
	shm_arbiter_buffer_set_active(__mma_strm_buf_In, 1);
	arbiterMonitor();
	/*
	shm_arbiter_buffer_dump_stats ( __mma_strm_buf_Out) ;
	shm_arbiter_buffer_dump_stats ( __mma_strm_buf_In) ;
	*/
	shm_stream_destroy(__mma_strm_strm_Out);
	shm_stream_destroy(__mma_strm_strm_In);
	deinitialize_events();
	printf("DONE!Found %i errors!\nSkipped %li events\nProcessed In %li / Out %li", 0, processedIn+processedOut, processedIn, processedOut);
	return 0;
}
void _mm_print_streams()
{
	_mm_print_strm_Out();
	_mm_print_strm_In();
}

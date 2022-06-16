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

typedef struct _account
{
	int id;
	int balance;
	int ubound;
	int lbound;
	struct _account *next;
	struct _account *prev;
} account;

size_t account_count = 0;
int accounts_size = 541;
account *accounts[541];

account *getAccount(int id)
{
	int hash = id % accounts_size;
	account *entry = accounts[hash];
	account *cur = entry;
	if (entry != NULL)
	{
		do
		{
			if (cur->id == id)
			{
				return cur;
			}
			cur = cur->next;
		} while (entry != cur);
	}
	cur = (account *)malloc(sizeof(account));
	cur->id = id;
	cur->balance = -2;
	cur->ubound = -2;
	cur->lbound = -2;
	if (entry == NULL)
	{
		accounts[hash] = cur;
		cur->next = cur;
		cur->prev = cur;
	}
	else
	{
		cur->next = entry;
		cur->prev = entry->prev;
		cur->next->prev = cur;
		cur->prev->next = cur;
	}
	account_count++;
	return cur;
}
void clearAccounts()
{
	for (int i = 0; i < accounts_size && account_count>0; i++)
	{
		account *first = accounts[i];
		account *cur = first;
		if (cur != NULL)
		{
			do
			{
				account *next = cur->next;
				account_count--;
				free(cur);
				cur = next;
			} while (first != cur);
			accounts[i] = NULL;
		}
	}
}
int clearAccount(int id)
{
	int hash = id % accounts_size;
	account *entry = accounts[hash];
	account *cur = entry;
	if (entry != NULL)
	{
		if(entry->id==id)
		{
			if(entry->next==entry)
			{
				accounts[hash]=NULL;
			}
			else
			{
				accounts[hash]=entry->next;
				entry->next->prev=entry->prev;
				entry->prev->next=entry->next;
			}
			free(entry);
			account_count--;
			return 1;
		}
		do
		{
			if (cur->id == id)
			{
				cur->next->prev=cur->prev;
				cur->prev->next=cur->next;
				free(cur);
				account_count--;
				return 1;
			}
			cur = cur->next;
		} while (entry != cur);
	}
	return 0;
}

#define MM_ERRORBUFSIZE 1024
char errorBuffer[MM_ERRORBUFSIZE];
size_t errorCount = 0;
void registerError(char *msg)
{
	printf("MISMATCH DETECTED: %s\n", msg);
	errorCount++;
}

void processBalanceCheck(int acc, int bal)
{
	//printf("Balance: %i @ %i\n", bal, acc);
	account *account = getAccount(acc);
	if (account->balance < -2 || account->ubound < -2 || account->lbound < -2)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Found balance for supposedly invalid account %i!", acc);
		registerError(errorBuffer);
	}
	else
	{
		if(account->balance>=0)
		{
			if(account->balance!=bal)
			{
				snprintf(errorBuffer, MM_ERRORBUFSIZE, "Balance mismatch for account %i: expected %i, but found %i!", acc, account->balance, bal);
				registerError(errorBuffer);
				account->balance=-1;
				return;
			}
		}
		else
		{
			if(account->ubound>=0&&account->lbound>=0&&account->ubound<bal&&account->lbound>bal)
			{
				snprintf(errorBuffer, MM_ERRORBUFSIZE, "Balance mismatch for account %i: expected amount between %i and %i, but found %i!", acc, account->lbound, account->ubound, bal);
				registerError(errorBuffer);
				account->ubound=-1;
				account->lbound=-1;
				return;
			}
			else if(account->ubound>=0&&account->ubound<bal)
			{
				snprintf(errorBuffer, MM_ERRORBUFSIZE, "Balance mismatch for account %i: expected amount of at most %i, but found %i!", acc, account->ubound, bal);
				registerError(errorBuffer);
				account->ubound=-1;
				return;
			}
			else if(account->lbound>=0&&account->lbound>bal)
			{
				snprintf(errorBuffer, MM_ERRORBUFSIZE, "Balance mismatch for account %i: expected amount of at least %i, but found %i!", acc, account->lbound, bal);
				registerError(errorBuffer);
				account->lbound=-1;
				return;
			}
		}
	}
	account->balance=bal;
	account->ubound=-1;
	account->lbound=-1;
}
void processSuccessfulDeposit(int acc, int amount)
{
	//printf("Deposit: %i to %i\n", amount, acc);
	account *account = getAccount(acc);
	if(amount<=0)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Deposit of non-positive amount %i should have failed!", amount);
		registerError(errorBuffer);
	}
	if (account->balance < -2 || account->ubound < -2 || account->lbound < -2)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Found deposit for supposedly invalid account %i!", acc);
		registerError(errorBuffer);
	}
	if(account->balance>=0)
	{
		account->balance+=amount;
	}
	else
	{
		if(account->ubound>=0)
		{
			account->ubound+=amount;
		}
		if(account->lbound>=0)
		{
			account->lbound+=amount;
		}
		else
		{
			account->lbound=amount;
		}
	}
}
void processSuccessfulWithdraw(int acc, int amount)
{
	//printf("Withdraw: %i from %i\n", amount, acc);
	account *account = getAccount(acc);
	if (amount <= 0)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Withdrawal of non-positive amount %i should have failed!", amount);
		registerError(errorBuffer);
	}
	if (account->balance >= amount)
	{
		account->balance -= amount;
		return;
	}
	if (account->balance < 0)
	{
		if (account->balance < -2 || account->ubound < -2 || account->lbound < -2)
		{
			snprintf(errorBuffer, MM_ERRORBUFSIZE, "Withdrawal from invalid account %i should have failed!", acc);
			registerError(errorBuffer);
		}
		else
		{
			if (account->ubound >= 0)
			{
				if(account->ubound < amount)
				{
					snprintf(errorBuffer, MM_ERRORBUFSIZE, "Account %i's balance should be too low to withdraw %i!", account->id, amount);
					registerError(errorBuffer);
					account->ubound=-1;
				}
				else
				{
					account->ubound-=amount;
					if(account->lbound>=0)
					{
						if(account->lbound<amount)
						{
							account->lbound=0;
						}
						else
						{
							account->lbound-=amount;
						}
					}
				}
			}
			else if(account->lbound>=0)
			{
				if(account->lbound<amount)
				{
					account->lbound=0;
				}
				else
				{
					account->lbound-=amount;
				}
			}
		}
	}
	else
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Account %i's balance should be too low to withdraw %i!", account->id, amount);
		registerError(errorBuffer);
		account->balance = -1;
	}
}
void processFailedWithdraw(int acc, int amount)
{
	//printf("Failed withdraw: %i from %i\n", amount, acc);
	account *account = getAccount(acc);
	if (amount <= 0)
	{
		return;
	}
	if (account->balance > amount || account->lbound > amount)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Account %i's balance should be enough to withdraw %i!", account->id, amount);
		registerError(errorBuffer);
		account->balance = -1;
		account->lbound = -1;
	}
	else
	{
		if (account->balance < 0 && (account->ubound < 0 || account->ubound > amount - 1))
		{
			account->ubound = amount - 1;
		}
	}
}
void processSuccessfulTransfer(int acc, int target, int amount)
{
	//printf("Transfer: %i from %i to %i\n", amount, acc, target);
	processSuccessfulWithdraw(acc, amount);
	processSuccessfulDeposit(target, amount);
}
void processFailedTransfer(int acc, int target, int amount)
{
	//printf("Failed Transfer: %i from %i (to %i)\n", amount, acc, target);
	processFailedWithdraw(acc, amount);
}
void processInvalidAccount(int acc)
{
	//printf("Invalid account: %i\n", acc);
	account *account = getAccount(acc);
	if (account->balance > -2 || account->lbound > -2 || account->ubound > -2)
	{
		snprintf(errorBuffer, MM_ERRORBUFSIZE, "Account %i should be invalid, but has been legally accessed before!", account->id);
		registerError(errorBuffer);
	}
	account->balance = -3;
	account->lbound = -3;
	account->ubound = -3;
}

inline void consumeIn(int n)
{
	shm_arbiter_buffer_drop(__mma_strm_buf_In, n);
	processedIn+=n;
}
inline void consumeOut(int n)
{
	shm_arbiter_buffer_drop(__mma_strm_buf_Out, n);
	processedOut+=n;
}

int __mm_monitor_running = 1;
void arbiterMonitor()
{
	int outskip = 0;
	int inskip = 0;
	int amount = 0;
	int target = 0;
	int account = 0;
	while (__mm_monitor_running)
	{
	mm_state_out:
	{
		if(processedOut-processedIn>1)
		{
			printf("In %li / Out %li\n", processedIn, processedOut);
			return;
		}
		__mma_strm_tlen_Out = shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1 + outskip, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out));
		if (__mma_strm_tlen_Out == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_out;
		}
		int curcount = __mma_strm_ilen_Out + __mma_strm_blen_Out;
		int consume = 0;
		while (outskip > 0 && curcount > 0)
		{
			_mm_strm_out_Out *curev = __mma_strm_istrt_Out;
			if (curev->head.kind == __MM_EVENTCONST_ENUM_OUT_hole)
			{
				int missed = curev->cases.hole.missed;
				//printf("Hole @ OUT: %li (%i, %i)\n", processedOut, missed, outskip);
				processedOut+= missed - 1;
				clearAccounts();
				if (missed > outskip)
				{
					inskip += (missed - outskip) - 1;
					outskip = 0;
					consumeOut(consume + 1);
					//printf("OUT->IN : %li / %li (%i)\n", processedOut, processedIn, inskip);
					goto mm_state_in;
				}
				outskip -= missed;
				skippedEvents+=missed;
			}
			else
			{
				outskip -= 1;
				skippedEvents+=1;
			}
			curcount -= 1;
			__mma_strm_ilen_Out -= 1;
			__mma_strm_istrt_Out++;
			if (__mma_strm_ilen_Out == 0)
			{
				__mma_strm_istrt_Out = __mma_strm_bstrt_Out;
				__mma_strm_ilen_Out = __mma_strm_blen_Out;
				__mma_strm_bstrt_Out = NULL;
				__mma_strm_blen_Out = 0;
			}
			consume++;
		}
		if (outskip > 0)
		{
			consumeOut(consume);
			goto mm_state_out;
		}
		if (curcount > 0)
		{
			//printf("OUT: %li (%li)\n", __mma_strm_istrt_Out->head.kind, processedOut);
			switch (__mma_strm_istrt_Out->head.kind)
			{
			case __MM_EVENTCONST_ENUM_OUT_hole:
				//printf("Hole @ OUT: %li (%i)\n", processedOut,  __mma_strm_istrt_Out->cases.hole.missed);
				inskip += __mma_strm_istrt_Out->cases.hole.missed-1;
				skippedEvents+=__mma_strm_istrt_Out->cases.hole.missed;
				processedOut+=__mma_strm_istrt_Out->cases.hole.missed - 1;
				consumeOut(consume+1);
				clearAccounts();
				goto mm_state_in;
				break;
			case __MM_EVENTCONST_ENUM_OUT_Balance:
				processBalanceCheck(__mma_strm_istrt_Out->cases.balance.acc, __mma_strm_istrt_Out->cases.balance.bal);
				consumeOut(consume+1);
				goto mm_state_in;
				break;
			case __MM_EVENTCONST_ENUM_OUT_Deposit:
				account = __mma_strm_istrt_Out->cases.deposit.acc;
				consumeOut(consume+1);
				goto mm_state_deposit_amount;
				break;
			case __MM_EVENTCONST_ENUM_OUT_Withdraw:
				account = __mma_strm_istrt_Out->cases.deposit.acc;
				consumeOut(consume+1);
				goto mm_state_withdraw_amount;
				break;
			case __MM_EVENTCONST_ENUM_OUT_TransferTo:
				account = __mma_strm_istrt_Out->cases.transferTo.acc;
				consumeOut(consume+1);
				goto mm_state_transfer_target;
				break;
			default:
				consumeOut(consume+1);
				goto mm_state_in;
				break;
			}
		}
		else
		{
			consumeOut(consume);
			goto mm_state_out;
		}
	}

	mm_state_deposit_amount:
	{
		if (inskip > 0)
		{
			goto mm_state_in;
		}
		__mma_strm_tlen_In = shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In));
		if (__mma_strm_tlen_In == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_deposit_amount;
		}
		if (__mma_strm_ilen_In > 0)
		{
			switch (__mma_strm_istrt_In->head.kind)
			{
			case __MM_EVENTCONST_ENUM_IN_Number:
				amount = __mma_strm_istrt_In->cases.number.num;
				consumeIn(1);
				goto mm_state_deposit_response;
				break;
			default:
				goto mm_state_in;
				break;
			}
		}
		else
		{
			goto mm_state_deposit_amount;
		}
	}

	mm_state_deposit_response:
	{
		if (outskip > 0)
		{
			goto mm_state_out;
		}
		__mma_strm_tlen_Out = shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out));
		if (__mma_strm_tlen_Out == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_deposit_response;
		}
		if (__mma_strm_ilen_Out > 0)
		{
			switch (__mma_strm_istrt_Out->head.kind)
			{
			case __MM_EVENTCONST_ENUM_OUT_DepositSuccess:
				processSuccessfulDeposit(account, amount);
				consumeOut(1);
				goto mm_state_in;
				break;
			default:
				goto mm_state_out;
				break;
			}
		}
		else
		{
			goto mm_state_deposit_response;
		}
	}

	mm_state_withdraw_amount:
	{
		if (inskip > 0)
		{
			goto mm_state_in;
		}
		__mma_strm_tlen_In = shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In));
		if (__mma_strm_tlen_In == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_withdraw_amount;
		}
		if (__mma_strm_ilen_In > 0)
		{
			switch (__mma_strm_istrt_In->head.kind)
			{
			case __MM_EVENTCONST_ENUM_IN_Number:
				amount = __mma_strm_istrt_In->cases.number.num;
				consumeIn(1);
				goto mm_state_withdraw_response;
				break;
			default:
				goto mm_state_in;
				break;
			}
		}
		else
		{
			goto mm_state_withdraw_amount;
		}
	}

	mm_state_withdraw_response:
	{
		if (outskip > 0)
		{
			goto mm_state_out;
		}
		__mma_strm_tlen_Out = shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out));
		if (__mma_strm_tlen_Out == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_withdraw_response;
		}
		if (__mma_strm_ilen_Out > 0)
		{
			switch (__mma_strm_istrt_Out->head.kind)
			{
			case __MM_EVENTCONST_ENUM_OUT_WithdrawSuccess:
				processSuccessfulWithdraw(account, amount);
				consumeOut(1);
				goto mm_state_in;
				break;
			case __MM_EVENTCONST_ENUM_OUT_WithdrawFail:
				processFailedWithdraw(account, amount);
				consumeOut(1);
				goto mm_state_in;
				break;
			default:
				goto mm_state_out;
				break;
			}
		}
		else
		{
			goto mm_state_withdraw_response;
		}
	}

	mm_state_transfer_target:
	{
		if (inskip > 0)
		{
			goto mm_state_in;
		}
		__mma_strm_tlen_In = shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In));
		if (__mma_strm_tlen_In == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_transfer_target;
		}
		if (__mma_strm_ilen_In > 0)
		{
			switch (__mma_strm_istrt_In->head.kind)
			{
			case __MM_EVENTCONST_ENUM_IN_Number:
				target = __mma_strm_istrt_In->cases.number.num;
				consumeIn(1);
				goto mm_state_transfer_target_response;
				break;
			default:
				goto mm_state_in;
				break;
			}
		}
		else
		{
			goto mm_state_transfer_target;
		}
	}

	mm_state_transfer_target_response:
	{
		if (outskip > 0)
		{
			goto mm_state_out;
		}
		__mma_strm_tlen_Out = shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out));
		if (__mma_strm_tlen_Out == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_transfer_target_response;
		}
		if (__mma_strm_ilen_Out > 0)
		{
			switch (__mma_strm_istrt_Out->head.kind)
			{
			case __MM_EVENTCONST_ENUM_OUT_TransferAmount:
				consumeOut(1);
				goto mm_state_transfer_amount;
				break;
			case __MM_EVENTCONST_ENUM_OUT_InvalidAccount:
				processInvalidAccount(target);
				consumeOut(1);
				goto mm_state_in;
				break;
			default:
				goto mm_state_out;
				break;
			}
		}
		else
		{
			goto mm_state_transfer_target_response;
		}
	}

	mm_state_transfer_amount:
	{
		if (inskip > 0)
		{
			goto mm_state_in;
		}
		__mma_strm_tlen_In = shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In));
		if (__mma_strm_tlen_In == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_transfer_amount;
		}
		if (__mma_strm_ilen_In > 0)
		{
			switch (__mma_strm_istrt_In->head.kind)
			{
			case __MM_EVENTCONST_ENUM_IN_Number:
				amount = __mma_strm_istrt_In->cases.number.num;
				consumeIn(1);
				goto mm_state_transfer_response;
				break;
			default:
				goto mm_state_in;
				break;
			}
		}
		else
		{
			goto mm_state_transfer_amount;
		}
	}

	mm_state_transfer_response:
	{
		if (outskip > 0)
		{
			goto mm_state_out;
		}
		__mma_strm_tlen_Out = shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out));
		if (__mma_strm_tlen_Out == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_Out, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_Out, 1, ((void **)(&__mma_strm_istrt_Out)), (&__mma_strm_ilen_Out), ((void **)(&__mma_strm_bstrt_Out)), (&__mma_strm_blen_Out)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_transfer_response;
		}
		if (__mma_strm_ilen_Out > 0)
		{
			switch (__mma_strm_istrt_Out->head.kind)
			{
			case __MM_EVENTCONST_ENUM_OUT_TransferSuccess:
				processSuccessfulTransfer(account, target, amount);
				consumeOut(1);
				goto mm_state_in;
				break;
			case __MM_EVENTCONST_ENUM_OUT_TransferFail:
				processFailedTransfer(account, target, amount);
				consumeOut(1);
				goto mm_state_in;
				break;
			default:
				goto mm_state_out;
				break;
			}
		}
		else
		{
			goto mm_state_transfer_response;
		}
	}

	mm_state_in:
	{
		if(processedIn-processedOut>1)
		{
			printf("Out %li / In %li\n", processedOut, processedIn);
			return;
		}
		__mma_strm_tlen_In = shm_arbiter_buffer_peek(__mma_strm_buf_In, inskip + 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In));
		if (__mma_strm_tlen_In == 0)
		{
			if (atomic_load_explicit(&__mm_strm_done_In, memory_order_acquire) && (shm_arbiter_buffer_peek(__mma_strm_buf_In, 1, ((void **)(&__mma_strm_istrt_In)), (&__mma_strm_ilen_In), ((void **)(&__mma_strm_bstrt_In)), (&__mma_strm_blen_In)) == 0))
			{
				__mm_monitor_running = 0;
				break;
			}
			goto mm_state_in;
		}
		int curcount = __mma_strm_ilen_In + __mma_strm_blen_In;
		int consume = 0;
		while (inskip > 0 && curcount > 0)
		{
			_mm_strm_out_In *curev = __mma_strm_istrt_In;
			if (curev->head.kind == __MM_EVENTCONST_ENUM_IN_hole)
			{
				int missed = curev->cases.hole.missed;
				//printf("Hole @ In: %li (%i, %i)\n", processedIn, missed, inskip);
				processedIn+=missed-1;
				clearAccounts();
				if (missed > inskip)
				{
					outskip += (missed - inskip) - 1;
					inskip = 0;
					consumeIn(consume+1);
					//printf("IN->OUT : %li / %li (%i)\n", processedIn, processedOut, outskip);
					goto mm_state_out;
				}
				inskip -= missed;
			}
			else
			{
				inskip -= 1;
			}
			curcount -= 1;
			__mma_strm_ilen_In -= 1;
			__mma_strm_istrt_In++;
			if (__mma_strm_ilen_In == 0)
			{
				__mma_strm_istrt_In = __mma_strm_bstrt_In;
				__mma_strm_ilen_In = __mma_strm_blen_In;
				__mma_strm_bstrt_In = NULL;
				__mma_strm_blen_In = 0;
			}
			consume++;
		}
		if (inskip > 0)
		{
			consumeIn(consume);
			goto mm_state_in;
		}
		if (curcount > 0)
		{
			//printf("IN: %li (%li)\n", __mma_strm_istrt_In->head.kind, processedIn);
			switch (__mma_strm_istrt_In->head.kind)
			{
			case __MM_EVENTCONST_ENUM_IN_hole:
				//printf("Hole @ In: %li (%i)\n", processedIn,  __mma_strm_istrt_In->cases.hole.missed);
				outskip += __mma_strm_istrt_In->cases.hole.missed - 1;
				processedIn+=__mma_strm_istrt_In->cases.hole.missed - 1;
				clearAccounts();
				break;
			default:
				break;
			}
			consumeIn(consume+1);
			goto mm_state_out;
		}
		else
		{
			consumeIn(consume);
			goto mm_state_in;
		}
	}
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
	printf("Found %li errors!\nSkipped %li events\nProcessed In %li / Out %li", errorCount, skippedEvents, processedIn, processedOut);
	return 0;
}
void _mm_print_streams()
{
	_mm_print_strm_Out();
	_mm_print_strm_In();
}

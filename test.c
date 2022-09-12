#include "shamon.h"
#include "mmlib.h"
#include "monitor.h"
#include "./compiler/cfiles/compiler_utils.h"
#include <threads.h>
#include <stdio.h>
#include <stdatomic.h>

struct _EVENT_hole
{
  uint64_t n;
};
typedef struct _EVENT_hole EVENT_hole;
// event declarations for stream type Primes
struct _EVENT_Prime {
	int n;
	int p;

};
typedef struct _EVENT_Prime EVENT_Prime;

// input stream for stream type Primes
struct _STREAM_Primes_in {
    shm_event head;
    union {
        EVENT_Prime Prime;
    }cases;
};
typedef struct _STREAM_Primes_in STREAM_Primes_in;

// output stream for stream type Primes
struct _STREAM_Primes_out {
    shm_event head;
    union {
        EVENT_hole hole;
        EVENT_Prime Prime;
    }cases;
};
typedef struct _STREAM_Primes_out STREAM_Primes_out;
        // event declarations for stream type NumberPairs
struct _EVENT_NumberPair {
	int i;
	int n;
	int m;

};
typedef struct _EVENT_NumberPair EVENT_NumberPair;

// input stream for stream type NumberPairs
struct _STREAM_NumberPairs_in {
    shm_event head;
    union {
        EVENT_NumberPair NumberPair;
    }cases;
};
typedef struct _STREAM_NumberPairs_in STREAM_NumberPairs_in;

// output stream for stream type NumberPairs
struct _STREAM_NumberPairs_out {
    shm_event head;
    union {
        EVENT_hole hole;
        EVENT_NumberPair NumberPair;
    }cases;
};
typedef struct _STREAM_NumberPairs_out STREAM_NumberPairs_out;
        

    typedef struct _STREAM_Primes_ARGS {
    	int pos;

    } STREAM_Primes_ARGS;
            

STREAM_Primes_ARGS stream_args_P_0;
STREAM_Primes_ARGS stream_args_P_1;

int *arbiter_counter;
// monitor buffer
shm_monitor_buffer *monitor_buffer;

// globals code
STREAM_NumberPairs_out *arbiter_outevent;

#include "./compiler/cfiles/intmap.h"
     intmap buf;
     int count = 0;
 
     int process(int n, int p, int pos, int opos)
     {
         if(pos < opos)
         {
             int oval=0;
             if(intmap_get(&buf, n, &oval))
             {
                 

        arbiter_outevent = (STREAM_NumberPairs_out *)shm_monitor_buffer_write_ptr(monitor_buffer);
         arbiter_outevent->head.kind = 2;
    arbiter_outevent->head.id = (*arbiter_counter)++;
    ((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.i = n;
((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.n = oval;
((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.m = p;

         shm_monitor_buffer_write_finish(monitor_buffer);
        }
             count -= intmap_remove_upto(&buf, n);
         }
         else
         {
             if(count<10)
             {
                 intmap_insert(&buf, n, p);
                 count++;
             }
         }
         return n+1;
     }
 
bool SHOULD_KEEP_P(shm_stream * s, shm_event * e) {
    return true;
}
            

atomic_int count_event_streams = 2;

// declare event streams
shm_stream *EV_SOURCE_P_0;
shm_stream *EV_SOURCE_P_1;


// event sources threads
thrd_t THREAD_P_0;
thrd_t THREAD_P_1;


// declare arbiter thread
thrd_t ARBITER_THREAD;

// Arbiter buffer for event source P (0)
shm_arbiter_buffer *BUFFER_P0;

// Arbiter buffer for event source P (1)
shm_arbiter_buffer *BUFFER_P1;




// buffer groups

bool Ps_ORDER_EXP (void *args1, void *args2) {
    return ((STREAM_Primes_ARGS *) args1)->pos > ((STREAM_Primes_ARGS *) args2)->pos;
}        


buffer_group BG_Ps;
        

int PERF_LAYER_P (shm_arbiter_buffer *buffer) {
    shm_stream *stream = shm_arbiter_buffer_stream(buffer);   
    STREAM_Primes_in *inevent;
    STREAM_Primes_out *outevent;   

    // wait for active buffer
    while ((!shm_arbiter_buffer_active(buffer))){
        sleep_ns(10);
    }
    while(true) {
        inevent = stream_filter_fetch(stream, buffer, &SHOULD_KEEP_P);
        
        if (inevent == NULL) {
            // no more events
            break;
        }
        outevent = shm_arbiter_buffer_write_ptr(buffer);
        
        memcpy(outevent, inevent, sizeof(STREAM_Primes_out));
        shm_arbiter_buffer_write_finish(buffer);
        shm_stream_consume(stream, 1);
    }  
    atomic_fetch_add(&count_event_streams, -1);   
}


bool exists_open_streams() {
    return count_event_streams > 0;
}
    
bool check_n_events(shm_arbiter_buffer* b, size_t n) {
    // checks if there are exactly n elements on a given stream s
    void* e1; size_t i1;
	void* e2; size_t i2;
	return shm_arbiter_buffer_peek(b,0, &e1, &i1, &e2, &i2) == n;
}


bool are_events_in_head(shm_arbiter_buffer *b, size_t ev_size, int event_kinds[], int n_events) {
    char* e1; size_t i1;
	char* e2; size_t i2;
	int count = shm_arbiter_buffer_peek(b, n_events, (void **)&e1, &i1,(void**) &e2, &i2);
	if (count < n_events) {
	    return false;
    }
    
    
	int i = 0;
	while (i < i1) {
	    shm_event * ev = (shm_event *) (e1);
	     if (ev->kind != event_kinds[i]) {
	        return false;
	    }
	    i+=1;
	    e1 += ev_size;
	}

	i = 0;
	while (i < i2) {
	    shm_event * ev = (shm_event *) e2;
	     if (ev->kind != event_kinds[i1+i]) {
	        return false;
	    }
	    i+=1;
	    e2 += ev_size;
	}

	return true;
}

shm_event * get_event_at_index(char* e1, size_t i1, char* e2, size_t i2, size_t size_event, int element_index) {
	if (element_index < i1) {
		return (shm_event *) (e1 + (element_index*size_event));
	} else {
		element_index -=i1;
		return (shm_event *) (e2 + (element_index*size_event));
	}
}

//arbiter outevent
STREAM_NumberPairs_out *arbiter_outevent;
int RULE_SET_rs();


int RULE_SET_rs() {
dll_node **chosen_streams; // used for match fun
            int TEMPARR0[] = {2};
int TEMPARR1[] = {2};

            
                if (are_events_in_head(BUFFER_P0, sizeof(STREAM_Primes_out), TEMPARR1, 1)) {
                    
                if (are_events_in_head(BUFFER_P1, sizeof(STREAM_Primes_out), TEMPARR0, 1)) {
                    
            if(true ) {
                
                char* e1_P; size_t i1_P;
	            char* e2_P; size_t i2_P;
	            shm_arbiter_buffer_peek(BUFFER_P0, 1, (void**)&e1_P, &i1_P,(void**) &e2_P, &i2_P);
            
                STREAM_Primes_out * event_for_ln = (STREAM_Primes_out *) get_event_at_index(e1_P, i1_P, e2_P, i2_P, sizeof(STREAM_Primes_out), 0);
int ln = event_for_ln->cases.Prime.n;

STREAM_Primes_out * event_for_lp = (STREAM_Primes_out *) get_event_at_index(e1_P, i1_P, e2_P, i2_P, sizeof(STREAM_Primes_out), 0);
int lp = event_for_lp->cases.Prime.p;

STREAM_Primes_out * event_for_rn = (STREAM_Primes_out *) get_event_at_index(e1_P, i1_P, e2_P, i2_P, sizeof(STREAM_Primes_out), 0);
int rn = event_for_rn->cases.Prime.n;

STREAM_Primes_out * event_for_rp = (STREAM_Primes_out *) get_event_at_index(e1_P, i1_P, e2_P, i2_P, sizeof(STREAM_Primes_out), 0);
int rp = event_for_rp->cases.Prime.p;


                if(ln == rn)
             {
                 

        arbiter_outevent = (STREAM_NumberPairs_out *)shm_monitor_buffer_write_ptr(monitor_buffer);
         arbiter_outevent->head.kind = 2;
    arbiter_outevent->head.id = (*arbiter_counter)++;
    ((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.i = ln;
((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.n = lp;
((STREAM_NumberPairs_out *) arbiter_outevent)->cases.NumberPair.m = rp;

         shm_monitor_buffer_write_finish(monitor_buffer);
        	shm_arbiter_buffer_drop(BUFFER_P0, 1);
	shm_arbiter_buffer_drop(BUFFER_P1, 1);

intmap_clear(&buf);
                 stream_args_P_0.pos
= ln + 1;
                 stream_args_P_1.pos
= rn + 1;
             }
             else if(stream_args_P_0.pos
<stream_args_P_1.pos
)
             {
                 stream_args_P_0.pos
= process(ln, lp, stream_args_P_0.pos
, stream_args_P_1.pos
);
                 	shm_arbiter_buffer_drop(BUFFER_P0, 1);

}
             else
             {
                 stream_args_P_1.pos
= process(rn, rp, stream_args_P_1.pos
, stream_args_P_0.pos
);
                 	shm_arbiter_buffer_drop(BUFFER_P1, 1);

}
         
            }
            
                    
                }
                    
                }
            
            int TEMPARR2[] = {1};

            
            if (chosen_streams != NULL) {
                free(chosen_streams);
            }
            bg_update(&BG_Ps, Ps_ORDER_EXP);
            chosen_streams = bg_get_first_n(&BG_Ps, 2);

        
            if (chosen_streams != NULL) {
                
            {
                shm_stream *F = chosen_streams[0]->stream;
shm_arbiter_buffer *BUFFER_F = chosen_streams[0]->buffer;
STREAM_Primes_ARGS stream_args_F = *((STREAM_Primes_ARGS *)chosen_streams[0]->args);
shm_stream *S = chosen_streams[1]->stream;
shm_arbiter_buffer *BUFFER_S = chosen_streams[1]->buffer;
STREAM_Primes_ARGS stream_args_S = *((STREAM_Primes_ARGS *)chosen_streams[1]->args);

                
                if (are_events_in_head(BUFFER_F, sizeof(STREAM_Primes_out), TEMPARR2, 1)) {
                    
            if(true ) {
                
                char* e1_F; size_t i1_F;
	            char* e2_F; size_t i2_F;
	            shm_arbiter_buffer_peek(BUFFER_F, 1, (void**)&e1_F, &i1_F,(void**) &e2_F, &i2_F);
            
                STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.hole.n;


                if(
stream_args_F.pos< 
stream_args_S.pos)
             {
                 count -= intmap_remove_upto(&buf, 
stream_args_F.pos+n);
             }
             
stream_args_F.pos+= n;
         
            }
            
                    	shm_arbiter_buffer_drop(BUFFER_F, 1);

                }
            }
            
            {
                shm_stream *F = chosen_streams[1]->stream;
shm_arbiter_buffer *BUFFER_F = chosen_streams[1]->buffer;
STREAM_Primes_ARGS stream_args_F = *((STREAM_Primes_ARGS *)chosen_streams[1]->args);
shm_stream *S = chosen_streams[0]->stream;
shm_arbiter_buffer *BUFFER_S = chosen_streams[0]->buffer;
STREAM_Primes_ARGS stream_args_S = *((STREAM_Primes_ARGS *)chosen_streams[0]->args);

                
                if (are_events_in_head(BUFFER_F, sizeof(STREAM_Primes_out), TEMPARR2, 1)) {
                    
            if(true ) {
                
                char* e1_F; size_t i1_F;
	            char* e2_F; size_t i2_F;
	            shm_arbiter_buffer_peek(BUFFER_F, 1, (void**)&e1_F, &i1_F,(void**) &e2_F, &i2_F);
            
                STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.hole.n;


                if(
stream_args_F.pos< 
stream_args_S.pos)
             {
                 count -= intmap_remove_upto(&buf, 
stream_args_F.pos+n);
             }
             
stream_args_F.pos+= n;
         
            }
            
                    	shm_arbiter_buffer_drop(BUFFER_F, 1);

                }
            }
            
            }
                    
            
            int TEMPARR3[] = {2};

            
            if (chosen_streams != NULL) {
                free(chosen_streams);
            }
            bg_update(&BG_Ps, Ps_ORDER_EXP);
            chosen_streams = bg_get_first_n(&BG_Ps, 2);

        
            if (chosen_streams != NULL) {
                
            {
                shm_stream *F = chosen_streams[0]->stream;
shm_arbiter_buffer *BUFFER_F = chosen_streams[0]->buffer;
STREAM_Primes_ARGS stream_args_F = *((STREAM_Primes_ARGS *)chosen_streams[0]->args);
shm_stream *S = chosen_streams[1]->stream;
shm_arbiter_buffer *BUFFER_S = chosen_streams[1]->buffer;
STREAM_Primes_ARGS stream_args_S = *((STREAM_Primes_ARGS *)chosen_streams[1]->args);

                
                if (are_events_in_head(BUFFER_F, sizeof(STREAM_Primes_out), TEMPARR3, 1)) {
                    
            if(true ) {
                
                char* e1_F; size_t i1_F;
	            char* e2_F; size_t i2_F;
	            shm_arbiter_buffer_peek(BUFFER_F, 1, (void**)&e1_F, &i1_F,(void**) &e2_F, &i2_F);
            
                STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.Prime.n;

STREAM_Primes_out * event_for_p = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int p = event_for_p->cases.Prime.p;


                stream_args_F.pos
= process(n, p, stream_args_F.pos
, stream_args_S.pos
);
         
            }
            
                    	shm_arbiter_buffer_drop(BUFFER_F, 1);

                }
            }
            
            {
                shm_stream *F = chosen_streams[1]->stream;
shm_arbiter_buffer *BUFFER_F = chosen_streams[1]->buffer;
STREAM_Primes_ARGS stream_args_F = *((STREAM_Primes_ARGS *)chosen_streams[1]->args);
shm_stream *S = chosen_streams[0]->stream;
shm_arbiter_buffer *BUFFER_S = chosen_streams[0]->buffer;
STREAM_Primes_ARGS stream_args_S = *((STREAM_Primes_ARGS *)chosen_streams[0]->args);

                
                if (are_events_in_head(BUFFER_F, sizeof(STREAM_Primes_out), TEMPARR3, 1)) {
                    
            if(true ) {
                
                char* e1_F; size_t i1_F;
	            char* e2_F; size_t i2_F;
	            shm_arbiter_buffer_peek(BUFFER_F, 1, (void**)&e1_F, &i1_F,(void**) &e2_F, &i2_F);
            
                STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.Prime.n;

STREAM_Primes_out * event_for_p = (STREAM_Primes_out *) get_event_at_index(e1_F, i1_F, e2_F, i2_F, sizeof(STREAM_Primes_out), 0);
int p = event_for_p->cases.Prime.p;


                stream_args_F.pos
= process(n, p, stream_args_F.pos
, stream_args_S.pos
);
         
            }
            
                    	shm_arbiter_buffer_drop(BUFFER_F, 1);

                }
            }
            
            }
                    
            }
int arbiter() {
    while (exists_open_streams()) {
    	RULE_SET_rs();
	}
    shm_monitor_set_finished(monitor_buffer);
}
    
int main(int argc, char **argv) {
	initialize_events(); // Always call this first
	arbiter_counter = malloc(sizeof(int));
	*arbiter_counter = 10;
	init_intmap(&buf);
 
	stream_args_P_0.pos = 0;
	stream_args_P_1.pos = 0;


	// connect to event source P_0
	EV_SOURCE_P_0 = shm_stream_create("P_0", argc, argv);
	BUFFER_P0 = shm_arbiter_buffer_create(EV_SOURCE_P_0,  sizeof(STREAM_Primes_out), 8);

	// connect to event source P_1
	EV_SOURCE_P_1 = shm_stream_create("P_1", argc, argv);
	BUFFER_P1 = shm_arbiter_buffer_create(EV_SOURCE_P_1,  sizeof(STREAM_Primes_out), 8);


     // activate buffers
	shm_arbiter_buffer_set_active(BUFFER_P0, true);
	shm_arbiter_buffer_set_active(BUFFER_P1, true);

 	monitor_buffer = shm_monitor_buffer_create(sizeof(STREAM_NumberPairs_out), 64);
 	
 		// init buffer groups
	init_buffer_group(&BG_Ps);
	bg_insert(&BG_Ps, EV_SOURCE_P_0, BUFFER_P0,&stream_args_P_0,Ps_ORDER_EXP);
	bg_insert(&BG_Ps, EV_SOURCE_P_1, BUFFER_P1,&stream_args_P_1,Ps_ORDER_EXP);
        

 	
     // create source-events threads
	thrd_create(&THREAD_P_0, PERF_LAYER_P,BUFFER_P0);
	thrd_create(&THREAD_P_1, PERF_LAYER_P,BUFFER_P1);


     // create arbiter thread
     thrd_create(&ARBITER_THREAD, arbiter, 0);
     
 
    // monitor
    STREAM_NumberPairs_out * received_event;
    while(true) {
        received_event = fetch_arbiter_stream(monitor_buffer);
        if (received_event == NULL) {
            break;
        }

		if (received_event->head.kind == 2) {
			int i = received_event->cases.NumberPair.i;
			int n = received_event->cases.NumberPair.n;
			int m = received_event->cases.NumberPair.m;
            //printf("monitor: %d %d %d\n", i, n, m);

		  if (true ) {
		      if(n!=m)
         {
           printf("Error at index %i: %i is not equal to %i\n", i, n, m);
         }
     
		  }
		}else {
            printf("%d\n", received_event->head.kind);
        }
        
        shm_monitor_buffer_consume(monitor_buffer, 1);
    }
    
 	
//	destroy_buffer_group(&BG_Ps);

	// destroy event sources
	shm_stream_destroy(EV_SOURCE_P_0);
	shm_stream_destroy(EV_SOURCE_P_1);


	// destroy arbiter buffers
	shm_arbiter_buffer_free(BUFFER_P0);
	shm_arbiter_buffer_free(BUFFER_P1);

	
	
}

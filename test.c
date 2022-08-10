#include "shamon.h"
#include "mmlib.h"
#include "monitor.c"
#include <threads.h>
#include <stdio.h>


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
        

bool SHOULD_KEEP_Left(shm_stream * s, shm_event * e) {
    if (e->kind == 2) {
        return true;
    }
        
    return false;
}
bool SHOULD_KEEP_Right(shm_stream * s, shm_event * e) {
    if (e->kind == 2) {
        return true;
    }
        
    return false;
}


int count_event_streams = 2;

// declare event streams
shm_stream *EV_SOURCE_Left;
shm_stream *EV_SOURCE_Right;

//declare flags for event streams
bool is_Left_done;
bool is_Right_done;

// event sources threads
thrd_t THREAD_Left;
thrd_t THREAD_Right;

// declare arbiter thread
thrd_t ARBITER_THREAD;

// Arbiter buffer for event source Left
shm_arbiter_buffer *BUFFER_Left;

// Arbiter buffer for event source Right
shm_arbiter_buffer *BUFFER_Right;


// monitor buffer
shm_monitor_buffer *monitor_buffer;

int PERF_LAYER_Left (void *arg) {
    shm_arbiter_buffer *buffer = BUFFER_Left;
    shm_stream *stream = shm_arbiter_buffer_stream(buffer);   

    STREAM_Primes_in *inevent;
    STREAM_Primes_out *outevent;   

    // wait for active buffer
    while ((!shm_arbiter_buffer_active(buffer))){
        sleep_ns(10);
    }
    
    is_Left_done = 0;
    while(true) {
        inevent = stream_filter_fetch(stream, buffer, &SHOULD_KEEP_Left);
        
        if (inevent == NULL) {
            // no more events
            is_Left_done = 1;
            break;
        }
        outevent = shm_arbiter_buffer_write_ptr(buffer);
        
        switch ((inevent->head).kind) {

			case 2:

				(outevent->head).kind = 2;
				(outevent->head).id = (inevent->head).id;
int n  = inevent->cases.Prime.n ;
int p  = inevent->cases.Prime.p ;

				outevent->cases.Prime.n = n ;
				outevent->cases.Prime.p = p ;

				break;
            default:
                printf("Default case executed in thread for event source Left. Exiting thread...");
                return 1;
        }
        shm_arbiter_buffer_write_finish(buffer);
        shm_stream_consume(stream, 1);
    }     
}

int PERF_LAYER_Right (void *arg) {
    shm_arbiter_buffer *buffer = BUFFER_Right;
    shm_stream *stream = shm_arbiter_buffer_stream(buffer);   

    STREAM_Primes_in *inevent;
    STREAM_Primes_out *outevent;   

    // wait for active buffer
    while ((!shm_arbiter_buffer_active(buffer))){
        sleep_ns(10);
    }
    
    is_Right_done = 0;
    while(true) {
        inevent = stream_filter_fetch(stream, buffer, &SHOULD_KEEP_Right);
        
        if (inevent == NULL) {
            // no more events
            is_Right_done = 1;
            break;
        }
        outevent = shm_arbiter_buffer_write_ptr(buffer);
        
        switch ((inevent->head).kind) {

			case 2:

				(outevent->head).kind = 2;
				(outevent->head).id = (inevent->head).id;
int n  = inevent->cases.Prime.n ;
int p  = inevent->cases.Prime.p ;

				outevent->cases.Prime.n = n ;
				outevent->cases.Prime.p = p ;

				break;
            default:
                printf("Default case executed in thread for event source Right. Exiting thread...");
                return 1;
        }
        shm_arbiter_buffer_write_finish(buffer);
        shm_stream_consume(stream, 1);
    }     
}

bool exists_open_streams() {
    int c = 0;
	c += is_Left_done;
	c += is_Right_done;
	return c < count_event_streams;
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

int RULE_SET_rs(int *);

int RULE_SET_rs(int *arbiter_counter) {
    STREAM_NumberPairs_out *outevent;   
    
        int TEMPARR0[] = {2};
int TEMPARR1[] = {2};

    if (are_events_in_head(BUFFER_Left, sizeof(STREAM_Primes_out), TEMPARR0, 1) && are_events_in_head(BUFFER_Right, sizeof(STREAM_Primes_out), TEMPARR1, 1)) {
        if(true ) {
            
                char* e1_Left; size_t i1_Left;
	            char* e2_Left; size_t i2_Left;
	            shm_arbiter_buffer_peek(BUFFER_Left, 1, (void**)&e1_Left, &i1_Left,(void**) &e2_Left, &i2_Left);
            
                char* e1_Right; size_t i1_Right;
	            char* e2_Right; size_t i2_Right;
	            shm_arbiter_buffer_peek(BUFFER_Right, 1, (void**)&e1_Right, &i1_Right,(void**) &e2_Right, &i2_Right);
            
            STREAM_Primes_out * event_for_ln = (STREAM_Primes_out *) get_event_at_index(e1_Left, i1_Left, e2_Left, i2_Left, sizeof(STREAM_Primes_out), 0);
int ln = event_for_ln->cases.Prime.n;

STREAM_Primes_out * event_for_lp = (STREAM_Primes_out *) get_event_at_index(e1_Left, i1_Left, e2_Left, i2_Left, sizeof(STREAM_Primes_out), 0);
int lp = event_for_lp->cases.Prime.p;

STREAM_Primes_out * event_for_rn = (STREAM_Primes_out *) get_event_at_index(e1_Right, i1_Right, e2_Right, i2_Right, sizeof(STREAM_Primes_out), 0);
int rn = event_for_rn->cases.Prime.n;

STREAM_Primes_out * event_for_rp = (STREAM_Primes_out *) get_event_at_index(e1_Right, i1_Right, e2_Right, i2_Right, sizeof(STREAM_Primes_out), 0);
int rp = event_for_rp->cases.Prime.p;


            if(ln == rn)
 		   {
 		       

        outevent = shm_monitor_buffer_write_ptr(monitor_buffer);
         outevent->head.kind = 2;
    outevent->head.id = (*arbiter_counter)++;
    ((STREAM_NumberPairs_out *) outevent)->cases.NumberPair.i = ln;
((STREAM_NumberPairs_out *) outevent)->cases.NumberPair.n = lp;
((STREAM_NumberPairs_out *) outevent)->cases.NumberPair.m = rp;

         shm_monitor_buffer_write_finish(monitor_buffer);
        	shm_arbiter_buffer_drop(BUFFER_Left, 1);
	shm_arbiter_buffer_drop(BUFFER_Right, 1);

}
 		   else if(ln < rn)
 		   {
 		       	shm_arbiter_buffer_drop(BUFFER_Left, 1);

}
 		   else
 		   {
 		       	shm_arbiter_buffer_drop(BUFFER_Right, 1);

}
 		
        }
    }
        
        int TEMPARR2[] = {1};

    if (are_events_in_head(BUFFER_Left, sizeof(STREAM_Primes_out), TEMPARR2, 1)) {
        if(true ) {
            
                char* e1_Left; size_t i1_Left;
	            char* e2_Left; size_t i2_Left;
	            shm_arbiter_buffer_peek(BUFFER_Left, 1, (void**)&e1_Left, &i1_Left,(void**) &e2_Left, &i2_Left);
            
            STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_Left, i1_Left, e2_Left, i2_Left, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.hole.n;


            
        }
    }
        
        int TEMPARR3[] = {1};

    if (are_events_in_head(BUFFER_Right, sizeof(STREAM_Primes_out), TEMPARR3, 1)) {
        if(true ) {
            
                char* e1_Right; size_t i1_Right;
	            char* e2_Right; size_t i2_Right;
	            shm_arbiter_buffer_peek(BUFFER_Right, 1, (void**)&e1_Right, &i1_Right,(void**) &e2_Right, &i2_Right);
            
            STREAM_Primes_out * event_for_n = (STREAM_Primes_out *) get_event_at_index(e1_Right, i1_Right, e2_Right, i2_Right, sizeof(STREAM_Primes_out), 0);
int n = event_for_n->cases.hole.n;


            
        }
    }
        
}

int arbiter() {
    int arbiter_counter = 10;
    while (exists_open_streams()) {
    	RULE_SET_rs(&arbiter_counter);
	}
    shm_monitor_set_finished(monitor_buffer);
}
    
int main(int argc, char **argv) {
    initialize_events(); // Always call this first
    
	// connect to event source Left
	EV_SOURCE_Left = shm_stream_create("Left", argc, argv);
	BUFFER_Left = shm_arbiter_buffer_create(EV_SOURCE_Left,  sizeof(STREAM_Primes_out), 8);

	// connect to event source Right
	EV_SOURCE_Right = shm_stream_create("Right", argc, argv);
	BUFFER_Right = shm_arbiter_buffer_create(EV_SOURCE_Right,  sizeof(STREAM_Primes_out), 8);


    // activate buffers
	shm_arbiter_buffer_set_active(BUFFER_Left, true);
	shm_arbiter_buffer_set_active(BUFFER_Right, true);

	monitor_buffer = shm_monitor_buffer_create(sizeof(STREAM_NumberPairs_out), 64);

    // create source-events threads
	thrd_create(&THREAD_Left, PERF_LAYER_Left,0);
	thrd_create(&THREAD_Right, PERF_LAYER_Right,0);


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

		  if (true ) {
		      if(n!=m){
           printf("Error at index %i: %i is not equal to %i\n", i, n, m);
         }
     
		  }
		}
        
    }
    
     
    // destroy event sources
	shm_stream_destroy(EV_SOURCE_Left);
	shm_stream_destroy(EV_SOURCE_Right);

    // destroy arbiter buffers
	shm_arbiter_buffer_free(BUFFER_Left);
	shm_arbiter_buffer_free(BUFFER_Right);

	// destroy monitor buffer
	// shm_monitor_buffer_destroy(monitor_buffer);
}


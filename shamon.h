#ifndef SHAMON_H_
#define SHAMON_H_


typedef struct _shamon shamon;
typedef struct _shm_event shm_event;
typedef struct _shm_stream shm_stream;

shamon *shamon_create(void);
void shamon_destroy(shamon *);

void shamon_add_stream(shamon *, shm_stream *);
shm_event *shamon_get_next_ev(shamon *);



#endif // SHAMON_H_

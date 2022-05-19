#include "source.h"

size_t source_control_get_records_num(struct source_control *sc) {
    return ((sc->size - sizeof(struct source_control))/sizeof(struct event_record));
}

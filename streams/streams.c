#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "stream-drregex.h"
#include "stream-funs.h"
#include "stream-generic.h"
#include "stream-regex.h"
#include "stream-regexrw.h"

const char *find_next_part(const char *params) {
    params = strchr(params, ':');
    if (!params || *(params + 1) == '\0')
        return NULL;
    return params + 1;
}

const char *get_next_part(const char *params, char out[256], char delim) {
    /* get next file */
    if (!params || *params == '\0')
        return NULL;

    const char *param_end = strchr(params + 1, delim);
    if (param_end) {
        assert(params - param_end < 256 && "Parameter too long");
        strncpy(out, params, param_end - params);
        out[param_end - params] = 0;
        return param_end + 1;
    } else {
        strcpy(out, params);
        return NULL;
    }
}
static const char *get_spec(const char *stream_name, int argc, char *argv[]) {
    size_t slen = strlen(stream_name);
    for (int i = 1; i < argc; ++i) {
        if (strncmp(stream_name, argv[i], slen) == 0) {
            return argv[i];
        }
    }
    return NULL;
}

struct stream_rec {
    const char *name;
    const char *descr;
};

static struct stream_rec avail_streams[] = {
    {"calls", "connect to dynamorio libfuns.so and track calls of a function"},
    {"files", "open given files and read from them (the files can be pipes)"},
    {"regex", "read stdin and parse it using regexes"},
    {"regexrw", "read stdin and parse it using regexes in a separate process"},
    {"drregex",
     "read stdin and stdout and parse it using regexes (DynamoRIO based)"},
    {"generic", "receive events based purely on the information from source"},
    {NULL, NULL} /* to mark the end */
};

shm_stream *shm_stream_create(const char *stream_name, const char *spec) {
    const char *next = find_next_part(spec); /* skip the name */
    if (!next) {
        fprintf(stderr,
                "error: no source specified for stream with name '%s'\n",
                stream_name);
        return NULL;
    }

    char source[256] = {0};
    next = get_next_part(next, source, ':');
    if (source[0] == 0) {
        fprintf(stderr, "error: invalid source specified\n");
        return NULL;
    }

    if (strncmp(source, "calls", 6) == 0) {
        if (!next || *next == 0) {
            fprintf(
                stderr,
                "error: source 'calls' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'calls' takes only one parameter, "
                            "ignoring others\n");
            return NULL;
        }

        return shm_create_funs_stream(key);
    } else if (strncmp(source, "regex", 6) == 0) {
        if (!next || *next == 0) {
            fprintf(
                stderr,
                "error: source 'regex' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'regex' ignoring further "
                            "parameter (FOR NOW)\n");
        }

        return shm_create_sregex_stream(key);
    } else if (strncmp(source, "regexrw", 8) == 0) {
        if (!next || *next == 0) {
            fprintf(
                stderr,
                "error: source 'regexrw' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'regexrw' ignoring further "
                            "parameter (FOR NOW)\n");
        }

        return shm_create_sregexrw_stream(key);
    } else if (strncmp(source, "drregex", 8) == 0) {
        if (!next || *next == 0) {
            fprintf(
                stderr,
                "error: source 'drregex' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'drregex' ignoring further "
                            "parameter (FOR NOW)\n");
        }

        return shm_create_drregex_stream(key);
    } else if (strncmp(source, "generic", 8) == 0) {
        if (!next || *next == 0) {
            fprintf(
                stderr,
                "error: source 'generic' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'generic' ignoring further "
                            "parameter (FOR NOW)\n");
        }

        return shm_create_generic_stream(key);
    }

    fprintf(stderr, "Unknown stream. Available streams:\n");
    struct stream_rec *it = &avail_streams[0];
    while (it->name != NULL) {
        fprintf(stderr, "  %-16s\t:%s\n", it->name, it->descr);
        ++it;
    }
    abort();
    return NULL;
}

shm_stream *shm_stream_create_from_argv(const char *stream_name, int argc, char *argv[]) {
    const char *spec = get_spec(stream_name, argc, argv);
    if (!spec) {
        fprintf(stderr, "error: did not find spec for stream '%s'\n",
                stream_name);
        return NULL;
    }

    return shm_stream_create(stream_name, spec);
}

/* This is something that most of the streams do, so have it as a helper fun */
size_t stream_mk_event_kinds(const char *stream_name, struct buffer *shmbuffer,
                             size_t *max_ev_size) {
    size_t evs_num;
    size_t ev_size, max_size = 0;
    struct event_record *events = buffer_get_avail_events(shmbuffer, &evs_num);
    for (size_t i = 0; i < evs_num; ++i) {
        ev_size = events[i].size;
        events[i].kind = shm_mk_event_kind(events[i].name, ev_size,
                                           (const char *)events[i].signature);
        if (ev_size > max_size)
            max_size = ev_size;

        printf("[%s] event '%s', kind: '%lu', size: '%lu', signature: '%s'\n",
               stream_name, events[i].name, events[i].kind, events[i].size,
               events[i].signature);
    }

    if (max_ev_size)
        *max_ev_size = max_size;

    return evs_num;
}

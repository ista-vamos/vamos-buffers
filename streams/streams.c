#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#include "stream-funs.h"
#include "stream-regex.h"
#include "stream-drregex.h"

/*
static const char *get_arg(const char *name, size_t len,
                           int argc, char *argv[]) {
        for (int i = 0; i < argc; ++i) {
            if (strncmp(argv[i], name, len) == 0) {
                return argv[i];
            }
        }
        return NULL;
}


const char *get_next_part(const char *params, char out[256]) {
    const char *part_end = find_next_part(params);
    if (!part_end)
        return NULL;
    strncpy(out, params, part_end - params - 1);
    return part_end;
}
*/

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
static const char *get_params(const char *stream_name,
                              int argc, char *argv[]) {
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

static
struct stream_rec avail_streams[] = {
  {"calls", "connect to dynamorio libfuns.so and track calls of a function"},
  {"files", "open given files and read from them (the files can be pipes)"},
  {"regex", "read stdin and parse it using regexes"},
  {"drregex", "read stdin and stdout and parse it using regexes (DynamoRIO based)"},
  {NULL, NULL} /* to mark the end */
};

struct source_control;

shm_stream *shm_stream_create(const char *stream_name,
                              struct source_control **control,
                              int argc,
                              char *argv[]) {
    const char *params = get_params(stream_name, argc, argv);
    if (!params) {
        fprintf(stderr, "error: did not find spec for stream '%s'\n", stream_name);
        return NULL;
    }

    const char *next = find_next_part(params); /* skip the name */
    if (!next) {
        fprintf(stderr, "error: no source specified for stream with name '%s'\n", stream_name);
        return NULL;
    }

    char source[256] = {0};
    next = get_next_part(next, source, ':');
    if (source[0] == 0) {
        fprintf(stderr, "error: invalid source specified\n");
        return NULL;
    }

    if (strncmp(source, "calls", 5) == 0) {
        if (!next || *next == 0) {
            fprintf(stderr, "error: source 'calls' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'calls' takes only one parameter, ignoring others\n");
            return NULL;
        }

        return shm_create_funs_stream(key, control);
    } else if (strncmp(source, "regex", 5) == 0) {
        if (!next || *next == 0) {
            fprintf(stderr, "error: source 'regex' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'regex' ignoring further parameter (FOR NOW)\n");
        }

        return shm_create_sregex_stream(key, control);
    } else if (strncmp(source, "drregex", 7) == 0) {
        if (!next || *next == 0) {
            fprintf(stderr, "error: source 'drregex' needs the key to SHM as parameter\n");
            return NULL;
        }
        char key[256];
        next = get_next_part(next, key, ';');
        if (next) {
            fprintf(stderr, "warning: source 'drregex' ignoring further parameter (FOR NOW)\n");
        }

        return shm_create_drregex_stream(key, control);
    }
#if 0
    else if (strncmp(name, "files", 5) == 0) {
        const char *params = get_arg("files:", 6, argc, argv);
        if (!params) {
            fprintf(stderr, "error: stream 'files' takes as parameters files to open\n");
            return NULL;
        }

        char file[256];
        params = get_next_part(params);
        if (!params) {
            fprintf(stderr, "error: stream 'files' needs some parameters\n");
            return NULL;
        }
        params = get_next_param(params, file);
        shm_stream *s = shm_create_fds_stream();
        while (params) {
            printf("Opening file '%s' ...", file);
            int fd = open(file, O_RDONLY);
            if (fd == -1) {
                perror("open failed");
                continue;
            }
            shm_stream_fds_add_fd((shm_stream_fds*)s, fd);

            /* get next file */
            params = get_next_param(params, file);
        }
        return s;
    } else if (strncmp(name, "stdin", 5) == 0) {
        if (get_arg("stdin", 5, argc, argv)) {
            fprintf(stderr, "warning: stream 'stdin' takes no parameters, ignoring them\n");
        }
        return shm_create_stdin_stream();
    }
#endif
    fprintf(stderr, "Unknown stream. Available streams:\n");
    struct stream_rec *it = &avail_streams[0];
    while (it->name != NULL) {
        fprintf(stderr, "  %-16s\t:%s\n", it->name, it->descr);
        ++it;
    }
    abort();
    return NULL;
}

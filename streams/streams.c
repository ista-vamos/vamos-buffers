#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#include "stream-funs.h"
#include "stream-stdin.h"
#include "stream-fds.h"

static const char *get_arg(const char *name, size_t len,
                           int argc, char *argv[]) {
        for (int i = 0; i < argc; ++i) {
            if (strncmp(argv[i], name, len) == 0) {
                return argv[i];
            }
        }
        return NULL;
}

const char *get_first_param_ptr(const char *params) {
    params = strchr(params, ':');
    if (!params || *(params + 1) == '\0')
        return NULL;
    return params + 1;
}

const char *get_next_param(const char *params, char out[256]) {
    /* get next file */
    if (!params || *params == '\0')
        return NULL;

    const char *param_end = strchr(params + 1, ';');
    if (param_end) {
        assert(params - param_end < 256 && "Parameter too long");
        strncpy(out, params, params - param_end);
        out[params - param_end] = 0;
        return param_end + 1;
    } else {
        strcpy(out, params);
        return NULL;
    }
}

shm_stream *shm_stream_create(const char *name,
                              const char *signature,
                              int argc,
                              char *argv[]) {
    (void)signature;
    if (strncmp(name, "calls", 5) == 0) {
        const char *params = get_arg("calls:", 6, argc, argv);
        if (!params) {
            fprintf(stderr, "error: stream 'calls' takes as the parameter the key to SHM\n");
            return NULL;
        }

        char key[256];
        params = get_first_param_ptr(params);
        if (!params) {
            fprintf(stderr, "error: stream 'calls' needs some parameters\n");
            return NULL;
        }
        params = get_next_param(params, key);
        shm_stream *s = shm_create_funs_stream(key);
        if (get_next_param(params, key)) {
            fprintf(stderr, "warning: stream 'calls' takes just one parameter, ignoring the rest\n");
        }
        return s;

    }
#if 0
    else if (strncmp(name, "files", 5) == 0) {
        const char *params = get_arg("files:", 6, argc, argv);
        if (!params) {
            fprintf(stderr, "error: stream 'files' takes as parameters files to open\n");
            return NULL;
        }

        char file[256];
        params = get_first_param_ptr(params);
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
    return NULL;
}


add_library(syscalls SHARED
            syscalls.c
            ../fastbuf/shm_monitored.c
            ../fastbuf/shm_common.c)

use_DynamoRIO_extension(syscalls drmgr)
configure_DynamoRIO_client(syscalls)


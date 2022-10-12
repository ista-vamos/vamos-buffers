#ifndef SHMAON_SHM_BUFFER_SIZE_H
#define SHMAON_SHM_BUFFER_SIZE_H

#ifdef SHM_BUFFER_SIZE_PAGES
#error "Size of buffer already defined"
#endif

#define SHM_BUFFER_SIZE_PAGES 2

#endif

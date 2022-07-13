from ctypes import CDLL, c_char_p, c_void_p, c_int, c_size_t
from os.path import dirname, abspath, join as pathjoin

so_file = abspath(pathjoin(dirname(__file__), "..", "libshamon-lib.so"))

lib = CDLL(so_file)

lib.source_control_define_str.argtypes = [c_char_p]
lib.source_control_define_str.restype = c_void_p
lib.create_shared_buffer.argtypes = [c_char_p, c_size_t, c_void_p]
lib.create_shared_buffer.restype = c_void_p
def create_shared_buffer(key : str, elem_size : int, events: str):
    control = lib.source_control_define_str(c_char_p(events.encode('ascii')))
    return c_void_p(lib.create_shared_buffer(c_char_p(key.encode('ascii')),
                                             c_size_t(elem_size),
                                             control))

lib.destroy_shared_buffer.argtypes = [c_void_p]
def destroy_shared_buffer(buff):
    lib.destroy_shared_buffer(buff)

def buffer_monitor_attached(buff):
    return lib.buffer_monitor_attached(buff)

lib.buffer_start_push.argtypes = [c_void_p]
lib.buffer_start_push.restype = c_void_p
def buffer_start_push(buff):
    return c_void_p(lib.buffer_start_push(buff))

lib.buffer_partial_push.argtypes = [c_void_p, c_void_p, c_void_p, c_size_t]
lib.buffer_partial_push.restype = c_void_p
def buffer_partial_push(buff, addr, elem, size):
    return c_void_p(lib.buffer_partial_push(buff, addr, elem, c_size_t(size)))

lib.buffer_partial_push_str.argtypes = [c_void_p, c_void_p, c_size_t, c_void_p]
lib.buffer_partial_push_str.restype = c_void_p
def buffer_partial_push_str(buff, addr, evid, s):
    return c_void_p(lib.buffer_partial_push_str(buff, addr,
                                                evid, s))

lib.buffer_finish_push.argtypes = [c_void_p]
def buffer_finish_push(buff):
    return lib.buffer_finish_push(buff)

lib.signature_get_size.argtypes = [c_char_p]
def signature_get_size(sig):
    return lib.signature_get_size(c_char_p(sig.encode('ascii')))

if __name__ == "__main__":
    b = initialize_shared_buffer(b"/key", 32)
    lib.buffer_push(b, b'ahoj', 4)
    a = buffer_start_push(b)
    a = buffer_partial_push_str(b, a, 1, b"string!")
    buffer_finish_push(b)
    destroy_shared_buffer(b)

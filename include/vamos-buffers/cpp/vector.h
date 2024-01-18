#ifndef VAMOS_BUFFERS_CPP_VECTOR_H
#define VAMOS_BUFFERS_CPP_VECTOR_H

#include <cassert>

#error "This class must be debugged and fixed, it does not work"

namespace vamos {

template <typename ElemTy>
class vec {
    ElemTy *_data{nullptr};
    size_t _size{0};
    size_t _alloc_size{0};

   public:
    ~vec() { free(_data); }

    void swap(vec &rhs) {
        auto tmp = rhs;
        rhs = *this;
        *this = rhs;
    }

    size_t size() const { return _size; }
    size_t empty() const { return _size == 0; }
    size_t alloc_size() const { return _alloc_size; }
    constexpr size_t elem_size() { return sizeof(ElemTy); }

    ElemTy *at(size_t idx) {
        assert(idx < _size);
        return _data + idx;
    }

    ElemTy *get(size_t idx) {
        if (idx < _size)
            return _data + idx;
        return nullptr;
    }

    const ElemTy *get(size_t idx) const {
        return const_cast<const ElemTy *>(get(idx));
    }

    /**
     * Return the pointer to the next free space in the vector and increase
     * the number of elements in the vector by one.
     * The returned pointer should be used to write the element into the vector,
     * e.g.:
     *
     *   int *ptr = VEC_EXTEND_N(vec, 4);
     *   *ptr = 1;
     *
     * If there is not enough space to write a single element, the memory is
     * reallocated to hold `n` new elements.
     */
    ElemTy *extend_n(size_t n) {
        if (_size >= _alloc_size) {
            _alloc_size += n;
            _data = static_cast<ElemTy *>(
                realloc(_data, _alloc_size * elem_size()));
            assert(_data != nullptr && "Memory re-allocation failed");
        }
        assert((_size < _alloc_size) && "Vector too small");
        return _data[_size++];
    }

    ElemTy *extend() { return extend_n(16); }

    size_t push(const ElemTy &e) {
        if (_size >= _alloc_size) {
            _alloc_size += 10;
            _data = static_cast<ElemTy *>(
                realloc(_data, _alloc_size * elem_size()));
            assert(_data != nullptr && "Memory re-allocation failed");
        }
        assert((_size < _alloc_size) && "Vector too small");
        _data[_size++] = e;
        return _size;
    }

    size_t push(const ElemTy *e) { return push(*e); }

    size_t push(ElemTy &&e) {
        if (_size >= _alloc_size) {
            _alloc_size += 10;
            _data = static_cast<ElemTy *>(
                realloc(_data, _alloc_size * elem_size()));
            assert(_data != nullptr && "Memory re-allocation failed");
        }
        assert((_size < _alloc_size) && "Vector too small");
        _data[_size++] = std::move(e);
        return _size;
    }

    /**
     * Remove the last element from the vector (decrease its size by 1)
     */
    size_t pop() { return --_size; }
    void clear() { _size = 0; }

    ElemTy *back() { return &_data[_size]; }
    const ElemTy *back() const { return &_data[_size]; }

    template <typename VecTy>
    class _iterator {
       protected:
        VecTy *v;
        size_t pos;
        static const size_t npos = ~static_cast<size_t>(0);

        _iterator(VecTy &v0) : v(&v0), pos(v0._size == 0 ? npos : 0) {}
        _iterator() : pos(npos) {}
        _iterator(const _iterator &) = default;

        friend class vec;

       public:
        _iterator &operator++() {
            ++pos;
            if (pos == v->_size)
                pos = npos;
            return *this;
        }

        _iterator operator++(int) {
            auto tmp = *this;
            operator++();
            return tmp;
        }

        bool operator==(const _iterator &rhs) const { return pos == rhs.pos; }
        bool operator!=(const _iterator &rhs) const { return !operator==(rhs); }
    };

    struct iterator : public _iterator<vec> {
        iterator(vec &v) : _iterator<vec>(v) {}
        iterator() : _iterator<vec>() {}
        iterator(const iterator &) = default;

        ElemTy &operator*() { return this->v->_data[this->pos]; }
        ElemTy *operator->() { return &this->v->_data[this->pos]; }
    };

    struct const_iterator : public _iterator<const vec> {
        const_iterator(const vec &v) : _iterator<const vec>(v) {}
        const_iterator() : _iterator<const vec>() {}
        const_iterator(const const_iterator &) = default;

        const ElemTy &operator*() const { return this->v->_data[this->pos]; }
        const ElemTy *operator->() const { return &this->v->_data[this->pos]; }
    };

    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(); }
    const_iterator begin() const { return const_iterator(*this); }
    const_iterator end() const { return const_iterator(); }
};

}  // namespace vamos

#endif

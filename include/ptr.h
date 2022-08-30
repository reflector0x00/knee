#pragma once
#include <cstdint>
#include <type_traits>

// TODO: I still don't know how to fight reinterpret_cast in constexpr
//  so it's kinda temporary solution
template<typename P>
union ptr_t_base {
    P _ptr;
    void* _vptr;
    const void* _cvptr;
    uint32_t _iptr;
public:
    constexpr ptr_t_base() = default;
    constexpr ptr_t_base(const ptr_t_base& other) = default;
    constexpr ptr_t_base(ptr_t_base&& other) = default;
    constexpr ptr_t_base(std::nullptr_t ptr) : _ptr(ptr) {}
    constexpr ptr_t_base(void* ptr) : _vptr(ptr) {}
    constexpr ptr_t_base(const void* ptr) : _cvptr(ptr) {}
    constexpr ptr_t_base(uint32_t ptr) : _iptr(ptr) {}

    constexpr ptr_t_base& operator=(const ptr_t_base& other) = default;
    constexpr ptr_t_base& operator=(ptr_t_base&& other) = default;

    template <typename T>
    constexpr ptr_t_base& operator +=(const T& other) {
        _iptr += other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator -=(const T& other) {
        _iptr -= other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator *=(const T& other) {
        _iptr *= other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator /=(const T& other) {
        _iptr /= other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator &=(const T& other) {
        _iptr &= other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator |=(const T& other) {
        _iptr |= other;
        return *this;
    };
    template <typename T>
    constexpr ptr_t_base& operator ^=(const T& other) {
        _iptr ^= other;
        return *this;
    };

    template <typename T>
    constexpr ptr_t_base operator +(const T& other) const {
        return _iptr + other;
    }
    template <typename T>
    constexpr ptr_t_base operator -(const T& other) const {
        return _iptr - other;
    }
    template <typename T>
    constexpr ptr_t_base operator *(const T& other) const {
        return _iptr * other;
    }
    template <typename T>
    constexpr ptr_t_base operator /(const T& other) const {
        return _iptr / other;
    }
    template <typename T>
    constexpr ptr_t_base operator &(const T& other) const {
        return _iptr & other;
    }
    template <typename T>
    constexpr ptr_t_base operator |(const T& other) const {
        return _iptr | other;
    }
    template <typename T>
    constexpr ptr_t_base operator ^(const T& other) const {
        return _iptr ^ other;
    }


    constexpr bool operator >(ptr_t_base other) const {
        return _ptr > other._ptr;
    }
    constexpr bool operator <(ptr_t_base other) const {
        return _ptr < other._ptr;
    }
    constexpr bool operator <=(ptr_t_base other) const {
        return _ptr <= other._ptr;
    }
    constexpr bool operator >=(ptr_t_base other) const {
        return _ptr >= other._ptr;
    }
    constexpr bool operator ==(ptr_t_base other) const {
        return _ptr == other._ptr;
    }
    constexpr bool operator !=(ptr_t_base other) const {
        return _ptr != other._ptr;
    }

    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator >(const T& other) const {
        return _vptr > other;
    }
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator <(const T& other) const {
        return _vptr < other;
    }
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator >=(const T& other) const {
        return _vptr >= other;
    }
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator <=(const T& other) const {
        return _vptr <= other;
    }
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator ==(const T& other) const {
        return _vptr == other;
    }
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    constexpr bool operator !=(const T& other) const {
        return _vptr != other;
    }


    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator >(const T& other) const {
        return _iptr > other;
    }
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator <(const T& other) const {
        return _iptr < other;
    }
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator >=(const T& other) const {
        return _iptr >= other;
    }
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator <=(const T& other) const {
        return _iptr <= other;
    }
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator ==(const T& other) const {
        return _iptr == other;
    }
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr bool operator !=(const T& other) const {
        return _iptr != other;
    }

    constexpr typename std::remove_pointer<P>::type& operator->() const {
        return *_ptr;
    }
    constexpr typename std::remove_pointer<P>::type& operator[](std::size_t i) const {
        return _ptr[i];
    }


    template<typename T>
    operator T*() const {
        return reinterpret_cast<T*>(_vptr);
    }
    constexpr operator void*() const {
        return _vptr;
    }
    constexpr operator uint32_t() const {
        return _iptr;
    }
};


typedef ptr_t_base<uint8_t*> ptr_t;
typedef ptr_t_base<const uint8_t*> const_ptr_t;

static_assert(sizeof(ptr_t) == 4);
static_assert(sizeof(const_ptr_t) == 4);

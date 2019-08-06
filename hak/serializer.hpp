#pragma once

// this is a slightly modified version of the serializer used in the higan emulator made by byuu
// https://github.com/byuu/higan/blob/master/nall/serializer.hpp

//serializer: a class designed to save and restore the state of classes.
//
//benefits:
//- data() will be portable in size (it is not necessary to specify type sizes.)
//- data() will be portable in endianness (always stored internally as little-endian.)
//- one serialize function can both save and restore class states.
//
//caveats:
//- only plain-old-data can be stored. complex classes must provide serialize(serializer&);
//- floating-point usage is not portable across different implementations

#include <utility>
#include <cstring>

#include <types.hpp>

namespace hak {

    struct serializer;

    template<typename T>
    struct has_serialize {
        template<typename C> static auto test(decltype(std::declval<C>().serialize(std::declval<serializer&>()))*) -> char;
        template<typename C> static auto test(...) -> long;
        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };

    struct serializer {
        enum Mode : u32 { Load, Save, Size };

        explicit operator bool() const {
            return _size;
        }

        auto mode() const -> Mode {
            return _mode;
        }

        auto data() const -> const u8* {
            return _data;
        }

        auto size() const -> u32 {
            return _size;
        }

        auto capacity() const -> u32 {
            return _capacity;
        }

        template<typename T> auto real(T& value) -> serializer& {
            u32 size = sizeof(T);
            // potentially not cross-platform safe
            auto p = (u8*)&value;
            if(_mode == Save) {
                for(u32 n = 0; n < size; n++) _data[_size++] = p[n];
            } else if(_mode == Load) {
                for(u32 n = 0; n < size; n++) p[n] = _data[_size++];
            } else {
                _size += size;
            }
            return *this;
        }

        template<typename T> auto boolean(T& value) -> serializer& {
            if(_mode == Save) {
                _data[_size++] = (bool)value;
            } else if(_mode == Load) {
                value = (bool)_data[_size++];
            } else if(_mode == Size) {
                _size += 1;
            }
            return *this;
        }

        template<typename T> auto integer(T& value) -> serializer& {
            u32 size = std::is_same<bool, T>::value ? 1 : sizeof(T);
            if(_mode == Save) {
                T copy = value;
                for(u32 n = 0; n < size; n++) _data[_size++] = copy, copy >>= 8;
            } else if(_mode == Load) {
                value = 0;
                for(u32 n = 0; n < size; n++) value |= (T)_data[_size++] << (n << 3);
            } else if(_mode == Size) {
                _size += size;
            }
            return *this;
        }

        template<typename T> auto enumeration(T& value) -> serializer& {
            u32 copy = (u32) value;
            u32 size = sizeof(value);
            if(_mode == Save) {
                for(u32 n = 0; n < size; n++) _data[_size++] = copy, copy >>= 8;
            } else if(_mode == Load) {
                copy = 0;
                for(u32 n = 0; n < size; n++) copy |= (u32)_data[_size++] << (n << 3);
                value = static_cast<T>(copy);
            } else if(_mode == Size) {
                _size += size;
            }
            return *this;
        }

        template<typename T, int N> auto array(T (&array)[N]) -> serializer& {
            for(u32 n = 0; n < N; n++) operator()(array[n]);
            return *this;
        }

        template<typename T> auto array(T array, u32 size) -> serializer& {
            for(u32 n = 0; n < size; n++) operator()(array[n]);
            return *this;
        }


        template<typename T> auto operator()(T& value, typename std::enable_if<has_serialize<T>::value>::type* = 0) -> serializer& { value.serialize(*this); return *this; }
        template<typename T> auto operator()(T& value, typename std::enable_if<std::is_integral<T>::value>::type* = 0) -> serializer& { return integer(value); }
        template<typename T> auto operator()(T& value, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0) -> serializer& { return real(value); }
        template<typename T> auto operator()(T& value, typename std::enable_if<std::is_enum<T>::value>::type* = 0) -> serializer& { return enumeration(value); }
        template<typename T> auto operator()(T& value, typename std::enable_if<std::is_array<T>::value>::type* = 0) -> serializer& { return array(value); }
        template<typename T> auto operator()(T& value, u32 size, typename std::enable_if<std::is_pointer<T>::value>::type* = 0) -> serializer& { return array(value, size); }

        serializer() = default;

        explicit serializer(u32 capacity) {
            _mode = Save;
            _data = new u8[capacity]();
            _size = 0;
            _capacity = capacity;
        }

        serializer(const u8* data, u32 capacity) {
            _mode = Load;
            _data = new u8[capacity];
            _size = 0;
            _capacity = capacity;
            memcpy(_data, data, capacity);
        }

        ~serializer() {
            if(_data) delete[] _data;
        }

    private:
        Mode _mode = Size;
        u8* _data = nullptr;
        u32 _size = 0;
        u32 _capacity = 0;
    };

}

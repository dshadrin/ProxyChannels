#pragma once
//////////////////////////////////////////////////////////////////////////
//     xx xx	 	word	 	TLV type number
//     xx xx	 	word	 	TLV length value
//     -----------------------------------------
//              ......	TLV data
///////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <type_traits>
#include <limits>
#include <boost/endian/conversion.hpp>

///////////////////////////////////////////////////////////////////////////
#define BITS_IN_BYTE 8
static_assert(sizeof(uint32_t) == sizeof(float), "Unexpected float size");
static_assert(sizeof(uint64_t) == sizeof(double), "Unexpected double size");

namespace oscar
{
//     typedef uint8_t base_symbol_t;
    typedef char base_symbol_t;
    typedef std::vector<base_symbol_t> flap_storage;
    constexpr const size_t TLV_HEADER_SIZE = sizeof(uint16_t) * 2;

    namespace tlv
    {
        //////////////////////////////////////////////////////////////////////////
        // Set TLV type
        inline void set_type(uint16_t _Type, base_symbol_t* _Ptr)
        {
            const uint16_t type = boost::endian::native_to_big(_Type);
            memcpy(_Ptr, &type, sizeof(uint16_t));
        }

        //////////////////////////////////////////////////////////////////////////
        // Set TLV data size
        template<class T>
        void set_data_size(base_symbol_t* _Ptr, typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type _Data)
        {
            const uint16_t size = boost::endian::native_to_big(static_cast<uint16_t>(sizeof(_Data)));
            memcpy(_Ptr + sizeof(uint16_t), &size, sizeof(uint16_t));
        }

        template<class T>
        void set_data_size(base_symbol_t* _Ptr, typename std::enable_if<std::is_arithmetic<typename T::value_type>::value || std::is_enum<typename T::value_type>::value, T>::type const& _Data)
        {
            const uint16_t size = boost::endian::native_to_big(static_cast<uint16_t>(_Data.size() * sizeof(typename T::value_type)));
            memcpy(_Ptr + sizeof(uint16_t), &size, sizeof(uint16_t));
        }

        template<class T>
        void set_data_size(base_symbol_t* _Ptr, typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type _Data, size_t _Count)
        {
            const uint16_t size = boost::endian::native_to_big(static_cast<uint16_t>(sizeof(_Data) * _Count));
            memcpy(_Ptr + sizeof(uint16_t), &size, sizeof(uint16_t));
        }

        //////////////////////////////////////////////////////////////////////////
        // store data to TLV
        template<class T>
        void store_data(base_symbol_t* _PtrPlace, typename std::enable_if<std::is_integral<T>::value, T>::type _Data)
        {
            const T val = boost::endian::native_to_big(_Data);
            memcpy(_PtrPlace, &val, sizeof(T));
        }

        template<class T>
        void store_data(base_symbol_t* _PtrPlace, typename std::enable_if<std::is_enum<T>::value, T>::type _Data)
        {
            store_data<typename std::underlying_type<T>::type>(_PtrPlace, static_cast<typename std::underlying_type<T>::type>(_Data));
        }

        template<class T>
        void store_data(base_symbol_t* _PtrPlace, typename std::enable_if<std::is_same<T, float>::value, T>::type _Data)
        {
            store_data<uint32_t>(_PtrPlace, *reinterpret_cast<uint32_t*>(&_Data));
        }

        template<class T>
        void store_data(base_symbol_t* _PtrPlace, typename std::enable_if<std::is_same<T, double>::value, T>::type _Data)
        {
            store_data<uint64_t>(_PtrPlace, *reinterpret_cast<uint64_t*>(&_Data));
        }

        template<class T>
        void store_data(base_symbol_t* _PtrPlace, typename std::enable_if<std::is_arithmetic<typename T::value_type>::value || std::is_enum<typename T::value_type>::value, T>::type const& _Data)
        {
            const size_t size = sizeof(typename T::value_type) * _Data.size();
            base_symbol_t* ptr = _PtrPlace;

            size_t off = 0;
            for (auto v : _Data)
            {
                store_data<typename T::value_type>(ptr + off, v);
                off += sizeof(typename T::value_type);
            }
        }

        template<>
        inline void store_data<std::string>(base_symbol_t* _PtrPlace, std::string const& _Data)
        {
            memcpy(_PtrPlace, _Data.c_str(), _Data.length());
        }

        //////////////////////////////////////////////////////////////////////////
        // store C-array data to TLV
        template<class T>
        void store_data_array(base_symbol_t* _Ptr,
            const typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type* _PData,
            uint16_t _Count)
        {
            const uint16_t size = (uint16_t)(sizeof(T) * _Count);
            base_symbol_t* ptr = _Ptr + TLV_HEADER_SIZE;

            for (size_t off = 0, i = 0; off < size; off += sizeof(T), ++i)
            {
                store_data<T>(ptr + off, _PData[i]);
            }
        }

        ///////////////////////////////////////////////////////////////////////////
        // create TLV
        ///////////////////////////////////////////////////////////////////////////
        template<typename T>
        void create(uint16_t _Type, const T& _Val, base_symbol_t* _Ptr)
        {
            set_type(_Type, _Ptr);
            set_data_size<T>(_Ptr, _Val);
            store_data<T>(_Ptr + TLV_HEADER_SIZE, _Val);
        }

        // bool, char, int and other integral type, float, double, enum
        template<typename T>
        void create(uint16_t _Type,
            const typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type* _PVal,
            size_t _Count,
            base_symbol_t* _Ptr)
        {
            assert(_Count < std::numeric_limits<uint16_t>::max() / sizeof(T));
            set_type(_Type, _Ptr);
            set_data_size<T>(_Ptr, static_cast<T>(0), _Count);
            store_data_array<T>(_Ptr, _PVal, (uint16_t)_Count);
        }

        ///////////////////////////////////////////////////////////////////////////
        // return TLV type
        inline uint16_t get_type(const base_symbol_t* _Ptr)
        {
            uint16_t _Ret;
            memcpy(&_Ret, _Ptr, sizeof(uint16_t));
            return boost::endian::big_to_native(_Ret);
        }

        //////////////////////////////////////////////////////////////////////////
        // return TLV data size
        inline uint16_t get_data_size(const base_symbol_t* _Ptr)
        {
            uint16_t _Ret;
            memcpy(&_Ret, _Ptr + sizeof(uint16_t), sizeof(uint16_t));
            return boost::endian::big_to_native(_Ret);
        }

        //////////////////////////////////////////////////////////////////////////
        // return TLV full size
        inline uint16_t get_size(const base_symbol_t* _Ptr)
        {
            return TLV_HEADER_SIZE + get_data_size(_Ptr);
        }

        //////////////////////////////////////////////////////////////////////////
        // calculate full size
        template<typename T>
        size_t calc_size(typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type)
        {
            constexpr const size_t size = 2 * sizeof(uint16_t) + sizeof(T);
            return size;
        }

        template<typename T>
        size_t calc_size(const typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type*, size_t elem_count)
        {
            const size_t size = 2 * sizeof(uint16_t) + sizeof(T) * elem_count;
            return size;
        }

        // std containers (string, vector, list, etc.)
        template<typename T>
        size_t calc_size(typename std::enable_if<std::is_arithmetic<typename T::value_type>::value || std::is_enum<typename T::value_type>::value, T>::type const& _Val)
        {
            const size_t size = (2 * sizeof(uint16_t) + _Val.size() * sizeof(typename T::value_type));
            return size;
        }

        ///////////////////////////////////////////////////////////////////////////
        // utilities
        ///////////////////////////////////////////////////////////////////////////
        template<typename T>
        void big_to_native_inplace(typename std::enable_if<std::is_integral<T>::value, T>::type& _Elem)
        {
            boost::endian::big_to_native_inplace(_Elem);
        }

        template<typename T>
        void big_to_native_inplace(typename std::enable_if<std::is_same<T, float>::value, T>::type& _Elem)
        {
            boost::endian::big_to_native_inplace(*reinterpret_cast<uint32_t*>(&_Elem));
        }

        template<typename T>
        void big_to_native_inplace(typename std::enable_if<std::is_same<T, double>::value, T>::type& _Elem)
        {
            boost::endian::big_to_native_inplace(*reinterpret_cast<uint64_t*>(&_Elem));
        }

        template<typename T>
        void big_to_native_inplace(typename std::enable_if<std::is_enum<T>::value, T>::type& _Elem)
        {
            boost::endian::big_to_native_inplace(*reinterpret_cast<typename std::underlying_type<T>::type*>(&_Elem));
        }

        //////////////////////////////////////////////////////////////////////////
        // return TLV value
        ///////////////////////////////////////////////////////////////////////////
        template<typename T>
        T get_value_item(const base_symbol_t* _PtrPlace)
        {
            T _Ret;
            memcpy(&_Ret, _PtrPlace, sizeof(T));
            big_to_native_inplace<T>(_Ret);
            return (T)_Ret;
        }

//         template<typename T>
//         T get_value_item(const char* _PtrPlace)
//         {
//             T _Ret;
//             memcpy(&_Ret, _PtrPlace, sizeof(T));
//             big_to_native_inplace<T>(_Ret);
//             return (T)_Ret;
//         }

        //////////////////////////////////////////////////////////////////////////
        template<typename T>
        typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, T>::type get_value(const base_symbol_t* _Ptr)
        {
            assert(sizeof(T) == get_data_size(_Ptr));
            return get_value_item<T>(_Ptr + TLV_HEADER_SIZE);
        }

        // std containers (vector, list, etc.)
        template<typename T>
        typename std::enable_if<std::is_arithmetic<typename T::value_type>::value || std::is_enum<typename T::value_type>::value, T>::type get_value(const base_symbol_t* _Ptr)
        {
            T _Ret;
            const size_t len = get_data_size(_Ptr);
            const base_symbol_t* ptr = _Ptr + TLV_HEADER_SIZE;
            
            for (size_t off = 0; off < len; off += sizeof(typename T::value_type))
            {
                _Ret.push_back(get_value_item<typename T::value_type>(ptr + off));
            }
            return _Ret;
        }

        template<>
        inline std::string get_value<std::string>(const base_symbol_t* _Ptr)
        {
            std::string _Ret;
            const size_t len = get_data_size(_Ptr);
            _Ret.assign((const char*)_Ptr + TLV_HEADER_SIZE, len);
            return _Ret;
        }

        //////////////////////////////////////////////////////////////////////////
        // get arrays from TLV as std::vector<T>
        //////////////////////////////////////////////////////////////////////////
        template<typename T>
        std::vector<T> get_array(const base_symbol_t* _Ptr)
        {
            std::vector<T> _Ret;
            const size_t size = get_data_size(_Ptr);
            const base_symbol_t* pdata = _Ptr + TLV_HEADER_SIZE;
            for (size_t off = 0; off < size; off += sizeof(T))
            {
                T _Elem;
                memcpy(&_Elem, pdata + off, sizeof(T));
                big_to_native_inplace<T>(_Elem);
                _Ret.push_back(_Elem);
            }
            return _Ret;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

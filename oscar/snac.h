#pragma once
//////////////////////////////////////////////////////////////////////////
//     xx xx	 	word	 	Family(service) id number
//     xx xx	 	word	 	Family subtype id number
//     xx xx	 	word	 	SNAC flags
//     xx xx xx xx	dword	 	SNAC request id
//     --------------------------------------------------
//              ......	    SNAC data
///////////////////////////////////////////////////////////////////////////

#include "tlv.h"
#include <tuple>
#include <numeric>

///////////////////////////////////////////////////////////////////////////

namespace oscar
{
    constexpr const size_t SNAC_HEADER_SIZE = sizeof(uint16_t) * 3 + sizeof(uint32_t);
    constexpr const size_t FLAP_HEADER_SIZE = sizeof(uint8_t) * 2 + sizeof(uint16_t) * 2;
    const uint8_t FLAP_MARK = 0x2A;

    namespace ostd
    {
        //////////////////////////////////////////////////////////////////////////
        class snac_base
        {
        public:
            snac_base( ) : size( SNAC_HEADER_SIZE ) {}

        protected:
            static uint32_t generate_request_id( );
            size_t size;

        public:
            ~snac_base( ) {}
        };

        //////////////////////////////////////////////////////////////////////////
        template<typename _Snac_Traits>
        class snac : private snac_base
        {
        public:
            typedef _Snac_Traits SnacTraits;
            static constexpr const size_t ArgNum = std::tuple_size<typename SnacTraits::TlvData>::value;

            snac( uint16_t _flags = 0xFFFF ) : flags( _flags ), request_id( generate_request_id( ) ) {}

            // add data to SNAC
            template<typename ..._Types>
            void add_data( flap_storage& _Stg, _Types&&... args )
            {
                size_t array[sizeof...(args)+1] = { elem_size( args )..., 0 };
                calc_size( array, sizeof...(args) );
                data = std::make_tuple( args... );

                _Stg.resize( get_size( ) + FLAP_HEADER_SIZE );
                base_symbol_t* ptr = fill_header( &_Stg[FLAP_HEADER_SIZE] );
                store_all_data( ptr, std::make_index_sequence<sizeof...(_Types)>{} );
            }

            // size of SNAC
            uint16_t get_size( ) const { return static_cast<uint16_t>(size); }

            // get data
            const typename SnacTraits::TlvData& get_data( ) const { return data; }

        private:
            template<typename Y>
            size_t elem_size( Y elem ) const
            {
                return tlv::calc_size<Y>( elem );
            }

            // calculate SNAC size (call after add_data)
            void calc_size( const size_t* array, size_t count )
            {
                size = std::accumulate( array, array + count, SNAC_HEADER_SIZE );
                if ( size > std::numeric_limits<uint16_t>::max( ) )
                {
                    std::runtime_error( "Overflow SNAC size" );
                }
            }

            // fill SNAC header
            base_symbol_t* fill_header(base_symbol_t* _Ptr ) const
            {
                base_symbol_t* ptr = _Ptr;
                tlv::store_data<decltype(ServiceId)>( ptr, ServiceId );
                ptr += sizeof( ServiceId );
                tlv::store_data<decltype(SubtypeId)>( ptr, SubtypeId );
                ptr += sizeof( SubtypeId );
                tlv::store_data<decltype(flags)>( ptr, flags );
                ptr += sizeof( flags );
                tlv::store_data<decltype(request_id)>( ptr, request_id );
                ptr += sizeof( request_id );
                return ptr;
            }

            // store data to raw
            template<int NUM>
            base_symbol_t* store_data(base_symbol_t*& _Ptr )
            {
                base_symbol_t* ptr = _Ptr;
                typedef typename std::tuple_element<NUM, typename SnacTraits::TlvData>::type _ElemType;
                typedef typename std::tuple_element<NUM, typename SnacTraits::TlvTraits>::type _ElemTraits;
                tlv::create<_ElemType>( _ElemTraits::TlvTypeVal, std::get<NUM>( data ), _Ptr );
                _Ptr += tlv::calc_size<_ElemType>( std::get<NUM>( data ) );
                return ptr;
            }

            // store all data to snac memory
            template<size_t... I>
            void store_all_data(base_symbol_t*& _Ptr, std::index_sequence<I...> )
            {
                base_symbol_t* ptrs[sizeof...(I)+1] = { store_data<I>( _Ptr )..., 0 };
            }

        private:
            enum : uint16_t { ServiceId = SnacTraits::ServiceId };
            enum : uint16_t { SubtypeId = SnacTraits::SubtypeId };
            const uint16_t flags;
            const uint32_t request_id;
            typename SnacTraits::TlvData data;

        public:
            ~snac( ) {}
        };
    }

}

///////////////////////////////////////////////////////////////////////////


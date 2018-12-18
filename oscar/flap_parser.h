#pragma once
//////////////////////////////////////////////////////////////////////////
#include "snac.h"
#include "proto.h"
#include "flap_parser.h"
//////////////////////////////////////////////////////////////////////////

namespace oscar
{
    //////////////////////////////////////////////////////////////////////////
    static constexpr const size_t FLAP_DATA_SIZE_OFFSET = 2 * sizeof( uint8_t ) + sizeof( uint16_t );
    static constexpr const size_t FLAP_CHANNEL_OFFSET = sizeof( uint8_t );

    namespace ostd
    {
        //////////////////////////////////////////////////////////////////////////
        class flap_parser
        {
        public:
            //////////////////////////////////////////////////////////////////////////
            // TLV iterator
            //////////////////////////////////////////////////////////////////////////
            class tlv_iterator
            {
            public:
                tlv_iterator( const flap_parser& _Fp )
                    : fp( _Fp )
                    , begin( _Fp.end_ptr( ) )
                    , end( _Fp.end_ptr( ) ) {}

                tlv_iterator( const flap_parser& _Fp, const base_symbol_t* _Begin )
                    : fp( _Fp )
                    , begin( _Begin )
                    , end( _Begin == _Fp.end_ptr( ) ? _Begin : _Begin + tlv::get_size( _Begin ) )
                {
                    if ( end > _Fp.end_ptr( ) )
                    {
                        throw std::runtime_error( "Parsing SNAC structure error" );
                    }
                }

                tlv_iterator operator++( )
                {
                    begin = end;
                    if ( end != fp.end_ptr( ) )
                    {
                        end += tlv::get_size( begin );
                        if ( end > fp.end_ptr( ) )
                        {
                            throw std::runtime_error( "Iterate SNAC structure error" );
                        }
                    }
                    return *this;
                }

                bool operator !=( const tlv_iterator& rh )
                {
                    return begin != rh.begin;
                }

                const base_symbol_t* get( ) const { return begin; }

            private:
                const flap_parser& fp;
                const base_symbol_t* begin;
                const base_symbol_t* end;
            };

            friend tlv_iterator;
            typedef tlv_iterator iterator;

        public:
            //////////////////////////////////////////////////////////////////////////
            // FLAP parser members
            //////////////////////////////////////////////////////////////////////////
            flap_parser( const flap_storage& _Stg );
            ~flap_parser( ) = default;

            FlapChannel get_channel( ) const { return static_cast<FlapChannel>(stg[FLAP_CHANNEL_OFFSET]); }
            uint16_t    get_snac_size( ) const { return tlv::get_value_item<uint16_t>( &stg[FLAP_DATA_SIZE_OFFSET] ); }
            SnacService get_snac_service( ) const { return static_cast<SnacService>(tlv::get_value_item<uint16_t>( &stg[FLAP_HEADER_SIZE] )); }
            SnacSubtype get_snac_subtype( ) const { return static_cast<SnacSubtype>(tlv::get_value_item<uint16_t>( &stg[FLAP_HEADER_SIZE + sizeof( uint16_t )] )); }
            uint16_t    get_snac_flags( ) const { return tlv::get_value_item<uint16_t>( &stg[FLAP_HEADER_SIZE + sizeof( uint16_t ) * 2] ); }
            uint32_t    get_snac_reqid( ) const { return tlv::get_value_item<uint32_t>( &stg[FLAP_HEADER_SIZE + sizeof( uint16_t ) * 3] ); }

            iterator begin( ) const
            {
                const base_symbol_t* first = &stg[FLAP_HEADER_SIZE] + SNAC_HEADER_SIZE;
                return tlv_iterator( *this, first );
            }

            iterator end( ) const
            {
                return tlv_iterator( *this );
            }

            size_t size( ) const { return tlv_count; }

        private:
            const base_symbol_t* end_ptr( ) const { return &stg[FLAP_HEADER_SIZE] + get_snac_size( ); }
            size_t calc_tlv_count( );

        private:
            flap_storage stg;
            size_t tlv_count;
        };
    }
}

//////////////////////////////////////////////////////////////////////////

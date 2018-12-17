#pragma once
//////////////////////////////////////////////////////////////////////////
//     2A	 	byte	 	FLAP id byte
//     xx	 	byte	 	FLAP channel
//     xx xx	word	 	FLAP datagram seq number
//     xx xx	word	 	FLAP data size
//     ---------------------------------------------
//              ......	FLAP data
///////////////////////////////////////////////////////////////////////////

#include "proto.h"
#include "snac.h"

///////////////////////////////////////////////////////////////////////////

namespace oscar
{
    namespace ostd
    {
        //////////////////////////////////////////////////////////////////////////
        class flap_base
        {
        public:
            flap_base( ) : stg( FLAP_HEADER_SIZE, 0 ) {}
            ~flap_base() {}

        protected:
            flap_storage stg;
        };

        ///////////////////////////////////////////////////////////////////////////
        template<FlapChannel _Channel, typename _SnacTraits>
        class flap_builder : private flap_base
        {
        public:
            flap_builder( ) : sequence_number( 0 ) {}
            ~flap_builder() {}

            template<typename ..._Types>
            void add_data( _Types&&... args )
            {
                snac_data.add_data( stg, args... );
                stg[0] = FLAP_MARK;
                stg[1] = channel;
                tlv::store_data<uint16_t>( &stg[sizeof( uint16_t )], sequence_number );
                tlv::store_data<uint16_t>( &stg[OFFSET_SNAC_SIZE], snac_data.get_size( ) );
            }

            const flap_storage& get_storage( ) const { return stg; }

        private:
            static constexpr const size_t OFFSET_SNAC_SIZE = sizeof( uint8_t ) * 2 + sizeof( uint16_t );
            enum : uint8_t { channel = (uint8_t)_Channel };
            uint16_t sequence_number;
            snac<_SnacTraits> snac_data;
        };

        //////////////////////////////////////////////////////////////////////////
        template<FlapChannel _Channel, typename _SnacTraits, typename ..._Types>
        flap_storage make_flap_package( _Types&&... args )
        {
            flap_builder<_Channel, _SnacTraits> _flap;
            _flap.add_data( args... );
            return _flap.get_storage( );
        }
    }

}

///////////////////////////////////////////////////////////////////////////

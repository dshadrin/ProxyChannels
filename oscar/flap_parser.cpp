/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "flap_parser.h"

//////////////////////////////////////////////////////////////////////////

namespace oscar
{
    //////////////////////////////////////////////////////////////////////////
    ostd::flap_parser::flap_parser( const flap_storage& _Stg ) : stg( _Stg )
    {
        if ( _Stg.empty( ) ||
            _Stg[0] != FLAP_MARK ||
            !(get_channel( ) >= FlapChannel::FlapChannelBegin && get_channel( ) < FlapChannel::FlapChannelEnd) ||
            _Stg.size( ) < get_snac_size( ) + FLAP_HEADER_SIZE )
        {
            throw std::runtime_error( "Incorrect flap structure" );
        }

        tlv_count = calc_tlv_count( );
    }

    //////////////////////////////////////////////////////////////////////////
    size_t ostd::flap_parser::calc_tlv_count( )
    {
        size_t num = 0;
        for ( iterator it = begin( ); it != end( ); ++it ) ++num;
        return num;
    }

}

//////////////////////////////////////////////////////////////////////////

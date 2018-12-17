#pragma once
//////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <tuple>
#include <string>
#include "tlv.h"

//////////////////////////////////////////////////////////////////////////
enum class FlapChannel : uint8_t
{
    FlapChannelBegin,
// ------------------------------    
    Control = (uint8_t) FlapChannelBegin,
    Message,
// ------------------------------    
    FlapChannelEnd
};

enum class SnacService : uint16_t
{
    SnacServiceBegin = (uint16_t) FlapChannel::FlapChannelEnd,
// ------------------------------    
    Start = (uint16_t)SnacServiceBegin,
    Stop,
    Message,
// ------------------------------    
    SnacServiceEnd
};

enum class SnacSubtype : uint16_t
{
    SnacSubtypeBegin = (uint16_t) SnacService::SnacServiceEnd,
// ------------------------------    
    LogFileName = (uint16_t) SnacSubtypeBegin,
    LogStop,
    LogMessage,
// ------------------------------    
    SnacSubtypeEnd
};

enum class TlvType : uint16_t
{
    TlvTypeBegin = (uint16_t) SnacSubtype::SnacSubtypeEnd,
// ------------------------------    
    LogModule = (uint16_t)TlvTypeBegin,
    LogTimestampSec,
    LogTimestampMicrosec,
    LogSeverity,
    LogText,
    LogNamePrefix,
    LogNameSuffix,
    LogChannel,
// ------------------------------    
    TlvTypeEnd
};

namespace oscar
{
    //////////////////////////////////////////////////////////////////////////
    namespace ostd
    {
        template<TlvType _TlvType, typename _DataType>
        struct Tlv_Traits
        {
            enum : uint16_t { TlvTypeVal = (uint16_t)_TlvType };
            typedef _DataType DataType;
        };

        // ATTENTION!!! 'long int' type packs as int64_t for prevent possible issues in compile time
        typedef Tlv_Traits<TlvType::LogModule, std::string> Tlv_ModuleTag;
        typedef Tlv_Traits<TlvType::LogTimestampSec, time_t> Tlv_TimestampSec;
        typedef Tlv_Traits<TlvType::LogTimestampMicrosec, int64_t> Tlv_TimestampMicrosec;
        typedef Tlv_Traits<TlvType::LogSeverity, uint8_t> Tlv_Severity;
        typedef Tlv_Traits<TlvType::LogText, std::string> Tlv_LogText;
        typedef Tlv_Traits<TlvType::LogNamePrefix, std::string> Tlv_LogNamePrefix;
        typedef Tlv_Traits<TlvType::LogNameSuffix, std::string> Tlv_LogNameSuffix;
        typedef Tlv_Traits<TlvType::LogChannel, int8_t> Tlv_LogChannel;

        template<SnacService _ServiceId, SnacSubtype _SubtypeId, typename ..._Tlv>
        struct Snac_Traits
        {
            enum : uint16_t { ServiceId = (uint16_t)_ServiceId };
            enum : uint16_t { SubtypeId = (uint16_t)_SubtypeId };
            typedef std::tuple<_Tlv...> TlvTraits;
            typedef std::tuple<typename _Tlv::DataType...> TlvData;
        };

        typedef Snac_Traits<SnacService::Start, SnacSubtype::LogFileName, Tlv_LogNamePrefix, Tlv_LogNameSuffix, Tlv_LogChannel, Tlv_TimestampSec, Tlv_TimestampMicrosec> Snac_NewFile;
        typedef Snac_Traits<SnacService::Stop, SnacSubtype::LogStop, Tlv_LogChannel, Tlv_TimestampSec, Tlv_TimestampMicrosec> Snac_Stop;
        typedef Snac_Traits<SnacService::Message, SnacSubtype::LogMessage, Tlv_ModuleTag, Tlv_Severity, Tlv_LogText, Tlv_LogChannel, Tlv_TimestampSec, Tlv_TimestampMicrosec> Snac_Message;

        //////////////////////////////////////////////////////////////////////////
        template<typename TlvTraits>
        typename TlvTraits::DataType tlv_parse( const uint8_t * _TlvPlace )
        {
            if ( tlv::get_type( _TlvPlace ) != TlvTraits::TlvTypeVal )
            {
                throw std::runtime_error( "TLV protocol error" );
            }

            return tlv::get_value<typename TlvTraits::DataType>( _TlvPlace );
        }
    }
}


//////////////////////////////////////////////////////////////////////////

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
///////////////////////////////////////////////////////////////////////////

#include "snac.h"
#include "tlv.h"
#include "flap_builder.h"
#include <atomic>
#include <cassert>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////

namespace oscar
{
    ///////////////////////////////////////////////////////////////////////
    uint32_t ostd::snac_base::generate_request_id()
    {
        static std::atomic<uint32_t> num;
#ifdef _DEBUG
        if (num == 0)
        {
            num.fetch_add(111);
        }
#endif // _DEBUG
        return num.fetch_add(1);
    }

    //////////////////////////////////////////////////////////////////////////
}


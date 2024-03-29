/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

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


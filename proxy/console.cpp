// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "console.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CConsoleActor, "CON ");

//////////////////////////////////////////////////////////////////////////

CConsoleActor::CConsoleActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_ioService(CManager::instance()->IoService())
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    ,m_in(m_ioService, ::dup(stdin))
#endif
{

}

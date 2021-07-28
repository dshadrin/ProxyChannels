/*
 * Copyright (C) 2014-2017 Rhonda Software.
 * All rights reserved.
 */

#include "stdinc.h"
#include "terminal.h"
#include "manager.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CTerminal, "UART");

//////////////////////////////////////////////////////////////////////////
CTerminal::CTerminal(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_ioService(CManager::instance()->IoService()),
    m_pt(pt),
    m_inBuffSize(pt.get<uint16_t>("input_buffer_size", 1024)),
    m_strPortName(MakePortName(pt)),
    m_ReadBuffer(new std::vector<char>(m_inBuffSize)),
    bWriteInProgress(false),
    bIsOpened(false)
{
    Setup();
}

//////////////////////////////////////////////////////////////////////////
CTerminal::~CTerminal()
{
    LOG_INFO << "Destroyed UART client (port = " << GetName() << ")";
}

//////////////////////////////////////////////////////////////////////////
bool CTerminal::Setup()
{
    boost::system::error_code ec;
    try
    {
        if (!CManager::instance()->IsStopped())
        {
            m_port.reset( new boost::asio::serial_port( m_ioService ) );
            m_port->open( m_strPortName, ec );

            if (ec)
            {
                SetupAsync();
                return false;
            }
            else
            {
                try
                {
                    std::string valueStr;
                    std::ostringstream oss;
                    oss << UART_CLIENT " connected on " << m_strPortName << std::endl;

                    unsigned int baud = m_pt.get<unsigned int>( "option.baud_rate", 115200 );
                    boost::asio::serial_port_base::baud_rate baud_option( baud );
                    m_port->set_option( baud_option ); // set the baud rate after the m_port has been opened
                    oss << "                      baud rate         : " << baud << std::endl;

                    boost::asio::serial_port_base::flow_control flow_control_option( FlowControlType( m_pt, &valueStr ) );
                    m_port->set_option( flow_control_option ); // set the baud rate after the m_port has been opened
                    oss << "                      flow control      : " << valueStr << std::endl;

                    boost::asio::serial_port_base::parity parity_option( ParityType( m_pt, &valueStr ) );
                    m_port->set_option( parity_option ); // set the baud rate after the m_port has been opened
                    oss << "                      parity            : " << valueStr << std::endl;

                    boost::asio::serial_port_base::stop_bits stop_bits_option( StopBits( m_pt, &valueStr ) );
                    m_port->set_option( stop_bits_option ); // set the baud rate after the m_port has been opened
                    oss << "                      stop bits         : " << valueStr << std::endl;

                    unsigned int chSize = m_pt.get<unsigned int>( "option.character_size", 8 );
                    boost::asio::serial_port_base::character_size character_size_option( chSize );
                    m_port->set_option( character_size_option ); // set the baud rate after the m_port has been opened
                    oss << "                      character size    : " << chSize << std::endl;

                    LOG_INFO << oss.str();
                    if (m_sigOutputMessage.empty())
                    {
                        m_sigOutputMessage.connect( std::bind( &CTerminal::DoWrite, this, std::placeholders::_1 ) );
                    }
                    bIsOpened = true;
                }
                catch (std::exception& e)
                {
                    APP_EXCEPTION_ERROR( GMSG << "Error UART set option on port " << m_strPortName << ": " << e.what() );
                }
            }

            if (!m_WriteBuffer.empty())
            {
                bWriteInProgress = true;
                WriteStart();
            }
            ReadStart();
        }
    }
    catch (const std::exception&)
    {

    }
    return true;
}

void CTerminal::SetupAsync()
{
    bIsOpened = false;
    bWriteInProgress = false;
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    if (CManager::instance() != nullptr)
    {
        if (!CManager::instance()->IsStopped())
        {
            thread_pool& tp = CManager::instance()->ThreadPool();
            tp.SetWorkUnit( std::bind( &CTerminal::Setup, this ), false );
        }
    }
}

void CTerminal::Start()
{
    // do nothing
}

void CTerminal::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
    m_sigInputMessage.disconnect_all_slots();
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
    DoClose(ec);
}

//////////////////////////////////////////////////////////////////////////
void CTerminal::ReadStart()
{ // Start an asynchronous read and call read_complete when it completes or fails
    m_port->async_read_some(boost::asio::buffer(*m_ReadBuffer.get()),
                           std::bind(&CTerminal::ReadComplete,
                               this,
                               std::placeholders::_1,
                               std::placeholders::_2));
}

//////////////////////////////////////////////////////////////////////////
void CTerminal::ReadComplete(const boost::system::error_code& error, size_t bytes_transferred)
{ // the asynchronous read operation has now completed or failed and returned an error
    if (!error)
    { // read completed, so process the data
        if( (bytes_transferred > 1) )
        {
            m_ReadBuffer->resize(bytes_transferred);
            PMessage msg(m_ReadBuffer.release());
            m_ReadBuffer.reset(new std::vector<char>(m_inBuffSize));
            m_sigInputMessage(msg);
        }
        ReadStart(); // start waiting for another asynchronous read again
    }
    else
    {
        if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
        {
            LOG_WARN << UART_CLIENT " disconnected from " << m_strPortName;
            SetupAsync();
            return; // ignore it because the connection canceled the timer
        }
        DoClose(error);
    }
}
//////////////////////////////////////////////////////////////////////////    
void CTerminal::DoClose(const boost::system::error_code& error)
{ // something has gone wrong, so Close the socket & make this object inactive
    if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
        return; // ignore it because the connection canceled the timer

    m_port->close();

    if (error)
    {
        LOG_WARN << "Error UART: " << error.message();
    }
}
//////////////////////////////////////////////////////////////////////////
void CTerminal::DoWrite(PMessage msg)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    boost::algorithm::replace_all(*msg, "\r\n", "\n");
    for (char ch : *msg)
    {
        m_WriteBuffer.push(ch); // store in write buffer
    }

    if (!bWriteInProgress)
    {
        bWriteInProgress = true;
        WriteStart();
    }
}
//////////////////////////////////////////////////////////////////////////
void CTerminal::WriteStart(void)
{
    if (bIsOpened)
    {
        m_port->async_write_some( boost::asio::buffer( &m_WriteBuffer.front(), 1 ),
                                  std::bind( &CTerminal::WriteComplete,
                                             this,
                                             std::placeholders::_1 ) );
    }
}
//////////////////////////////////////////////////////////////////////////
void CTerminal::WriteComplete(const boost::system::error_code& error)
{
    if (bIsOpened)
    {
        if (!error)
        {
            std::unique_lock<std::mutex> lock( m_mtx );
            m_WriteBuffer.pop();  // remove the completed data
            if (!m_WriteBuffer.empty()) // if there is anything left to be written
            {
                bWriteInProgress = true;
                WriteStart();          // then start sending the next item in the buffer
            }
            else
            {
                bWriteInProgress = false;
            }
        }
        else
        {
            DoClose( error );
        }
    }
}

std::string CTerminal::MakePortName(boost::property_tree::ptree& pt)
{
    std::string portName;
    try
    {
        uint16_t pNumber = pt.get<uint16_t>("port");
        std::string pName =
#ifdef WIN32
            pt.get<std::string>("port_name.windows", "COM");
#else
            pt.get<std::string>("port_name.linux", "/dev/tty");
#endif
        portName = pName + std::to_string(pNumber);
    }
    catch (const std::exception& e)
    {
        APP_EXCEPTION_ERROR(GMSG << "Error UART configuration: " << e.what());
    }
    return portName;
}

boost::asio::serial_port_base::flow_control::type CTerminal::FlowControlType(boost::property_tree::ptree& pt, std::string* str)
{
    const std::string fc = boost::algorithm::to_upper_copy(pt.get<std::string>("option.flow_control", "NONE"));

    boost::asio::serial_port_base::flow_control::type fcType = boost::asio::serial_port_base::flow_control::none;
    if (boost::algorithm::starts_with(fc, "SOFT"))
    {
        fcType = boost::asio::serial_port_base::flow_control::software;
        if (str)
            *str = "XON/XOFF";
    }

    else if (boost::algorithm::starts_with(fc, "HARD"))
    {
        fcType = boost::asio::serial_port_base::flow_control::hardware;
        if (str)
            *str = "HARD";
    }

    else
    {
        if (str)
            *str = "NONE";
    }

    return fcType;
}

boost::asio::serial_port_base::parity::type CTerminal::ParityType(boost::property_tree::ptree& pt, std::string* str)
{
    const std::string pStr = boost::algorithm::to_upper_copy(pt.get<std::string>("option.parity", "NONE"));

    boost::asio::serial_port_base::parity::type parity = boost::asio::serial_port_base::parity::none;
    if (pStr == "ODD")
    {
        parity = boost::asio::serial_port_base::parity::odd;
        if (str)
            *str = "ODD";
    }

    else if (pStr == "EVEN")
    {
        parity = boost::asio::serial_port_base::parity::even;
        if (str)
            *str = "EVEN";
    }

    else
    {
        if (str)
            *str = "NONE";
    }

    return parity;
}

boost::asio::serial_port_base::stop_bits::type CTerminal::StopBits(boost::property_tree::ptree& pt, std::string* str)
{
    const std::string sbStr = boost::algorithm::to_upper_copy(pt.get<std::string>("option.parity", "1"));

    boost::asio::serial_port_base::stop_bits::type sb = boost::asio::serial_port_base::stop_bits::one;
    if (sbStr == "1.5")
    {
        sb = boost::asio::serial_port_base::stop_bits::onepointfive;
        if (str)
            *str = "1.5";
    }

    else if (sbStr == "2")
    {
        sb = boost::asio::serial_port_base::stop_bits::two;
        if (str)
            *str = "2";
    }

    else
    {
        if (str)
            *str = "1";
    }

    return sb;
}

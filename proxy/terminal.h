/*
 * Copyright (C) 2018 Rhonda Software.
 * All rights reserved.
 */

//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
#include "actor.h"
#include <boost/asio/serial_port_base.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class CTerminal : public CActor
{
public:
    explicit CTerminal(boost::property_tree::ptree& pt);
    virtual ~CTerminal();

    void Start(void) override;
    void Stop(void) override;
    std::string GetName() const override { return m_strPortName; }

private:
    void ReadStart(void);
    void ReadComplete(const boost::system::error_code& error, size_t bytes_transferred);
    void DoClose(const boost::system::error_code& error);
    void DoWrite(PMessage msg);
    void WriteStart(void);
    void WriteComplete(const boost::system::error_code& error);

    static std::string MakePortName(boost::property_tree::ptree& pt);
    static boost::asio::serial_port_base::flow_control::type FlowControlType(boost::property_tree::ptree& pt, std::string* str);
    static boost::asio::serial_port_base::parity::type ParityType(boost::property_tree::ptree& pt, std::string* str);
    static boost::asio::serial_port_base::stop_bits::type StopBits(boost::property_tree::ptree& pt, std::string* str);

private:
    boost::asio::io_service&                m_ioService;  // the main IO service that runs this connection
    const size_t                            m_inBuffSize; // maximum amount of data to read in one operation
    const std::string                       m_strPortName;
    boost::asio::serial_port                m_port;       // the serial m_port this instance is connected to
    std::unique_ptr<std::vector<char>>      m_ReadBuffer;
    boost::mutex                            m_mtx;
    std::queue<char>                        m_WriteBuffer;// buffered write data
    bool                                    bWriteInProgress;

    DECLARE_MODULE_TAG;
};

//////////////////////////////////////////////////////////////////////////

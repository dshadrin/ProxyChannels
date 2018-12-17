#include "logclient.h"
#include "utils/timestamp.h"
#include <iostream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <ctime>

#ifndef WIN32
#include <sys/time.h>
#include <boost/thread.hpp>
#endif

//////////////////////////////////////////////////////////////////////////
size_t G_TagSize = 4;
size_t G_MaxMessageSize = 4096;

//////////////////////////////////////////////////////////////////////////
std::unique_ptr<CLogClient> CLogClient::s_logClientPtr;

CLogClient::CLogClient(const boost::property_tree::ptree& pt) :
    m_thread(std::bind(&CLogClient::ThreadLogClient, this)),
    m_writeInProgress(false),
    m_stopInProgress(false),
    m_loggingMode(ELoggingMode::eNoLogging),
    m_host(pt.get<std::string>("host", "localhost")),
    m_port(pt.get<uint16_t>("port", 2003)),
    m_thresholdSeveriry(StringToSeverity(pt.get<std::string>("severity", "DEBUG"))),
    m_socket(m_ioService),
    m_transferred(0)
{
    G_TagSize = pt.get<size_t>("module_tag_size", 3);
    G_MaxMessageSize = pt.get<size_t>("max_message_size", 4096);

    size_t retryCount = pt.get<size_t>("retry", 5);
    size_t retryNumber = 1;
    bool isConnected = false;

    do
    {
        std::cout << "Try connect to logger: " << retryNumber++ << std::endl;
        isConnected = Connect();

    } while (!isConnected && retryNumber <= retryCount);

    if (!isConnected)
    {
        Stop();
    }
    else
    {
        m_loggingMode = ELoggingMode::eLoggingToServer;
        OpenNewLogFile("Log", "rpc");
    }
}

CLogClient* CLogClient::Get()
{
    static boost::mutex mtx;

    if (!s_logClientPtr)
    {
        boost::mutex::scoped_lock lock(mtx);
        if (!s_logClientPtr)
        {
            boost::property_tree::ptree pt = CLogClient::ReadConfiguration();
            s_logClientPtr.reset(new CLogClient(pt));
        }
    }
    return s_logClientPtr.get();
}

void CLogClient::Stop()
{
    CloseLogFile();
    m_stopInProgress = true;
    WaitQueueEmpty();
    if (m_socket.is_open())
    {
        m_socket.shutdown(boost::asio::socket_base::shutdown_both);
        m_socket.close();
    }
    m_ioService.stop();
    m_thread.join();
}

void CLogClient::Push(std::shared_ptr<SLogPackage> package)
{
    if (m_loggingMode == ELoggingMode::eLoggingToServer)
    {
        if (!m_stopInProgress)
        {
            m_queue.push(package);
            WriteNextMessage();
        }
    }
    else
    {
        std::cout << "[" << TS::GetTimestampStr(package->timestamp) << "][" << package->tag << "][" << SeverityToString(package->severity) << "] - " << package->message << std::endl;
    }
}


void CLogClient::WaitQueueEmpty()
{
    while (!m_queue.empty())
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
}

boost::property_tree::ptree CLogClient::ReadConfiguration()
{
    boost::property_tree::ptree ptLogger;

    try
    {
        boost::filesystem::path cfgname(boost::filesystem::current_path());
        cfgname /= "proxy.xml";

        if (boost::filesystem::exists(cfgname))
        {
            boost::property_tree::ptree pt;
            boost::property_tree::xml_parser::read_xml(cfgname.string(), pt);
            ptLogger = pt.get_child("proxy.logger.client");
        }
        else
        {
            std::cout << "Cannot load config file (" << cfgname << ")" << std::endl;
        }
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("Wrong configuration file.");
    }

    return std::move(ptLogger);
}

void CLogClient::ThreadLogClient()
{
    boost::asio::io_service::work work(m_ioService);
    m_ioService.run();
}

bool CLogClient::Connect()
{
    boost::asio::ip::tcp::resolver resolver(m_ioService);
    boost::asio::ip::tcp::resolver::query query(m_host, std::to_string(m_port));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    boost::system::error_code ec;
    bool status = true;
    boost::asio::connect(m_socket, iter, ec);
    if (ec.value() != 0)
    {
        status = false;
        std::cerr << "Logger connection error: " << ec.message() << std::endl;
    }
    else
    {
        std::cerr << "Logger client connected: " << m_socket.local_endpoint(ec).address().to_string() << ":" << m_socket.local_endpoint(ec).port() << " -> "
                                                 << m_socket.remote_endpoint(ec).address().to_string() << ":" << m_socket.remote_endpoint(ec).port() << std::endl ;
    }

    return status;
}

void CLogClient::WriteHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_transferred += bytesTransferred;
        if (m_transferred < m_currentMessage.size())
        { // send reminder
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(&m_currentMessage[m_transferred], m_currentMessage.size() - m_transferred),
                                     std::bind(&CLogClient::WriteHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            {
                boost::mutex::scoped_lock lock(m_mutex);
                m_writeInProgress = false;
            }
            WriteNextMessage();
        }
    }
    else
    {
        m_loggingMode = ELoggingMode::eNoLogging;
    }
}

void CLogClient::WriteNextMessage()
{
    boost::mutex::scoped_lock lock(m_mutex);
    if (!m_writeInProgress)
    {
        boost::optional<std::shared_ptr<SLogPackage>> op = m_queue.pop_try(0);
        if (op)
        {
            m_writeInProgress = true;
            std::shared_ptr<SLogPackage> package = *op;
            switch (package->command)
            {
                case ELogCommand::eMessage:
                    m_currentMessage = oscar::ostd::make_flap_package<FlapChannel::Message, oscar::ostd::Snac_Message, std::string, uint8_t, std::string, int8_t, time_t, int64_t>
                        (std::move(package->tag), std::move(package->severity), std::move(package->message), std::move(package->lchannel), std::move(package->timestamp.tv_sec), std::move(package->timestamp.tv_nsec));
                    break;
                case ELogCommand::eChangeFile:
                    m_currentMessage = oscar::ostd::make_flap_package<FlapChannel::Control, oscar::ostd::Snac_NewFile, std::string, std::string, time_t, int64_t>
                        (std::move(package->message), std::move(package->tag), std::move(package->lchannel), std::move(package->timestamp.tv_sec), std::move(package->timestamp.tv_nsec));
                    break;
                case ELogCommand::eStop:
                    m_currentMessage = oscar::ostd::make_flap_package<FlapChannel::Control, oscar::ostd::Snac_Stop, time_t, int64_t>
                        (std::move(package->lchannel), std::move(package->timestamp.tv_sec), std::move(package->timestamp.tv_nsec));
                    break;
                default:
                    throw std::runtime_error("Wrong logging package.");
            }

            m_transferred = 0;
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(&m_currentMessage[0], m_currentMessage.size()),
                                     std::bind(&CLogClient::WriteHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
}

void CLogClient::CloseLogFile()
{
    std::shared_ptr<SLogPackage> logPackage(new SLogPackage());
    logPackage->command = ELogCommand::eStop;
    Push(logPackage);
}

void CLogClient::OpenNewLogFile(std::string namePrefix, std::string nameSuffix)
{
    std::shared_ptr<SLogPackage> logPackage(new SLogPackage());
    logPackage->command = ELogCommand::eChangeFile;
    logPackage->message = namePrefix;
    logPackage->tag = nameSuffix;
    Push(logPackage);
}

void NetSendToLogger(std::shared_ptr<SLogPackage> logPackage)
{
    CLogClient::Get()->Push(logPackage);
}

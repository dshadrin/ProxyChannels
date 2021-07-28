#pragma once

//////////////////////////////////////////////////////////////////////////
namespace bpt = boost::property_tree;

//////////////////////////////////////////////////////////////////////////
class CConfigurator
{
public:
    CConfigurator(const std::string& xmlPath = "");
    ~CConfigurator() = default;

    static CConfigurator * instance();
    static void init(const std::string& xmlPath = "");

    bpt::ptree& getSubTree(const std::string& path);

    template<class T>
    T get(const std::string& path) const;

    template<class T>
    T get(const std::string& path, const T& defaultValue) const;

    void set(const std::string& path, const std::string& value);

private:
    std::unique_ptr<bpt::ptree> m_pt;

private:
    static std::mutex sConfMtx;
    static std::unique_ptr<CConfigurator> s_Configurator;
    DECLARE_MODULE_TAG;
};

template<class T>
T CConfigurator::get(const std::string& path) const
{
    T value{};

    try
    {
        value = m_pt->get<T>(path);
    }
    catch (const std::exception& e)
    {
        APP_EXCEPTION_ERROR(GMSG << "Configurator error: " << e.what());
    }
    return value;
}

template<class T>
T CConfigurator::get(const std::string& path, const T& defaultValue) const
{
    T value{};

    try
    {
        value = m_pt->get<T>(path, defaultValue);
    }
    catch (const std::exception& e)
    {
        APP_EXCEPTION_ERROR(GMSG << "Configurator error: " << e.what());
    }
    return value;
}

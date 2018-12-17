#pragma once

#include <queue>
#include <boost/thread.hpp>
#include <boost/optional.hpp>

//////////////////////////////////////////////////////////////////////////
template<typename _Type>
class CLockQueue
{
public:
    typedef _Type value_type;

    CLockQueue() = default;
    ~CLockQueue() = default;
    CLockQueue(const CLockQueue&) = delete;
    CLockQueue(CLockQueue&&) = delete;
    CLockQueue& operator=(const CLockQueue&) = delete;
    CLockQueue& operator=(CLockQueue&&) = delete;

    void push(const value_type& value)
    {
        boost::mutex::scoped_lock lock(m_mutex);
        m_queue.push(value);
        m_cond.notify_all();
    }

    value_type pop()
    {
        boost::mutex::scoped_lock lock(m_mutex);
        while (m_queue.empty())
        {
            m_cond.wait(lock);
        }

        value_type result = m_queue.front();
        m_queue.pop();
        return result;
    }

    boost::optional<value_type> pop_try(uint64_t timeoutMs)
    {
        boost::chrono::high_resolution_clock::time_point tpEnd = boost::chrono::high_resolution_clock::now() + boost::chrono::milliseconds(timeoutMs);
        boost::mutex::scoped_lock lock(m_mutex);
        while (m_queue.empty() &&
               tpEnd > boost::chrono::high_resolution_clock::now())
        {
            m_cond.wait_until(lock, tpEnd);
        }

        if (!m_queue.empty())
        {
            value_type result = m_queue.front();
            m_queue.pop();
            return boost::make_optional(std::move(result));
        }
        return boost::optional<value_type>{};
    }

    bool empty()
    {
        boost::mutex::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

private:
    boost::mutex m_mutex;
    boost::condition_variable m_cond;
    std::queue<value_type> m_queue;
};

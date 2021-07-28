#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
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
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(value);
        m_cond.notify_all();
    }

    value_type pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
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
        std::chrono::high_resolution_clock::time_point tpEnd = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeoutMs);
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty() &&
               tpEnd > std::chrono::high_resolution_clock::now())
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
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<value_type> m_queue;
};

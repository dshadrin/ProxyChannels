/*
 * Copyright (C) 2018
 * All rights reserved.
 */

#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//! \brief the structure for inform caller about finish running process
struct timer_job
{
    boost::chrono::high_resolution_clock::time_point tmPoint; //!< time point for first timer event
    boost::chrono::microseconds tmPeriod;                     //!< period of periodic timer

    //!< \note callback MUST return as fast as possible
    std::function<bool()> callback;     //!< if set, callback will called (if callback returns false then stop periodic timer)

    auto operator<(const timer_job& right) const -> bool  { return tmPoint < right.tmPoint; }
};

//! \brief Class implements thread pool functionality
class thread_pool
{
public:
    thread_pool();
    ~thread_pool();

    void SetTimer(const timer_job& tj);          //!< start new timer
    void SetWorkUnit(std::function<void()> wu);  //!< start new work unit

private:
    thread_pool(thread_pool const&) = delete;
    thread_pool& operator=(thread_pool const&) = delete;

private:
    // timers queue
    std::queue<timer_job> tj_queue;             //!< queue for new timer jobs
    boost::mutex tj_mtx;                        //!< mutex for guard tj_queue
    boost::condition_variable tj_cond;          //!< condition_variable for guard tj_queue
    void timer_thread();                        //!< work thread for timer jobs
    std::unique_ptr<boost::thread> ptj_thread;  //!< thread object for timer_thread

    // work threads queue
    std::queue<std::function<void()> > wt_queue;//!< queue for new work units
    boost::mutex wt_mtx;                        //!< mutex for guard wt_queue
    boost::condition_variable wt_cond;          //!< condition_variable for guard wt_queue
    void work_thread();                         //!< work thread for work units
    boost::thread_group wt_group;               //!< thread group for management of work threads

private:
    // common thread pool members
    uint32_t available_threads;                 //!< max number of work threads
    bool tp_stop;                               //!< common stop flag for thread_pool
    DECLARE_MODULE_TAG;
};


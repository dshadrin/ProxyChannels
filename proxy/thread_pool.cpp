/*
 * Copyright (C) 2014-2016 Rhonda Software.
 * All rights reserved.
 */

#include "stdinc.h"
#include "thread_pool.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(thread_pool, "POOL");

//////////////////////////////////////////////////////////////////////////
thread_pool::thread_pool()
    : available_threads(boost::thread::hardware_concurrency())
    , tp_stop(false)
{
    if (available_threads == 0)
    {
        available_threads = 3;
    }
}

//////////////////////////////////////////////////////////////////////////
thread_pool::~thread_pool()
{
    tp_stop = true;

    if (ptj_thread)
    {
        tj_cond.notify_one();
        ptj_thread->join();
        ptj_thread.reset(nullptr);
    }

    wt_cond.notify_all();
    wt_group.interrupt_all();
    wt_group.join_all();
}

//////////////////////////////////////////////////////////////////////////
void thread_pool::timer_thread()
{
    std::deque<timer_job> timer_jobs_queue;

    try
    {
        while ( !tp_stop )
        {
            boost::chrono::high_resolution_clock::time_point tm_point;

            if (timer_jobs_queue.empty())
            {
                tm_point = boost::chrono::high_resolution_clock::now() + boost::chrono::seconds(3600);
            }
            else
            {
                tm_point = timer_jobs_queue.front().tmPoint;
            }

            boost::unique_lock<boost::mutex> lock(tj_mtx);
            tj_cond.wait_until(lock, tm_point, [this](){ return tp_stop || !tj_queue.empty(); });

            if (tp_stop)
            {
                continue;
            }

            tm_point = boost::chrono::high_resolution_clock::now();
            if (!tj_queue.empty())
            {
                timer_job tj = tj_queue.front();
                tj_queue.pop();
                if (tj.callback)
                {
                    if (tj.tmPoint > tm_point)
                    {
                        timer_jobs_queue.push_back(tj);
                    }
                    else if (tj.tmPeriod != boost::chrono::microseconds::zero())
                    {
                        tj.tmPoint += tj.tmPeriod;
                        if (tj.tmPoint > tm_point)
                        {
                            timer_jobs_queue.push_back(tj);
                        }
                        else
                        {
                            tj.tmPoint = tm_point + tj.tmPeriod;
                            timer_jobs_queue.push_back(tj);
                        }
                    }
                    std::sort(std::begin(timer_jobs_queue), std::end(timer_jobs_queue));
                }
            }

            while ( !timer_jobs_queue.empty() && timer_jobs_queue.front().tmPoint <= tm_point )
            {
                timer_job tj = timer_jobs_queue.front();
                timer_jobs_queue.pop_front();
                if ( tj.callback() && tj.tmPeriod != boost::chrono::microseconds::zero() )
                {
                    do { tj.tmPoint += tj.tmPeriod; } while (tj.tmPoint <= tm_point);
                    timer_jobs_queue.push_back(tj);
                    std::sort(std::begin(timer_jobs_queue), std::end(timer_jobs_queue));
                }
            }

        }
    }
    catch ( ... )
    { /* do nothing */
    }
}

//////////////////////////////////////////////////////////////////////////
void thread_pool::SetTimer(const timer_job& tj)
{
    if (!ptj_thread)
    {
        ptj_thread.reset(new boost::thread(std::bind(&thread_pool::timer_thread, this)));
    }

    boost::lock_guard<boost::mutex> lock(tj_mtx);
    tj_queue.push(tj);
    tj_cond.notify_one();
}

//////////////////////////////////////////////////////////////////////////
void thread_pool::SetWorkUnit(std::function<void()> wu)
{
    if (available_threads > 0)
    {
        wt_group.create_thread(std::bind(&thread_pool::work_thread, this));
        --available_threads;
    }

    boost::lock_guard<boost::mutex> lock(wt_mtx);
    wt_queue.push(wu);
    wt_cond.notify_one();
}

//////////////////////////////////////////////////////////////////////////
void thread_pool::work_thread()
{
    try
    {
        while ( !tp_stop )
        {
            std::function<void()> work_unit;

            {
                boost::unique_lock<boost::mutex> lock(wt_mtx);
                wt_cond.wait(lock, [this](){ return tp_stop || !wt_queue.empty(); });

                if ( tp_stop )
                {
                    continue;
                }

                if (!wt_queue.empty())
                {
                    work_unit = wt_queue.front();
                    wt_queue.pop();
                }
            }

            try
            {
                if ( work_unit )
                {
                    work_unit();
                }
            }
            catch (std::exception& e)
            {
                APP_EXCEPTION_ERROR(GMSG << "Thread pool: Work unit crashed: " << e.what());
            }
        }
    }
    catch (std::exception& e)
    {
        APP_EXCEPTION_ERROR(GMSG << "Thread pool crashed: " << e.what());
    }
}

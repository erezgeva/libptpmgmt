/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy connect to a service class.
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CONNECT_SRV_HPP
#define CONNECT_SRV_HPP

#include "proxy/config_parser.hpp"
#include "common/termin.hpp"

#include <atomic>
#include <thread>

__CLKMGR_NAMESPACE_BEGIN

class ConnectSrv;

// A thread object, connecting to a service
class Thread4TimeBase
{
  private:
    std::thread self;
    friend class ConnectSrv;

  protected:
    size_t timeBaseIndex;
    // flag to indicate the thread need to stop
    bool stopThread = false;

    // The actual thread loop
    virtual void thread_loop() = 0;
    // Close the socket
    virtual void close() = 0;
    // Initilizing
    virtual bool init() = 0;

  public:

    Thread4TimeBase(size_t index) : timeBaseIndex(index) {}
    virtual ~Thread4TimeBase() = default;
};

class ConnectSrv : End
{
  private:
    // List of all threads objects
    std::map<size_t, std::unique_ptr<Thread4TimeBase>> threadList;
    /*
     * Modern CPUs are allow to run the threads before
     * the main thread end the initializing,
     * regardless of the code flow of the main thread.
     * The atomic guarantee it should not happens.
     * Threads will wait till initializing is done.
     */
    std::atomic_bool waitInit{true};

    void close_all() {
        // Close the sockets
        for(const auto &it : threadList)
            it.second->close();
        threadList.clear();
    }
    // End method, to stop all threads
    bool stop() override final {
        for(const auto &it : threadList)
            it.second->stopThread = true;
        return true;
    }
    // End method, to wait till all threads ends
    // Then close all sockets and clean the list
    bool finalize() override final {
        // Wait for threads to end
        for(const auto &it : threadList)
            it.second->self.join();
        close_all();
        return true;
    }
    // Thread call back. Wait till initilization id done
    // and then start the actual thread
    static void thread_start(ConnectSrv *parent, Thread4TimeBase *child) {
        // Ensure we start after initializing ends
        while(parent->waitInit)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        child->thread_loop();
    }
    // method to start the thread
    void start(Thread4TimeBase *child) {
        child->self = std::thread(thread_start, this, child);
    }

  protected:
    ConnectSrv() = default;
    virtual ~ConnectSrv() = default;

    // Is the configuration valid for creating a record
    virtual bool isValid(const TimeBaseCfgFull &cfg) = 0;
    // Create the record
    virtual Thread4TimeBase *alloc(size_t timeBaseIndex,
        const TimeBaseCfgFull &cfg) = 0;

  public:

    // Create all records based on configuration from JsonConfigParser
    bool registerSrv() {
        // Create all records required based on the current configuration
        for(const auto &cfg : JsonConfigParser::getInstance()) {
            if(isValid(cfg)) {
                size_t timeBaseIndex = cfg.base.timeBaseIndex;
                Thread4TimeBase *rec = alloc(timeBaseIndex, cfg);
                if(rec == nullptr) // We fail to allocate
                    return false;
                threadList[timeBaseIndex].reset(rec);
            }
        }
        // initializing before creating the threads
        for(const auto &it : threadList) {
            if(!it.second->init()) {
                // We need to close all other sockets, despite of previous success
                close_all();
                return false;
            }
        }
        // Ensure threads start after finish initializing
        waitInit.store(false);
        for(const auto &it : threadList)
            // Create a thread for each record
            start(it.second.get());
        return true;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* CONNECT_SRV_HPP */

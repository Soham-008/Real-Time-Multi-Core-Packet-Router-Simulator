#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "packet.h"

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mtx;
    std::condition_variable cv;
    bool shutdown;
    size_t max_size;
    
public:
    ThreadSafeQueue(size_t max_size = 10000) 
        : shutdown(false), max_size(max_size) {}
    
    bool push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (shutdown) return false;
        
        if (queue.size() >= max_size) {
            return false;
        }
        
        queue.push(item);
        cv.notify_one();
        return true;
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty() || shutdown; });
        
        if (shutdown && queue.empty()) {
            return false;
        }
        
        item = queue.front();
        queue.pop();
        return true;
    }
    
    bool try_pop(T& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (!cv.wait_for(lock, timeout, [this] { return !queue.empty() || shutdown; })) {
            return false;
        }
        
        if (shutdown && queue.empty()) {
            return false;
        }
        
        item = queue.front();
        queue.pop();
        return true;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
    
    void set_shutdown() {
        std::unique_lock<std::mutex> lock(mtx);
        shutdown = true;
        cv.notify_all();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        std::queue<T> empty;
        std::swap(queue, empty);
    }
};

#endif
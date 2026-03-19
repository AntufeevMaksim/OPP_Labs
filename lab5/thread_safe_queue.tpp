#pragma once

#include <pthread.h>
#include <queue>
#include <memory>
#include <atomic>

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
    {
        pthread_mutex_init(&elems_mutex, NULL);
        pthread_cond_init(&not_empty_cond, NULL);
    }
    ~ThreadSafeQueue()
    {
        pthread_mutex_destroy(&elems_mutex);
        pthread_cond_destroy(&not_empty_cond);
    }
    void push(const T &elem)
    {
        pthread_mutex_lock(&elems_mutex);
        elems.push(elem);
        pthread_cond_signal(&not_empty_cond);
        pthread_mutex_unlock(&elems_mutex);
    }
    void push(T&& elem)
    {
        pthread_mutex_lock(&elems_mutex);
        elems.push(std::move(elem));
        pthread_cond_signal(&not_empty_cond);
        pthread_mutex_unlock(&elems_mutex);  
    }
    T pop()
    {
        pthread_mutex_lock(&elems_mutex);
        while (elems.empty()) // убрать цикл?
        {
            pthread_cond_wait(&not_empty_cond,

                              &elems_mutex);
        }
        T elem = std::move(elems.front());
        elems.pop();
        pthread_mutex_unlock(&elems_mutex);
        return elem;
    }
    std::pair<T, bool> try_pop()
    {
        pthread_mutex_lock(&elems_mutex);
        if (elems.empty())
        {
            pthread_mutex_unlock(&elems_mutex);
            return make_pair(T{}, false);
        }
        T elem = std::move(elems.front());
        elems.pop();
        pthread_mutex_unlock(&elems_mutex);
        return make_pair(std::move(elem), true);
    }

    ThreadSafeQueue(ThreadSafeQueue&& other) noexcept
    {
        pthread_mutex_init(&elems_mutex, NULL);
        pthread_cond_init(&not_empty_cond, NULL);
        
        pthread_mutex_lock(&other.elems_mutex);
        
        elems = std::move(other.elems);
        pthread_mutex_unlock(&other.elems_mutex);
    }

    size_t size() const
    {
        pthread_mutex_lock(&elems_mutex);
        size_t size = elems.size();
        pthread_mutex_unlock(&elems_mutex);
        return size;
    }
private:
    std::queue<T> elems;
    mutable pthread_mutex_t elems_mutex;
    pthread_cond_t not_empty_cond;
};
#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {
    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;
        
        std::mutex m_queueMutex;
        std::condition_variable m_condition;
        bool m_stop;

    public:
        ThreadPool(size_t threads) : m_stop(false) {
            for(size_t i = 0; i < threads; ++i) {
                m_workers.emplace_back([this] {
                    for(;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->m_queueMutex);
                            this->m_condition.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
                            if(this->m_stop && this->m_tasks.empty()) return;
                            task = std::move(this->m_tasks.front());
                            this->m_tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            
            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                if(m_stop) throw std::runtime_error("enqueue on stopped ThreadPool");
                m_tasks.emplace([task](){ (*task)(); });
            }
            m_condition.notify_one();
            return res;
        }

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_stop = true;
            }
            m_condition.notify_all();
            for(std::thread &worker: m_workers) worker.join();
        }
};

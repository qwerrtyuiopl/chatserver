#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

namespace thread
{
#define DEFAULTMAX 2024
    class Mutex
    {
    public:
        Mutex()
        {
            pthread_mutex_init(&_mutex, 0);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        pthread_mutex_t *get()
        {
            return &_mutex;
        }

    private:
        pthread_mutex_t _mutex;
    };
    class Guard
    {
    public:
        Guard(Mutex &mutex) : _mutex(mutex)
        {
            _mutex.lock();
        }
        ~Guard()
        {
            _mutex.unlock();
        }

    private:
        Mutex &_mutex;
    };
    class Condition
    {
    public:
        Condition()
        {
            pthread_cond_init(&_cond, 0);
        }
        ~Condition()
        {
            pthread_cond_destroy(&_cond);
        }
        void wait(Mutex &mutex)
        {
            pthread_cond_wait(&_cond, mutex.get());
        }
        void brosdcast()
        {
            pthread_cond_broadcast(&_cond);
        }
        void signal()
        {
            pthread_cond_signal(&_cond);
        }

    private:
        pthread_cond_t _cond;
    };
    template <class T>
    class BlackQueue
    {
    public:
        BlackQueue(size_t max = DEFAULTMAX) : _max(max)
        {
        }
        void pushBack(const T &v)
        {
            {
                Guard guard(_mutex);
                while (_queue.size() >= _max)
                {
                    _cond.wait(_mutex);
                }
                _queue.push(v);
                _cond.brosdcast();
            }
        }
        T popFront()
        {
            {
                Guard Guard(_mutex);
                while (_queue.size() == 0)
                {
                    _cond.wait(_mutex);
                }
                T t(std::move(_queue.front())); // 右值拷贝构造
                _queue.pop();
                _cond.brosdcast();
                return t;
            }
        }
        /*
            不受max限制
        */
        void swap(BlackQueue<T> &other)
        {
            {
                Guard guard(_mutex);
                other.swap(_queue); // 两个都加锁
                _cond.brosdcast();
            }
        }
        void swap(std::queue<T> &q)
        {
            {
                Guard guard(_mutex);
                q.swap(_queue);
                _cond.brosdcast();
            }
        }

    private:
        Mutex _mutex;
        Condition _cond;
        size_t _max;
        std::queue<T> _queue;
    };
    class Thread
    {
    public:
        using func = std::function<void()>;
        Thread(func &f) : _func(f)
        {
        }
        Thread(func &&f) : _func(f)
        {
        }
        ~Thread()
        {
        }
        void start()
        {
            _thread = pthread_create(&_thread, NULL, run, (void *)this);
        }
        void join()
        {
            pthread_join(_thread, NULL);
        }

    private:
        static void *run(void *this_)
        {
            ((Thread *)this_)->_func();
            return NULL;
        }

    private:
        pthread_t _thread;
        func _func;
    };
    class ThreadPool
    {
    public:
        ThreadPool(size_t n) : isStart(false), endNum(0)
        {
            for (int i = 0; i < n; ++i)
            {
                _thread.push_back(Thread(std::bind(ThreadPool::run, this)));
            }
        }
        ThreadPool(ThreadPool &) = delete;
        ThreadPool(ThreadPool &&) = delete;
        // class data
        // {
        //     virtual ~data() = 0;
        // };
        // using dataPtr = std::shared_ptr<data>;
        // using reback = std::function<void()>;
        using func = std::function<void()>;
        void push_back(const func &v)
        {
            _value.pushBack(v);
        }
        void push_back(func &&v)
        {
            _value.pushBack(v);
        }
        void start()
        {
            isStart = true;
            endNum = 0;
            for (int i = 0; i < _thread.size(); i++)
                _thread[i].start();
        }
        void stop()
        {
            if (isStart)
            {
                Thread t(std::bind(ThreadPool::stopThread, this));
                t.start();
                for (int i = 0; i < _thread.size(); i++)
                {
                    _thread[i].join();
                    endNum++;
                }
                t.join();
            }
            isStart = true;
        }

    private:
        static void run(ThreadPool *this_)
        {
            std::queue<func> task;
            while (1)
            {
                this_->_value.swap(task);
                int len = task.size();
                while (len--)
                {
                    task.front()();
                    task.pop();
                }
            }
        }
        static void stopThread(ThreadPool *this_)
        {
            while (this_->endNum != this_->_thread.size())
            {
                this_->_value.pushBack([]()
                                       { pthread_exit(NULL); });
                usleep(10);
            }
        }

    private:
        BlackQueue<func> _value;
        std::vector<Thread> _thread;
        bool isStart;
        size_t endNum;
    };
}
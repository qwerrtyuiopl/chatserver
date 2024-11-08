#include <vector>
#include <memory>
#include <functional>
#include <semaphore.h>
namespace threadpool
{
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
    private:
        std::vector<T> queue;
    };
    class ThreadPool
    {
    public:
        class data
        {
            virtual ~data() = 0;
        };
        using dataPtr = std::shared_ptr<data>;
        using reback = std::function<void()>;
        using func = std::function<void(dataPtr, reback)>;
        void push_back(const func &v)
        {
            _value.push_back(v);
        }
        void push_back(func &&v)
        {
            _value.push_back(v);
        }

    private:
        std::vector<func> _value;
    };
}
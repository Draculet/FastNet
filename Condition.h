#include "Mutex.h"
#include <pthread.h>
#include <sys/time.h>

namespace net
{
	class Condition
	{
		public:
			explicit Condition(Mutex &mutex):mutex_(mutex)
			{
				pthread_cond_init(&cond_, NULL);
			}
			Condition(const Condition &)=delete;
			Condition(Condition &&)=delete;
			void operator=(const Condition &)=delete;
			~Condition()
			{
				pthread_cond_destroy(&cond_);
			}
			
			void wait()
			{
				mutex_.unassight();
				pthread_cond_wait(&cond_, mutex_.get());
				mutex_.assight();
			}
			void waitForSec(int sec)
			{
				struct timeval now;
				struct timespec fin;
				gettimeofday(&now, NULL);
				fin.tv_sec = now.tv_sec + sec;
				fin.tv_nsec = now.tv_usec * 1000;
				mutex_.unassight();
				pthread_cond_timedwait(&cond_, mutex_.get(), &fin);
				mutex_.assight();
			}
			void notify()
			{
				pthread_cond_signal(&cond_);
			}
			void notifyAll()
			{
				pthread_cond_broadcast(&cond_);
			}
			
		private:
			Mutex &mutex_;
			pthread_cond_t cond_;
	};
}

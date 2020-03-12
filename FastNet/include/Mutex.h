#ifndef __NET_MUTEX_H__
#define __NET_MUTEX_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "Noncopyable.h"
#include "Gettid.h"

namespace base
{
	class Mutex : base::noncopyable
	{
		public:
			Mutex():owner_(0){pthread_mutex_init(&mutex_, NULL);}
			~Mutex()
			{
				if (owner_ != 0)
				{
					printf("~Mutex Error\n");
					exit(-1);
				}
				pthread_mutex_destroy(&mutex_);
			}
			void lock()
			{
				pthread_mutex_lock(&mutex_);
				assight();//在锁保护下修改
			}
			void unlock()
			{
				unassight();//在锁保护下修改
				pthread_mutex_unlock(&mutex_);
			}
			pthread_mutex_t *get(){return &mutex_;}
			pid_t getOwner(){return owner_;}
		private:
			friend class Condition;
			//这两个成员配合condition使用,因为mutex多了线程与锁保持一致的成员owner
			//在使用condition时,conditon会自动解锁和上锁,由于并不是调用我们的上锁操作
			//锁与所属线程的关系无法保证一致,所以需要在wait之前调用unassight,返回之后调用assight
			void unassight(){owner_ = 0;}
			void assight(){owner_ = base::gettid();}
			
			pthread_mutex_t mutex_;
			pid_t owner_;
	};
	
	class MutexGuard
	{
		public:
			explicit MutexGuard(Mutex &mutex):mutex_(mutex)
			{
				mutex_.lock();
			}
			MutexGuard(const MutexGuard &)=delete;
			MutexGuard(MutexGuard &&)=delete;
			~MutexGuard(){mutex_.unlock();}
		private:
			Mutex &mutex_;//引用
	};
}

#endif
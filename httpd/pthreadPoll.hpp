#ifndef __PTHREADPOLL_HPP__
#define __PTHREADPOLL_HPP__

#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include "Log.hpp"

#define NUM 5

typedef int (*handler_t)(int);

class Task {
	private:
		int sock_;
		handler_t handler_;
	public:
		Task() {
			sock_ = -1;
			handler_ = NULL;
		}

		void setTask(int sock, handler_t handler) {
			sock_ = sock;
			handler_ = handler;
		}

		void runTask() {
			handler_(sock_);
		}

		~Task() {
		}
};


class threadPoll {
	private:
		int _total_num;
		int _idle_num;
		std::queue<Task> _task_queue;
		pthread_mutex_t _lock;
		pthread_cond_t _cond;
		volatile bool _is_quit;
	public:
		threadPoll(int num_ = NUM) : _total_num(num_), _idle_num(0), _is_quit(false) {
			pthread_mutex_init(&_lock, NULL);
			pthread_cond_init(&_cond, NULL);
		}

		void initPthread() {
			int i_ = 0;
			for(; i_ < _total_num; i_++) {
				pthread_t tid;
				pthread_create(&tid, NULL, thread_run, this);
			}
		}

		void pushTask(Task& t_) {
			lockQueue();
			if(_is_quit) {
				unlockQueue();
				return;
			}
			_task_queue.push(t_);
			wakeUpOneThread();
			unlockQueue();
		}

		void popTask(Task& t_) {
			t_ = _task_queue.front();
			_task_queue.pop();
		}

		void stopThread() {
			lockQueue();
			_is_quit = true;
			unlockQueue();

			while(_idle_num > 0) {
				wakeUpAllThread();
			}
			unlockQueue();
		}

		~threadPoll() {
			pthread_mutex_destroy(&_lock);
			pthread_cond_destroy(&_cond);
		}
	private:
		void lockQueue() {
			pthread_mutex_lock(&_lock);
		}

		void unlockQueue() {
			pthread_mutex_unlock(&_lock);
		}

		bool isEmpty() {
			return _task_queue.empty();
		}

		void wakeUpOneThread() {
			pthread_cond_signal(&_cond);
		}

		void wakeUpAllThread() {
			pthread_cond_broadcast(&_cond);
		}

		void threadIdle() {
			if(_is_quit) {
				unlockQueue();
				LOG(INFO, "thread quit...");
				_total_num--;
				pthread_exit((void*)1);
			}
			_idle_num++;
			pthread_cond_wait(&_cond, &_lock);
			_idle_num--;
		}

		static void *thread_run(void *arg) {
			threadPoll *pt_ = (threadPoll *)arg;
			pthread_detach(pthread_self());
			for( ; ; ) {
				pt_->lockQueue();
				while(pt_->isEmpty()) {
					pt_->threadIdle();
				}
				Task t_;
				pt_->popTask(t_);
				pt_->unlockQueue();
				LOG(INFO, "task has been taken, handler...");
				t_.runTask();
			}
		}
};

#endif

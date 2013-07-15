/*
 * main.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan
 */

#include <iostream>
#include <pthread.h>
#include "utils/block_queue.h"
#include "utils/task_manager.h"
#include "utils/timer_manager.h"
#include <gtest/gtest.h>

using namespace std;
using namespace wbase::common::utils;

#define P 10
#define C 10

block_queue<int> que(1);

void* produce(void *param)
{
	int v = *(int*)(param);
	for (int i = 0; i < P; i++) {
		que.timed_push_back(v, 100);
	}
	return NULL;
}

void* consume(void *)
{
	for (int i = 0; i < 10; i++) {
		int x;
		if (que.timed_pop_front(x, 10))
			cout << x;
	}

	return NULL;
}

class MyTask : public task
{
public:
	MyTask() { canceled = false; }
	virtual void run() {
		if (canceled) // in case being canceled before run
			return;
		for (int i = 0; i < 300; i++) {
			this->update_progress(i);
			if (canceled)
				return;
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			std::cout << (canceled ? "1" : "0");
		}
	}

	virtual void cancel() {
		std::cout << "set cancel" << std::endl;
		canceled = true;
	}

private:
	volatile bool canceled;
};

class TimerTask : public timer_task
{
public:
	TimerTask () { count = 0; canceled = false; }
	timer_task::ACTION timer_run() {
		if (canceled) {
			std::cout << "canceled" << std::endl;
			return timer_task::DONE;
		}
		std::cout << "timer run " << count++ << std::endl;
		if (count == 10) {
			std::cout << "done" << std::endl;
			return timer_task::DONE;
		} else {
			this->set_interval(1000*count);
			return timer_task::CONTINUE;
		}
	}
	virtual void cancel() {
		std::cout << "set canceled" << std::endl;
		canceled = true;
	}
private:
	int count;
	volatile bool canceled;
};


#define A(x, y) x##y

class BlockQueueTestFixture: public ::testing::Test
{

};

TEST_F(BlockQueueTestFixture, Test)
{
	timer_manager mgr;
	boost::shared_ptr<TimerTask> tt1(new TimerTask());
	tt1->set_timeout(10000);
	boost::shared_ptr<TimerTask> tt2(new TimerTask());
	tt2->set_timeout(0);
	//mgr.add(tt1);
	mgr.add(tt2);
	boost::this_thread::sleep(boost::posix_time::seconds(20));
	mgr.stop(tt2);
	boost::this_thread::sleep(boost::posix_time::seconds(100));
}

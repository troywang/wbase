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
	virtual int run() {
		if (canceled) // in case being canceled before run
			return -1;
		for (int i = 0; i < 30; i++) {
			this->update_progress(i);
			if (canceled)
				return -1;
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			std::cout << (canceled ? "1" : "0");
		}
		return 0;
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
	virtual int run() {
		if (canceled) {
			std::cout << "canceled" << std::endl;
			return -1;
		}
		std::cout << "timer run " << count++ << std::endl;
		if (count == 10) {
			std::cout << "done" << std::endl;
			return -1;
		} else {
			this->set_interval(1000*count);
			return 0;
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

int main(void)
{
	timer_manager mgr;
	boost::shared_ptr<TimerTask> tt1(new TimerTask());
	tt1->set_timeout(10000);
	boost::shared_ptr<TimerTask> tt2(new TimerTask());
	tt2->set_timeout(0);
	//mgr.add(tt1);
	mgr.add(tt2);
	//mgr.add(tt2);
	boost::this_thread::sleep(boost::posix_time::seconds(20));
	mgr.stop(tt2);
	boost::this_thread::sleep(boost::posix_time::seconds(100));
	return 0;

	task_manager tm(1, 10, 10000);
	taskp p1(new MyTask());
	tm.add(p1);
	boost::this_thread::sleep(boost::posix_time::seconds(10));
	cout << p1->string() << endl;
	p1->wait();
	cout << p1->string() << endl;
	taskp p2(new MyTask());
	taskp p3(new MyTask());
	std::vector<taskp> ts;
	ts.push_back(p2);
	ts.push_back(p3);
	tm.addv(ts);
	tm.timed_wait_all(ts, 10000);
	cout << p2->string() << endl;
	cout << p3->string() << endl;
	tm.stopv(ts);
	tm.wait_all(ts);
	cout << p2->string() << endl;
	cout << p3->string() << endl;
	boost::this_thread::sleep(boost::posix_time::seconds(30));
}



/*
 * main.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan
 */

#include <iostream>
#include <pthread.h>
#include "utils/block_queue.hpp"
#include "utils/fifo_task_mgr.hpp"
#include "utils/time_task_mgr.hpp"
#include <gtest/gtest.h>

using namespace std;
using namespace wbase::common::utils;

class FifoTask : public fifo_task
{
public:
	FifoTask() { canceled = false; }
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

class BlockQueueTestFixture: public ::testing::Test
{

};

TEST_F(BlockQueueTestFixture, Test)
{
	time_task_mgr mgr;
	time_task_mgr::taskp tt1(new TimerTask());
	tt1->set_timeout(10000);
	time_task_mgr::taskp tt2(new TimerTask());
	tt2->set_timeout(0);
	mgr.add(tt1);
	mgr.add(tt2);

	fifo_task_mgr fmgr;
	fifo_task_mgr::taskp ft1(new FifoTask());
	fifo_task_mgr::taskp ft2(new FifoTask());
	fmgr.add(ft1);
	fmgr.add(ft2);

	boost::this_thread::sleep(boost::posix_time::seconds(20));
	mgr.stop(tt2);
	boost::this_thread::sleep(boost::posix_time::seconds(100));
}

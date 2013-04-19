/*
 * thread_pool.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan@readsns.com
 */

#include <sstream>
#include "task_manager.h"

namespace wbase { namespace common { namespace utils {

task::task()
{
	m_status = WAITING;
	m_create = time(NULL);
	m_begin = m_end = 0;
	m_prog = 0;
	m_code = 0;
}

void task::start()
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_begin = time(NULL);
	m_status = RUNNING;
	lock.unlock();
	m_code = run();
	lock.lock();
	if (m_status == RUNNING) {
		m_status = COMPLETED;
		m_end = time(NULL);
		m_cond.notify_all(); //可能有多个线程同时在wait task
		lock.unlock();
		on_complete(m_code);
	}
}

void task::stop()
{
	boost::mutex::scoped_lock lock(m_mutex);
	lock.unlock();
	cancel();
	lock.lock();
	if (!is_done()) {
		m_status = CANCELED;
		m_end = time(NULL);
		m_cond.notify_all();
	} else {
		m_status = CANCELED; //m_status should always be CANCELED once stop() has been called
	}
	lock.unlock();
	on_cancel();
}

std::string task::string()
{
	std::ostringstream oss;
	oss << "[";
	oss << "create_time: " << m_create << ", ";
	oss << "begin_time: " << m_begin << ", ";
	oss << "end_time: " << m_end << ", ";
	oss << "progress: " << m_prog << ", ";
	oss << "return_code: " << m_code << ", ";
	oss << "status: " << m_status;
	oss << "]";
	return oss.str();
}

void task::wait()
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (is_done())
		return;
	m_cond.wait(lock);
}

bool task::timed_wait(uint32_t timeout)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	return timed_wait(until);
}

bool task::timed_wait(const boost::system_time &until)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (is_done())
		return true;
	if (!m_cond.timed_wait(lock, until)) {
		return false;
	}
	return true;
}

/***********************************************************
 * *****************	worker	  **************************
 * *********************************************************
 */
worker::worker(task_manager &mgr) : m_mgr(mgr)
{
	m_thread.reset(new boost::thread(boost::ref(*this)));
	std::cout << "worker created " << m_thread->get_id() << std::endl;
}

worker::~worker()
{
	if (m_thread.get()) {
		std::cout << "worker reclaimed " << m_thread->get_id() << std::endl;
		m_thread->interrupt();
		m_thread->join();
		m_thread.reset();
	}
}

void worker::acquire(taskp task)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (m_tsk.get()) { // should never happen
		return;
	}

	m_tsk = task;
	m_cond.notify_one();
}

void worker::operator()()
{
	boost::mutex::scoped_lock lock(m_mutex);
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			if (!m_tsk.get()) {
				m_cond.wait(lock);
			}
			m_tsk->start();
			m_active_time = time(NULL);
			m_mgr.put_task(m_tsk);
			m_tsk.reset();
			m_mgr.put_worker(shared_from_this());
		}
	} catch (boost::thread_interrupted &msg) {
	}
}

/***************************************************************
 * ******************* task manager ****************************
 * *************************************************************
 */
task_manager::task_manager(uint32_t min, uint32_t max, uint32_t idle,
		const boost::shared_ptr<scheduler> &scheduler)
{
	if (min > max) {
		max = min;
	}
	m_min = min;
	m_max = max;
	m_idle = idle;
	m_sched = scheduler;
	m_thread.reset(new boost::thread(boost::ref(*this)));
	for (uint32_t i = 0; i < m_min; i++) {
		m_idle_workers.push_back(workerp(new worker(*this)));
	}

	m_sem.reset(new boost::interprocess::interprocess_semaphore(m_max));
}

task_manager::~task_manager()
{
	if (m_thread.get()) {
		m_thread->interrupt();
		m_thread->join();
		m_thread.reset();
	}
	if (m_sem.get()) {
		m_sem.reset();
	}
}

void task_manager::add(const taskp &task)
{
	m_sched->add(task);
}

void task_manager::stop(const taskp &task)
{
	task->stop();
}

void task_manager::addv(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		m_sched->add(tasks[i]);
}

void task_manager::stopv(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		tasks[i]->stop();
}

void task_manager::wait(const taskp &task)
{
	task->wait();
}

bool task_manager::timed_wait(const taskp &task, uint32_t timeout)
{
	return task->timed_wait(timeout);
}

void task_manager::wait_all(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		tasks[i]->wait();
}

bool task_manager::timed_wait_all(const std::vector<taskp> &tasks, uint32_t timeout)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	for (size_t i = 0; i < tasks.size(); i++) {
		if (!tasks[i]->timed_wait(until))
			return false;
	}
	return true;
}

void task_manager::operator()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			taskp task;
			//TODO only do timed_get when current # of workers > m_idle
			if (!m_sched->timed_get(task, m_idle)) {
				reclaim_workers();
			} else {
				workerp worker;
				get_worker(worker);
				worker->acquire(task);
			}
		}
	} catch (boost::thread_interrupted &msg) {
	}
}

void task_manager::get_worker(workerp &wkr)
{
	m_sem->wait();
	boost::mutex::scoped_lock lock(m_mutex);
	if (!m_idle_workers.empty()) {
		wkr = m_idle_workers.front();
		//reuse the youngest one
		m_idle_workers.pop_front();
		m_active_workers.insert(wkr);
	} else {
		wkr.reset(new worker(*this));
		m_active_workers.insert(wkr);
	}
}

void task_manager::put_worker(workerp worker)
{
	boost::mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT(m_active_workers.erase(worker) > 0);
	//put the youngest one in front of the queue
	m_idle_workers.push_front(worker);
	m_sem->post();
}

void task_manager::reclaim_workers()
{
	boost::mutex::scoped_lock lock(m_mutex);
	size_t td = m_idle_workers.size() + m_active_workers.size() - m_min;
	while (td-- != 0 && !m_idle_workers.empty()) {
		m_idle_workers.pop_back(); //reclaim the oldest first
	}
}

} } }




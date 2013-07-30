/*
 * thread_pool.h
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan@readsns.com
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <set>
#include <deque>
#include <time.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "core/base.hpp"
#include "block_queue.hpp"

using namespace wbase::common::core;

namespace wbase { namespace common { namespace utils {

template<typename Task, template<class> class Sched>
class task_manager;

template<typename Task, template<class> class Sched>
class worker;

class task : public boost::noncopyable
{
public:
	template<typename Task, template<class> class Sched> friend class task_manager;
	template<typename Task, template<class> class Sched> friend class worker;

	enum status { WAITING, RUNNING, COMPLETED, CANCELED };

								task ();
					  virtual 	~task () {}
		  virtual std::string 	string();

					   status	stat() { return m_status; }
					 uint16_t 	progress() { return m_prog; }
					   time_t	create_time() { return m_create; }
					   time_t	start_time() { return m_begin; }
					   time_t	end_time() { return m_end; }

						 void	wait();
						 bool	timed_wait(uint32_t timeout_ms);
						 bool	timed_wait(const boost::system_time &until);

				 virtual void	run() = 0; //do the real work
				 virtual void	cancel() = 0; //interrupt run
				 virtual void	on_complete() {} //called when run completed
				 virtual void	on_cancel() {} //called when cancel completed

						 void	update_progress(uint16_t prog) { m_prog = prog; }

private:
						 void 	start();
						 void	stop();
						 bool	is_done() { return m_status == COMPLETED || m_status == CANCELED; }

private:
					   status	m_status;
					   time_t	m_create;
					   time_t	m_begin;
					   time_t	m_end;
					 uint16_t	m_prog;

				 boost::mutex	m_mutex;
	boost::condition_variable	m_cond;
};

template<typename Task, template<class> class Sched>
class worker : public boost::noncopyable, public boost::enable_shared_from_this<worker<Task, Sched> >
{
public:
	friend class task_manager<Task, Sched>;
	typedef typename Sched<Task>::taskp taskp;

	 worker	(task_manager<Task, Sched> &mgr) : m_mgr(mgr) {
		 m_thread.reset(new boost::thread(boost::ref(*this)));
	 }

	~worker	() {
		if (m_thread.get()) {
			m_thread->interrupt();
			m_thread->join();
			m_thread.reset();
		}
	}

	void operator()() {
		boost::mutex::scoped_lock lock(m_mutex);
		try
		{
			while (!boost::this_thread::interruption_requested())
			{
				while (!m_tsk.get()) {
					m_cond.wait(lock);
				}
				m_tsk->start();
				m_active_time = time(NULL);
				m_mgr.put_task(m_tsk);
				m_tsk.reset();
				m_mgr.put_worker((dynamic_cast<worker<Task, Sched>*>(this))->shared_from_this());
			}
		} catch (boost::thread_interrupted &msg) {
		}
	}

private:
	void acquire(taskp task) {
		boost::mutex::scoped_lock lock(m_mutex);
		assert(!m_tsk.get()); // should never happen
		m_tsk = task;
		m_cond.notify_one();
	}

private:
		  task_manager<Task, Sched>&	m_mgr;
			   				   taskp	m_tsk;
							  time_t	m_active_time; //last active time
					    boost::mutex	m_mutex;
	       boost::condition_variable	m_cond;
	boost::shared_ptr<boost::thread> 	m_thread;
};

template<typename T>
class scheduler : public boost::noncopyable
{
public:
	typedef boost::shared_ptr<T> taskp;

	// task_manager will call this function to queue tasks, should be thread safe
	virtual bool add_task(const taskp &task) = 0;
	/* task_manager will call the following two functions to get one task to
	 * run, it's the scheduler's decision which task is returned */
	//block until get one task
	virtual void get_task(taskp &task) = 0;
	// this function should be returned within timeout_ms, either return true
	// with task filled or false with a empty task
	virtual bool timed_get(taskp &task, uint32_t timeout_ms) = 0;
	// task manager will put task back to scheduler when it is completed or
	// canceled, scheduler can examine the status of the task and decide what
	// to do next, throw it away or reschedule it, etc.
	virtual bool put_task(const taskp &task) { return true; }

protected:
	virtual ~scheduler() {}
};

template<typename Task, template<class> class Sched>
class task_manager : public Sched<Task>
{
public:
	friend class worker<Task, Sched>;
	typedef typename Sched<Task>::taskp taskp;
	typedef boost::shared_ptr<worker<Task, Sched> > workerp;

	 task_manager (uint32_t min = 1, uint32_t max = 65535, uint32_t idle = 600 * 1000)
	 {
		if (min > max)
			max = min;

		m_min = min;
		m_max = max;
		m_idle = idle;
		m_thread.reset(new boost::thread(boost::ref(*this)));
		for (uint32_t i = 0; i < m_min; i++) {
			m_idle_workers.push_back(workerp(new worker<Task, Sched>(*this)));
		}
		m_sem.reset(new boost::interprocess::interprocess_semaphore(m_max));
	 }

	~task_manager () {
		if (m_thread.get()) {
			m_thread->interrupt(); //interrupt before post
			m_sem->post();
			m_thread->join();
			m_thread.reset();
		}
		if (m_sem.get()) {
			m_sem.reset();
		}
	}

	bool add(const taskp &task) { return this->add_task(task); }
	void stop(const taskp &task) { task->stop(); }

	void addv(const std::vector<taskp> &tasks) {
		for (size_t i = 0; i < tasks.size(); i++)
			add(tasks[i]);
	}
	void stopv(const std::vector<taskp> &tasks) {
		for (size_t i = 0; i < tasks.size(); i++)
			stop(tasks[i]);
	}

	void wait(const taskp &task) { task->wait(); }
	bool timed_wait(const taskp &task, uint32_t timeout_ms) {
		return task->timed_wait(timeout_ms);
	}
	void wait_all(const std::vector<taskp> &tasks) {
		for (size_t i = 0; i < tasks.size(); i++)
			tasks[i]->wait();
	}

	bool timed_wait_all(const std::vector<taskp> &tasks, uint32_t timeout_ms) {
		boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
		for (size_t i = 0; i < tasks.size(); i++) {
			if (!tasks[i]->timed_wait(until))
				return false;
		}
		return true;
	}

	void operator()() {
		try
		{
			while (!boost::this_thread::interruption_requested())
			{
				taskp task;
				//TODO only do timed_get when current # of workers > m_idle
				if (!this->timed_get(task, m_idle)) {
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

	void					set_min(size_t min) { m_min = min; }
	void					set_max(size_t max) { m_max = max; }
	void					set_idle(uint32_t idle) { m_idle = idle; }

	size_t					get_min() { return m_min; }
	size_t					get_max() { return m_max; }
	uint32_t				get_idle() { return m_idle; }

private:
	void reclaim_workers() {
		boost::mutex::scoped_lock lock(m_mutex);
		size_t td = m_idle_workers.size() + m_active_workers.size() - m_min;
		while (td-- != 0 && !m_idle_workers.empty()) {
			m_idle_workers.pop_back(); //reclaim the oldest first
		}
	}

	void put_worker(workerp worker) {
		boost::mutex::scoped_lock lock(m_mutex);
		BOOST_ASSERT(m_active_workers.erase(worker) > 0);
		//put the youngest one in front of the queue
		m_idle_workers.push_front(worker);
		m_sem->post();
	}

	void get_worker(workerp &wkr) {
		m_sem->wait(); // cannot be interrupted
		boost::this_thread::interruption_point();
		boost::mutex::scoped_lock lock(m_mutex);
		if (!m_idle_workers.empty()) {
			wkr = m_idle_workers.front();
			//reuse the youngest one
			m_idle_workers.pop_front();
			m_active_workers.insert(wkr);
		} else {
			wkr.reset(new worker<Task, Sched>(*this));
			m_active_workers.insert(wkr);
		}
	}

private:
	size_t		m_min;
	size_t		m_max;
	uint32_t	m_idle; /* start to reclaim when pass the idle time */

	std::set<workerp>		m_active_workers;
	std::deque<workerp>		m_idle_workers;
	boost::mutex			m_mutex; // mutex for workers
	std::auto_ptr<boost::thread>	m_thread;
	std::auto_ptr<boost::interprocess::interprocess_semaphore> m_sem;
};

} } }

#endif /* THREAD_POOL_H_ */

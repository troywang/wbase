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

#include "admin.h"
#include "block_queue.h"

namespace wbase { namespace common { namespace utils {

class	worker;
class 	task_manager;
class	scheduler;

class task : public boost::noncopyable
{
public:
	friend class	worker;
	friend class	task_manager;
	enum status { WAITING, RUNNING, COMPLETED, CANCELED };
	task ();
	virtual ~task () {}
	virtual std::string string();

	status				stat() { return m_status; }
	uint16_t 			progress() { return m_prog; }
	time_t				create_time() { return m_create; }
	time_t				start_time() { return m_begin; }
	time_t				end_time() { return m_end; }

	void				wait();
	bool				timed_wait(uint32_t timeout_ms);
	bool				timed_wait(const boost::system_time &until);

	//do the real work
	virtual void			run() = 0;
	//interrupt run
	virtual void		cancel() = 0;
	//called when run completed
	virtual void		on_complete() {}
	//called when cancel completed
	virtual void 		on_cancel() {}

	void 				update_progress(uint16_t prog) { m_prog = prog; }

private:
	void 				start();
	void				stop();
	bool				is_done() { return m_status == COMPLETED || m_status == CANCELED; }

private:
	status						m_status;
	time_t						m_create;
	time_t 						m_begin;
	time_t 						m_end;
	uint16_t 					m_prog;

	boost::mutex				m_mutex;
	boost::condition_variable	m_cond;
};

typedef boost::shared_ptr<task> taskp;
typedef boost::shared_ptr<worker> workerp;
typedef boost::shared_ptr<scheduler> schedulerp;

class worker : public boost::noncopyable, public boost::enable_shared_from_this<worker>
{
public:
	friend class	task_manager;
	worker	(task_manager &mgr);
	~worker	();
	void 					operator()();

private:
	void 					acquire(taskp task);

private:
	task_manager&			m_mgr;
	taskp					m_tsk;
	time_t					m_active_time; //last active time
	boost::mutex					 m_mutex;
	boost::condition_variable		 m_cond;
	boost::shared_ptr<boost::thread> m_thread;
};

class scheduler : public boost::noncopyable
{
public:
	virtual ~scheduler() {}
	// task_manager will call this function to queue tasks, should be thread safe
	virtual void add(const taskp &task) = 0;
	/* task_manager will call the following two functions to get one task to
	 * run, it's the scheduler's decision which task is returned */
	//block until get one task
	virtual void get(taskp &task) = 0;
	// this function should be returned within timeout_ms, either return true
	// with task filled or false with a empty task
	virtual bool timed_get(taskp &task, uint32_t timeout_ms) = 0;
	// put task back to scheduler when it is completed or canceled
	// scheduler can examine the status of the task and decide what to do next
	virtual void put(const taskp &task) {}
};

//FIFO scheduler
class fifo_scheduler : public scheduler
{
public:
	virtual void	add(const taskp &task) { m_tasks.push_back(task); }
	virtual void	get(taskp &task) { m_tasks.pop_front(task); }
	virtual bool	timed_get(taskp &task, uint32_t timeout_ms) { return m_tasks.timed_pop_front(task, timeout_ms); }

private:
	block_queue<taskp> 		m_tasks;
};

class task_manager : public boost::noncopyable
{
public:
	friend class 	worker;

	task_manager	(uint32_t min = 1,
					 uint32_t max = 65535,
					 uint32_t idle = 600 * 1000,
					 const schedulerp &scheduler = schedulerp(new fifo_scheduler()));

	~task_manager 	();

	void 					add(const taskp &task);
	void 					stop(const taskp &task);

	void					addv(const std::vector<taskp> &tasks);
	void					stopv(const std::vector<taskp> &tasks);

	void					wait(const taskp &task);
	bool					timed_wait(const taskp &task, uint32_t timeout_ms);

	void					wait_all(const std::vector<taskp> &tasks);
	bool					timed_wait_all(const std::vector<taskp> &tasks, uint32_t timeout_ms);

	void					operator()();

	void					set_min(size_t min) { m_min = min; }
	void					set_max(size_t max) { m_max = max; }
	void					set_idle(uint32_t idle) { m_idle = idle; }

	size_t					get_min() { return m_min; }
	size_t					get_max() { return m_max; }
	uint32_t				get_idle() { return m_idle; }

private:
	void 					reclaim_workers();
	void					put_worker(workerp worker);
	void					get_worker(workerp &worker);
	void					put_task(const taskp &task) { m_sched->put(task); }

private:
	ADMIN_VAR size_t		m_min;
	ADMIN_VAR size_t		m_max;
	ADMIN_VAR uint32_t		m_idle; /* start to reclaim when pass the idle time */

	std::set<workerp>		m_active_workers;
	std::deque<workerp>		m_idle_workers;
	boost::mutex			m_mutex; // mutex for workers
	boost::shared_ptr<scheduler>	m_sched; //fifo as default
	std::auto_ptr<boost::thread>	m_thread;
	std::auto_ptr<boost::interprocess::interprocess_semaphore> m_sem;
};

} } }

#endif /* THREAD_POOL_H_ */

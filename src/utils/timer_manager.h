/*
 * timer_manager.h
 *
 *  Created on: Apr 17, 2013
 *      Author: wanghuan
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_

#include "task_manager.h"

namespace wbase { namespace common { namespace utils {

class timer_task : public task
{
public:
	enum ACTION {INVALID = -1, CONTINUE, DONE};
	//call run() after timeout_ms every interval_ms unless timer_run() returns STOP
	//in subclass, cancel() should make timer_run() return STOP to prevent task from being scheduled repeatedly
	timer_task  (uint32_t timeout_ms = 1000, uint32_t interval_ms = 1000);
	virtual ~timer_task() {}
	virtual std::string 	string();

					void	run() { m_action = timer_run(); }
		virtual  ACTION		timer_run() = 0;

					void	set_timeout(uint32_t timeout_ms) { m_timeout = timeout_ms; }
					void	set_interval(uint32_t interval_ms) { m_interval = interval_ms; }

				uint32_t	timeout() { return m_timeout; }
				uint32_t 	interval() { return m_interval; }
			      ACTION	action() { return m_action; }

private:
	volatile uint32_t	m_timeout;
	volatile uint32_t	m_interval;
	ACTION				m_action;
};

class timer_scheduler : public scheduler
{
public:
	virtual void 	add(const taskp &task);
	virtual void 	get(/* out */taskp &task);
	virtual void 	put(const taskp &task);
	virtual bool 	timed_get(/* out */taskp &task, uint32_t timeout_ms);

private:
			 void	do_add(const boost::shared_ptr<timer_task> &task, const boost::system_time &until);

private:
	boost::mutex				m_mutex;
	boost::condition_variable	m_cond;
	std::multimap<boost::system_time, boost::shared_ptr<timer_task> >	m_tasks;
	typedef std::multimap<boost::system_time, boost::shared_ptr<timer_task> >::iterator task_iterator;
};

class timer_manager
{
public:
	timer_manager (uint32_t min = 1, uint32_t max = 65536, uint32_t idle = 10 * 60 * 1000);

			  void	add(const boost::shared_ptr<timer_task> &task);
			  void	stop(const boost::shared_ptr<timer_task> &task);

private:
	task_manager	m_task_mgr;
};

} } }

#endif /* TIMER_MANAGER_H_ */

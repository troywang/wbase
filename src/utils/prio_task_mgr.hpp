/*
 * prio_manager.h
 *
 *  Created on: Jul 17, 2013
 *      Author: king
 */

#ifndef PRIO_MANAGER_H_
#define PRIO_MANAGER_H_

#include "task_manager.hpp"

namespace wbase { namespace common { namespace utils {

enum PRIO {IDLE, LOW, NORMAL, HIGH, REALTIME};

class prio_task : public task
{
public:
	prio_task  (PRIO prio) { m_prio = prio; }
	virtual ~prio_task() {}

	virtual std::string 	string();
				   bool 	operator< (const prio_task &other) { return this->m_prio < other.m_prio; }
				   PRIO		prio() { return m_prio; }

private:
	PRIO	m_prio;
};

template<typename T>
class prio_scheduler : public scheduler<T>
{
public:
		 virtual 	~prio_scheduler() {}
	virtual void 	add_task(const taskp &task);
	virtual void 	get_task(/* out */taskp &task);
	virtual void 	put_task(const taskp &task);
	virtual bool 	timed_get(/* out */taskp &task, uint32_t timeout_ms);

private:
			 void	do_add(const boost::shared_ptr<prio_task> &task);

private:
	boost::mutex				m_mutex;
	boost::condition_variable	m_cond;
	std::map<PRIO, std::set<prio_task> >	m_tasks;
	typedef std::map<PRIO, std::set<prio_task> >::iterator task_iterator;
};

typedef task_manager<prio_task, prio_scheduler<prio_task> > prio_task_mgr;

} } }

#endif /* PRIO_MANAGER_H_ */

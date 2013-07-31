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

enum PRIO {IDLE = 0, LOW, NORMAL, HIGH, REALTIME};

class prio_task : public task
{
public:
	prio_task  (PRIO prio) { m_prio = prio; }
	virtual ~prio_task() {}

	virtual std::string 	string() {
		std::ostringstream oss;
		oss << task::string();
		oss << "[prio: " << m_prio << "]";
		return oss.str();
	}
				   PRIO		prio() { return m_prio; }

private:
	PRIO	m_prio;
};

template<typename T>
class prio_scheduler : public scheduler<T>
{
public:
	typedef typename scheduler<T>::taskp taskp;

		 virtual 	~prio_scheduler() {}
	virtual bool 	add_task(const taskp &task);
	virtual void 	get_task(/* out */taskp &task);
	virtual bool 	timed_get(/* out */taskp &task, uint32_t timeout_ms);

private:
			 void	do_add(const boost::shared_ptr<prio_task> &task);

private:
	boost::mutex				m_mutex;
	boost::condition_variable	m_cond;
	block_queue<taskp> 			m_tasks[REALTIME + 1];
};

template<typename T>
bool prio_scheduler<T>::add_task(const taskp &task)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_tasks[task->prio()].push_back(task);
	m_cond.notify_one();
	return true;
}

template<typename T>
void prio_scheduler<T>::get_task(taskp &task)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (true) {
		int max = REALTIME;
		int min = IDLE;
		for (int p = max; p >= min; --p) {
			if (!m_tasks[p].empty()) {
				m_tasks[p].pop_front(task);
				return;
			}
		}
		m_cond.wait(lock);
	}
}

template<typename T>
bool prio_scheduler<T>::timed_get(/* out */taskp &task, uint32_t timeout_ms)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
	boost::mutex::scoped_lock lock(m_mutex);
	while (true) {
		int max = REALTIME;
		int min = IDLE;
		for (int p = max; p >= min; --p) {
			if (!m_tasks[p].empty()) {
				m_tasks[p].pop_front(task);
				return true;
			}
		}
		if (!m_cond.timed_wait(lock, until))
			return false;
	}
	return true;
}

typedef task_manager<prio_task, prio_scheduler> prio_task_mgr;

} } }

#endif /* PRIO_MANAGER_H_ */

/*
 * fifo_task_mgr.hpp
 *
 *  Created on: Jul 24, 2013
 *      Author: king
 */

#ifndef FIFO_TASK_MGR_HPP_
#define FIFO_TASK_MGR_HPP_

#include "task_manager.hpp"

namespace wbase { namespace common { namespace utils {

typedef task fifo_task;

//FIFO scheduler
template<typename T>
class fifo_scheduler : public scheduler<T>
{
public:
	typedef	typename scheduler<T>::taskp taskp;

	virtual			~fifo_scheduler() {}
	virtual bool	add_task(const taskp &task) { m_tasks.push_back(task); return true; }
	virtual void	get_task(taskp &task) { m_tasks.pop_front(task); }
	virtual bool	timed_get(taskp &task, uint32_t timeout_ms) { return m_tasks.timed_pop_front(task, timeout_ms); }

private:
	block_queue<taskp> 		m_tasks;
};

typedef task_manager<fifo_task, fifo_scheduler> fifo_task_mgr;

} } }
#endif /* FIFO_TASK_MGR_HPP_ */

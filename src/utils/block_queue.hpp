/*
 * blockqueue.h
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan@readsns.com
 */
#ifndef __WBASE_BLOCK_QUEUE__
#define __WBASE_BLOCK_QUEUE__

#include <deque>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace wbase { namespace common { namespace utils {

template<typename Data>
class block_queue : public boost::noncopyable
{
public:
	block_queue   (size_t cap = -1) : m_cap(cap) {}
	~block_queue  () {}

	void 				push_back(const Data &data);
	void 				pop_front(/* out */ Data &data);

	void				push_front(const Data &data);
	void				pop_back(/* out */ Data &data);

	/* timeout in milliseconds */
	bool 				timed_push_back(const Data &data, uint32_t timeout);
	bool 				timed_pop_front(/* out */ Data &data, uint32_t timeout);

	bool				timed_push_front(const Data &data, uint32_t timeout);
	bool				timed_pop_back(/* out */ Data &data, uint32_t timeout);

	size_t				cap() { return m_cap; }
	size_t				size() { return m_queue.size(); }
	bool				empty() { return m_queue.empty(); }

private:
	size_t 							m_cap; //max object can stay in this queue, -1 means INT_MAX
	boost::mutex					m_mutex;
	boost::condition_variable		m_cond_en; //notify producer, en means enqueue
	boost::condition_variable		m_cond_de; //notify consumer, de means dequeue
	std::deque<Data> 				m_queue;
};

//implementation
template<typename Data>
void block_queue<Data>::push_back(const Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		m_cond_en.wait(lock);
	}
	m_queue.push_back(data);
	m_cond_de.notify_one();
}

template<typename Data>
void block_queue<Data>::pop_front(/* out */ Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		m_cond_de.wait(lock);
	}
	data = m_queue.front();
	m_queue.pop_front();
	m_cond_en.notify_one();
}

template<typename Data>
void block_queue<Data>::push_front(const Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		m_cond_en.wait(lock);
	}
	m_queue.push_front(data);
	m_cond_de.notify_one();
}

template<typename Data>
void block_queue<Data>::pop_back(/* out */ Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		m_cond_de.wait(lock);
	}
	data = m_queue.back();
	m_queue.pop_back();
	m_cond_en.notify_one();
}

template<typename Data>
bool block_queue<Data>::timed_push_back(const Data &data, uint32_t timeout)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		if (!m_cond_en.timed_wait(lock, boost::posix_time::milliseconds(timeout))) {
			return false;
		}
	}
	m_queue.push_back(data);
	m_cond_de.notify_one();
	return true;
}

template<typename Data>
bool block_queue<Data>::timed_pop_front(/* out */ Data &data, uint32_t timeout)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		if (!m_cond_de.timed_wait(lock, boost::posix_time::milliseconds(timeout))) {
			return false;
		}
	}
	data = m_queue.front();
	m_queue.pop_front();
	m_cond_en.notify_one();
	return true;
}

template<typename Data>
bool block_queue<Data>::timed_push_front(const Data &data, uint32_t timeout)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		if (!m_cond_en.timed_wait(lock, boost::posix_time::milliseconds(timeout))) {
			return false;
		}
	}
	m_queue.push_front(data);
	m_cond_de.notify_one();
	return true;
}

template<typename Data>
bool block_queue<Data>::timed_pop_back(/* out */ Data &data, uint32_t timeout)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		if (!m_cond_de.timed_wait(lock, timeout)) {
			return false;
		}
	}
	data = m_queue.back();
	m_queue.pop_back();
	m_cond_en.notify_one();
	return true;
}


}}}
#endif



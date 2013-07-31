/*
 * base.hpp
 *
 *  Created on: Jul 24, 2013
 *      Author: king
 */

#ifndef BASE_HPP_
#define BASE_HPP_

#include <map>
#include <set>
#include <deque>
#include <vector>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <endian.h>
#include <sys/time.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>


namespace wbase { namespace common { namespace core {

template<int v>
struct int2type {
	enum { value = v };
};

template<typename T, typename U>
class convertible {
	typedef char (&yes) [1];
	typedef char (&no)  [2];

	static yes f(const U *);
	static no  f(...);

public:
	enum { exists = sizeof( f( static_cast<const T*>(0) ) ) == sizeof(yes) };
	enum { mutual = exists && convertible<U, T>::exists };
	enum { same = false };
};

template<typename T>
class convertible<T, T> {
public:
	enum { exists = true, mutual = true, same = true };
};

#define SUPER_SUB_CLASS(T, U) \
	(convertible<U, T>::exists && \
			!convertible<T, void>::same)

#define SUPER_SUB_CLASS_STRICT(T, U) \
	(SUPER_SUB_CLASS(T, U) && \
			!convertible<T, U>::same)

}}}

#endif /* BASE_HPP_ */

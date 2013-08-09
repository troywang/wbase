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
#include <fstream>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <endian.h>
#include <sys/time.h>
#include <exception>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/begin.hpp>

#include <boost/fusion/include/cons.hpp>
#include <boost/fusion/include/invoke.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/push_front.hpp>

#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>


namespace wbase { namespace common { namespace core {

class exception_base : public std::exception {
public:
	exception_base(const std::string &str) : m_str(str) {}
	virtual ~exception_base() throw() {}

	const char* what() {
		return m_str.c_str();
	}

protected:
	std::string m_str;
};

template <int v>
struct int2type {
	enum { value = v };
};

template <typename T, typename U>
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

template <typename T>
class convertible <T, T> {
public:
	enum { exists = true, mutual = true, same = true };
};

#define SUPER_SUB_CLASS(T, U) \
	(convertible<U, T>::exists && \
			!convertible<T, void>::same)

#define SUPER_SUB_CLASS_STRICT(T, U) \
	(SUPER_SUB_CLASS(T, U) && \
			!convertible<T, U>::same)

template <class T, class U>
struct type_list {
	typedef T head;
	typedef U tail;
};

struct null_type {
};

template <typename TList>
struct length;

template <>
struct length <null_type> {
	enum { value = 0 };
};

template <class T, class U>
struct length <type_list <T, U> > {
	enum { value = length<U>::value + 1 };
};

#define type_list_0() null_type
#define type_list_1(T1) type_list<T1, null_type>
#define type_list_2(T1, T2) type_list<T1, type_list_1(T2)>
#define type_list_3(T1, T2, T3) type_list<T1, type_list_2(T2, T3)>
#define type_list_4(T1, T2, T3, T4) type_list<T1, type_list_3(T2, T3, T4)>
#define type_list_5(T1, T2, T3, T4, T5) type_list<T1, type_list_4(T2, T3, T4, T5)>
#define type_list_6(T1, T2, T3, T4, T5, T6) type_list<T1, type_list_5(T2, T3, T4, T5, T6)>
#define type_list_7(T1, T2, T3, T4, T5, T6, T7) type_list<T1, type_list_6(T2, T3, T4, T5, T6, T7)>
#define type_list_8(T1, T2, T3, T4, T5, T6, T7, T8) type_list<T1, type_list_7(T2, T3, T4, T5, T6, T7, T8)>
#define type_list_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) type_list<T1, type_list_8(T2, T3, T4, T5, T6, T7, T8, T9)>
#define type_list_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) type_list<T1, type_list_9(T2, T3, T4, T5, T6, T7, T8, T9, T10)>

template <typename TList, unsigned int index> struct type_at;

template <unsigned int i>
struct type_at<null_type, i> {
	typedef null_type value;
};

template <typename Head, typename Tail>
struct type_at<type_list<Head, Tail>, 0> {
	typedef Head value;
};

template <typename Head, typename Tail, unsigned int i>
struct type_at<type_list<Head, Tail>, i> {
	typedef typename type_at<Tail, i - 1>::value value;
};

template <typename R, typename ArgList>
class invoker_impl;

template <typename R>
class invoker_impl<R, null_type> {
public:
	virtual R operator()() = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1>
class invoker_impl<R, type_list_1(P1)> {
public:
	virtual R operator()(P1) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2>
class invoker_impl<R, type_list_2(P1, P2)> {
public:
	virtual R operator()(P1, P2) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3>
class invoker_impl<R, type_list_3(P1, P2, P3)> {
public:
	virtual R operator()(P1, P2, P3) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
class invoker_impl<R, type_list_4(P1, P2, P3, P4)> {
public:
	virtual R operator()(P1, P2, P3, P4) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class invoker_impl<R, type_list_5(P1, P2, P3, P4, P5)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class invoker_impl<R, type_list_6(P1, P2, P3, P4, P5, P6)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5, P6) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class invoker_impl<R, type_list_7(P1, P2, P3, P4, P5, P6, P7)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5, P6, P7) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class invoker_impl<R, type_list_8(P1, P2, P3, P4, P5, P6, P7, P8)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5, P6, P7, P8) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
class invoker_impl<R, type_list_9(P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5, P6, P7, P8, P9) = 0;
	virtual ~invoker_impl() {}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
class invoker_impl<R, type_list_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
public:
	virtual R operator()(P1, P2, P3, P4, P5, P6, P7, P8) = 0;
	virtual ~invoker_impl() {}
};

template <typename Invoker, typename Func>
class nonmember_handler : public invoker_impl<typename Invoker::result_type, typename Invoker::arg_list> {
public:
	typedef typename Invoker::result_type result_type;

	nonmember_handler(const Func &func) : m_func(func) {}

	result_type operator()() {
		return m_func();
	}

	result_type operator()(typename Invoker::param1 p1) {
		return m_func(p1);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2) {
		return m_func(p1, p2);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3) {
		return m_func(p1, p2, p3);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4) {
		return m_func(p1, p2, p3, p4);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5) {
		return m_func(p1, p2, p3, p4, p5);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6) {
		return m_func(p1, p2, p3, p4, p5, p6);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7) {
		return m_func(p1, p2, p3, p4, p5, p6, p7);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8) {
		return m_func(p1, p2, p3, p4, p5, p6, p7, p8);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8, typename Invoker::param9 p9) {
		return m_func(p1, p2, p3, p4, p5, p6, p7, p8, p9);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8, typename Invoker::param9 p9, typename Invoker::param10 p10) {
		return m_func(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}

private:
	Func m_func;
};

template <typename Invoker, typename Ptr2Obj, typename Ptr2MemFn>
class member_handler : public invoker_impl<typename Invoker::result_type, typename Invoker::arg_list> {
public:
	typedef typename Invoker::result_type result_type;

	member_handler (Ptr2Obj pObj, Ptr2MemFn pMemFn)
	: m_pObj(pObj), m_pMemFn(pMemFn) {}

	result_type operator()() {
		return (m_pObj->*m_pMemFn)();
	}

	result_type operator()(typename Invoker::param1 p1) {
		return (m_pObj->*m_pMemFn)(p1);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2) {
		return (m_pObj->*m_pMemFn)(p1, p2);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5, p6);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5, p6, p7);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5, p6, p7, p8);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8, typename Invoker::param9 p9) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
	}

	result_type operator()(typename Invoker::param1 p1, typename Invoker::param2 p2, typename Invoker::param3 p3, typename Invoker::param4 p4, typename Invoker::param5 p5,
			typename Invoker::param6 p6, typename Invoker::param7 p7, typename Invoker::param8 p8, typename Invoker::param9 p9, typename Invoker::param10 p10) {
		return (m_pObj->*m_pMemFn)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}

private:
	Ptr2Obj m_pObj;
	Ptr2MemFn m_pMemFn;
};

template <typename R, typename ArgList>
class invoker {
public:
	typedef R result_type;
	typedef ArgList arg_list;
	typedef invoker_impl<R, ArgList> Impl;
	typedef typename type_at<ArgList, 0>::value param1;
	typedef typename type_at<ArgList, 1>::value param2;
	typedef typename type_at<ArgList, 2>::value param3;
	typedef typename type_at<ArgList, 3>::value param4;
	typedef typename type_at<ArgList, 4>::value param5;
	typedef typename type_at<ArgList, 5>::value param6;
	typedef typename type_at<ArgList, 6>::value param7;
	typedef typename type_at<ArgList, 7>::value param8;
	typedef typename type_at<ArgList, 8>::value param9;
	typedef typename type_at<ArgList, 9>::value param10;

public:
	template <typename Func>
	invoker (const Func &func) : spImpl(new nonmember_handler<invoker, Func>(func))
	{}

	template <typename Ptr2Obj, typename Ptr2MemFn>
	invoker (Ptr2Obj pObj, Ptr2MemFn pMemFn) : spImpl(new member_handler<invoker, Ptr2Obj, Ptr2MemFn>(pObj, pMemFn))
	{}

public:
	R operator() () { return (*spImpl)(); }

	R operator() (param1 p1) {
		return (*spImpl)(p1);
	}

	R operator() (param1 p1, param2 p2) {
		return (*spImpl)(p1, p2);
	}

	R operator() (param1 p1, param2 p2, param3 p3) {
		return (*spImpl)(p1, p2, p3);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4) {
		return (*spImpl)(p1, p2, p3, p4);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5) {
		return (*spImpl)(p1, p2, p3, p4, p5);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6) {
		return (*spImpl)(p1, p2, p3, p4, p5, p6);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6, param7 p7) {
		return (*spImpl)(p1, p2, p3, p4, p5, p6, p7);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6, param7 p7, param8 p8) {
		return (*spImpl)(p1, p2, p3, p4, p5, p6, p7, p8);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6, param7 p7, param8 p8, param9 p9) {
		return (*spImpl)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
	}

	R operator() (param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6, param7 p7, param8 p8, param9 p9, param10 p10) {
		return (*spImpl)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}

private:
	std::auto_ptr<Impl> spImpl;
};


}}}

#endif /* BASE_HPP_ */

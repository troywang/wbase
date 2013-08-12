/*
 * dispatcher.hpp
 *
 *  Created on: Aug 5, 2013
 *      Author: king
 */

#ifndef DISPATCHER_HPP_
#define DISPATCHER_HPP_

#include "core/base.hpp"
#include "core/error.hpp"
#include "core/serialize.hpp"

namespace wbase { namespace common { namespace rpc {

using namespace wbase::common::core;

class dispatcher_error : public exception_base
{
public:
	dispatcher_error(const std::string& e): exception_base(e) {}
};

class dispatcher {
public:
	typedef boost::function<void(deserializer &, serializer &)> invoker_function_type;
	typedef std::map<std::string, invoker_function_type> dictionary_type;
	dictionary_type m_invokers;

public:
	template <typename ReturnList, typename ArgList, typename Base, typename Func>
	bool register_invoker(const std::string &name, Base base, Func func)
	{
		typename dictionary_type::const_iterator entry = m_invokers.find(name);
		if (entry != m_invokers.end())
			return false;

		this->m_invokers[name] = boost::bind(&dispatcher::invoker<ReturnList, ArgList, Base, Func>::template apply<void>, base, func, _1, _2);
		return true;
	}

	void dispatch(std::istream &in, std::ostream &out);

private:
	template <typename ReturnList, typename ArgList, typename Base, typename Func>
	struct invoker;
};

template <typename Head, typename Tail, typename ArgList, typename Base, typename Func>
struct dispatcher::invoker<type_list<Head, Tail>, ArgList, Base, Func> {

	template <typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl)
	{
		Head arg1;
		invoker<Tail, ArgList, Base, Func>::template apply<Head, void>(base, func, ds, sl, arg1);
		sl.serialize(arg1);
	}

	template <typename Arg1, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1)
	{
		Head arg2;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Head, void>(base, func, ds, sl, arg1, arg2);
		sl.serialize(arg2);
	}

	template <typename Arg1, typename Arg2, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2)
	{
		Head arg3;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Head, void>(base, func, ds, sl, arg1, arg2, arg3);
		sl.serialize(arg3);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3)
	{
		Head arg4;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4);
		sl.serialize(arg4);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4)
	{
		Head arg5;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5);
		sl.serialize(arg5);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5)
	{
		Head arg6;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6);
		sl.serialize(arg6);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6)
	{
		Head arg7;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
		sl.serialize(arg7);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7)
	{
		Head arg8;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
		sl.serialize(arg8);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7, Arg8 &arg8)
	{
		Head arg9;
		invoker<Tail, ArgList, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
		sl.serialize(arg9);
	}
};

template <typename Head, typename Tail, typename Base, typename Func>
struct dispatcher::invoker<null_type, type_list<Head, Tail>, Base, Func> {

	template <typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl)
	{
		Head arg1;
		ds.deserialize(arg1);
		invoker<null_type, Tail, Base, Func>::template apply<Head, void>(base, func, ds, sl, arg1);
	}

	template <typename Arg1, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1)
	{
		Head arg2;
		ds.deserialize(arg2);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Head, void>(base, func, ds, sl, arg1, arg2);
	}

	template <typename Arg1, typename Arg2, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2)
	{
		Head arg3;
		ds.deserialize(arg3);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Head, void>(base, func, ds, sl, arg1, arg2, arg3);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 arg1, Arg2 arg2, Arg3 arg3)
	{
		Head arg4;
		ds.deserialize(arg4);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4)
	{
		Head arg5;
		ds.deserialize(arg5);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5)
	{
		Head arg6;
		ds.deserialize(arg6);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6)
	{
		Head arg7;
		ds.deserialize(arg7);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7)
	{
		Head arg8;
		ds.deserialize(arg8);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7, Arg8 &arg8)
	{
		Head arg9;
		ds.deserialize(arg9);
		invoker<null_type, Tail, Base, Func>::template apply<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Head, void>(base, func, ds, sl, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}
};

template <typename Base, typename Func>
struct dispatcher::invoker<null_type, null_type, Base, Func> {

	template <typename Arg>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl)
	{
		(base->*func)();
	}

	template <typename Arg1, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1)
	{
		(base->*func)(arg1);
	}

	template <typename Arg1, typename Arg2, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2)
	{
		(base->*func)(arg1, arg2);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3)
	{
		(base->*func)(arg1, arg2, arg3);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4)
	{
		(base->*func)(arg1, arg2, arg3, arg4);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5)
	{
		(base->*func)(arg1, arg2, arg3, arg4, arg5);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6)
	{
		(base->*func)(arg1, arg2, arg3, arg4, arg5, arg6);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7)
	{
		(base->*func)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7, Arg8 &arg8)
	{
		(base->*func)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename>
	static inline void apply(Base base, Func func, deserializer &ds, serializer &sl, Arg1 &arg1, Arg2 &arg2, Arg3 &arg3, Arg4 &arg4, Arg5 &arg5, Arg6 &arg6, Arg7 &arg7, Arg8 &arg8, Arg9 &arg9)
	{
		(base->*func)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}
};


void dispatcher::dispatch(std::istream &in, std::ostream &out)
{
	std::string func_name;
	try {
		deserializer ds(in);
		ds.deserialize(func_name);

		typename dictionary_type::const_iterator entry = m_invokers.find(func_name);
		if (entry == m_invokers.end()) {
			throw dispatcher_error("unknown function: " + func_name);
		}

		serializer sl(out);
		entry->second(ds, sl);

	} catch (serial_exception &e) {
		throw dispatcher_error("invalid arguments supplied for " + func_name + ", " + e.what());
	}
}

} } }

#endif /* DISPATCHER_HPP_ */

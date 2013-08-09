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
	template <typename ArgList, typename Base, typename Func>
	bool register_invoker(const std::string &name, Base base, Func func)
	{
		typename dictionary_type::const_iterator entry = m_invokers.find(name);
		if (entry != m_invokers.end())
			return false;

		this->m_invokers[name] = boost::bind(&dispatcher::invoker<ArgList, Base, Func>::template apply<boost::fusion::nil>, base, func, _1, _2, boost::fusion::nil());
		return true;
	}

	void dispatch(std::istream &in, std::ostream &out);

private:
	template <typename ArgList, typename Base, typename Func>
	struct invoker;
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

template <typename Head, typename Tail, typename Base, typename Func>
struct dispatcher::invoker<type_list<Head, Tail>, Base, Func> {
	template <typename Args>
	static inline void
	apply(Base base, Func func, deserializer &ds, serializer &sl, const Args &args)
	{
		Head arg;
		ds.deserialize(arg);
		invoker<Tail, Base, Func>::template apply(base, func, ds, sl, boost::fusion::push_back(args, arg));
	}
};

template <typename Base, typename Func>
struct dispatcher::invoker<null_type, Base, Func> {
	template <typename Args>
	static inline void
	apply(Base base, Func func, deserializer &ds, serializer &sl, const Args &args)
	{
		boost::fusion::invoke(func, boost::fusion::push_front(args, base));
	}
};

} } }

#endif /* DISPATCHER_HPP_ */

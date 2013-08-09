/*
 * dispatcher.cpp
 *
 *  Created on: Aug 6, 2013
 *      Author: king
 */

#include "core/base.hpp"
#include "rpc/dispatcher.hpp"
#include <gtest/gtest.h>

using namespace wbase::common::core;
using namespace wbase::common::rpc;

class DispatcherTest : public ::testing::Test
{

};

class test_serial : public serializable
{
public:
	virtual ~test_serial() {}
	virtual void serialize(std::ostream &out) const {
		serializer s(out);
		s.serialize("what the heck");
	}
	virtual void deserialize(std::istream &in) {

	}
};

struct adder {
	int operator() (int a, int b) {
		return a + b;
	}
	std::string minus(int a, int b) {
		return "hello\n";
	}
	void print(int32_t b) {
		std::cout << "print called " << b << std::endl;
	}
};


TEST_F(DispatcherTest, TestAll)
{
	try {
		adder a;

		dispatcher disp;
		disp.register_invoker<type_list_1(int32_t)>("print", &a, &adder::print);

		std::ofstream out("/tmp/dispatcher");
		serializer sr(out);
		sr.serialize<std::string>("print");
		sr.serialize<int32_t>(3);
		out.close();

		std::ifstream in("/tmp/dispatcher");
		deserializer dsr(in);
		std::ofstream out2("/tmp/result");
		disp.dispatch(in, out2);
	} catch (serial_exception &e) {
		std::cerr << e.what() << std::endl;
	} catch (dispatcher_error &e) {
		std::cerr << e.what() << std::endl;
	}

}





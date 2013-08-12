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
		s.serialize(std::string("what the fuck\n"));
	}
	virtual void deserialize(std::istream &in) {
		deserializer d(in);
		std::string dummy;
		d.deserialize(dummy);
	}
};

struct adder {
	int operator() (int a, int b) {
		return a + b;
	}

	std::string minus(int a, int b) {
		return "hello\n";
	}

	void print(test_serial& t, int8_t &a, int8_t& t2, int8_t &b, test_serial t3, uint8_t c, uint16_t e, int32_t f, uint64_t i) {
		std::cout << "print called " << c << std::endl;
		a = 'f';
		t2 = 'u';
		b = 'k';
	}
};


TEST_F(DispatcherTest, TestAll)
{
	try {
		adder a;

		dispatcher disp;
		disp.register_invoker<type_list_4(test_serial, int8_t, int8_t, int8_t),
				type_list_5(test_serial, uint8_t, uint16_t, int32_t, uint64_t)>("print", &a, &adder::print);

		test_serial t;
		uint8_t u8;
		uint16_t u16;
		int32_t i32;
		uint64_t u64;

		std::ofstream out("/tmp/dispatcher");
		serializer sr(out);

		sr.serialize<std::string>("print");
		sr.serialize(t);
		sr.serialize(u8);
		sr.serialize(u16);
		sr.serialize(i32);
		sr.serialize(u64);
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





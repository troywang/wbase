/*
 * writable_test.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: king
 */

#include <gtest/gtest.h>
#include <fstream>
#include "core/serialize.hpp"

using namespace wbase::common::core;

struct test_serial : public serializable
{
	virtual ~test_serial() {}
	virtual void serialize(std::ostream &out) const {
		serializer s(out);
		s.serialize(a);
	}
	virtual void deserialize(std::istream &in) {
		deserializer s(in);
		s.deserialize(a);
	}

	uint32_t a;
};

class SerializeTest : public ::testing::Test
{
};

TEST_F(SerializeTest, TestAll)
{
	try {
		bool b = true;
		char buf[1024];
		int8_t i8 = -1;
		uint8_t ui8 = 255;
		int16_t i16 = -2;
		uint16_t ui16 = 100;
		int32_t i32 = 123;
		uint32_t ui32 = 2323;
		int64_t i64 = 13;
		uint64_t ui64 = 235;
		std::string str;
		str.assign(buf, sizeof(buf));
		std::vector<std::string> v;
		v.push_back("hello");
		v.push_back("world");
		std::set<std::string> st;
		st.insert("hello");
		st.insert("world");
		std::map<std::string, std::string> m;
		m.insert(std::make_pair("hello", "world"));
		test_serial t;
		t.a = 100;

		std::ofstream of("/tmp/test");
		serializer s(of);

		s.serialize(str);
		s.serialize(b);
		s.serialize(i8);
		s.serialize(ui8);
		s.serialize(i16);
		s.serialize(ui16);
		s.serialize(i32);
		s.serialize(ui32);
		s.serialize(i64);
		s.serialize(ui64);
		s.serialize(v);
		s.serialize(st);
		s.serialize(m);
		s.serialize(&t);
		of.close();

		std::ifstream in("/tmp/test");
		deserializer ds(in);

		ds.deserialize(str);
		ASSERT_EQ(memcmp(str.data(), buf, sizeof(buf)), 0);
		b = false;
		ds.deserialize(b);
		ASSERT_EQ(b, true);
		i8 = 0;
		ds.deserialize(i8);
		ASSERT_EQ(i8, -1);
		ui8 = 0;
		ds.deserialize(ui8);
		ASSERT_EQ(ui8, (uint8_t)255);
		i16 = 0;
		ds.deserialize(i16);
		ASSERT_EQ(i16, -2);
		ui16 = 0;
		ds.deserialize(ui16);
		ASSERT_EQ(ui16, (uint16_t)100);
		i32 = 0;
		ds.deserialize(i32);
		ASSERT_EQ(i32, 123);
		ui32 = 0;
		ds.deserialize(ui32);
		ASSERT_EQ(ui32, (uint32_t)2323);
		i64 = 0;
		ds.deserialize(i64);
		ASSERT_EQ(i64, 13);
		ui64 = 0;
		ds.deserialize(ui64);
		ASSERT_EQ(ui64, (uint64_t)235);
		v.clear();
		ds.deserialize(v);
		ASSERT_EQ(v.size(), (size_t)2);
		std::cout << v.front() << ", " << v.back() << std::endl;
		st.clear();
		ds.deserialize(st);
		ASSERT_EQ(st.size(), (size_t)2);
		m.clear();
		ds.deserialize(m);
		ASSERT_EQ(m.size(), (size_t)1);
		std::cout << m["hello"] << std::endl;
		t.a = 0;
		ds.deserialize(t);
		ASSERT_EQ(t.a, (uint32_t)100);

		in.close();
	} catch (serial_exception &e) {
		std::cout << e.what() << std::endl;
	}
}



/*
 * serialize.hpp
 *
 *  Created on: Jul 22, 2013
 *      Author: king
 */

#ifndef SERIALIZE_HPP_
#define SERIALIZE_HPP_

#include "base.hpp"

namespace wbase { namespace common { namespace core {

enum TYPE { NIL = 0, BOOL = 1, INT8 = 2, UINT8 = 3, INT16 = 4, UINT16 = 5, INT32 = 6, UINT32 = 7, INT64 = 8,
	UINT64 = 9, FLOAT = 10, DOUBLE = 11, STRING = 12, VECTOR = 13, SET = 14, MAP = 15 };

class serial_exception : public exception_base {
public:
	serial_exception(const std::string &str) : exception_base(str) {}
	virtual ~serial_exception() throw() {}
};

class serializable {
public:
	virtual ~serializable() {}
	virtual void serialize(std::ostream &out) const = 0;
	virtual void deserialize(std::istream &in) = 0;
};

class serializer {
public:
	serializer(std::ostream &stream) : m_out(stream) {}

	template<typename T>
	serializer& serialize(const T &t) {
		write_to(t, int2type<SUPER_SUB_CLASS(serializable, T) >()); //dispatch
		return *this;
	}

private:
	template<typename T>
	void write_to(const T &t, int2type<true>) {
		t.serialize(m_out);
	}

	template<typename T>
	void write_to(const T *t, int2type<true>) {
		t->serialize(m_out);
	}

	template<typename T>
	void write_to(const T &t, int2type<false>) {
		native_write(t);
	}

	void native_write(const bool b) {
		write_type(BOOL);
		write_data(b);
	}

	void native_write(const int8_t i8) {
		write_type(INT8);
		write_data(i8);
	}

	void native_write(const uint8_t ui8) {
		write_type(UINT8);
		write_data(ui8);
	}

	void native_write(const int16_t i16) {
		write_type(INT16);
		write_data(htobe16(i16));
	}

	void native_write(const uint16_t ui16) {
		write_type(UINT16);
		write_data(htobe16(ui16));
	}

	void native_write(const int32_t i32) {
		write_type(INT32);
		write_data(htobe32(i32));
	}

	void native_write(const uint32_t ui32) {
		write_type(UINT32);
		write_data(htobe32(ui32));
	}

	void native_write(const int64_t i64) {
		write_type(INT64);
		write_data(htobe64(i64));
	}

	void native_write(const uint64_t ui64) {
		write_type(UINT64);
		write_data(htobe64(ui64));
	}

	void native_write(const std::string &s) {
		write_type(STRING);
		uint32_t size = static_cast<uint32_t>(s.size());
		native_write(size);
		m_out.write(s.data(), size);
		if (!m_out) {
			throw serial_exception("failed to write data while serializing std::string");
		}
	}

	template<typename T>
	void native_write(const std::vector<T> &v) {
		try {
			write_type(VECTOR);
			uint32_t size = static_cast<uint32_t>(v.size());
			native_write(size);
			for (uint32_t i = 0; i < size; i++) {
				serialize(v[i]);
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::vector");
		}
	}

	template<typename K>
	void native_write(const std::set<K> &s) {
		try {
			write_type(SET);
			uint32_t size = static_cast<uint32_t>(s.size());
			native_write(size);
			typename std::set<K>::const_iterator it = s.begin();
			for (; it != s.end(); it++) {
				serialize(*it);
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::set");
		}
	}

	template<typename K, typename V>
	void native_write(const std::map<K, V> &m) {
		try {
			write_type(MAP);
			uint32_t size = static_cast<uint32_t>(m.size());
			native_write(size);

			typename std::map<K, V>::const_iterator it = m.begin();
			for (; it != m.end(); ++it)
			{
				serialize(it->first);
				serialize(it->second);
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::map");
		}
	}

	void write_type(TYPE t) {
		m_out.write(reinterpret_cast<char *>(&t), sizeof(t));
		if (!m_out) {
			std::ostringstream oss;
			oss << "failed to write native type " << t;
			throw serial_exception(oss.str());
		}
	}

	template<typename T>
	void write_data(T d) {
		m_out.write(reinterpret_cast<char *>(&d), sizeof(d));
		if (!m_out) {
			throw serial_exception("failed to write native data ");
		}
	}

private:
	std::ostream &m_out;
};

class deserializer {
public:
	deserializer(std::istream &stream) : m_in(stream) {}

	template<typename T>
	deserializer& deserialize(T &t) {
		read_from(t, int2type<SUPER_SUB_CLASS(serializable, T)>());
		return *this;
	}

private:
	template<typename T>
	void read_from(T &t, int2type<true>)
	{
		t.deserialize(m_in);
	}

	template<typename T>
	void read_from(T &t, int2type<false>)
	{
		native_read(t);
	}

	void native_read(bool &b) {
		read_type(BOOL);
		read_data(b);
	}

	void native_read(int8_t &i8) {
		read_type(INT8);
		read_data(i8);
	}

	void native_read(uint8_t &ui8) {
		read_type(UINT8);
		read_data(ui8);
	}

	void native_read(int16_t &i16) {
		read_type(INT16);
		read_data(i16);
		i16 = be16toh(i16);
	}

	void native_read(uint16_t &ui16) {
		read_type(UINT16);
		read_data(ui16);
		ui16 = be16toh(ui16);
	}

	void native_read(int32_t &i32) {
		read_type(INT32);
		read_data(i32);
		i32 = be32toh(i32);
	}

	void native_read(uint32_t &ui32) {
		read_type(UINT32);
		read_data(ui32);
		ui32 = be32toh(ui32);
	}

	void native_read(int64_t &i64) {
		read_type(INT64);
		read_data(i64);
		i64 = be64toh(i64);
	}

	void native_read(uint64_t &ui64) {
		read_type(UINT64);
		read_data(ui64);
		ui64 = be64toh(ui64);
	}

	void native_read(std::string &s) {
		char *buf = NULL;
		try {
			read_type(STRING);

			uint32_t size;
			native_read(size);
			if (size == 0)
				return;

			buf = new char[size];
			m_in.read(buf, size);
			if (!m_in) {
				throw serial_exception("failed to read data");
			} else {
				s.assign(buf, size);
			}
		} catch (serial_exception &e) {
			if (buf != NULL)
				delete buf;
			throw serial_exception(std::string(e.what()) + " while serializing std::string");
		}
		delete buf;
	}

	template<typename T>
	void native_read(std::vector<T> &v) {
		try {
			read_type(VECTOR);
			uint32_t size;
			native_read(size);
			for (uint32_t i = 0; i < size; i++) {
				T value;
				deserialize(value);
				v.push_back(value);
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::vector");
		}
	}

	template<typename K>
	void native_read(std::set<K> &s) {
		try {
			read_type(SET);
			uint32_t size;
			native_read(size);
			for (uint32_t i = 0; i < size; i++) {
				K key;
				deserialize(key);
				s.insert(key);
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::set");
		}
	}

	template<typename K, typename V>
	void native_read(std::map<K, V> &m) {
		try {
			read_type(MAP);
			uint32_t size;
			native_read(size);
			for (uint32_t i = 0; i < size; i++)
			{
				K key;
				V value;
				deserialize(key);
				deserialize(value);
				m.insert(std::make_pair(key, value));
			}
		} catch (serial_exception &e) {
			throw serial_exception(std::string(e.what()) + "\n while serializing std::map");
		}
	}

	void read_type(TYPE t) {
		TYPE _t = NIL;
		m_in.read(reinterpret_cast<char *>(&_t), sizeof(t));
		if (!m_in) {
			throw serial_exception("failed to read native type");
		}
		if (_t != t) {
			std::ostringstream oss;
			oss << "unmatched native type expect " << t << ", actual " << _t;
			throw serial_exception(oss.str());
		}
	}

	template<typename T>
	void read_data(T &d) {
		m_in.read(reinterpret_cast<char *>(&d), sizeof(d));
		if (!m_in) {
			throw serial_exception("failed to read native data ");
		}
	}

private:
	std::istream &m_in;
};



} } }


#endif /* SERIALIZE_HPP_ */

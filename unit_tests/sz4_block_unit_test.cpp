#include "config.h"

#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <cppunit/extensions/HelperMacros.h>

#include "conversion.h"

#include "sz4/defs.h"
#include "sz4/time.h"
#include "sz4/block.h"
#include "sz4/path.h"
#include "sz4/load_file_locked.h"

class Sz4BlockTestCase : public CPPUNIT_NS::TestFixture
{
	std::vector<sz4::value_time_pair<unsigned, unsigned> > m_v;
	void searchTest();
	void weigthedSumTest();
	void weigthedSumTest2();
	void pathParseTest();
	void blockLoadTest();

	CPPUNIT_TEST_SUITE( Sz4BlockTestCase );
	CPPUNIT_TEST( searchTest );
	CPPUNIT_TEST( weigthedSumTest );
	CPPUNIT_TEST( weigthedSumTest2 );
	CPPUNIT_TEST( pathParseTest );
	CPPUNIT_TEST( blockLoadTest );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Sz4BlockTestCase );

void Sz4BlockTestCase::setUp() {
	m_v.push_back(sz4::make_value_time_pair(1u, 1u));
	m_v.push_back(sz4::make_value_time_pair(3u, 3u));
	m_v.push_back(sz4::make_value_time_pair(5u, 5u));
	m_v.push_back(sz4::make_value_time_pair(7u, 7u));
	m_v.push_back(sz4::make_value_time_pair(9u, 9u));
}

void Sz4BlockTestCase::searchTest() {
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 0u)->value == 1u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 1u)->value == 1u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 2u)->value == 3u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 3u)->value == 3u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 4u)->value == 5u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 5u)->value == 5u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 7u)->value == 7u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 8u)->value == 9u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 9u)->value == 9u);
	CPPUNIT_ASSERT(search_entry_for_time(m_v.begin(), m_v.end(), 10u) == m_v.end());
}

void Sz4BlockTestCase::weigthedSumTest() {
	sz4::weighted_sum<unsigned, sz4::second_time_t> wsum;
	std::vector<sz4::value_time_pair<unsigned, sz4::second_time_t> > v = m_v;

	sz4::concrete_block<unsigned, sz4::second_time_t> block(0u);
	block.set_data(v);

	block.get_weighted_sum(0u, sz4::second_time_t(1), wsum);
	CPPUNIT_ASSERT_EQUAL(1ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(1l, wsum.weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(0u, 2u, wsum);
	CPPUNIT_ASSERT_EQUAL(4ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(2l, wsum.weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(0u, 4u, wsum);
	CPPUNIT_ASSERT_EQUAL(12ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(4l, wsum.weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(0u, 100u, wsum);
	CPPUNIT_ASSERT_EQUAL(1ull + 2ull * (3ull + 5ull + 7ull + 9ull), wsum.sum());
	CPPUNIT_ASSERT_EQUAL(9l, wsum.weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(2u, 3u, wsum);
	CPPUNIT_ASSERT_EQUAL(3ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(1l, wsum.weight());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.no_data_weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(1u, 3u, wsum);
	CPPUNIT_ASSERT_EQUAL(6ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(2l, wsum.weight());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.no_data_weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(1u, 1u, wsum);
	CPPUNIT_ASSERT_EQUAL(0ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.weight());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.no_data_weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(2u, 2u, wsum);
	CPPUNIT_ASSERT_EQUAL(0ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.weight());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.no_data_weight());

	wsum = sz4::weighted_sum<unsigned, sz4::second_time_t>();
	block.get_weighted_sum(0u, 0u, wsum);
	CPPUNIT_ASSERT_EQUAL(0ull, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.weight());
	CPPUNIT_ASSERT_EQUAL(0l, wsum.no_data_weight());
}

void Sz4BlockTestCase::weigthedSumTest2() {
	std::vector<sz4::value_time_pair<short, unsigned> > v;
	sz4::concrete_block<short, unsigned> block(0u);
	sz4::weighted_sum<short, sz4::second_time_t> wsum;

	v.push_back(sz4::make_value_time_pair((short)1, 1u));
	v.push_back(sz4::make_value_time_pair(std::numeric_limits<short>::min(), 2u));
	v.push_back(sz4::make_value_time_pair((short)1, 3u));
	block.set_data(v);

	block.get_weighted_sum(0u, 3u, wsum);
	CPPUNIT_ASSERT_EQUAL(2, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(2l, wsum.weight());
}

void Sz4BlockTestCase::pathParseTest() {
	CPPUNIT_ASSERT_EQUAL(sz4::second_time_t(21), sz4::path_to_date<sz4::second_time_t>(L"0000000021.sz4"));
	CPPUNIT_ASSERT_EQUAL(sz4::second_time_t(1), sz4::path_to_date<sz4::second_time_t>(L"0000000001.sz4"));
	CPPUNIT_ASSERT_EQUAL(sz4::invalid_time_value<sz4::second_time_t>::value(), sz4::path_to_date<sz4::second_time_t>(L"000000001.sz4"));

	sz4::nanosecond_time_t t[] = { sz4::nanosecond_time_t(1, 1), sz4::nanosecond_time_t(2, 2 ), sz4::invalid_time_value<sz4::nanosecond_time_t>::value() };
	CPPUNIT_ASSERT(t[0] == sz4::path_to_date<sz4::nanosecond_time_t>(L"00000000010000000001.sz4"));
	CPPUNIT_ASSERT(t[1] == sz4::path_to_date<sz4::nanosecond_time_t>(L"00000000020000000002.sz4"));
	CPPUNIT_ASSERT(t[2] == sz4::path_to_date<sz4::nanosecond_time_t>(L"00000000020000.sz4"));
}

void Sz4BlockTestCase::blockLoadTest() {
	char file_content[] = {
		 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
		 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
		 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
		 0x04, 0x00, 0x04, 0x00, 0x00, 0x00,
		 0x05, 0x00, 0x05, 0x00, 0x00, 0x00,
		 0x00, 0x80, 0x06, 0x00, 0x00, 0x00,
		 0x07, 0x00, 0x07, 0x00, 0x00, 0x00,
		};

	std::wstringstream file_name;
	file_name << L"/tmp/sz4block_unit_test." << getpid() << "." << time(NULL) << L".tmp";

	{
		std::ofstream ofs(SC::S2A(file_name.str()).c_str(), std::ios_base::binary);
		ofs.write(file_content, sizeof(file_content));
	}

	std::vector<sz4::value_time_pair<short, unsigned> > v;
	v.resize(7);
	CPPUNIT_ASSERT(sz4::load_file_locked(file_name.str(), reinterpret_cast<char*>(&v[0]), sizeof(file_content)));
	for (size_t i = 0; i < 7; i++) {
		if (i != 5)
			CPPUNIT_ASSERT(v.at(i).value == short(i + 1));
		else
			CPPUNIT_ASSERT(v.at(i).value == std::numeric_limits<short>::min());
	}

	sz4::concrete_block<short, unsigned> block(0u);
	block.set_data(v);

	CPPUNIT_ASSERT_EQUAL(7u, block.end_time());

	sz4::weighted_sum<short, unsigned> wsum;
	block.get_weighted_sum(0u, 7u, wsum);
	CPPUNIT_ASSERT_EQUAL(1 + 2 + 3 + 4 + 5 + 7, wsum.sum());
	CPPUNIT_ASSERT_EQUAL(6l, wsum.weight());

	unlink(SC::S2A(file_name.str()).c_str());
}

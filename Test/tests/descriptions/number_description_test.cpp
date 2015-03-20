#include "pydbc/descriptions/number_description.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit_toolbox/extensions/assert_equal_with_different_types.h>
#include <sqlext.h>
#include <limits.h>
#include <stdexcept>


class number_description_test : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE( number_description_test );

	CPPUNIT_TEST( positive_integer );
	CPPUNIT_TEST( negative_integer );
	CPPUNIT_TEST( max_positive_integer );
	CPPUNIT_TEST( min_negative_integer );
	CPPUNIT_TEST( larger_than_max_positive_overflows );

CPPUNIT_TEST_SUITE_END();

public:

	void positive_integer();
	void negative_integer();
	void max_positive_integer();
	void min_negative_integer();
	void larger_than_max_positive_overflows();

};

// Registers the fixture with the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( number_description_test );

void number_description_test::positive_integer()
{
	SQL_NUMERIC_STRUCT data = {
				18,
				0,
				1, // + sign
				// binary representation of 1234567890 from low to high bytes
				{0xd2, 0x02, 0x96, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
			};

	long const expected = 1234567890;
	pydbc::number_description description;
	auto const actual = description.make_field(reinterpret_cast<char const *>(&data));

	CPPUNIT_ASSERT_EQUAL(pydbc::field{expected}, actual);
}

void number_description_test::negative_integer()
{
	SQL_NUMERIC_STRUCT data = {
				18,
				0,
				0, // - sign
				// binary representation of 1234567890 from low to high bytes
				{0xd2, 0x02, 0x96, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
			};

	long const expected = -1234567890;
	pydbc::number_description description;
	auto const actual = description.make_field(reinterpret_cast<char const *>(&data));

	CPPUNIT_ASSERT_EQUAL(pydbc::field{expected}, actual);
}

void number_description_test::max_positive_integer()
{
	SQL_NUMERIC_STRUCT data = {
				18,
				0,
				1, // + sign
				// binary representation of 2^63 - 1
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
			};

	long const expected = std::numeric_limits<long>::max();
	pydbc::number_description description;
	auto const actual = description.make_field(reinterpret_cast<char const *>(&data));

	CPPUNIT_ASSERT_EQUAL(pydbc::field{expected}, actual);
}

void number_description_test::min_negative_integer()
{
	SQL_NUMERIC_STRUCT data = {
				18,
				0,
				0, // + sign
				// binary representation of 2^63
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
			};

	long const expected = std::numeric_limits<long>::min();
	pydbc::number_description description;
	auto const actual = description.make_field(reinterpret_cast<char const *>(&data));

	CPPUNIT_ASSERT_EQUAL(pydbc::field{expected}, actual);
}

void number_description_test::larger_than_max_positive_overflows()
{
	SQL_NUMERIC_STRUCT data = {
				18,
				0,
				1, // + sign
				// binary representation of 2^63 = maximum positive integer + 1
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
			};

	pydbc::number_description description;
	std::cout << "\n\n" << description.make_field(reinterpret_cast<char const *>(&data)) << "\n\n";
	CPPUNIT_ASSERT_THROW(description.make_field(reinterpret_cast<char const *>(&data)), std::overflow_error);
}
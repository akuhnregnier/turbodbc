/**
 *  @file field.cpp
 *  @date 19.12.2014
 *  @author mkoenig
 *  @brief
 *
 *  $LastChangedDate$
 *  $LastChangedBy$
 *  $LastChangedRevision$
 *
 */

#include <pydbc/field.h>

#include <boost/python/to_python_converter.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <vector>

namespace pydbc { namespace bindings {

struct field_to_object : boost::static_visitor<PyObject *> {
	static result_type convert(field const & f) {
		return apply_visitor(field_to_object(), f);
	}

	template<typename Value>
	result_type operator()(Value const & value) const {
		return boost::python::incref(boost::python::object(value).ptr());
	}
};

struct nullable_field_to_object : boost::static_visitor<PyObject *> {
	static result_type convert(nullable_field const & field) {
		if (field) {
			return field_to_object::convert(*field);
		} else {
			return boost::python::incref(boost::python::object().ptr());
		}
	}
};


struct nullable_field_from_object{
	static bool is_convertible (PyObject * object)
	{
		return
				boost::python::extract<long>(object).check()
			or	boost::python::extract<double>(object).check();
	}

	static nullable_field convert(PyObject * object)
	{
		boost::python::object python_value{boost::python::handle<>{boost::python::borrowed(object)}};

		{
			boost::python::extract<long> extractor(object);
			if (extractor.check()) {
				return pydbc::field(extractor());
			}
		}

		{
			boost::python::extract<double> extractor(object);
			if (extractor.check()) {
				return pydbc::field(extractor());
			}
		}

		throw std::runtime_error("Could not convert python value to C++");
	}
};

struct vector_nullable_field_from_object{
	static bool is_convertible (PyObject * object)
	{
		return PyList_Check(object);
	}

	static std::vector<nullable_field> convert(PyObject * object)
	{
		auto const size = PyList_Size(object);
		std::vector<nullable_field> result;
		for (unsigned int i = 0; i != size; ++i) {
			result.push_back(boost::python::extract<nullable_field>(PyList_GetItem(object, i)));
		}
		return result;
	}
};

template<typename Converter> struct boost_python_converter
{
	using target = decltype(Converter::convert(std::declval<PyObject*>()));

	static void * is_convertible(PyObject* object)
	{
		if(Converter::is_convertible(object)){
			return object;
		} else {
			return nullptr;
		}
	}

	static void convert(PyObject* object, boost::python::converter::rvalue_from_python_stage1_data* data)
	{
		void* storage = (reinterpret_cast<boost::python::converter::rvalue_from_python_storage<target>*>(data))->storage.bytes;
		new (storage) target(Converter::convert(object));
		data->convertible = storage;
	}
};



void for_field()
{
	boost::python::converter::registry::push_back(
		& boost_python_converter<nullable_field_from_object>::is_convertible,
		& boost_python_converter<nullable_field_from_object>::convert,
		boost::python::type_id<typename boost_python_converter<nullable_field_from_object>::target>()
	);

	boost::python::converter::registry::push_back(
		& boost_python_converter<vector_nullable_field_from_object>::is_convertible,
		& boost_python_converter<vector_nullable_field_from_object>::convert,
		boost::python::type_id<typename boost_python_converter<vector_nullable_field_from_object>::target>()
	);

	boost::python::to_python_converter<field, field_to_object>();
	boost::python::to_python_converter<nullable_field, nullable_field_to_object>();
	boost::python::implicitly_convertible<long, field>();

	bool const disable_proxies = true;
	boost::python::class_<std::vector<field>>("vector_of_fields")
    	.def(boost::python::vector_indexing_suite<std::vector<field>, disable_proxies>() );
	boost::python::class_<std::vector<nullable_field>>("vector_of_nullable_fields")
    	.def(boost::python::vector_indexing_suite<std::vector<nullable_field>, disable_proxies>() );
}

} }



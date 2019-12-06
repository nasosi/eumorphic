// Eumorphic factory example
#include <eumorphic.hpp>

#include <string>
#include <vector>
#include <iostream>

template <class T> using container = std::vector<T>;
using collection_t = eumorphic::collection< container >;

// The following two functions are the entirety of the factory
namespace detail
{
	template <class Collection>
	auto append_impl(Collection&& c, std::string type = "Init")
	{
		return std::forward<Collection>(c)
			.insert_if(type == "int", int(7))
			.insert_if(type == "double", double(3.14159))
			.insert_if(type == "string", std::string("String"));
	}
}

template <class Collection>
auto append(Collection&& c, std::string type = "")
{	//This function is needed to throw when the item type is unknown
	auto s_prior = c.size();
	auto appended_c = detail::append_impl(std::forward < Collection >(c), type);
	if (s_prior == appended_c.size() && type != "" )
	{
		throw std::runtime_error("Unknown type insertion was requested.");
	}
	return appended_c;
}

int main()
{
	std::vector< std::string > item_types = { {"int", "int", "double", "int", "int", "string"} };

	// The following line will construct an empty collection 
	// compliant with the append function 
	auto items = append( collection_t{} );
	
	// Construct the items
	for (const auto& item_type : item_types)
	{
		append( items, item_type);
	}
	
	// Process the collection
	eumorphic::for_each(items, [](auto&& item)
		{
			std::cout << item << '\n';
		});

	// If we try to add an  unkown type
	try
	{
		append( items, "text");
	}
	catch ( std::runtime_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
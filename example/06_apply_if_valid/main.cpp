#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a {};  
void print(const a&) { std::cout << "a\n"; }
void print2(const a&) {std::cout << "a2\n";}

struct b {};  
void print(const b&) { std::cout << "b\n"; }

struct c {};  
void print(const c&) { std::cout << "c\n"; } 
void print2(const c&) { std::cout << "c2\n"; }

void print(int i) { std::cout << i << "\n"; }

template <class T> using container = std::vector<T>;
using heap_collection = eumorphic::collection< container, a, b, c >;

int main()
{
	heap_collection collection;

	collection.insert(a{});
	for (auto i = 0; i != 3; i++)
	{
		collection.insert(c{});
	}
	collection.insert(b{});
	collection.insert(b{});

	std::cout << "Execute over all types for which the expression is valid\n";
	eumorphic::for_each(collection, [](auto&& v)
		{
			auto can_apply = boost::hana::is_valid([](auto&& p) -> decltype(print2(p)) {});

			if constexpr (decltype(can_apply(v))::value)
			{
				print2(v);
			}
		});

	auto extended_collection = collection.add_types_and_copy<int>();
	extended_collection.insert(2);
	extended_collection.insert(3);

	std::cout << "Extended collection\n";
	eumorphic::for_each(extended_collection, [](auto&& v)
		{
			print(v);
		});

}
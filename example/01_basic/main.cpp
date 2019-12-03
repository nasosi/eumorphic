#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a {};  
void print(const a&) { std::cout << "a\n"; }

struct b {};  
void print(const b&) { std::cout << "b\n"; }

struct c {};  
void print(const c&) { std::cout << "c\n"; } 

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

	a a1;
	collection.insert(a1);

	std::cout << "Execute over all types: \n";
	eumorphic::for_each( collection, [](auto&& v)
		{
			print(v);
		});

}
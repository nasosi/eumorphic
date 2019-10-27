#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a {};  
void print(const a&) { std::cout << "a"; }

struct b {};  
void print(const b&) { std::cout << "b"; }

struct c {};  
void print(const c&) { std::cout << "c"; } 
void print2(const c&) { std::cout << "cc"; }

template <class T> using container = std::vector<T>;
using heap_collection = eumorphic::collection< container, a, b, c >;

template <class T>
using max10_per_type = eumorphic::bounded_array<T, 10>;
using stack_collection = eumorphic::collection< max10_per_type, a, b, c >;

using ordered_collection = eumorphic::ordered_collection< container, a, b, c >;

int main()
{	
	stack_collection collection;

	collection.insert(a{});
	for (auto i = 0; i != 3; i++)
	{
		collection.insert(c{});
	}
	collection.insert(b{});
	collection.insert(b{});

	std::cout << "On the stack:\n";
	eumorphic::for_each(collection, [](auto&& v)
	{
		print(v);
		std::cout << ": " << typeid(v).name() << '\n';
	});
	
	std::cout << "Only c:\n";
	eumorphic::subset_for_each<c> (collection, [](auto&& v)
	{
		print2(v); // <-- Only in class c.
		std::cout << ": " << typeid(v).name() << '\n';
	});

}
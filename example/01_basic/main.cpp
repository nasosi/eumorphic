#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a {};  
void print(const a&) { std::cout << "a"; }

struct b {};  
void print(const b&) { std::cout << "b"; }

struct c {};  
void print(const c&) { std::cout << "c"; } 

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

		std::cout << "On the heap: \n";
		eumorphic::for_each( collection, [](auto&& v)
			{
				print(v);
				std::cout << ": " << typeid(v).name() << '\n';
			});
	

}
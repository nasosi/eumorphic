#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a { int id; };
void print(const a& aa) { std::cout << "a(" << aa.id << ")"; }

struct b { int id; };
void print(const b& bb) { std::cout << "b(" << bb.id << ")"; }

struct c { int id; };
void print(const c& cc) { std::cout << "c(" << cc.id << ")"; }

template <class T> using container = std::vector<T>;
using heap_collection = eumorphic::collection< container, a, b, c >;

int main()
{
	heap_collection collection;

	collection.insert(a{0});
	for (auto i = 0; i != 5; i++)
	{
		collection.insert(c{i});
	}
	collection.insert(b{0});
	collection.insert(b{1});
	collection.insert(b{2});

	std::cout << "On the heap access from the middle: \n";
	eumorphic::for_each( collection, 2, [](auto&& v)
		{
			print(v);
			std::cout << ": " << typeid(v).name() << '\n';
		});
}
#include <eumorphic.hpp>

#include <iostream>
#include <typeinfo>

struct a { int id; };
void print(const a& aa) { std::cout << "a(" << aa.id << ")"; }

struct b { int id; };
void print(const b& bb) { std::cout << "b(" << bb.id << ")"; }

struct c { int id;  };
void print(const c& cc) { std::cout << "c(" << cc.id << ")"; }

template <class T> using container = std::vector<T>;
using ordered_collection = eumorphic::ordered_collection< container, a, b, c >;


int main()
{
	ordered_collection collection;
	int id = 0;
	collection.insert(a{id++});
	for (auto i = 0; i != 3; i++)
	{
		collection.insert(c{ id++ });
	}
	collection.insert(b{ id++ });
	collection.insert(b{ id++ });
	collection.insert(a{ id++ });

	a a1;
	collection.insert(a1);

	std::cout << "Ordered collection:\n";
	eumorphic::for_each(collection, [](auto&& v)
	{
		print(v);
		std::cout << ": " << typeid(v).name() << '\n';
	});
}
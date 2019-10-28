#include <eumorphic.hpp>

#include <boost/poly_collection/base_collection.hpp>
#include <boost/poly_collection/any_collection.hpp>
#include <boost/poly_collection/algorithm.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/mpl/vector.hpp>

#include <exception>
#include <iostream>
#include <numeric>
#include <variant>
#include <memory>
#include <chrono>
#include <random>
#include <vector>
#include <limits>
#include <cmath>

double d = 0;

constexpr std::size_t test_size = 5000; // This needs to be restricted because of the stack frame size for the stack container
constexpr std::size_t average_repetitions = 100;

constexpr std::size_t stack_array_size(std::size_t x)
{
	if constexpr (test_size > 100) // This could be 6 or something, but because of the stochasticity of the benchmarking random generator we may get into trouble
	{
		return test_size / 2;
	}
	return test_size;

}
std::chrono::high_resolution_clock::time_point tic()
{
	return std::chrono::high_resolution_clock::now();
}

double toc(const std::chrono::high_resolution_clock::time_point& tp)
{
	auto elapsed = std::chrono::high_resolution_clock::now() - tp;
	auto d = std::chrono::duration_cast<std::chrono::duration< double >>(elapsed);
	return d.count();
}


struct A
{
	A(double a = 1.0) : data(a) {}
	virtual ~A() { }
	void get_data( double& d) { d += data; } //Had to add this to make boost::any_collection work
	virtual void run() { data += 1; }
public:
	double data;
	
};
struct B : A   { using A::A;   virtual ~B() { }  virtual void run() override { data /= 2; } };
struct C : B   { using B::B;   virtual ~C() { }  virtual void run() override { data *= data; } };
struct D : C { using C::C;   double data2[5]; virtual ~D() { }  void run()   final override { data = data * data * data; } };
struct CA : C  { using C::C;   virtual ~CA() { } virtual void run() override { data -= 31; } };
struct CB : CA { using CA::CA; double data3[10];  virtual ~CB() { } void run()   final override { data *= 32; } };

constexpr std::size_t num_types = 10;

// Generic static dispatch
template <class T, class Container >
void insert_default_value_of(Container& c )
{
	c.insert(T{ });
}

template <class T, class Container, class E, class R >
void insert_rnd_value_of( Container& c, E& gen, const R &rnd )
{
	c.insert( T{ rnd( gen ) });
}

// any_collection specializations
namespace boost {
	BOOST_TYPE_ERASURE_MEMBER(has_run, run)
	BOOST_TYPE_ERASURE_MEMBER(has_get_data, get_data) 
}
using run_concept = boost::has_run<void()> ;
using get_data_concept = boost::has_get_data<void( double &)> ;

using concept_=boost::mpl::vector<get_data_concept, run_concept>;

using any_collection = boost::any_collection<concept_> ;
using base_collection	= boost::base_collection<A>;
using vec_collection	= std::vector<std::unique_ptr<A>>;

using var_t = std::variant<A, B, C, D, CA, CB>;
using variant_collection = std::vector< var_t>;

void do_not_optimize_out(any_collection& c)
{
	if (c.size() > 0)
	{
		boost::poly_collection::for_each<A, B, C, D, CA, CB>(
			c.begin(), c.end(), [&](auto& x)
			{
				x.get_data(d);
			});
	}
	else
	{
		d = (int)c.size();
	}
}

void process(variant_collection& collection)
{
	for (auto e : collection)
	{
		std::visit([](auto&& v)
			{
				v.run();
			}, e);
	}

}

void process(any_collection& collection)
{
	boost::poly_collection::for_each<A, B, C, D, CA, CB>(
		collection.begin(), collection.end(), [&](auto& x)
		{
			x.run();
		});
}

// Boost base_colleciton specializations
template < class Base >
void process( boost::base_collection<Base>& collection ) 
{
	boost::poly_collection::for_each<A, B, C, D, CA, CB>(
		collection.begin(), collection.end(), [&](auto& x)
		{
			x.run();
		});
}

template < class Base >
void do_not_optimize_out(boost::base_collection<Base>& c) 
{ 
	if (c.size() > 0)
	{
		d += c.begin()->data;
	}
	else
	{
		d = (int)c.size();
	}
}


// eumorphic collection specializations
template <class T> using container = std::vector<T>;
using heap_collection = eumorphic::collection< container, A, B, C, D, CA, CB >;

template <class T> using stack_container = eumorphic::bounded_array<T, stack_array_size( test_size) >; // Because of the stochasticity we cannot dtermine the required size. We may crash but this is working.
using stack_collection = eumorphic::collection< stack_container, A, B, C, D, CA, CB >;

using ordered_heap_collection = eumorphic::ordered_collection< container, A, B, C, D, CA, CB >;


template < template <typename...> class Vec, class ...Types >
void process(eumorphic::collection<Vec, Types...>& c)
{
	eumorphic::for_each( c, [](auto& x) { x.run(); });
}

template < template <typename...> class Vec, class ...Types >
void do_not_optimize_out(eumorphic::collection<Vec, Types...>& c)
{
	eumorphic::for_each(c, [](auto& x) { d += x.data; });
}

// vector of pointers overloads
template <class T>
void insert_default_value_of(vec_collection& v )
{
	v.push_back(std::make_unique<T>());
}

template <class T,  class E, class R >
void insert_rnd_value_of( vec_collection& v, E& gen, const R& rnd)
{
	v.push_back(std::make_unique<T>( rnd(gen) ));
}

void process(vec_collection& v) 
{ 
	for (auto &p : v) 
	{ 
		p->run(); 
	} 
}

void do_not_optimize_out(vec_collection& v)
{
	if (v.size() > 0)
	{
		for ( auto &e : v )
		d += e->data;
	}
	else
	{
		d = (int)v.size();
	}
}
template <class T>
void insert_default_value_of(variant_collection& v)
{
	v.push_back(T{});
}

template <class T, class E, class R >
void insert_rnd_value_of(variant_collection& v, E& gen, const R& rnd)
{
	v.push_back( T{ rnd(gen) });
}

void do_not_optimize_out( variant_collection& col )
{
	if (col.size() > 0)
	{
		for (auto e : col)
		{
			std::visit([](auto&& v)
				{
					d += v.data;
				}, e);
		}
	}
	else
	{
		d = (int)col.size();
	}
}


// Utilities
template <class Cont, class E, class R1, class R2  >
void random_fill(Cont&container, E& gen, const R1& rnd_type, const R2& rnd, std::size_t count )
{
	for ( std::size_t i = 0; i != count; i++)
	{
		switch (rnd_type(gen))
		{
			case 0: insert_rnd_value_of<A>( container, gen, rnd ); break;
			case 1: insert_rnd_value_of<B>( container, gen, rnd ); break;
			case 2: insert_rnd_value_of<C>( container, gen, rnd ); break;
			case 3: insert_rnd_value_of<D>( container, gen, rnd ); break;
			case 4: insert_rnd_value_of<CA>(container, gen, rnd); break;
			case 5: insert_rnd_value_of<CB>(container, gen, rnd); break;
		}
	}
}


// The actual benchmark code
template <class Cont>
double benchmark_insert( std::size_t num_elems)
{
	Cont container;

	//std::cout << "i";

	auto start = tic();
	{
		for ( auto i = 0; i != num_elems; i++)
		{
			insert_default_value_of<C>(container);
		}
	}
	double t = toc(start);
	double duration = t / container.size();
	do_not_optimize_out(container);

	return duration;
}

template <class Cont>
double benchmark_processing( std::size_t num_elems)
{
	Cont container;

	//std::cout << "p";
	std::random_device					gen;
	std::uniform_int_distribution<>			rnd_type(0, 5 );
	std::uniform_real_distribution<double>	rnd_value(0, 100000);
	random_fill(container, gen, rnd_type, rnd_value, num_elems);

	auto start = tic();
	{
		process(container);
	}
	double t = toc(start);
	double duration = t / container.size();
	do_not_optimize_out(container);

	return duration;
}

double mean( const std::vector<double>& v)
{
	if (v.size() == 0)
	{
		throw std::runtime_error("No elements in vector.");
	}
	double m = 0;
	for (auto e : v)
	{
		m += e;
	}

	return m / v.size();
}

double min_elem(const std::vector<double>& v)
{
	if (v.size() == 0)
	{
		throw std::runtime_error("No elements in vector.");
	}
	double m = std::numeric_limits<double>::max();
	for (auto e : v)
	{
		m = std::min(m, e);
	}

	return m;
}

double max_elem(const std::vector<double>& v)
{
	if (v.size() == 0)
	{
		throw std::runtime_error("No elements in vector.");
	}
	double m = std::numeric_limits<double>::lowest();
	for (auto e : v)
	{
		m = std::max(m, e);
	}

	return m;
}

double stdev(std::vector<double> vec)
{
	if ( vec.size() == 0 ) return NAN;

	double sum = 0;
	double mu = mean(vec);
	for (auto e : vec)
	{
		sum += std::pow(e - mu, 2);
	}

	return std::sqrt(sum / (vec.size() - 1));
}

void display_results( std::string label, const std::vector<double> &timings )
{
	// filter out outliers (+/-3 sigma level)
	auto mu = mean(timings);
	auto sigma = stdev(timings);
	auto low = mu - 3 * sigma;
	auto high = mu + 3 * sigma;
	std::vector<double> inliers;
	std::copy_if(timings.begin(), timings.end(), std::back_inserter(inliers),
		[&low, &high]( double v) 
		{
			return low < v  && v < high;
		});


	std::cout << 
		label << ", " << 
		mean(inliers) * 1e9 << ", " <<
		min_elem(inliers) * 1e9 << ", " <<
		max_elem(inliers) * 1e9 << '\n';

}



int main()
{
	std::size_t processing_elem_count = test_size;
	std::size_t insert_elem_count     = processing_elem_count;

	std::vector<double>			vec_processing_timings, vec_insert_timings;
	std::vector<double>			var_processing_timings, var_insert_timings;
	std::vector<double>			eoc_processing_timings, eoc_insert_timings;
	std::vector<double>			bc_processing_timings,  bc_insert_timings;
	std::vector<double>			ec_processing_timings,  ec_insert_timings;
	std::vector<double>			esc_processing_timings, esc_insert_timings;
	std::vector<double>			any_processing_timings, any_insert_timings;

	std::default_random_engine		gen;
	std::uniform_int_distribution<int>	rnd_test_type(0, 2*7 - 1);

	std::size_t samples = num_types * average_repetitions;
	for (auto i = 0; i != samples; i++)
	{
		auto test_num = rnd_test_type(gen);
		// std::cout << "[" << test_num;
		switch (rnd_test_type(gen))
		{
		case 0: vec_insert_timings.push_back(benchmark_insert  <vec_collection>(insert_elem_count)); break;
		case 1: var_insert_timings.push_back(benchmark_insert  <variant_collection>(insert_elem_count)); break;
		case 2: bc_insert_timings.push_back(benchmark_insert  <base_collection>(insert_elem_count)); break;
		case 3: any_insert_timings.push_back(benchmark_insert  <any_collection>(insert_elem_count)); break;
		case 4: eoc_insert_timings.push_back(benchmark_insert  <ordered_heap_collection>(insert_elem_count)); break;
		case 5: ec_insert_timings.push_back(benchmark_insert  <heap_collection>(insert_elem_count)); break;
		case 6: esc_insert_timings.push_back(benchmark_insert<stack_collection>(insert_elem_count)); break;

		case 7: vec_processing_timings.push_back(benchmark_processing  <vec_collection>( processing_elem_count)); break;
		case 8: var_processing_timings.push_back(benchmark_processing  <variant_collection>(processing_elem_count)); break;
		case 9: bc_processing_timings.push_back(benchmark_processing  <base_collection>( processing_elem_count)); break;
		case 10: any_processing_timings.push_back(benchmark_processing  <any_collection>(processing_elem_count)); break;
		case 11: eoc_processing_timings.push_back(benchmark_processing  <ordered_heap_collection>(processing_elem_count)); break;
		case 12: ec_processing_timings.push_back(benchmark_processing  <heap_collection>( processing_elem_count)); break;
		case 13: esc_processing_timings.push_back(benchmark_processing<stack_collection>( processing_elem_count)); break;


		default: throw std::runtime_error("Unhandled test type id."); break;
		}
		// std::cout << "]";
	}

	volatile double do_not_optimize = d;

	std::cout << "Test size, " << test_size << '\n';
	std::cout << "label, mean time (ns), min time (ns), max time (ns )\n";

	try
	{
		display_results("Vector of pointers insertion        ", vec_insert_timings);
		display_results("Variant vector insertion            ", var_insert_timings);
		display_results("boost::base_collection insertion    ", bc_insert_timings);
		display_results("boost::any_collection insertion     ", any_insert_timings);
		display_results("eumorphic::ordered_ collection ins. ", eoc_insert_timings);
		display_results("eumorphic::collection insertion     ", ec_insert_timings);
		display_results("eumorphic::collection (stack) ins.  ", esc_insert_timings);

		display_results("Vector of pointers processing       ", vec_processing_timings);
		display_results("Variant vector processing           ", var_processing_timings);
		display_results("boost::base_collection processing   ", bc_processing_timings);
		display_results("boost::any_collection processing    ", any_processing_timings);
		display_results("eumorphic::ordered_collection pr.   ", eoc_processing_timings); 
		display_results("eumorphic::collection processing    ", ec_processing_timings);
		display_results("eumorphic::collection (stack) pr.   ", esc_processing_timings);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n' << "Sampling size is to small, please rerun or increase the number of samples.\n";
		return 1;
	}
}
#include <boost/hana.hpp>

#include <vector>
#include <utility>
#include <array>

namespace eumorphic
{
	namespace detail
	{
		namespace hana = boost::hana;

		template <typename Iterable, typename T>
		constexpr auto index_of(Iterable const& iterable, T const& element) {
			constexpr auto size = decltype(hana::size(iterable)){ };
			constexpr auto dropped = decltype(hana::size(
				hana::drop_while(iterable, hana::not_equal.to(element))
			)){ };
			return size - dropped;
		}
	} //namespace detail

	template <template <typename...> class Container, class ...Types>
	struct collection
	{
		static constexpr auto types               = boost::hana::tuple_t<Types...>;
		static constexpr auto segment_container_template = boost::hana::template_<Container>;
		static constexpr auto segments_container_t = boost::hana::transform(types, segment_container_template);
		using  types_collection            = typename decltype(boost::hana::unpack(segments_container_t, boost::hana::template_<boost::hana::tuple>))::type;

		types_collection data_;

		template <class T>
		auto& get_segment()
		{
			using namespace detail;
			constexpr auto  type_index = index_of(segments_container_t, hana::type_c< Container<T> >);
			return hana::at(data_, type_index);
		}

		template <class T>
		constexpr void insert( T&& value )
		{
			get_segment<T>().push_back(std::forward<T>(value));
		}

		std::size_t size()
		{
			using namespace detail;
			std::size_t s = 0;
			hana::for_each( data_, [&s](auto& vec) { s += vec.size(); } );
			return s;
		}

	};

	template <template <typename...> class Container, class F, class ...Types>
	void for_each(collection<Container, Types...>& col, F&& f)
	{
		using namespace detail;
		hana::for_each(col.data_, [&f](auto& vec)
			{
				for (auto& elem : vec) { f(elem); }
			});
	}

	// Made the constructor do the work, because I couldn't make a function properly deduce the types
	// TODO: Investigate if we can do this with a function
	template <class ...SubTypes>
	struct subset_for_each
	{
		template <class Collection, class F>
		subset_for_each(Collection& col, F&& f)
		{
			static  constexpr auto  subset_types = boost::hana::tuple_t<SubTypes...>;
			static  constexpr auto  subset_segments_container_t = boost::hana::transform(subset_types, Collection::segment_container_template);

			using namespace detail;
			hana::for_each( col.data_, [&f](auto& vec)
			{
				using Container_t = std::decay_t<decltype(vec)>;

				constexpr auto  sub_type_index = index_of(subset_segments_container_t, hana::type_c< Container_t >);

				if constexpr (sub_type_index < sizeof...(SubTypes)) // Is there a better way to check if a type is in a tuple?
				{
					for (auto& elem : vec) { f(elem); }
				}
			});
		}
	};


	template <template <typename...> class Container, class ...Types>
	class ordered_collection : public collection< Container, Types...>
	{
		std::size_t next_index = 0;
		std::vector< std::size_t > types_order;
	public:

		using Base = collection< Container, Types...>;

		using Base::Base;
		template <class T>
		constexpr void insert(T&& value)
		{
			using namespace detail;
			constexpr auto  type_index = index_of(segments_container_t, hana::type_c< Container<T> >);

			types_order.push_back( type_index );
	
			hana::at(data_, type_index).push_back(std::forward<T>(value));
		}

		// TODO: Facility to remove elements in ordered_collection

		template <template <typename...> class Container, class F, class ...Types>
		friend void for_each(ordered_collection<Container, Types...>& col, F&& f);
	};

	// There is no reason why this shouldn't be for_each
	template <template <typename...> class Container, class F, class ...Types>
	void for_each( ordered_collection<Container, Types...>& col, F&& f)
	{
		// This needs some serious testing.
		using namespace detail;

		std::array< std::size_t, sizeof...(Types) > ti; // Per type indexing

		for (auto& e : ti) { e = 0; }
		auto  iter = col.types_order.begin();
		while (iter != col.types_order.end())
		{
			hana::for_each(col.data_, [&f, &iter, &col, &ti](auto& vec)
				{
					using namespace detail;
					using Container_t = typename std::decay_t<decltype(vec)>;
					using Collection_t = std::decay_t<decltype(col)>;
					constexpr auto type_index = index_of(Collection_t::segments_container_t, hana::type_c< Container_t >);
					while ((*iter) == type_index && iter != col.types_order.end() )
					{
						f(vec [ ti[type_index]] );
						ti[type_index] += 1;
						iter++;
					}
				});
		}
	}

	// Minimally developed just as proof of concept
	template< class T, std::size_t MAX_SIZE >
	struct bounded_array : std::array<T, MAX_SIZE>
	{
		using Base = std::array<T, MAX_SIZE>;
		
		using Base::Base;

		const Base::size_type& size() const noexcept { return size_; }

		void push_back( const T& elem)
		{
			this->operator[](size_++) = elem;
		}

		Base::iterator begin() noexcept
		{
			return Base::iterator(Base::data() );
		}

		Base::const_iterator begin() const noexcept
		{
			return Base::const_iterator(Base::data() );
		}

		Base::iterator end() noexcept
		{
			return Base::iterator(Base::data() + size_);
		}

		Base::const_iterator end() const noexcept
		{
			return Base::const_iterator(Base::data() + size_);
		}

	private:
		std::size_t size_ = 0;
	};


} //namespace eumoprhic
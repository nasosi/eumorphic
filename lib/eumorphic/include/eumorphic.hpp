#pragma once

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
			using Td = std::decay_t<T>;
			get_segment<Td>().push_back(std::forward<T>(value));
		}

		std::size_t size()
		{
			using namespace detail;
			std::size_t s = 0;
			hana::for_each( data_, [&s](auto& vec) { s += vec.size(); } );
			return s;
		}

	};

	template <class Container, class F, class ...Types>
	void for_each(Container&& col, std::size_t start, F&& f)
	{
		using namespace detail;
		std::size_t last_index = 0;
		hana::for_each(col.data_, [&f, &last_index, &start](auto& vec)
			{
				
				if (last_index + vec.size() > start)
				{
					auto i0 = (last_index < start) ? start - last_index : 0;

					for (auto i = i0; i != vec.size(); i++)
					{
						f(vec[i]);
					}
				}
				last_index += vec.size();
			});
	}

	template <class Container, class F, class ...Types>
	void for_each(Container&& col, F&& f)
	{
		using namespace detail;
		hana::for_each(col.data_, [&f](auto& vec)
			{
				for (auto& elem : vec) { f(elem); }
			});
	}

	// Not working atm
	//template <class Container, class F, class ...Types>
	//void for_each_if_valid(Container&& col, F&& f)
	//{
	//		using namespace detail;
	//		hana::for_each(col.data_, [&f](auto& vec) 
	//			{
	//				auto maybe_apply_f_on = hana::sfinae([&f](auto&& p) 
	//					-> decltype(f(p), bool()){ // It fails here. Does it mean that sfinae is not trasitive?
	//						f(p);
	//						return true;
	//					});
	//				for (auto& element : vec) { maybe_apply_f_on( element ); }
	//			});	
	// }

	// Made the constructor do the work, because I couldn't make a function that properly deduces the types
	// TODO: Investigate if we can do this with a function
	template <class ...SubTypes>
	struct subset_for_each
	{
		template <class Collection, class F>
		subset_for_each(Collection&& col, F&& f)
		{
			static  constexpr auto  subset_types = boost::hana::tuple_t<SubTypes...>;
			using Collection_t = std::decay_t<Collection>;
                        [[maybe_unused]] static  constexpr auto  subset_segments_container_t = boost::hana::transform(subset_types, Collection_t::segment_container_template);
			
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
						using Td = std::decay_t<T>;
                        constexpr auto  type_index = index_of(Base::segments_container_t, hana::type_c< Container<Td> >);

                        types_order.push_back( type_index );

                        hana::at(Base::data_, type_index).push_back(std::forward<T>(value));
		}

		// TODO: Facility to remove elements in ordered_collection

                template <template <typename...> class Container1, class F, class ...Types1>
                friend void for_each(ordered_collection<Container1, Types1...>& col, F&& f);
	};

	template <template <typename...> class Container, class F, class ...Types>
	void for_each( ordered_collection<Container, Types...>& col, F&& f)
	{
		// This needs some serious testing.
		using namespace detail;

		std::array< std::size_t, sizeof...(Types) > ti{}; // Per type indexing

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

                const typename Base::size_type& size() const noexcept { return size_; }

		void push_back( const T& elem)
		{
			this->operator[](size_++) = elem;
		}

                typename Base::iterator begin() noexcept
		{
                        return typename Base::iterator(Base::data() );
		}

                typename Base::const_iterator begin() const noexcept
		{
                        return typename Base::const_iterator(Base::data() );
		}

                typename Base::iterator end() noexcept
		{
                        return typename Base::iterator(Base::data() + size_);
		}

                typename Base::const_iterator end() const noexcept
		{
                        return typename Base::const_iterator(Base::data() + size_);
		}

	private:
		std::size_t size_ = 0;
	};


} //namespace eumoprhic

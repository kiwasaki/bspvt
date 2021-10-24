
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace ns{

///////////////////////////////////////////////////////////////////////////////////////////////////
//関数定義
///////////////////////////////////////////////////////////////////////////////////////////////////

//並列実行
namespace anon{ template<class T, class F, class MakeIndexTuple> inline void in_parallel_impl(const T n, F &&f, MakeIndexTuple &&make_index_tuple, const size_t nt)
{
	std::atomic<T> i(0);
	std::vector<std::thread> threads(nt);

	for(auto &thread: threads){

		auto g = [n, &i](const F &f, const MakeIndexTuple &make_index_tuple)
		{
			for(T j; (j = i.fetch_add(1)) < n; ){
				f(make_index_tuple(j));
			}
		};
		thread = std::thread(g, std::forward<F>(f), std::forward<MakeIndexTuple>(make_index_tuple));
	}
	for(auto &thread : threads){
		thread.join();
	}
}};

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class F> inline std::enable_if_t<std::is_integral<T>::value> in_parallel(const std::tuple<T, T, T> &n, F &&f, const size_t nt)
{
	const T nx = std::get<0>(n);
	const T ny = std::get<1>(n);
	const T nz = std::get<2>(n);

	if((nx <= 0) || (ny <= 0) || (nz <= 0)){
		throw std::invalid_argument("in_parallel: n must be a positive value.");
	}
	if((ny > std::numeric_limits<T>::max() / nx) || (nz > std::numeric_limits<T>::max() / (nx * ny))){
		throw std::overflow_error("in_parallel: n is too big.");
	}
	auto make_index_tuple = [nx, ny](const T i)
	{
		const T z = (i / (nx * ny) * 1);
		const T y = (i - (nx * ny) * z) / nx;
		const T x = (i - (nx * ny) * z) - nx * y;
		return std::make_tuple(x, y, x);
	};
	anon::in_parallel_impl(nx * ny * nz, std::forward<F>(f), std::move(make_index_tuple), nt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class F> inline std::enable_if_t<std::is_integral<T>::value> in_parallel(const std::tuple<T, T> &n, F &&f, const size_t nt)
{
	const T nx = std::get<0>(n);
	const T ny = std::get<1>(n);

	if((nx <= 0) || (ny <= 0)){
		throw std::invalid_argument("in_parallel: n must be a positive value.");
	}
	if(ny > std::numeric_limits<T>::max() / nx){
		throw std::overflow_error("in_parallel: n is too big.");
	}
	auto make_index_tuple = [nx](const T i)
	{
		const T y = i / nx * 1;
		const T x = i - nx * y;
		return std::make_tuple(x, y);
	};
	anon::in_parallel_impl(nx * ny, std::forward<F>(f), std::move(make_index_tuple), nt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class F> inline std::enable_if_t<std::is_integral<T>::value> in_parallel(const T n, F &&f, const size_t nt)
{
	if(n <= 0){
		throw std::invalid_argument("in_parallel: n must be a positive value.");
	}
	anon::in_parallel_impl(n, std::forward<F>(f), [](const T i){ return i; }, nt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

}; //namespace ns

///////////////////////////////////////////////////////////////////////////////////////////////////

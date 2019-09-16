// what even is a tween?
// shorhand for between??
// what even is between???

#ifndef COMMON_MOTION_HPP
#define COMMON_MOTION_HPP

float linear_curve(float x) { return x; };
float quadratic_curve(float x) { return x*x; };
float cubic_curve(float x) { return x*x*x; };

using curve_t = decltype(&linear_curve);

struct advance_result
{
	bool success = false;
	Program::duration remaining = 0s;
	explicit operator bool() { return success; }
};
struct multi_advance_result : public advance_result
{
	range<size_t> updated = range<size_t>{0,0};
	using advance_result::operator bool;
};

template <typename Type, curve_t curve = linear_curve>
struct motion
{
	Type start;
	Type end;
	Program::duration total = 0s;
	Program::duration elapsed = 0s;

	decltype(auto) value()
	{
		auto ratio = total == 0s ? 0 : elapsed.count()/total.count();
		return lerp(start, end, curve(ratio));
	}

	bool done()
	{
		return elapsed >= total;
	}

	advance_result advance(Program::duration delta)
	{
		elapsed += delta;
		if(done())
		{
			elapsed = total;
			return {false};
		}
		return {true};
	}

	void reset()
	{
		elapsed = 0s;
	}

	bool move(Type& target, Program::duration delta)
	{
		bool success = advance(delta).success;
		target = value();
		return !success;
	}
};

// welcome to proper looping
// TODO: for know total duration mod(remaining, total)
// TODO: for unknown total while(move, !success){delta = remaining}
template <typename Motion, typename Target>
bool loop(Target&& target, Motion& motion, Program::duration delta)
{
	if(auto [success, remaining] = motion.move(std::forward<Target>(target), delta);
		!success)
	{
		motion.reset();
		motion.advance(remaining);
		return true;
	}
	return false;
}

template <typename... Motions>
class ensemble // TODO
{
	std::tuple<Motions...> instruments;
	bool done();
	bool advance(Program::duration delta);
	void reset();
	template <typename T>
	bool move(T& target, Program::duration delta);
};

template <typename... Motions>
class symphony
{
	std::tuple<Motions...> movements;
	size_t current_index = 0;

	template<typename F, size_t I = sizeof...(Motions) - 1>
	void for_all(F&& f)
	{
		static_assert(I >= 0 && I < sizeof...(Motions));
		std::forward<F>(f)(std::get<I>(movements));
		if constexpr (I > 0)
			for_all<F, I-1>(std::forward<F>(f));
	}

	multi_advance_result advance(Program::duration delta, range<size_t> updated)
	{
		auto [success, remaining] = support::apply_for(current_index, [&delta](auto&& movement)
		{
			return movement.advance(delta);
		}, movements);

		if(!success)
		{
			if(current_index == sizeof...(Motions) - 1)
				return {success, remaining, updated};

			++current_index;

			if(remaining > 0s)
				return advance(remaining, {updated.lower(), updated.upper()+1});
			else
				return {true, remaining, updated};
		}
		else
		{
			return {success, remaining, updated};
		}
	}

	public:

	symphony() = default;
	symphony(Motions... motions) : movements{motions...} {}

	bool done()
	{
		return std::get<sizeof...(Motions) - 1>(movements).done();
	}

	auto advance(Program::duration delta)
	{
		return advance(delta, {current_index, current_index + 1});
	}

	template <size_t... I>
	std::tuple<decltype(std::declval<Motions>().value())...> value(std::index_sequence<I...>)
	{
		return {std::get<I>(movements).value()...};
	}

	std::tuple<decltype(std::declval<Motions>().value())...> value()
	{
		return value(std::make_index_sequence<sizeof...(Motions)>{});
	}

	void reset()
	{
		for_all([](auto& movement) {movement.reset();});
		current_index = 0;
	}

	template <typename T>
	advance_result move(T& target, Program::duration delta)
	{
		auto result = advance(delta);
		support::apply_for(current_index, [&target](auto&& movement)
		{
			target = movement.value();
		}, movements);
		return result;
	}

	template <typename... T,
		 std::enable_if_t<sizeof...(T) == sizeof...(Motions)>* = nullptr>
	advance_result move(std::tuple<T...>&& targets, Program::duration delta)
	{
		auto r = advance(delta);
		support::apply_for(r.updated, [](auto&& movement, auto&& target)
		{
			target = movement.value();
		}, movements, std::forward<std::tuple<T...>>(targets));
		return r;
	}
};

#endif /* end of include guard */

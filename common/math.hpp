#ifndef COMMON_MATH_HPP
#define COMMON_MATH_HPP
#include "simple/support.hpp"
#include "simple/geom.hpp"

namespace common
{

using namespace simple;

constexpr auto half = geom::vector<float,2>::one(.5f);
const float tau = 2*std::acos(-1);

template <typename Number, typename Ratio = float>
[[nodiscard]] constexpr
Number lerp(Number from, Number to, Ratio ratio)
{
	return from + (to - from) * ratio;
}

template <typename Vector>
[[nodiscard]] constexpr
Vector project_on_unit(Vector x, Vector surface)
{
	return surface * surface(x);
}

template <typename Vector>
[[nodiscard]] constexpr
Vector project(Vector x, Vector surface)
{
	return project_on_unit(x,surface) / surface.magnitude();
}

template <typename Vector>
[[nodiscard]] constexpr
Vector reject_from_unit(Vector x, Vector surface)
{
	return x - project_on_unit(x,surface);
}

template <typename Vector>
[[nodiscard]] constexpr
Vector reject(Vector x, Vector surface)
{
	return x - project(x,surface);
}

template <typename Vector>
[[nodiscard]] constexpr
Vector reflect_in_unit(Vector x, Vector surface)
{
	return project_on_unit(x,surface) - reject_from_unit(x,surface);
}

template <typename Vector>
[[nodiscard]] constexpr
Vector reflect(Vector x, Vector surface)
{
	return project(x,surface) - reject(x,surface);
}

template <typename Vector>
[[nodiscard]] constexpr
Vector rotate_scale(Vector x, Vector half_angle)
{
	return reflect_in_unit(
			reflect_in_unit(x, Vector::i()),
			half_angle );
}

template <typename Vector>
[[nodiscard]] constexpr
Vector rotate(Vector x, Vector half_angle)
{
	return reflect( reflect(x, Vector::i()), half_angle );
}

template <typename Vector>
[[nodiscard]] constexpr
Vector normalize(Vector x)
{
	return x / support::root2(x.quadrance());
}

template <size_t Exponent = 3, typename Value = float>
class protractor
{
	public:
	using vector = geom::vector<Value,2>;

	using array = std::array<vector, (size_t(1)<<Exponent) + 1>;

	static constexpr array circle = []()
	{
		using support::midpoint;

		array points{};

		points.front() = vector::i();
		points.back() = -vector::i();

		auto bisect = [](auto begin, auto end,
		auto& self)
		{
			if(end - begin <= 2)
				return;

			auto middle = midpoint(begin, end);
			*(middle) = normalize(midpoint(*begin, *(end - 1)));

			self(begin, middle + 1,
			self);
			self(middle, end,
			self);
		};

		if(Exponent > 0)
		{
			auto middle = midpoint(points.begin(), points.end());
			*middle = vector::j();

			bisect(points.begin(), middle + 1,
			bisect);
			bisect(middle, points.end(),
			bisect);
		}

		return points;
	}();


	static constexpr vector tau(Value factor)
	{
		assert(Value{0} <= factor && factor < Value{1});
		Value index = factor * (protractor::circle.size() - 1);
		int whole = index;
		Value fraction = index - whole;
		return lerp(circle[whole], circle[whole+1], fraction);
	}

	static constexpr vector tau()
	{
		return -vector::i();
	}

	static constexpr vector small_radian(Value value)
	{
		return {support::root2(Value{1} - value*value), value};
	}

	static vector radian(Value angle)
	{
		return {std::cos(angle), std::sin(angle)};
	}

	static constexpr vector angle(Value tau_factor)
	{
		if(tau_factor < (Value{1}/(circle.size()-1))/16 )
			return small_radian(tau_factor * common::tau);
		else
			return protractor::tau(tau_factor);
	}

};

} // namespace common

#endif /* end of include guard */


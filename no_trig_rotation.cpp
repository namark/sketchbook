#include "common/sketchbook.hpp"

// you know what projection is right? you better do cause i sure don't
constexpr
float2 project(float2 x, float2 surface)
{
	return surface * surface(x)
		/ surface.magnitude(); // can skip this, then angles will also scale (like complex numbers)
}

// rejection is the other part
// projection + rejection = original vector
constexpr
float2 reject(float2 x, float2 surface)
{
	return x - project(x,surface);
}

// reflection simply negates the rejection
constexpr
float2 reflect(float2 x, float2 surface)
{
	return project(x,surface) - reject(x,surface);
}

// rotation is just two reflections
constexpr
float2 rotate(float2 x, float2 half_angle)
{
	return reflect( reflect(x, float2::i()), half_angle );
}

constexpr
float2 normalize(float2 x)
{
	// return x / x.quadrance(); // fun!
	return x / support::root2(x.quadrance());
}

// the tricky part is specifying angles as vectors, so here are some approximations that can help
template <size_t Exponent = 3, typename Value = float>
class protractor
{
	public:
	using vector = geom::vector<Value,2>;

	// one approach is to approximate the circle with a regular polygon
	// and then linearly interpolate on that polygon to get angles
	//
	// we actually just need to approximate a half circle, since
	// our rotation doubles angles

	// we'll generate the half circle polygon
	// by starting with a flat(tau/2) angle and repeatedly bisecting it

	// the number of section will always be a power of two
	// since bisection is essentially multiplying them by two
	// so our number of iteration/precision parameter is the exponent
	using array = std::array<vector, (size_t(1)<<Exponent) + 1>;

	static constexpr array circle = []()
	{
		using support::halfway;

		array points{};

		// end points of half polygon/circle
		points.front() = vector::i();
		points.back() = -vector::i();

		auto bisect = [](auto begin, auto end,
		auto& self) // ugly recursive lambda trick -_-
		{
			// can't squeeze a new value between two adjacent elements,
			// so the array must be full already
			if(end - begin <= 2)
				return;

			// otherwise

			// we're going to put the new bisector in the middle of provided range
			auto middle = halfway(begin, end);
			// the bisector itself lies halfway between the two points,
			// since both are on circle.
			// need to normalize it to stay on the circle for subsequent bisections
			*(middle) = normalize(halfway(*begin, *(end - 1)));

			// do the same for the two bisected parts
			self(begin, middle + 1,
			self);
			self(middle, end,
			self);
		};

		if(Exponent > 0)
		{
			// have to handle the case of first bisection separately,
			// since can't normalize vector 0 :/
			auto middle = halfway(points.begin(), points.end());
			*middle = vector::j();

			bisect(points.begin(), middle + 1,
			bisect);
			bisect(middle, points.end(),
			bisect);
		}

		return points;
	}();


	// fraction of full circle
	static constexpr vector tau(Value factor)
	{
		assert(Value{0} <= factor && factor < Value{1});
		Value index = factor * (protractor::circle.size() - 1);
		int whole = index;
		Value fraction = index - whole;
		return way(circle[whole], circle[whole+1], fraction);
		// linear interpolation more or less works thanks to normalizing/dividing projection
		// otherwise this method is no good
	}

	// full circle
	static constexpr vector tau()
	{
		return -vector::i();
	}

	// small angle in radians
	static constexpr vector small_radian(Value value)
	{
		return {support::root2(Value{1} - value*value), value};
		// cause sine of small angle is pretty much equal to that angle
	}

	// any angle in radians, not constexpr -_-
	static vector radian(Value angle)
	{
		// using trigonometric function, very optional
		return {std::cos(angle), std::sin(angle)};
	}

	// decides for you weather to use tau() or small_radian()
	// is not necessarily smart
	static vector angle(Value tau_factor)
	{
		const static auto tau = std::acos(-1) * 2;
		if(tau_factor < (Value{0.5f}/(circle.size()-1))/10 )
			return small_radian(tau_factor * tau);
		else
			return protractor::tau(tau_factor);
	}

};

float2 angle = float2::one(1.f);
float regular_angle = 0.f;

void start(Program& program)
{
	program.draw_loop = [&](auto frame, auto delta)
	{
		frame.begin_sketch()
			.rectangle(rect{frame.size})
			.fill(rgb::white(0))
		;

		constexpr auto half = float2::one(0.5);

		// the outline of expected path (circle)
		frame.begin_sketch()
			.ellipse(anchored_rect2f{
				float2::one(300), frame.size/2, half
			})
			.outline(0xff00ff_rgb)
		;

		// draw rotated point (should follow the circle)
		frame.begin_sketch()
			.ellipse(anchored_rect2f{
				float2::one(10),
				frame.size/2 + rotate(float2::j(150), angle),
				float2::one(0.5f)
			})
			.fill(0xff00ff_rgb);
		;

		// draw the angle itself and a unit circle
		frame.begin_sketch()
			.line(frame.size/2, frame.size/2 + angle * 50)
			.ellipse(anchored_rect2f{
				float2::one(100), frame.size/2, half
			})
			.outline(0xffff00_rgb)
		;

		// draw the protractor polygon
		// smaller radius than unit circle just to not overlap
		{ auto sketch = frame.begin_sketch();
			for(size_t i = 1; i < protractor<>::circle.size(); ++i)
			{
				sketch.line(frame.size/2 + protractor<>::circle[i-1]*45, frame.size/2 + protractor<>::circle[i] * 45);
			}
			sketch.outline(0x00ffff_rgb);
		}

		// ijkl just free-form move the endpoint of the angle
		if(pressed(scancode::j))
			angle.x() -= 1.f * delta.count();
		if(pressed(scancode::l))
			angle.x() += 1.f * delta.count();
		if(pressed(scancode::k))
			angle.y() += 1.f * delta.count();
		if(pressed(scancode::i))
			angle.y() -= 1.f * delta.count();

		// left and right set the angle using the elaborate polygon-lerp scheme
		auto move_regular_angle = [&](float direction)
		{
			regular_angle += direction * delta.count() * 0.1f;
			regular_angle = support::wrap(regular_angle,1.f);
			angle = protractor<>::tau(regular_angle);
		};
		if(pressed(scancode::right))
			move_regular_angle(1);
		if(pressed(scancode::left))
			move_regular_angle(-1);

		// a and d use small radian approximation
		// we rotate the angle itself by another small angle
		if(pressed(scancode::a))
			angle = rotate(angle, protractor<>::small_radian(delta.count() * 0.1f));
		if(pressed(scancode::d))
			angle = rotate(angle, protractor<>::small_radian(delta.count() * -0.1f));

	};
}

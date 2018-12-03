/* You can drag the circle and the line segment, and change the radius, the
 * segment size and direction by holding the control key.
 * The circle is highlighted green if the line segment intersects it.
 * The projection point of the circle center on the line is drawn as a small
 * grey circle. The intersection points - even smaller red(first/entry),
 * blue(second/exit) and purple(touch/tangent/never happens) circles.
 * Holding shift will scale the mouse motion down to allow a bit more precise
 * positioning.
 */

#include "common/sketchbook.hpp"

constexpr float circle_radius = .12;
constexpr float corner_radius = 14.f;
constexpr float precision_mode_motion_scale = .01f;

constexpr float2 half = float2::one(.5f);

struct line
{
	float2 start;
	float2 direction;
	explicit operator range2f() const;
};

struct circle
{
	float2 center;
	float radius;
	float magnitude() const;
	explicit operator range2f() const;
	bool contains(float2 point) const;
};

line l;
circle c;

bool resize_mode = false;
bool precision_mode = false;
float2* dragged_point = nullptr;
float* dragged_radius = nullptr;
circle* dragged_circle = nullptr;

bool is_near(float2 corner, float2 position);

sketch arrow(sketch s, float2 start, float2 end);

float2 project(float2 a, float2 b);

void start(Program& program)
{
	program.key_down = [](scancode code, keycode)
	{
		switch(code)
		{
			case scancode::lctrl:
			case scancode::rctrl:
				resize_mode = true;
				break;

			case scancode::lshift:
			case scancode::rshift:
				precision_mode = true;
			break;

			default: break;
		}
	};

	program.key_up = [&program](scancode code, keycode)
	{
		switch(code)
		{
			case scancode::escape:
				program.end();
				break;

			case scancode::lctrl:
			case scancode::rctrl:
				resize_mode = false;
				break;

			case scancode::lshift:
			case scancode::rshift:
				precision_mode = false;
			break;

			default: break;
		}
	};

	program.mouse_down = [](float2 position, auto)
	{
		if(resize_mode)
		{
			if(is_near(l.start + l.direction, position))
				dragged_point = &l.direction;
			else if(is_near(c.center + float2::i(c.radius), position))
				dragged_radius = &c.radius;
		}
		else
		{
			if(is_near(l.start, position))
				dragged_point = &l.start;
			else if(c.contains(position))
				dragged_circle = &c;
		}
	};

	program.mouse_up = [](auto, auto)
	{
		dragged_point = nullptr;
		dragged_circle = nullptr;
		dragged_radius = nullptr;
	};

	program.mouse_move = [](auto, float2 motion)
	{
		if(precision_mode)
			motion *= precision_mode_motion_scale;
		if(dragged_circle)
			dragged_circle->center += motion;
		if(dragged_point)
			(*dragged_point) += motion;
		if(dragged_radius)
			(*dragged_radius) += motion.x();
	};

	program.draw_once = [](auto frame)
	{
		const float2 center = frame.size / 2;
		c.center = center;
		c.radius = frame.size.x() * circle_radius;
		l.start = center - 2 * c.radius;
		l.direction = float2::one(4 * c.radius);
	};

	program.draw_loop = [](auto frame, auto)
	{
		const float2 center = c.center - l.start;
		const float2 center_projection = project(center, l.direction);
		const float2 center_perpendicular = center - center_projection;
		const bool projection_in_segment = range{0.f, l.direction[0]}.fix().contains(center_projection[0]);
		const bool line_in_circle = (center_perpendicular).magnitude() < c.magnitude();
		const bool poke_check = c.contains(l.start) || c.contains(l.start + l.direction);

		const float magnitude_of_solution = (c.magnitude() - center_perpendicular.magnitude()) / l.direction.magnitude();

		frame.begin_sketch()
			.rectangle(rect{ frame.size })
			.fill(0xffffff_rgb)
		;

		frame.begin_sketch()
			.ellipse(range2f(c))
			.fill(poke_check || (projection_in_segment && line_in_circle) ? 0x00aa00_rgb : 0xaaaaaa_rgb)
		;

		arrow(frame.begin_sketch(), l.start, l.start + l.direction)
			.line_width(1).outline(0x770077_rgb)
		;

		frame.begin_sketch()
			.ellipse(rect{float2::one(corner_radius), l.start, half})
			.line_width(1).outline(0x555555_rgb)
		;

		frame.begin_sketch()
			.ellipse(rect{float2::one(corner_radius/2), l.start + center_projection, half})
			.line_width(3).outline(0x555555_rgb)
			.fill(0xcccccc_rgb);
		;

		if(resize_mode)
		{
			const float2 radius_point = c.center + float2::i(c.radius);
			const float2 line_point = l.start + l.direction;
			frame.begin_sketch()
				.ellipse(rect{float2::one(corner_radius), radius_point, half})
				.ellipse(rect{float2::one(corner_radius), line_point, half})
				.line_width(1).outline(0x555555_rgb)
			;
		}

		if(magnitude_of_solution > 0)
		{
			const float solution_length = sqrt(magnitude_of_solution);
			const float2 solutions[2] =
			{
				center_projection - solution_length * l.direction,
				center_projection + solution_length * l.direction
			};
			frame.begin_sketch()
				.ellipse(rect{float2::one(corner_radius/4), l.start + solutions[0], half})
				.line_width(3).outline(0x555555_rgb)
				.fill(0xff0000_rgb);
			;
			frame.begin_sketch()
				.ellipse(rect{float2::one(corner_radius/4), l.start + solutions[1], half})
				.line_width(3).outline(0x555555_rgb)
				.fill(0x0000ff_rgb);
			;
		}
		// teah, like this will ever happen...
		// can add some tolerance in the checks if want this to be more likely.
		else if(magnitude_of_solution == 0)
		{
			frame.begin_sketch()
				.ellipse(rect{float2::one(corner_radius), l.start + center_projection, half})
				.line_width(3).outline(0x555555_rgb)
				.fill(0xff00ff_rgb);
			;
		}

	};
}

line::operator range2f() const
{
	auto result = range2f{start, start + direction};
	result.fix();
	return result;
}

float circle::magnitude() const
{
	return radius * radius;
}

circle::operator range2f() const
{
	return {center - radius, center + radius};
}

bool circle::contains(float2 point) const
{
	return (center - point).magnitude() < magnitude();
}

bool is_near(float2 corner, float2 position)
{
	return circle{corner, corner_radius}.contains(position);
}

sketch arrow(sketch s, float2 start, float2 end)
{
	float2 direction = end - start;
	float2 perpendicular = direction.mix<1,0>() * float2(-1.f,1.f);
	float2 shaft = start + direction * .8f;
	s.line(start, end);
	s.line(end, shaft + perpendicular * .1f);
	s.line(end, shaft - perpendicular * .1f);
	return s;
}

float2 project(float2 a, float2 b)
{
	return b * b(a) / b.magnitude();
}

/* You can drag the two ranges(axis-aligned rectangles) around, and resize
 * them by holding the control key and dragging the bounds(corners).
 * The result of the intersection function is highlighted green if it's valid
 * and red otherwise. There is also an arrow drawn pointing from the lower
 * bound to the upper bound of the intersection. It seems to carry some
 * information in the invalid case that might be useful?
 */

#include "common/sketchbook.hpp"

constexpr float corner_radius = 14.f;
constexpr float initial_range_size = .25f;
constexpr float2 half = float2::one(.5f);

range2f a {};
range2f b {};

bool resize_mode = false;
range2f* dragged = nullptr;
float2* dragged_corner = nullptr;

bool is_near(float2 corner, float2 position);

sketch arrow(sketch s, float2 start, float2 end);

void print_test_case();

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

			case scancode::space:
				print_test_case();
			break;

			default: break;
		}
	};

	program.key_up = [&program](scancode code, keycode)
	{
		switch(code)
		{
			case scancode::leftbracket:
			case scancode::c:
				if(pressed(scancode::rctrl) || pressed(scancode::lctrl))
			case scancode::escape:
				program.end();
			break;

			case scancode::lctrl:
			case scancode::rctrl:
				resize_mode = false;
			break;

			default: break;
		}
	};

	program.mouse_down = [](float2 position, auto)
	{
		if(resize_mode)
		{
			if(is_near(b.upper(), position))
				dragged_corner = &b.upper();
			else if(is_near(b.lower(), position))
				dragged_corner = &b.lower();
			else if(is_near(a.upper(), position))
				dragged_corner = &a.upper();
			else if(is_near(a.lower(), position))
				dragged_corner = &a.lower();
		}
		else
		{
			if(b.contains(position))
				dragged = &b;
			else if (a.contains(position))
				dragged = &a;
		}
	};

	program.mouse_up = [](auto, auto)
	{
		dragged = nullptr;
		dragged_corner = nullptr;
	};

	program.mouse_move = [](auto, float2 motion)
	{
		if(dragged)
			(*dragged) += motion;
		if(dragged_corner)
			(*dragged_corner) += motion;
	};

	program.draw_once = [](auto frame)
	{
		const auto size = frame.size * initial_range_size;
		auto r = rect{size, frame.size/2, float2::one(0.25f)};
		a = r;
		b = rect{r, float2::one(0.75f)};
	};

	program.draw_loop = [](auto frame, auto)
	{

		frame.begin_sketch()
			.rectangle(rect{ frame.size })
			.fill(0xffffff_rgb)
		;

		frame.begin_sketch()
			.rectangle(a)
			.fill(0xaaaaaa_rgb)
		;

		frame.begin_sketch()
			.rectangle(b)
			.fill(0xaaaaaa_rgb)
		;

		const auto c = intersection(a,b);
		frame.begin_sketch()
			.rectangle(c.fix())
			.fill(c.valid() ? 0x00ff00_rgb : 0xff0000_rgb)
		;

		arrow(frame.begin_sketch(), c.lower(), c.upper())
			.line_width(1).outline(0x770077_rgb)
		;

		if(resize_mode)
		{
			frame.begin_sketch()
				.ellipse(rect{float2::one(corner_radius), b.upper(), half})
				.ellipse(rect{float2::one(corner_radius), b.lower(), half})
				.ellipse(rect{float2::one(corner_radius), a.upper(), half})
				.ellipse(rect{float2::one(corner_radius), a.lower(), half})
				.line_width(1).outline(0x555555_rgb)
			;
		}

	};
}

bool is_near(float2 corner, float2 position)
{
	return (corner - position).magnitude() < corner_radius * corner_radius;
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

void print_test_case()
{
	const auto i = intersection(a,b);
	std::cout << '{' << '\n';
	std::cout << '\t' << "auto a = range{" << "vector" << a.lower() << ", vector" << a.upper() << "};" << '\n';
	std::cout << '\t' << "auto b = range{" << "vector" << b.lower() << ", vector" << b.upper() << "};" << '\n';
	std::cout << '\t' << "auto i = range{" << "vector" << i.lower() << ", vector" << i.upper() << "};" << '\n';
	std::cout << '\t' << "assert(intersection(a,b) == i);" << '\n';
	std::cout << '\t' << "assert(i.valid() == " << (i.valid() ? "true" : "false") << ");" << '\n';
	std::cout << '\t' << "assert(i.valid() == intersects(a,b));" << '\n';
	std::cout << '}' << '\n';
	std::cout << '\n';
}

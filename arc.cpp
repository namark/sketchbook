/* Nothing interesting here, just testing the arc function...
 * There bunch of points in top left stacked on top of each other...
 * You can drag them around and see what they do I guess...
 */

#include "common/sketchbook.hpp"

constexpr float corner_radius = 14.f;
constexpr float2 half = float2::one(.5f);
const float tau = 2*std::acos(-1);


struct point
{
	enum
	{
		radius,
		angle_start,
		angle_end,
		center,

		count
	};
};

std::array<float2, point::count> points;

float2* dragged_point = nullptr;

bool is_near(float2 corner, float2 position);


void start(Program& program)
{

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

			default: break;
		}
	};

	program.mouse_down = [](float2 position, auto)
	{
		for(int i = 0; i < point::count; ++i)
			if(is_near(points[i], position))
				dragged_point = &points[i];
	};

	program.mouse_up = [](auto, auto)
	{
		dragged_point = nullptr;
	};

	program.mouse_move = [](auto, float2 motion)
	{
		if(dragged_point)
			(*dragged_point) += motion;
	};

	program.draw_loop = [](auto frame, auto)
	{

		frame.begin_sketch()
			.rectangle(rect{ frame.size })
			.fill(0xffffff_rgb)
		;

		const float2 angle_start = tau * points[point::angle_start]/frame.size;
		const float2 angle_end = tau * points[point::angle_end]/frame.size;
		frame.begin_sketch()
			.arc(points[point::center], rangef{angle_start.x(), angle_end.x()},
				points[point::radius].x())
			.line_width(2).outline(0x0_rgb)
			.fill(0xaaaaaa_rgb)
		;

		{ auto sketch = frame.begin_sketch();
			for(int i = 0; i < point::count; ++i)
				sketch.ellipse(rect{float2::one(corner_radius), points[i], half});
			sketch.line_width(1).outline(0x555555_rgb);
		}

	};
}

bool is_near(float2 corner, float2 position)
{
	return (corner - position).magnitude() < corner_radius * corner_radius;
}

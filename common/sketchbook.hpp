#include <cstring>
#include <cstdio>
#include <thread>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <array>

#include "simple/support.hpp"
#include "simple/graphical.hpp"
#include "simple/interactive/initializer.h"
#include "simple/interactive/event.h"

#include "simple_vg.h"
#include "simple_vg.cpp"

using namespace std::chrono_literals;
using namespace simple;
using namespace graphical::color_literals;
using namespace support::literals;

using support::overloaded;
using support::range;

using graphical::gl_window;
using graphical::int2;
using graphical::float2;
using vg::range2f;
using rect = vg::anchored_rect2f;
using rgb = graphical::rgb_vector;
using rgba = graphical::rgba_vector;
using rgb24 = graphical::rgb_pixel;
using rgba32 = graphical::rgba_pixel;

using interactive::scancode;
using interactive::keycode;
using interactive::mouse_button;

using std::vector;

constexpr int max_int = std::numeric_limits<int>::max();

support::random::engine::tiny<unsigned> tiny_rand{std::random_device{}};
support::random::distribution::naive_int<int> tiny_int_dist{0, max_int};
support::random::distribution::naive_real<float> tiny_float_dist{0, 1};

auto trand_int(decltype(tiny_int_dist)::param_type range = {0, max_int})
{ return tiny_int_dist(tiny_rand, range); };

auto trand_float(decltype(tiny_float_dist)::param_type range = {0, 1})
{ return tiny_float_dist(tiny_rand, range); };

auto trand_float2()
{ return float2(trand_float(), trand_float()); };

template <typename Number, typename Ratio = float>
Number lerp(Number from, Number to, Ratio ratio)
{
	return from + (to - from) * ratio;
}

class Program
{
	template <typename T>
	using o = std::optional<T>;
	static constexpr auto no = std::nullopt;
	using duration = std::chrono::high_resolution_clock::duration;

	using draw_fun = std::function<void(frame)>;
	using draw_loop_fun = std::function<void(frame, duration delta_time)>;
	using key_fun = std::function<void(scancode, keycode)>;
	using mouse_move_fun = std::function<void(float2, float2)>;
	using mouse_button_fun = std::function<void(float2, mouse_button)>;


	bool run = true;
	Program(const int argc, const char * const * const argv) : argc(argc), argv(argv) {}
	friend int main(int, const char **);
	public:
	const int argc;
	const char * const * const argv;

	o<duration> frametime = no;
	std::string name = "";
	int2 size = int2(400,400);


	// nop works ok with function pointers without having to specify template params :/
	draw_fun draw_once = support::nop<void, frame>;
	draw_loop_fun draw_loop = support::nop<void, frame, duration>;
	key_fun key_down = support::nop<void, scancode, keycode>;
	key_fun key_up = support::nop<void, scancode, keycode>;
	mouse_move_fun mouse_move = support::nop<void, float2, float2>;
	mouse_button_fun mouse_down = support::nop<void, float2, mouse_button>;
	mouse_button_fun mouse_up = support::nop<void, float2, mouse_button>;

	bool running() { return run; }
	void end() { run = false; }
};

void start(Program&);

int main(int argc, char const* argv[]) try
{
	Program program{argc, argv};
	start(program);

	graphical::initializer graphics;
	interactive::initializer interactions;

	gl_window::global.require<gl_window::attribute::major_version>(2);
	gl_window::global.request<gl_window::attribute::stencil>(8);
	gl_window win(program.name, program.size, gl_window::flags::borderless);
	bool vsync = win.request_vsync(program.frametime
		? gl_window::vsync_mode::disabled
		: gl_window::vsync_mode::enablded
	);
	if(!vsync && !program.frametime)
		program.frametime = 16ms;
	float2 win_size = float2(win.size());

	glewInit();
	auto canvas = vg::canvas(vg::canvas::flags::antialias | vg::canvas::flags::stencil_strokes);
	canvas.clear();

	program.draw_once(canvas.begin_frame(win_size));

	auto& now = std::chrono::high_resolution_clock::now;
	auto t = now();
	while(program.running())
	{
		using namespace interactive;
		while(auto event = next_event()) std::visit(overloaded
		{
			[&program](const key_pressed& e)
			{
				if(!e.data.repeat)
					program.key_down(e.data.scancode, e.data.keycode);
			},
			[&program](const key_released& e)
			{
				program.key_up(e.data.scancode, e.data.keycode);
			},
			[&program](const mouse_down& e)
			{
				program.mouse_down(float2(e.data.position), e.data.button);
			},
			[&program](const mouse_up& e)
			{
				program.mouse_up(float2(e.data.position), e.data.button);
			},
			[&program](const mouse_motion& e)
			{
				program.mouse_move(float2(e.data.position), float2(e.data.motion));
			},
			[](auto) { }
		}, *event);
		program.draw_loop(canvas.begin_frame(win_size), now()-t);
		t = now();
		win.update();
		if(program.frametime)
			std::this_thread::sleep_for(*program.frametime);
	}

	return 0;
}
catch(...)
{
	if(errno)
		std::puts(std::strerror(errno));
	throw;
}

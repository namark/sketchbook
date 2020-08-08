#include <cstring>
#include <cstdio>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <array>
#include <queue>
#include <list>

#include "simple/support.hpp"
#include "simple/graphical.hpp"
#include "simple/musical.hpp"
#include "simple/interactive/initializer.h"
#include "simple/interactive/event.h"
#include "simple/motion.hpp"

#include "simple_vg.h"
#include "simple_vg.cpp" // TODO: woops, don't do this
#include "math.hpp"

#if defined __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined __ANDROID__
#undef main
#endif

using namespace simple::vg;
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
using scancode = interactive::scancode;
using keycode = interactive::keycode;
using mouse_button = interactive::mouse_button;
using common::lerp;
using simple::support::make_range;

constexpr int max_int = std::numeric_limits<int>::max();

support::random::engine::tiny<unsigned> tiny_rand{std::random_device{}};
support::random::distribution::naive_int<int> tiny_int_dist{0, max_int};
support::random::distribution::naive_real<float> tiny_float_dist{0, 1};

constexpr
auto trand_int(decltype(tiny_int_dist)::param_type range = {0, max_int})
{ return tiny_int_dist(tiny_rand, range); };

auto trand_float(decltype(tiny_float_dist)::param_type range = {0, 1})
{ return tiny_float_dist(tiny_rand, range); };

auto trand_float2()
{ return float2(trand_float(), trand_float()); };

template <size_t FPS>
class framerate
{
	public:
	constexpr static auto frames_per_second = FPS;
	// thanks howard hinnant, everything other than this amazing duration type caused a lot of stutter
	using tick = std::chrono::duration<std::uintmax_t, std::ratio<1, FPS>>;
	constexpr static auto frametime = tick(1);
};

class Program
{
	public:
	using clock = std::chrono::steady_clock;
	using duration = std::chrono::duration<float>;

	private:

	using draw_fun = std::function<void(frame)>;
	using draw_loop_fun = std::function<void(frame, duration delta_time)>;
	using key_fun = std::function<void(scancode, keycode)>;
	using mouse_move_fun = std::function<void(float2, float2)>;
	using mouse_button_fun = std::function<void(float2, mouse_button)>;

	using wave_fun = std::function<float(float ratio)>;
	struct wave
	{
		wave_fun function;
		duration total;
		duration remaining;
		explicit operator bool() { return bool(function); }
	};
	std::array<wave, 32> waves = {};
	std::mutex dam;

	std::list<std::pair<framebuffer, draw_fun>> framebuffers;
	void create_framebuffers(const canvas& canvas)
	{
		for(auto&& [fb, draw] : framebuffers)
		{
			fb.create(canvas);
			draw(canvas.begin_frame(fb));
		}
	}

	bool run = true;
	Program(const int argc, const char * const * const argv) : argc(argc), argv(argv) {}

	public:
	const int argc;
	const char * const * const argv;

	std::optional<duration> frametime = std::nullopt;
	std::string name = "";
	int2 size = int2(400,400);
	bool fullscreen = false;
	graphical::display::mode display;


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

	auto request_wave(const wave& w, size_t canal)
	{
		return request_wave(wave{w}, canal);
	}

	bool request_wave(wave&& wave, size_t canal)
	{
		std::scoped_lock lock(dam);

		assert(canal < waves.size());

		auto& current = waves[canal];

		if(current && current.remaining != 0s)
		{
			return false; // { false,  waves[canal].remaining }
		}
		current = std::move(wave);
		current.remaining = current.total;
		return true;
	}

	void require_wave(wave wave, size_t canal)
	{
		std::scoped_lock lock(dam);

		assert(canal < waves.size());
		auto& current = waves[canal];
		current = std::move(wave);
		current.remaining = current.total;
	}

	const framebuffer& request_framebuffer(int2 size, draw_fun draw,enum framebuffer::flags flags = framebuffer::flags::none)
	{
		framebuffers.emplace_back(
			std::make_pair(
				framebuffer(size, flags),
				std::move(draw)
			)
		);
		return framebuffers.back().first;
	}

	void remove_framebuffer(const framebuffer& framebuffer)
	{
		framebuffers.erase(std::find_if(
			framebuffers.begin(),
			framebuffers.end(),
			[&](auto&& fb)
			{
				return &fb.first == &framebuffer;
			}
		));
	}

	friend int main(int argc, char* argv[]);
};

template <typename T, motion::curve_t<float> curve = motion::linear_curve<float>>
using movement = motion::movement<Program::duration, T, float, curve>;
using motion::melody;

inline void process_events(Program&);

void start(Program&);

int main(int argc, char* argv[]) try
{
	Program program{argc, argv};


	graphical::initializer graphics;
	program.display = (*graphics.displays().begin()).current_mode();
	start(program);

	sdlcore::initializer interactions(sdlcore::system_flag::event);

	// TODO: make optional
	// TODO: improve simple musical to work comfortably with any type, not just mono 8 bit (even that is a pain as you can see)
	musical::initializer music;
	using namespace musical;
	device_with_callback ocean
	(
		basic_device_parameters{spec{}
			.set_channels(spec::channels::mono)
			.set_format(format::int8)
		},
		[&program](auto& device, auto buffer)
		{
			std::fill(buffer.begin(), buffer.end(), device.silence());

			auto lock = std::scoped_lock(program.dam);
			const auto tick = Program::duration(1.f/device.obtained().get_frequency());
			std::transform(buffer.begin(), buffer.end(), buffer.begin(), [&program,&tick](auto v)
			{
				int8_t typed_v = 0;
				memcpy(&typed_v, &v, 1);
				float new_v = 0;

				for(auto&& wave : program.waves)
				{
					if(wave && wave.remaining >= tick)
					{
						wave.remaining -= tick;
						if(wave.remaining < tick)
							wave.remaining = 0s;

						new_v += wave.function(1.f - (wave.remaining.count()/wave.total.count())) * 127.f;
					}
				}
				new_v = new_v + float(typed_v);
				new_v = std::clamp(new_v, -127.f,127.f);

				typed_v = new_v;
				memcpy(&v, &typed_v, 1);
				return v;
			});
		}
	);
	ocean.play();

	gl_window::global.require<gl_window::attribute::major_version>(2);
	gl_window::global.request<gl_window::attribute::stencil>(8);
	auto flags = gl_window::flags::borderless;
	if(program.fullscreen)
		flags = flags | gl_window::flags::fullscreen_desktop;
	gl_window win(program.name, program.size, flags);
#if defined __EMSCRIPTEN__
	bool vsync{!program.frametime};
#else
	bool vsync = win.request_vsync(program.frametime
		? gl_window::vsync_mode::disabled
		: gl_window::vsync_mode::enabled
	);
#endif

	if(!vsync && !program.frametime)
	{
		std::cout << "vsync didn't work"  << '\n';
		program.frametime = framerate<60>::frametime;
	}

#if !defined NANOVG_GLES2 && !defined NANOVG_GLES3
	glewInit();
#endif
	auto canvas = vg::canvas(vg::canvas::flags::antialias | vg::canvas::flags::stencil_strokes);
	canvas.clear();

	program.create_framebuffers(canvas);
	auto stolen_framebuffers = std::move(program.framebuffers);
	glViewport(0,0, win.size().x(), win.size().y());
	program.draw_once(canvas.begin_frame(float2(win.size())));

	auto& now = Program::clock::now;
	auto frame_start = now();
	auto main_loop = [&]()
	{
		auto delta_time = now() - frame_start;
		frame_start = now();

		process_events(program);
		canvas.clear();
		program.draw_loop(canvas.begin_frame(float2(win.size())), delta_time);
		win.update();
	};
#if defined __EMSCRIPTEN__
	int framerate = program.frametime ? 1.0f / program.frametime->count() : 0;
	struct
	{
		using main_loop_t = decltype(main_loop);
		main_loop_t* this_ptr;
		const decltype(&main_loop_t::operator()) call_ptr = &main_loop_t::operator();
	} split_lambda{ &main_loop };
	auto c_main_loop = [](void* data)
	{
		auto lambda = static_cast<decltype(split_lambda)*>(data);
		std::invoke(lambda->call_ptr, lambda->this_ptr);
	};
	emscripten_set_main_loop_arg(c_main_loop, &split_lambda, framerate, 1);
#else
	while(program.running())
	{
		main_loop();

		if(program.frametime)
			std::this_thread::sleep_for(*program.frametime - (now() - frame_start));
	}
#endif

	return 0;
}
catch(...)
{
	if(errno)
		std::puts(std::strerror(errno));
	throw;
}

void process_events(Program& program)
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
		[&program](const quit_request&)
		{
			program.end();
		},
		[](const window_size_changed& w)
		{
			glViewport(0,0,
				w.data.value.x(),
				w.data.value.y()
			);
		},
		[](auto) { }
	}, *event);
}



#if defined __ANDROID__
extern "C" __attribute__((visibility("default")))
int SDL_main(int argc, char* argv[]) { return main(argc, argv); }
#endif


#include <cstdio>
#include <cerrno>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "common/sketchbook.hpp"

using namespace std::literals;
using namespace musical;
using namespace common;

using support::wrap;

constexpr std::array<std::array<float,19>,4> notes {{
	{
		51.91, 55.00, 58.27, 61.74, 65.41, 69.30,
		73.42, 77.78, 82.41, 87.31, 92.50, 98.00,
		103.83, 110.00, 116.54, 123.47, 130.81,
		138.59, 146.83,
	},

	{
		155.56, 164.81, 174.61, 185.00, 196.00,
		207.65, 220.00, 233.08, 246.94, 261.63,
		277.18, 293.66, 311.13, 329.63, 349.23,
		369.99, 392.00, 415.30, 440.00,
	},

	{
		466.16, 493.88, 523.25, 554.37, 587.33,
		622.25, 659.25, 698.46, 739.99, 783.99,
		830.61, 880.00, 932.33, 987.77, 1046.50,
		1108.73, 1174.66, 1244.51, 1318.51,
	},

	{
		1396.91, 1479.98, 1567.98, 1661.22,
		1760.00, 1864.66, 1975.53, 2093.00,
		2217.46, 2349.32, 2489.02, 2637.02,
		2793.83, 2959.96, 3135.96, 3322.44,
		3520.00, 3729.31, 3951.07,
	},
}};

constexpr std::array<scancode,19> keys {
	scancode::a,
	scancode::w,
	scancode::s,
	scancode::e,
	scancode::d,
	scancode::r,
	scancode::f,
	scancode::t,
	scancode::g,
	scancode::y,
	scancode::h,
	scancode::u,
	scancode::j,
	scancode::i,
	scancode::k,
	scancode::o,
	scancode::l,
	scancode::p,
	scancode::semicolon
};
auto get_key_index(scancode key)
{
	auto it = std::find(keys.begin(), keys.end(), key);
	return keys.end() != it ? std::optional(it - keys.begin()) : std::nullopt;
};

float sinus(float angle)
{
	return rotate(float2::i(), protractor<>::tau(wrap(angle,1.f))).y();
}

void start(Program& program)
{
	program.key_down = [&program](scancode key, auto){
		auto level =
			pressed(scancode::lshift) ? 1 :
			pressed(scancode::lctrl) ? 2 :
			pressed(scancode::lalt) ? 3 :
			0;
		auto index = get_key_index(key);
		if(index)
		{
			auto wave = [note=notes[level][*index]](auto ratio)
			{
				constexpr float fade_in = 2;
				constexpr float fade_out = 2;
				float fade = std::min(ratio*fade_in, (1-ratio)*fade_out);
				return lerp(0.f,sinus(ratio * note), std::min(fade, 1.f));
			};

			int canal = 32;
			while(canal --> 0 && !program.request_wave({std::move(wave), 1000ms}, canal));
		}
	};
}


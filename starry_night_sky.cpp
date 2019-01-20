// based on: https://www.khanacademy.org/computer-programming/starry-night-sky-o_o/1545831539

#include "common/sketchbook.hpp"

int stars = 100;
float hstart = 300.f;  // left edge mountain height
float hend = 300.f; // right edge mountain height
float h = 1; //mountain roughness
float r = 100; //mountain random range
float moon_radius = 50;
float2 moon_area = float2(1.f, 0.5f);

rgb skyColorFrom (rgb24(0_u8, 0_u8, 51_u8)),
	skyColorTo (rgb24(51_u8, 51_u8, 51_u8));


bool request_draw = false;
void start(Program& program)
{
	program.frametime = 16ms;

	if(program.argc > 2)
	{
		using support::ston;
		using seed_t = decltype(tiny_rand());
		tiny_rand.seed({ ston<seed_t>(program.argv[1]), ston<seed_t>(program.argv[2]) });
	}

	program.key_up = [&](scancode code, keycode)
	{
		if(code == scancode::escape)
			program.end();
		if(code == scancode::space)
			request_draw = true;
	};

	program.draw_loop = [&](auto frame, auto)
	{
		if(request_draw)
		{
			program.draw_once(std::move(frame));
			request_draw = false;
		}
	};

	program.draw_once = [](auto frame)
	{
		std::cout << "seed: " << std::hex << std::showbase << tiny_rand << '\n';

		frame.begin_sketch()
			.rectangle(rect{frame.size})
			.fill(rgb::white(0))
		;

		//sky
		// TODO: use nanovg gradient
		for(float i = 0.f; i < frame.size.y(); ++i)
		{
			frame.begin_sketch()
				.line_width(2)
				.line({0.f,i},{frame.size.x(),i})
				.outline(lerp(skyColorFrom,skyColorTo,i/frame.size.y()))
			;
		}

		//moon
		frame.begin_sketch()
			.ellipse(rect{float2::one(moon_radius), trand_float2() * moon_area * frame.size, float2::one(0.5f)})
			.fill(rgb::white());
		;

		//stars
		for(int i=0; i < stars; ++i)
		{
			// TODO: find a better way to approximate points/stars of various sizes
			frame.begin_sketch()
				.rectangle(rect{
					trand_int({0,3}) * float2::one(),
					round(trand_float2() * frame.size)
				})
				.fill(rgb::white());
		}

		// generate mountains
		std::vector peaks = {hstart, hend};
		auto rng = r;
		while(peaks.size() < frame.size.x() + 113)
		{
			for(size_t i = 0; i < peaks.size(); i += 2)
			{
				const float variation = trand_float({-1.f, 1.f}) * rng;
				const float height = (peaks[i] + peaks[i+1])/2 + variation;
				peaks.insert(peaks.begin() + i + 1, height);
			}
			rng = rng * std::pow(2,-h);
		}

		// draw mountains
		{ auto mountain_sketch = frame.begin_sketch();

			for(size_t i = 0; i < frame.size.x(); ++i)
				mountain_sketch.line(
					{float(i), peaks[i]},
					{float(i), frame.size.y()} );

			mountain_sketch
				.line_width(2)
				.outline(rgb::white(0));
			;
		}

	};
}

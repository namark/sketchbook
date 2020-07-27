#include "common/sketchbook.hpp"
using namespace common;

struct vertex
{
	float2 origin;
	float2 normal;
};

struct polygon
{
	std::vector<vertex> vertices;
	explicit operator range2f()
	{
		constexpr auto infinity = float2::one(std::numeric_limits<float>::infinity());
		range2f ret {infinity, -infinity};
		for(auto&& v : vertices)
		{
			ret.lower().min(v.origin);
			ret.upper().max(v.origin);
		}
		return ret;
	}
};


void update_normals(polygon& p)
{
	if(p.vertices.begin() == p.vertices.end())
		return;

	auto current = p.vertices.begin();
	auto previous = p.vertices.end()-1;

	do
	{
		auto direction = current->origin - previous->origin;
		previous->normal = rotate(direction, protractor<>::tau(1.f/4));
		previous = current++;
	}
	while(current != p.vertices.end());
}

bool convex_contains(polygon polygon, float2 point)
{
	return support::all_of(polygon.vertices, [&point](auto vertex)
	{
		return vertex.normal(point - vertex.origin) > 0;
	});
}

void lerplerp_lerp(range<vertex*> buffer)
{
	auto start = (buffer.begin()+0)->origin;
	auto middle = (buffer.begin()+1)->origin;
	auto end = (buffer.begin()+2)->origin;
	auto step = 1.f / (buffer.end() - buffer.begin());
	auto ratio = 0.f;
	for(auto&& vertex : buffer)
	{
		vertex.origin = lerp
		(
			lerp(start, middle, ratio),
			lerp(middle, end, ratio),
			ratio
		), float2{};
		ratio += step;
	}
}

struct circle
{
	float2 center;
	float radius;
	explicit operator range2f()
	{
		return {center - radius, center + radius};
	}
};

bool contain(range<circle*> circles, float2 point)
{
	for(auto [center, radius] : circles)
		if(quadrance(center - point) < radius*radius)
			return true;
	return false;
}

polygon make_nose(float2 aspect)
{
	const std::array<vertex, 3> nose_control
	{{
		{aspect * float2(0.5f, 3.5f/4) + float2(-0.20f, -0.20f)},
		{aspect * float2(0.5f, 3.5f/4) + float2(+0.20f, -0.20f)},
		// {aspect * float2(0.5f, 3.f/4) + float2(+0.00f, +0.20f)},
		{aspect * float2(0.5f, 3.5f/4) + float2(+0.00f, +0.00f)},
	}};

	polygon nose{};
	for(size_t i = 0; i < nose_control.size(); ++i)
	{
		using support::wrap;
		auto& curr = nose_control[i].origin;
		auto& next = nose_control[wrap(i+1, nose_control.size())].origin;
		auto& prev = nose_control[wrap(i-1, nose_control.size())].origin;
		auto curve_control_prev = lerp(curr, prev, 1.f/2);
		auto curve_control_curr = curr;
		auto curve_control_next = lerp(curr, next, 1.f/2);
		auto length = std::max(
			geom::length(curve_control_curr - curve_control_prev),
			geom::length(curve_control_curr - curve_control_next)
		);
		auto curve_vertex_count = std::max(40.f/length,3.f); // welp magic number 40
		auto& vertices = nose.vertices;
		auto start = vertices.size();
		vertices.resize(start + curve_vertex_count);
		vertices[start+0] = vertex{curve_control_prev};
		vertices[start+1] = vertex{curve_control_curr};
		vertices[start+2] = vertex{curve_control_next};
		lerplerp_lerp({
			vertices.data() + start,
			vertices.data() + vertices.size()
		});
	}
	update_normals(nose);
	return nose;
};

void some_fur(frame& frame, rgb_pixel color, range<circle*> eyes, const polygon& nose)
{
	const auto aspect = frame.size/frame.size.x();
	const auto fur_length = support::average(frame.size.x(),frame.size.y())*12/400;
	auto fur = frame.begin_sketch();

	for(int i = 0; i < (5000 * frame.size.x() / 400); ++i)
	{
		float2 root;
		do
			root = trand_float2() * aspect;
		while(contain(eyes, root) || convex_contains(nose, root));
		root *= frame.size.x();
		auto angle = trand_float();

		auto stem = rotate_scale(trand_float2() * fur_length,
			protractor<>::tau(angle < 1 ? angle : 0));

		fur.line(root, root + stem);
	}
	fur.outline(color);
};

const unsigned fur_seed = trand_int();

std::array<circle,2> eyes;
polygon nose;
range2f nose_bounds;

const framebuffer * furbuffer;

using poke_motion = motion<float2, quadratic_curve>;
symphony<poke_motion, poke_motion> poke;

void start(Program& program)
{
	// program.size = int2{200,400};
	// program.size = int2{300,400};
	// program.size = int2{200,400}.mix<1,0>();
	// program.size = int2{400,400};
	program.fullscreen = true;
	furbuffer = &program.request_framebuffer(
		program.fullscreen ? program.display.size : program.size,
		[](auto frame)
		{
			auto aspect = frame.size/frame.size.x();
			auto radius = aspect.x()/10;
			eyes = std::array<circle,2>
			{{
				{aspect/4, radius},
				{aspect/float2(4.f/3, 4.f), radius},
			}};
			nose = make_nose(aspect);
			nose_bounds = range2f(nose) * frame.size.x();
			some_fur(frame, 0xbbbbbb_rgb, make_range(eyes), nose);
			some_fur(frame, 0xcccccc_rgb, make_range(eyes), nose);
			some_fur(frame, 0xdddddd_rgb, make_range(eyes), nose);
			some_fur(frame, 0x777777_rgb, make_range(eyes), nose);
			some_fur(frame, 0x888888_rgb, make_range(eyes), nose);
		}
	);

	program.draw_loop = [](auto frame, auto delta)
	{
		tiny_rand.seed({fur_seed, 13});
		frame.begin_sketch()
			.rectangle(rect{frame.size})
			.fill(0xaaaaaa_rgb)
		;

		for(auto&& eye : eyes)
		{
			auto blink = eye;
			blink.radius /= 2;
			blink.center -= blink.radius/2;
			frame.begin_sketch()
				.ellipse(range2f(eye) * frame.size.x())
				.fill(paint::radial_gradient(
					range2f(blink) * frame.size.x(),
					{0.f, 1.f},
					{
						rgba_vector(0xffffff_rgb),
						rgba_vector(0x000000_rgb)
					}
				))
			;
		}

		float2 poke_value;
		poke.move(poke_value, delta);

		{ auto nose_sketch = frame.begin_sketch();
			nose_sketch.move(nose.vertices.front().origin * frame.size.x() + poke_value);
			for(size_t i = 1; i < nose.vertices.size(); ++i)
				nose_sketch.vertex(nose.vertices[i].origin * frame.size.x() + poke_value);

			auto size = nose_bounds.upper() - nose_bounds.lower();
			nose_sketch.fill(paint::radial_gradient(
				range2f{
					nose_bounds.lower() + size*0.2,
					nose_bounds.lower() + size*0.5
				} + poke_value,
				{0.f, 1.f},
				{
					rgba_vector(0xffffff_rgb),
					rgba_vector(0x000000_rgb)
				}
			));
		}

		frame.begin_sketch()
			.rectangle(rect2f{frame.size})
			.fill(furbuffer->paint())
		;
	};

	program.mouse_down = [&program](auto position, auto)
	{
		auto nose_half = (nose_bounds.upper() - nose_bounds.lower())/2;
		auto offset = (nose_bounds.lower() + nose_half) - position;
		if(abs(offset) < nose_half)
		{
			program.request_wave({[poke_ratio = nose_half.length()/offset.length()](auto ratio)
			{
				constexpr float fade_in = 2;
				constexpr float fade_out = 2;
				float fade = std::min(ratio*fade_in, (1-ratio)*fade_out);
				return lerp(0.f,std::sin(ratio * 170 * poke_ratio), std::min(fade, 1.f));
			}, 100ms}, 0);
			poke = symphony(
				poke_motion{float2::zero(), offset/2, 100ms},
				poke_motion{offset/2, float2::zero(), 100ms}
			);
		}
	};
}

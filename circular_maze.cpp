/*
 */

#include "common/sketchbook.hpp"

using namespace common;
using support::wrap;

constexpr float2 size(range2f x)
{
	return x.upper() - x.lower();
}

[[nodiscard]]
constexpr float2 rotate(float2 v, float angle)
{
	return rotate(v, protractor<>::tau(angle));
}

[[nodiscard]]
constexpr float distance(float2 a, float2 b)
{
	return support::root2(quadrance(a-b));
}

// ugh! dealing with modulo arithmetic is harder than i thought...
// could be because i chose (0,1) instead of (-1,1)?
// there has got to be a better way to do this regardless.
// maybe just mod/wrap at the last moment and otherwise work in normal arithmetic
// hint; mod_cmp used in both of below,
// fix a, consider b and (b +/- mod),
// calculate diff and abs_diff and chose min by abs_diff
// return both chosen abs_diff and corresponding diff
[[nodiscard]]
constexpr float mod_distance(float a, float b, float mod)
{
	auto minmax = std::minmax(a,b);
	return std::min( minmax.second - minmax.first, minmax.first + mod - minmax.second);
}
[[nodiscard]]
constexpr float mod_difference(float a, float b, float mod)
{
	const auto distance = mod_distance(a,b,mod);
	const auto difference = b > a ? +distance : -distance;
	return support::abs(b-a) > distance ? -difference : +difference;
}

constexpr float cord_length(float slice_angle, float radius = 1.f)
{

	// very clear - self descriptive
	// very simple - naive direct implementation
	// fast?
	// + distance can be replaced with quadrance
	return distance(
		rotate(float2::i(radius), slice_angle),
		float2::i(radius)
	);

	// // very obscure - need a picture to understand
	// // very complicated - need advanced calculus to implement
	// // slow?
	// return 2 * radius * sin(slice_angle/2 * tau);

};

constexpr range2f fit(float2 area, float2 unit)
{
	// N dimensional =)
	auto scale = area/unit;
	auto min_dimension = support::min_element(scale) - scale.begin();
	auto fit_area = scale[min_dimension] * unit;
	auto cenering_mask = float2::one() - float2::unit(min_dimension);
	return range2f{float2::zero(), fit_area}
		+ (area - fit_area)/2 * (cenering_mask);
};

class circular_maze
{
	int layers = 10;
	float2 _screen_size;
	float fov;
	rangef fov_range;
	range2f bounds;

	float corridor_radius;
	float wall_wdith;
	float initial_radius;

	float2 center;

	using float_NxN = std::vector<std::vector<float>>;
	float_NxN walls;
	float_NxN paths;


	std::optional<float> closest_wall(int level, float angle)
	{
		auto closest = support::min_element(walls[level], [&](auto a, auto b)
		{
			return abs(a - angle) < abs(b - angle);
		});

		if(closest != walls[level].end())
			return *closest;
		return std::nullopt;
	}

	float path_edge_proximity(int level, float angle)
	{
		float proximity = std::numeric_limits<float>::infinity();
		const float radius = corridor_radius/tau/2/(initial_radius + level * corridor_radius);
		for(auto&& path : paths[level])
		{
			proximity = std::min(mod_distance(path + radius, angle, 1.f), proximity);
			proximity = std::min(mod_distance(path - radius, angle, 1.f), proximity);
		}
		return proximity * tau * (level*corridor_radius + initial_radius);
	}

	float wall_proximity(int level, float angle)
	{
		auto closest = closest_wall(level, angle);
		return closest
			? mod_distance(*closest, angle, 1.f) * tau * (level * corridor_radius + initial_radius)
			: std::numeric_limits<float>::infinity();
	}

	float proximity(int level, float angle)
	{
		return std::min(
			path_edge_proximity(level,angle),
			wall_proximity(level,angle)
		);
	}

	public:
	float current_angle = 0;
	float player_level = -1;

	circular_maze(float2 screen_size) :
		layers(7),
		_screen_size(screen_size),
		fov(1.f/8),
		fov_range{-fov/2,+fov/2},
		bounds{fit(screen_size,{cord_length(fov),1.f})},
		corridor_radius( size(bounds).y() / (layers+2) ),
		wall_wdith(corridor_radius/6),
		initial_radius(corridor_radius * 2),
		center{bounds.lower()+(bounds.upper() - bounds.lower()) * float2{0.5f,1.f}}
	{
		walls.resize(layers);
		paths.resize(layers);

		for(int i = 0; i < layers * 5; ++i)
		{
			int level;
			float angle;
			int breaker = 2000;
			do
			{
				level = trand_int({0,layers});
				angle = trand_float();
			}
			while(proximity(level, angle) < corridor_radius && breaker --> 0);
			paths[level].push_back(angle);
		}

		for(int i = 0; i < layers*layers; ++i)
		{
			int level;
			float angle;
			int breaker = 2000;
			do
			{
				level = trand_int({0,layers-1});
				angle = trand_float();
			}
			while((proximity(level, angle) < corridor_radius ||
				path_edge_proximity(level + 1, angle) < corridor_radius) && breaker --> 0);
			walls[level].push_back(angle);
		}

	}

	const float2& screen_size() { return _screen_size; }

	std::optional<float> hit_test(float angle, float level, const float_NxN& elements)
	{
		if(level < 0 || level >= layers)
			return std::nullopt;

		for(auto&& element : elements[level])
		{
			const auto radius = level * corridor_radius + initial_radius;
			const auto player_position = -float2::j(radius);
			const auto element_position = rotate(float2::i(radius), wrap(angle + element, 1.f));
			if(quadrance(player_position - element_position) < corridor_radius * corridor_radius / 4)
				return element;
		}
		return std::nullopt;
	}

	std::optional<float> wall_hit_test(float angle)
	{
		return hit_test(angle, player_level, walls);
	}

	std::optional<float> path_hit_test_up(float angle)
	{
		return hit_test(angle, player_level + 1, paths);
	}

	std::optional<float> path_hit_test_up(float angle, float level)
	{
		return hit_test(angle, level + 1, paths);
	}

	std::optional<float> path_hit_test_down(float angle)
	{
		return hit_test(angle, player_level, paths);
	}

	std::optional<float> path_hit_test_down(float angle, float level)
	{
		return hit_test(angle, level, paths);
	}

	void draw(vg::frame& frame)
	{
		frame.begin_sketch()
			.rectangle(rect{ frame.size })
			.fill(0x1d4151_rgb)
		;

		auto fov_range_up = fov_range - 1.f/4;


		{auto sketch = frame.begin_sketch();
			float radius = initial_radius;
			for(int i = 0; i < layers; ++i)
			{
				sketch.arc(center, fov_range_up * tau, radius - corridor_radius/2);
				radius += corridor_radius;
			}
			sketch.line_width(wall_wdith).outline(0xfbfbf9_rgb);
		}

		{auto sketch = frame.begin_sketch();
		float radius = initial_radius - corridor_radius/2;
		for(size_t level = 0; level < paths.size(); ++level)
		{
			float path_arc_angle = (corridor_radius * 0.8)/tau/radius;
			auto path_arc_range = range{-path_arc_angle, path_arc_angle}/2;
			for(size_t angle = 0; angle < paths[level].size(); ++angle)
			{
				const auto path_angle = wrap(
					paths[level][angle] + current_angle,
				1.f);
				if(!(fov_range_up + 1.f).intersects(path_arc_range + path_angle))
					continue;

				// TODO: approximate arc with a polygin so that we don't need to convert to radians,
				// here and everywhere
				sketch.arc(center, (path_arc_range + path_angle) * tau, radius);
			}
			radius += corridor_radius;
		} sketch.line_width(wall_wdith + 3).outline(0x1d4151_rgb); }

		{auto sketch = frame.begin_sketch();
		for(size_t level = 0; level < walls.size(); ++level)
		{
			for(size_t angle = 0; angle < walls[level].size(); ++angle)
			{
				const auto wall_angle = wrap(
					walls[level][angle] + current_angle,
				1.f);
				if(!(fov_range_up + 1.f).contains(wall_angle))
					continue;
				sketch.line(
					center + rotate(
						float2::i(initial_radius + corridor_radius*(float(level)-0.5f)),
						wall_angle
					),
					center + rotate(
						float2::i(initial_radius + corridor_radius*(float(level)+0.5f)),
						wall_angle
					)
				);
			}
		} sketch.line_width(wall_wdith).outline(0xfbfbf9_rgb); }

		frame.begin_sketch().ellipse(rect{
			float2::one(corridor_radius - wall_wdith -3),
			center - float2::j(player_level * corridor_radius + initial_radius),
			half
		}).fill(0x00feed_rgb);
	};

	void diagram(vg::frame& frame)
	{
		auto fov_range_up = fov_range - 1.f/4;
		auto sketch = frame.begin_sketch();
		float radius = initial_radius;
		for(int i = 0; i < layers; ++i)
		{
			sketch.arc(center, fov_range_up * tau, radius);
			radius += corridor_radius;
		}

		for(size_t level = 0; level < walls.size(); ++level)
		{
			for(size_t angle = 0; angle < walls[level].size(); ++angle)
			{
				const auto wall_angle = wrap(
					walls[level][angle] + current_angle,
				1.f);
				if(!(fov_range_up + 1.f).contains(wall_angle))
					continue;
				sketch.ellipse(rect{
					float2::one(4),
					center +
					rotate(
						float2::i(initial_radius + corridor_radius*level),
						wall_angle
					),
					half
				});
			}
		}

		for(size_t level = 0; level < paths.size(); ++level)
		{
			for(size_t angle = 0; angle < paths[level].size(); ++angle)
			{
				const auto path_angle = wrap(
					paths[level][angle] + current_angle,
				1.f);
				if(!(fov_range_up + 1.f).contains(path_angle))
					continue;

				sketch.line(
					center + rotate(
						float2::i(initial_radius + corridor_radius*level),
						path_angle
					),
					center + rotate(
						float2::i(initial_radius + corridor_radius*(float(level)-1)),
						path_angle
					)
				);
			}
		}

		sketch.ellipse(rect{
			float2::one(corridor_radius),
			center - float2::j(player_level * corridor_radius + initial_radius),
			half
		});

		sketch.line_width(1).outline(0x0_rgb);
	}

} maze(float2::one(400));

using radial_motion_t = motion<float, quadratic_curve>;
using circular_motion_t = motion<float, quadratic_curve>;

symphony<circular_motion_t, radial_motion_t>
radial_motion;

struct radial_movement
{
	float path = 0;
	float level = 0;
};
std::queue<radial_movement> radial_movements;

bool diagram = false;

void start(Program& program)
{
	program.size = int2(maze.screen_size());

	program.key_down = [](scancode code, keycode)
	{
		switch(code)
		{
			case scancode::j:
			{
				auto level = !empty(radial_movements) ? radial_movements.back().level : maze.player_level;
				auto angle = !empty(radial_movements)
					? wrap(3/4.f - radial_movements.back().path, 1.f)
					: maze.current_angle;
				auto path = maze.path_hit_test_down(angle, level);
				if(path)
					radial_movements.push({*path, level - 1});
			}
			break;

			case scancode::k:
			{
				auto level = !empty(radial_movements) ? radial_movements.back().level : maze.player_level;
				auto angle = !empty(radial_movements)
					? wrap(3/4.f - radial_movements.back().path, 1.f)
					: maze.current_angle;
				auto path = maze.path_hit_test_up(angle, level);
				if(path)
					radial_movements.push({*path, level + 1});
			}
			break;

			case scancode::d:
				diagram = !diagram;
			break;

			default: break;
		}
	};

	// program.mouse_down = [](float2 position, auto)
	// {
	// };
    //
	// program.mouse_up = [](auto, auto)
	// {
	// };
    //
	// program.mouse_move = [](auto, float2 motion)
	// {
	// };

	program.draw_loop = [](auto frame, auto delta)
	{

		maze.draw(frame);
		if(diagram)
			maze.diagram(frame);

		if(!radial_motion.done())
		{
			float unwrapped_angle = maze.current_angle;
			auto result = radial_motion.move(std::forward_as_tuple(
				unwrapped_angle,
				maze.player_level)
			, delta);
			maze.current_angle = wrap(unwrapped_angle, 1.f);

			if(!result.success)
				radial_movements.pop();
		}
		else
		{
			if(!empty(radial_movements))
			{
				auto movement = radial_movements.front();

				auto circular_distance = mod_difference(maze.current_angle, wrap(3/4.f - movement.path, 1.f), 1.f);
				auto radial_distance = movement.level - maze.player_level;
				radial_motion = symphony(
					circular_motion_t{
						maze.current_angle,
						maze.current_angle + circular_distance,
						abs(circular_distance) * 10s},
					radial_motion_t{
						maze.player_level,
						movement.level,
						abs(radial_distance) * 100ms}
				);
			}

			if(pressed(scancode::h))
			{
				auto new_angle = wrap(maze.current_angle + 0.001f, 1.f);
				if(!maze.wall_hit_test(new_angle))
					maze.current_angle = new_angle;
			}

			if(pressed(scancode::l))
			{
				auto new_angle = wrap(maze.current_angle - 0.001f, 1.f);
				if(!maze.wall_hit_test(new_angle))
					maze.current_angle = new_angle;
			}
		}

	};
}

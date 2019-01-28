// based on: https://www.khanacademy.org/computer-programming/asteroid-game/1486828627

// I, J, K, L to move, mouse to aim and shoot.

#include "common/sketchbook.hpp"

constexpr float light_speed = 40;
constexpr float drag_factor = 0.1;

float2 drag(float2 velocity, float max_speed = light_speed, float factor = drag_factor)
{
	return factor * velocity/max_speed;
}

struct projectile
{
	float2 position = float2::zero();
	float2 velocity = float2::zero();
	float2 acceleration = float2::zero();

	void move(Program::duration)
	{
		velocity += acceleration;
		position += velocity;
	}
};

struct line : public projectile
{
	float width = 1;
	rgb color = rgb(0xff00ff_rgb);

	void draw(vg::frame& frame)
	{
		frame.begin_sketch()
			.line(position - velocity, position)
			.line_width(width)
			.outline(color)
		;
	}

};

struct circle : public projectile
{
	float radius = 10;
	rgb color = rgb(0xff00ff_rgb);

	void draw(vg::frame& frame)
	{
		frame.begin_sketch()
			.ellipse(rect{radius * float2::one(), position, float2::one(0.5)})
			.fill(color)
		;
	}
};

using body = std::variant<line, circle>;

std::vector<body> bodies;

circle* crc = nullptr;

void start(Program& program)
{

	if(program.argc > 2)
	{
		using support::ston;
		using seed_t = decltype(tiny_rand());
		tiny_rand.seed({ ston<seed_t>(program.argv[1]), ston<seed_t>(program.argv[2]) });
	}
	program.frametime = 16ms;

	std::cout << "seed: " << std::hex << std::showbase << tiny_rand << '\n';

	bodies.push_back(circle{float2(program.size/2)});
	bodies.reserve(10000);
	crc = &std::get<circle>(bodies.back());

	program.key_up = [&](scancode code, keycode)
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

	program.key_down = [&](scancode code, keycode)
	{
		switch(code)
		{
			case scancode::i:
			break;
			case scancode::j:
			break;
			case scancode::k:
			break;
			case scancode::l:
			break;

			default: break;
		}
	};

	program.mouse_down = [&](float2 position, auto)
	{
		if(bodies.size() < bodies.capacity())
			bodies.push_back(line{crc->position, (position - crc->position) * 0.1f});
	};

	program.draw_loop = [&](auto frame, auto delta_time)
	{
		frame.begin_sketch()
			.rectangle(rect{frame.size})
			.fill(rgb::white(0))
		;

		crc->acceleration = float2::zero();
		if(pressed(scancode::i))
			crc->acceleration -= float2::j(0.1);
		if(pressed(scancode::k))
			crc->acceleration += float2::j(0.1);
		if(pressed(scancode::j))
			crc->acceleration -= float2::i(0.1);
		if(pressed(scancode::l))
			crc->acceleration += float2::i(0.1);

		for(auto&& body : bodies)
		{
			std::visit( [&frame, &delta_time] (auto& body)
			{
				body.move(delta_time);
				body.draw(frame);
				body.velocity -= drag(body.velocity);
				constexpr float padding = 5;
				auto fullsize = frame.size + 2 * padding;
				body.position = ((fullsize + (body.position + padding)) % fullsize) - padding;
			}, body);
		}
		std::cout << std::dec << "Size: " << bodies.size() << " FPS: " << 1/delta_time.count() << '\n';
	};

}

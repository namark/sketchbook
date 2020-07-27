#include "common/sketchbook.hpp"

using quadratic_motion = motion<float, quadratic_curve>;

float block = 0;
auto diagonal_back_and_forth = symphony(
	quadratic_motion{0,1, 500ms},
	quadratic_motion{1,0, 500ms}
);

float2 block2 = float2::zero();
auto square_around = symphony(
	quadratic_motion{0,1, 500ms},
	quadratic_motion{0,1, 500ms},
	quadratic_motion{1,0, 500ms},
	quadratic_motion{1,0, 500ms}
);

void start(Program& program)
{
	program.draw_loop = [](auto frame, auto delta)
	{

		frame.begin_sketch()
			.rectangle(rect{ frame.size })
			.fill(0xffffff_rgb)
		;

		frame.begin_sketch()
			.rectangle(rect{ frame.size/100, float2::one(10) + block*20 })
			.fill(0x0_rgb)
		;

		frame.begin_sketch()
			.rectangle(rect{ frame.size/100, float2::one(10) + float2::j(30) + block2*20 })
			.fill(0x0_rgb)
		;

		loop(block, diagonal_back_and_forth, delta);
		loop(std::forward_as_tuple(
				block2.x(),
				block2.y(),
				block2.x(),
				block2.y()
			),
			square_around, delta
		);

	};
}

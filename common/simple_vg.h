#ifndef SIMPLE_VG_H
#define SIMPLE_VG_H

#include <memory>
#include "nanovg_full.h"
#include "simple/support/enum_flags_operators.hpp"
#include "simple/geom/vector.hpp"
#include "simple/graphical/color_vector.hpp"
#include "simple/graphical/common_def.h"

namespace simple::vg
{
	using float2 = geom::vector<float, 2>;
	using graphical::rgb_vector;
	using graphical::rgba_vector;
	using graphical::rgb_pixel;
	using graphical::rgba_pixel;
	using range2f = support::range<float2>;
	using rect2f = geom::segment<float2>;
	using anchored_rect2f = geom::anchored_segment<float2>;

	class frame;

	class canvas
	{
		public:

		enum class flags
		{
			nothing = 0,
			antialias = NVG_ANTIALIAS,
			stencil_strokes = NVG_STENCIL_STROKES,
			debug = NVG_DEBUG
		};

		canvas(flags) noexcept;

		canvas& clear(const rgba_vector& color = rgba_vector::white()) noexcept;
		canvas& clear(const rgba_pixel& color) noexcept;

		frame begin_frame(float2 size, float pixelRatio = 1) noexcept;

		private:

		struct deleter
		{
			void operator()(NVGcontext* context) const noexcept;
		};

		std::unique_ptr<NVGcontext, deleter> raw;
	};
	using ::operator |;
	using ::operator &;
	using ::operator &&;

	class sketch;

	class frame
	{
		public:
			sketch begin_sketch() noexcept;
			~frame() noexcept;
			frame(const frame&) = delete;
			frame(frame&&) noexcept;
			const float2 size;
			const float pixelRatio;
		private:
			NVGcontext* context;
			frame(NVGcontext*, float2 size, float pixelRatio = 1) noexcept;
			friend class canvas;
	};

	class sketch
	{
		public:
			enum class cap
			{
				butt = NVG_BUTT,
				round = NVG_ROUND,
				square = NVG_SQUARE
			};

			enum class join
			{
				miter = NVG_MITER,
				round = NVG_ROUND,
				bevel = NVG_BEVEL
			};

			sketch& ellipse(const range2f&) noexcept;
			sketch& rectangle(const range2f&) noexcept;
			sketch& line(float2 from, float2 to) noexcept;

			sketch& fill(const rgba_vector& color) noexcept;
			sketch& fill(const rgba_pixel& color) noexcept;
			sketch& fill() noexcept;
			sketch& line_cap(cap) noexcept;
			sketch& line_join(join) noexcept;
			sketch& line_width(float) noexcept;
			sketch& miter_limit(float) noexcept;
			sketch& outline(const rgba_vector& color) noexcept;
			sketch& outline(const rgba_pixel& color) noexcept;
			sketch& outline() noexcept;

			sketch(const sketch&) = delete;
			sketch(sketch&&) noexcept;
			~sketch() noexcept;
		private:
			NVGcontext* context;
			sketch(NVGcontext*) noexcept;
			friend class frame;
	};

} // namespace simple::vg

template<> struct simple::support::define_enum_flags_operators<simple::vg::canvas::flags>
	: std::true_type {};

#endif /* end of include guard */

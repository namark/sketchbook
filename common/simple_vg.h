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
	using int2 = geom::vector<int, 2>;
	using graphical::rgb_vector;
	using graphical::rgba_vector;
	using graphical::rgb_pixel;
	using graphical::rgba_pixel;
	using range2f = support::range<float2>;
	using rangef = support::range<float>;
	using rect2f = geom::segment<float2>;
	using anchored_rect2f = geom::anchored_segment<float2>;

	class frame;
	class framebuffer;
	class sketch;

	class paint
	{
		NVGpaint raw;
		paint(NVGpaint) noexcept;

		public:
		paint() = delete;
		static paint radial_gradient(float2 center, rangef radius, support::range<rgba_vector>) noexcept;
		static paint radial_gradient(range2f range, rangef radius, support::range<rgba_vector>) noexcept;
		static paint linear_gradient() noexcept; // TODO
		static paint range_gradient() noexcept; // TODO
		friend class sketch;
		friend class framebuffer;
	};

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

		canvas(flags = flags::nothing) noexcept;

		canvas& clear(const rgba_vector& color = rgba_vector::white()) noexcept;
		canvas& clear(const rgba_pixel& color) noexcept;

		frame begin_frame(float2 size, float pixelRatio = 1) noexcept;
		frame begin_frame(framebuffer&) const noexcept;

		private:

		struct deleter
		{
			void operator()(NVGcontext* context) const noexcept;
		};

		std::unique_ptr<NVGcontext, deleter> raw;

		friend class framebuffer;
	};
	using ::operator |;
	using ::operator &;
	using ::operator &&;

	class framebuffer
	{
		public:
		enum class flags :
			std::underlying_type_t<NVGimageFlags>
		{
			none = 0,
			mipmap = NVG_IMAGE_GENERATE_MIPMAPS,
			repeat_x = NVG_IMAGE_REPEATX,
			repeat_y = NVG_IMAGE_REPEATY,
			flip_y = NVG_IMAGE_FLIPY,
			premultiplied = NVG_IMAGE_PREMULTIPLIED,
			nearest = NVG_IMAGE_NEAREST
		};

		const flags flags;
		const int2 size;

		framebuffer(int2 size, enum flags = flags::none) noexcept;
		framebuffer(const canvas&, int2 size, enum flags = flags::none);

		bool create(const canvas&);

		vg::paint paint(support::range<int2>, float opacity = 1, float angle = 0) const;
		vg::paint paint(float opacity = 1, float angle = 0) const;
		private:

		struct deleter
		{
			void operator()(NVGLUframebuffer*) const noexcept;
		};

		std::unique_ptr<NVGLUframebuffer, deleter> raw;

		friend class frame;
	};

	// TODO: prevent two frames with same context from coexisting
	class frame
	{
		public:
			sketch begin_sketch() noexcept;
			~frame() noexcept;
			frame(const frame&) = delete;
			frame(frame&&) noexcept;
			const float2 size;
			const float pixelRatio;
			const framebuffer * const buffer;
		private:
			NVGcontext* context;
			frame(NVGcontext*, float2 size, float pixelRatio = 1) noexcept;
			frame(NVGcontext*, const framebuffer&) noexcept;
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
			sketch& arc(float2 center, rangef angle, float radius) noexcept;
			sketch& arc(float2 center, float2 radius, float tau_factor, float anchor = 0) noexcept; // TODO
			sketch& move(float2) noexcept;
			sketch& vertex(float2) noexcept;

			sketch& scale(float2) noexcept;
			sketch& reset_matrix() noexcept;

			sketch& fill(const paint&) noexcept;
			sketch& fill(const rgba_vector&) noexcept;
			sketch& fill(const rgba_pixel&) noexcept;
			sketch& fill() noexcept;
			sketch& line_cap(cap) noexcept;
			sketch& line_join(join) noexcept;
			sketch& line_width(float) noexcept;
			sketch& miter_limit(float) noexcept;
			sketch& outline(const paint&) noexcept;
			sketch& outline(const rgba_vector&) noexcept;
			sketch& outline(const rgba_pixel&) noexcept;
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

template<> struct simple::support::define_enum_flags_operators<enum simple::vg::framebuffer::flags>
	: std::true_type {};

#endif /* end of include guard */

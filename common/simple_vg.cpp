#include "simple_vg.h"
#include "simple/support/enum.hpp"

using namespace simple::vg;

canvas::canvas(flags f) noexcept :
#if defined NANOVG_GL2
	raw(nvgCreateGL2(support::to_integer(f)))
#elif defined NANOVG_GL3
	raw(nvgCreateGL3(support::to_integer(f)))
#elif defined NANOVG_GLES2
	raw(nvgCreateGLES2(support::to_integer(f)))
#elif defined NANOVG_GLES3
	raw(nvgCreateGLES3(support::to_integer(f)))
#endif
{
}

void canvas::deleter::operator()(NVGcontext* context) const noexcept
{
#if defined NANOVG_GL2
	nvgDeleteGL2(context);
#elif defined NANOVG_GL3
	nvgDeleteGL3(context);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(context);
#elif defined NANOVG_GLES3
	nvgDeleteGLES3(context);
#endif
}

canvas& canvas::clear(const rgba_vector& color) noexcept
{
	glClearColor(color.r(),color.g(),color.b(),color.a());
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return *this;
}

canvas& canvas::clear(const rgba_pixel& color) noexcept
{
	return clear(rgba_vector(color));
}

frame canvas::begin_frame(float2 size, float pixelRatio) noexcept
{
	return frame(raw.get(), size, pixelRatio);
}

frame::frame(NVGcontext* context, float2 size, float pixelRatio) noexcept :
	size(size),
	pixelRatio(pixelRatio),
	context(context)
{
	nvgBeginFrame(context, size.x(), size.y(), pixelRatio);
}

frame::frame(frame&& other) noexcept :
	size(other.size),
	pixelRatio(other.pixelRatio),
	context(other.context)
{
	other.context = nullptr;
}

frame::~frame() noexcept
{
	if(context)
		nvgEndFrame(context);
}

sketch frame::begin_sketch() noexcept
{
	return sketch(context);
}

sketch::sketch(NVGcontext* context) noexcept :
	context(context)
{
	nvgSave(context);
	nvgBeginPath(context);
}

sketch::sketch(sketch&& other) noexcept
{
	context = other.context;
	other.context = nullptr;
}

sketch::~sketch() noexcept
{
	if(context)
		nvgRestore(context);
}

void ellipse(NVGcontext* context, const float2& center, const float2& radius) noexcept
{
	nvgEllipse(context, center.x(), center.y(), radius.x(), radius.y());
}

void rectangle(NVGcontext* context, const float2& position, const float2& size) noexcept
{
	nvgRect(context, position.x(), position.y(), size.x(), size.y());
}

sketch& sketch::ellipse(const range2f& bounds) noexcept
{
	const auto& radius = (bounds.upper() - bounds.lower())/2;
	const auto& center = bounds.lower() + radius;
	::ellipse(context, center, radius);
	return *this;
}

sketch& sketch::rectangle(const range2f& bounds) noexcept
{
	const auto& position = bounds.lower();
	const auto& size = bounds.upper() - bounds.lower();
	::rectangle(context, position, size);
	return *this;
}

sketch& sketch::line(float2 from, float2 to) noexcept
{
	nvgMoveTo(context, from.x(), from.y());
	nvgLineTo(context, to.x(), to.y());
	return *this;
}

sketch& sketch::fill(const rgba_vector& color) noexcept
{
	nvgFillColor(context, nvgRGBAf(color.r(), color.g(), color.b(), color.a()));
	return fill();
}

sketch& sketch::fill(const rgba_pixel& color) noexcept
{
	return fill(rgba_vector(color));
}

sketch& sketch::fill() noexcept
{
	nvgFill(context);
	return *this;
}

sketch& sketch::line_cap(cap c) noexcept
{
	nvgLineCap(context, support::to_integer(c));
	return *this;
}

sketch& sketch::line_join(join j) noexcept
{
	nvgLineJoin(context, support::to_integer(j));
	return *this;
}

sketch& sketch::line_width(float width) noexcept
{
	nvgStrokeWidth(context, width);
	return *this;
}

sketch& sketch::miter_limit(float limit) noexcept
{
	nvgMiterLimit(context, limit);
	return *this;
}

sketch& sketch::outline(const rgba_vector& color) noexcept
{
	nvgStrokeColor(context, nvgRGBAf(color.r(), color.g(), color.b(), color.a()));
	return outline();
}

sketch& sketch::outline(const rgba_pixel& color) noexcept
{
	return outline(rgba_vector(color));
}

sketch& sketch::outline() noexcept
{
	nvgStroke(context);
	return *this;
}


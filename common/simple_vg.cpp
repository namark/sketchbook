#include "simple_vg.h"
#include "simple/support/enum.hpp"
#include "simple/support/algorithm.hpp"

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

frame canvas::begin_frame(framebuffer& fb) const noexcept
{
	return frame(raw.get(), fb);
}

framebuffer::framebuffer(int2 size, enum flags flags) noexcept :
	flags(flags),
	size(size),
	raw(nullptr)
{}
framebuffer::framebuffer(const canvas& canvas, int2 size, enum flags flags) :
	framebuffer(size, flags)
{
	create(canvas);
}

bool framebuffer::create(const canvas& canvas)
{
	if(raw)
		return false;

	raw = decltype(raw)(
		nvgluCreateFramebuffer(
			canvas.raw.get(),
			size.x(), size.y(),
			support::to_integer(flags)
		)
	);
	assert(raw && "nanovg framebuffer must not be null");
	return true;
}

simple::vg::paint framebuffer::paint(simple::support::range<int2> range, float opacity, float angle) const
{
	auto size = range.upper() - range.lower();
	return nvgImagePattern(nullptr,
		range.lower().x(), range.lower().y(),
		size.x(), size.y(),
		angle, raw->image, opacity
	);
}

simple::vg::paint framebuffer::paint(float opacity, float angle) const
{
	return paint({int2::zero(), size}, opacity, angle);
}

void framebuffer::deleter::operator()(NVGLUframebuffer* raw) const noexcept
{
	nvgluDeleteFramebuffer(raw);
}

paint::paint(NVGpaint raw) noexcept : raw(raw) {}

paint paint::radial_gradient(float2 center, rangef radius, support::range<rgba_vector> color) noexcept
{
	// sneaky nanovg you don't neeed a constext for thiiis
	return paint(nvgRadialGradient(nullptr,
		center.x(), center.y(),
		radius.lower(), radius.upper(),
		nvgRGBAf(color.lower().r(), color.lower().g(), color.lower().b(), color.lower().a()),
		nvgRGBAf(color.upper().r(), color.upper().g(), color.upper().b(), color.upper().a())
	));
}

paint paint::radial_gradient(range2f bounds, rangef radius, support::range<rgba_vector> colors) noexcept
{
	auto size = (bounds.upper() - bounds.lower())/2;
	return radial_gradient(
		bounds.lower() + size,
		radius * size.x(),
		colors
	);
}

frame::frame(NVGcontext* context, float2 size, float pixelRatio) noexcept :
	size(size),
	pixelRatio(pixelRatio),
	buffer(nullptr),
	context(context)
{
	nvgBeginFrame(context, size.x(), size.y(), pixelRatio);
}

frame::frame(NVGcontext* context, const framebuffer& fb) noexcept :
	size(fb.size),
	pixelRatio(1),
	buffer(&fb),
	context(context)
{
	nvgluBindFramebuffer(buffer->raw.get());
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(
		0,0,
		buffer->size.x(), buffer->size.y()
	);
	nvgBeginFrame(context, size.x(), size.y(), pixelRatio);
}

frame::frame(frame&& other) noexcept :
	size(other.size),
	pixelRatio(other.pixelRatio),
	buffer(other.buffer),
	context(other.context)
{
	other.context = nullptr;
}

frame::~frame() noexcept
{
	if(context)
		nvgEndFrame(context);
	if(buffer)
		nvgluBindFramebuffer(nullptr);
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
	move(from);
	vertex(to);
	return *this;
}

sketch& sketch::move(float2 to) noexcept
{
	nvgMoveTo(context, to.x(), to.y());
	return *this;
}

sketch& sketch::vertex(float2 v) noexcept
{
	nvgLineTo(context, v.x(), v.y());
	return *this;
}

sketch& sketch::arc(float2 center, rangef angle, float radius) noexcept
{
	nvgBarc(context, center.x(), center.y(), radius, angle.lower(), angle.upper(), NVG_CW, 0);
	return *this;
}

sketch& sketch::fill(const paint& paint) noexcept
{
	nvgFillPaint(context, paint.raw);
	return fill();
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

sketch& sketch::outline(const paint& paint) noexcept
{
	nvgStrokePaint(context, paint.raw);
	return outline();
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


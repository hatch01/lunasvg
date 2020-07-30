#include "svgrootelement.h"
#include "svgelementtail.h"
#include "svgdocumentimpl.h"

namespace lunasvg {

SVGRootElement::SVGRootElement(SVGDocument* document) :
    SVGSVGElement(document)
{
}

Bitmap SVGRootElement::renderToBitmap(uint32_t width, uint32_t height, double dpi, unsigned int bgColor) const
{
    const SVGDocumentImpl* impl = document()->impl();
    double documentWidth = impl->documentWidth(dpi);
    double documentHeight = impl->documentHeight(dpi);
    if(documentWidth == -1 || documentHeight == -1)
    {
        RenderContext context(RenderModeBounding);
        RenderState &state = context.state();
        state.element = this;
        state.viewPort = Rect(0, 0, std::max(documentWidth, 1.0), std::max(documentHeight, 1.0));
        state.dpi = dpi;
        context.render(this, tail);

        documentWidth = state.bbox.width;
        documentHeight = state.bbox.height;
    }

    if(documentWidth == 0.0 || documentHeight == 0.0 || next == tail)
        return Bitmap();

    std::uint32_t bitmapWidth = width == 0 ? std::uint32_t(std::ceil(documentWidth)) : width;
    std::uint32_t bitmapHeight = height == 0 ? std::uint32_t(std::ceil(documentHeight)) : height;

    Bitmap bitmap(bitmapWidth, bitmapHeight);
    RenderContext context(RenderModeDisplay);
    RenderState& state = context.state();
    state.element = this;
    state.canvas.reset(bitmap.data(), bitmap.width(), bitmap.height(), bitmap.stride());
    state.canvas.clear(bgColor);
    state.viewPort = Rect(0, 0, bitmap.width(), bitmap.height());
    state.dpi = dpi;

    SVGGraphicsElement::render(context);
    RenderState& newState = context.state();
    if(viewBox().isSpecified() && viewBox().property()->isValid())
    {
        newState.matrix.multiply(calculateViewBoxTransform(state.viewPort, viewBox().property()->value()));
        newState.viewPort = viewBox().property()->value();
    }
    else
    {
        newState.matrix.multiply(calculateViewBoxTransform(state.viewPort, Rect(0, 0, documentWidth, documentHeight)));
        newState.viewPort = Rect(0, 0, documentWidth, documentHeight);
    }

    context.render(next, tail);
    state.canvas.convertToRGBA();
    return bitmap;
}

} // namespace lunasvg

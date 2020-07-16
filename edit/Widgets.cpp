#include "SDL_image.h"

#include "harfbuzz/hb-ft.h"
#include "harfbuzz/hb.h"

#include "fmt/format.h"

#include "utf8.h"

#define LAY_IMPLEMENTATION

#include "Widgets.hpp"

using namespace edit;

const int padding = 4;

SDL_Rect rectUnion(const SDL_Rect *lhs, const SDL_Rect *rhs){
	return {
		.x = std::max(lhs->x, rhs->x),
		.y = std::max(lhs->y, rhs->y),
		.w = std::min(lhs->w, rhs->w),
		.h = std::min(lhs->h, rhs->h)
	};
}

SDL_Rect getClipRect(SDL_Renderer *render, const SDL_Rect *bounds) noexcept{
	if(SDL_RenderIsClipEnabled(render)){
		SDL_Rect ret;
		SDL_RenderGetClipRect(render, &ret);
		return ret;
	}
	else{
		return *bounds;
	}
}

void edit::bail(const char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	std::vfprintf(stderr, "Error in SDL_Init: %s\n", va);
	va_end(va);
	std::exit(EXIT_FAILURE);
}

Layout::Layout(const Widget *parent_)
	: Widget(parent_)
{
	if(!parent_)
		throw std::runtime_error("layouts must have a parent widget");

	lay_init_context(&m_ctx);
}

Layout::~Layout(){ lay_destroy_context(&m_ctx); }

void Layout::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	lay_reset_context(&m_ctx);

	lay_reserve_items_capacity(&m_ctx, m_widgets.size());

	lay_id root = lay_item(&m_ctx);

	lay_set_size_xy(&m_ctx, root, bounds->w, bounds->h);

	lay_set_contain(&m_ctx, root, layType());

	m_widgetIds.clear();
	m_widgetIds.reserve(m_widgets.size());

	for(std::size_t i = 0; i < m_widgets.size(); i++){
		lay_id wid = lay_item(&m_ctx);

		lay_insert(&m_ctx, root, wid);

		auto w = m_widgets[i];
		if(!dynamic_cast<const Layout*>(w)){
			if(layType() == LAY_ROW){
				lay_set_margins_ltrb(&m_ctx, wid, i == 0 ? padding * 2 : 0, padding * 2, padding * 2, padding * 2);
			}
			else if(layType() == LAY_COLUMN){
				lay_set_margins_ltrb(&m_ctx, wid, padding * 2, i == 0 ? padding * 2 : 0, padding * 2, padding * 2);
			}
			else{
				lay_set_margins_ltrb(&m_ctx, wid, 0, 0, 0, 0);
			}
		}
		else{
			lay_set_margins_ltrb(&m_ctx, wid, 0, 0, 0, 0);
		}

		lay_set_size_xy(&m_ctx, wid, 0, 0);

		lay_set_behave(&m_ctx, wid, LAY_FILL);

		m_widgetIds.emplace_back(wid);
	}

	lay_run_context(&m_ctx);

	auto clipRect = getClipRect(render, bounds);

	if(!m_widgets.empty()){
		/*
		auto w = m_widgets[0];
		auto id = m_widgetIds[0];

		lay_vec4 rect = lay_get_rect(&m_ctx, id);

		SDL_Rect newBounds;
		newBounds.x = bounds->x + rect[0] + padding;
		newBounds.y = bounds->y + rect[1] + padding;
		newBounds.w = rect[2] - (padding * 2);
		newBounds.h = rect[3] - (padding * 2);

		SDL_Rect newClip = detail::rectUnion(newBounds, clipRect);

		SDL_RenderSetClipRect(render, &newClip);

		w->render(render, &newBounds);
		*/

		for(std::size_t i = 0; i < m_widgets.size(); i++){
			auto w = m_widgets[i];
			auto id = m_widgetIds[i];

			auto rect = lay_get_rect(&m_ctx, id);

			SDL_Rect newBounds;

			newBounds.x = bounds->x + rect[0];
			newBounds.y = bounds->y + rect[1];
			newBounds.w = rect[2];
			newBounds.h = rect[3];

			SDL_Rect newClip = rectUnion(&clipRect, &newBounds);

			SDL_RenderSetClipRect(render, &newClip);

			w->render(render, &newBounds);
		}
	}

	SDL_RenderSetClipRect(render, &clipRect);
}

const Widget *Layout::raycast(const std::uint16_t x, const std::uint16_t y, const SDL_Rect *bounds) const noexcept{
	if(!m_widgetIds.empty()){
		auto w = m_widgets[0];
		auto id = m_widgetIds[0];

		lay_vec4 rect = lay_get_rect(&m_ctx, id);

		SDL_Rect newBounds;
		newBounds.x = bounds->x + rect[0] + padding;
		newBounds.y = bounds->y + rect[1] + padding;
		newBounds.w = rect[2] - (padding * 2);
		newBounds.h = rect[3] - (padding * 2);

		if(auto ret = w->raycast(x, y, &newBounds)) return ret;

		for(std::size_t i = 1; i < m_widgets.size(); i++){
			w = m_widgets[i];
			id = m_widgetIds[i];

			rect = lay_get_rect(&m_ctx, id);

			SDL_Rect newBounds;
			newBounds.x = bounds->x + rect[0];
			newBounds.y = bounds->y + rect[1] + padding;
			newBounds.w = rect[2] - padding;
			newBounds.h = rect[3] - (padding * 2);

			if(auto ret = w->raycast(x, y, &newBounds)) return ret;
		}
	}

	return nullptr;
}

Image::Image(SDL_Renderer *render, const std::filesystem::path &path, const Widget *parent_)
	: Widget(parent_)
{
	auto img = IMG_Load(path.c_str());
	if(!img){
		throw std::runtime_error(fmt::format("Error in IMG_Load: {}", IMG_GetError()));
	}

	m_bounds.x = 0;
	m_bounds.y = 0;
	m_bounds.w = img->w;
	m_bounds.h = img->h;

	m_tex = SDL_CreateTextureFromSurface(render, img);

	SDL_FreeSurface(img);

	if(!m_tex){
		throw std::runtime_error(fmt::format("Error in SDL_CreateTextureFromSurface: {}", SDL_GetError()));
	}
}

void Image::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	SDL_Rect src;
	src.x = bounds->x + m_bounds.x;
	src.y = bounds->y + m_bounds.y;
	src.w = std::min(bounds->w - m_bounds.x, m_bounds.w);
	src.h = std::min(bounds->h - m_bounds.y, m_bounds.h);
	SDL_RenderCopy(render, m_tex, nullptr, &src);
}

void Rect::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	SDL_Rect rect;
	rect.x = bounds->x;
	rect.y = bounds->y;
	rect.w = std::min(m_bounds.w, bounds->w);
	rect.h = std::min(m_bounds.h, bounds->h);

	auto clipRect = getClipRect(render, bounds);

	clipRect = rectUnion(&clipRect, &rect);

	SDL_RenderSetClipRect(render, &clipRect);

	rect.w = m_bounds.w;
	rect.h = m_bounds.h;

	m_inner->render(render, &rect);
}

void SolidColor::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	SDL_Rect dst;
	dst.x = bounds->x + m_rect.x;
	dst.y = bounds->y + m_rect.y;
	dst.w = std::min(m_rect.w, bounds->w - m_rect.x);
	dst.h = std::min(m_rect.h, bounds->h - m_rect.y);

	SDL_SetRenderDrawColor(render, m_color.r, m_color.g, m_color.b, m_color.a);

	SDL_RenderFillRect(render, &dst);
}

void Button::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	SDL_SetRenderDrawColor(render, m_color->r, m_color->g, m_color->b, m_color->a);

	SDL_RenderFillRect(render, bounds);

	auto innerBounds = m_inner->bounds();

	auto centerX = bounds->w / 2;
	auto centerY = bounds->h / 2;

	auto innerXE = innerBounds->x + innerBounds->w;
	auto innerYE = innerBounds->y + innerBounds->h;

	SDL_Rect newBounds;
	newBounds.x = bounds->x + (centerX - (innerXE / 2));
	newBounds.y = bounds->y + (centerY - (innerYE / 2));
	newBounds.w = innerBounds->w;
	newBounds.h = innerBounds->h;

	m_inner->render(render, &newBounds);
}

const Widget *Button::raycast(const std::uint16_t x, const std::uint16_t y, const SDL_Rect *bounds) const noexcept{
	return
			(x >= bounds->x && y >= bounds->y) &&
			(x <= bounds->x + bounds->w) &&
			(y <= bounds->y + bounds->h)
		? this
		: nullptr;
}

Label::Label(const FontFace *face_, const Text &text_, const Widget *parent_) noexcept
	: Widget(parent_), m_face(face_), m_text(text_), m_hbBuf(hb_buffer_create()){}

Label::~Label(){ hb_buffer_destroy(m_hbBuf); }

void Label::update(SDL_Renderer *render, const float dt) noexcept{
	if(m_isDirty){
		if(m_text.str.empty()){
			m_img = nullptr;
			m_isDirty = false;
			return;
		}

		//hb_buffer_reset(m_hbBuf);
		hb_buffer_clear_contents(m_hbBuf);

		//hb_buffer_add_codepoints(m_hbBuf, codepoints.data(), -1, 0, -1);
		hb_buffer_add_utf8(m_hbBuf, m_text.str.c_str(), -1, 0, -1);

		hb_buffer_set_direction(m_hbBuf, HB_DIRECTION_LTR);
		hb_buffer_set_script(m_hbBuf, HB_SCRIPT_LATIN);
		hb_buffer_set_language(m_hbBuf, hb_language_from_string("en", -1));

		//hb_buffer_guess_segment_properties(m_hbBuf);

		hb_shape(m_face->m_hbFont, m_hbBuf, nullptr, 0);

		unsigned int numGlyphs;
		auto glyphInfos = hb_buffer_get_glyph_infos(m_hbBuf, &numGlyphs);
		//auto glyphPositions = hb_buffer_get_glyph_positions(m_hbBuf, &numGlyphs);

		std::vector<const Glyph*> glyphs;
		glyphs.reserve(numGlyphs);

		//auto ftFace = hb_ft_font_get_face(m_face->m_hbFont);

		const unsigned int lineHeight = 24;
		const long lineHeight64 = lineHeight * 64;

		long posX = 0, posY = lineHeight64;
		long maxX = 0, maxY = 0;

		for(unsigned int i = 0; i < numGlyphs; i++){
			auto glyphInfo = glyphInfos[i];
			//auto glyphPos = glyphPositions[i];

			auto cp = glyphInfo.codepoint;

			auto glyph = glyphs.emplace_back(m_face->getGlyph(cp));

			maxX = std::max(maxX, posX + glyph->m_pos.x + (glyph->m_srf->w * 64));
			maxY = std::max(maxY, std::max(posY - (glyph->m_pos.top * 64) + (glyph->m_srf->h * 64), lineHeight64));
			posX += glyph->m_pos.xAdv;
			//posY += glyph->m_pos.yAdv;
		}

		auto finalSrf = SDL_CreateRGBSurfaceWithFormat(0, maxX / 64, maxY / 64, 32, SDL_PIXELFORMAT_RGBA32);

		SDL_SetSurfaceBlendMode(finalSrf, SDL_BLENDMODE_BLEND);

		posX = 0, posY = lineHeight64 - (lineHeight64 / 8);

		for(unsigned int i = 0; i < numGlyphs; i++){
			//const auto glyphInfo = glyphInfos[i];
			//const auto glyphPos = glyphPositions[i];

			const auto glyph = glyphs[i];

			auto xOff = glyph->m_pos.left * 64;
			auto yOff = glyph->m_pos.top * 64;

			SDL_Rect dstRect;
			dstRect.x = (posX + xOff) / 64;
			dstRect.y = (posY - yOff) / 64;
			dstRect.w = glyph->m_srf->w;
			dstRect.h = glyph->m_srf->h;

			//SDL_SetSurfaceColorMod(finalSrf, m_textColor.r, m_textColor.g, m_textColor.b);

			SDL_BlitSurface(glyph->m_srf, nullptr, finalSrf, &dstRect);

			posX += glyph->m_pos.xAdv;
		}

		//auto srf = TTF_RenderUTF8_Blended(m_font, m_text.c_str(), m_textColor);

		auto tex = SDL_CreateTextureFromSurface(render, finalSrf);

		auto img = std::make_unique<Image>(tex, finalSrf->w, finalSrf->h);

		SDL_FreeSurface(finalSrf);

		m_img = std::move(img);

		m_isDirty = false;
	}
}

void Label::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	auto img = m_img.get();
	if(img) img->render(render, bounds);
}

TextEdit::TextEdit(
	const FontFace *face_,
	SDL_Color backgroundColor, SDL_Color textColor_,
	const Widget *parent_
) noexcept
	: Widget(parent_)
	, m_face(face_)
	, m_backgroundColor(backgroundColor)
	, m_textColor(textColor_)
	, m_cursorColor(m_textColor, SDL_Rect{ .x = 0, .y = 0, .w = 2, .h = 16 })
	, m_hbBuf(hb_buffer_create())
{}

void TextEdit::update(SDL_Renderer *render, const float dt) noexcept{
	if(m_timeDirty){
		totalAnimTime = cursorFadeOnTime + cursorOnTime + cursorFadeOffTime + cursorOffTime;
		m_timeDirty = false;
	}

	if(m_textDirty){
		if(m_text.str.empty()){
			m_cursorColor.bounds()->x = 0;
			m_cursorColor.bounds()->y = 0;

			m_img = nullptr;
			m_textDirty = false;
			return;
		}

		//hb_buffer_reset(m_hbBuf);
		hb_buffer_clear_contents(m_hbBuf);

		//hb_buffer_add_codepoints(m_hbBuf, codepoints.data(), -1, 0, -1);
		hb_buffer_add_utf8(m_hbBuf, m_text.str.c_str(), -1, 0, -1);

		hb_buffer_set_direction(m_hbBuf, HB_DIRECTION_LTR);
		hb_buffer_set_script(m_hbBuf, HB_SCRIPT_LATIN);
		hb_buffer_set_language(m_hbBuf, hb_language_from_string("en", -1));

		//hb_buffer_guess_segment_properties(m_hbBuf);

		hb_shape(m_face->m_hbFont, m_hbBuf, nullptr, 0);

		unsigned int numGlyphs;
		auto glyphInfos = hb_buffer_get_glyph_infos(m_hbBuf, &numGlyphs);
		//auto glyphPositions = hb_buffer_get_glyph_positions(m_hbBuf, &numGlyphs);

		std::vector<const Glyph*> glyphs;
		glyphs.reserve(numGlyphs);

		//auto ftFace = hb_ft_font_get_face(m_face->m_hbFont);

		const unsigned int lineHeight = 24;
		const long lineHeight64 = lineHeight * 64;

		long posX = 0, posY = lineHeight64;
		long maxX = 0, maxY = 0;

		for(unsigned int i = 0; i < numGlyphs; i++){
			auto glyphInfo = glyphInfos[i];
			//auto glyphPos = glyphPositions[i];

			auto cp = glyphInfo.codepoint;

			auto glyph = glyphs.emplace_back(m_face->getGlyph(cp));

			maxX = std::max(maxX, posX + glyph->m_pos.x + (glyph->m_srf->w * 64));
			maxY = std::max(maxY, std::max(posY - (glyph->m_pos.top * 64) + (glyph->m_srf->h * 64), lineHeight64));
			posX += glyph->m_pos.xAdv;
			//posY += glyph->m_pos.yAdv;
		}

		auto finalSrf = SDL_CreateRGBSurfaceWithFormat(0, maxX / 64, maxY / 64, 32, SDL_PIXELFORMAT_RGBA32);

		SDL_SetSurfaceBlendMode(finalSrf, SDL_BLENDMODE_BLEND);

		posX = 0, posY = lineHeight64 - (lineHeight64 / 8);

		for(unsigned int i = 0; i < numGlyphs; i++){
			//const auto glyphInfo = glyphInfos[i];
			//const auto glyphPos = glyphPositions[i];

			const auto glyph = glyphs[i];

			auto xOff = glyph->m_pos.left * 64;
			auto yOff = glyph->m_pos.top * 64;

			SDL_Rect dstRect;
			dstRect.x = (posX + xOff) / 64;
			dstRect.y = (posY - yOff) / 64;
			dstRect.w = glyph->m_srf->w;
			dstRect.h = glyph->m_srf->h;

			//SDL_SetSurfaceColorMod(finalSrf, m_textColor.r, m_textColor.g, m_textColor.b);

			SDL_BlitSurface(glyph->m_srf, nullptr, finalSrf, &dstRect);

			posX += glyph->m_pos.xAdv;
		}

		m_cursorColor.bounds()->h = lineHeight;
		m_cursorColor.bounds()->x = posX / 64;

		//auto srf = TTF_RenderUTF8_Blended(m_font, m_text.c_str(), m_textColor);

		auto tex = SDL_CreateTextureFromSurface(render, finalSrf);

		auto img = std::make_unique<Image>(tex, finalSrf->w, finalSrf->h);

		SDL_FreeSurface(finalSrf);

		m_img = std::move(img);

		animTime = cursorFadeOnTime;

		m_textDirty = false;
	}

	animTime += dt;

	float t = 0.f;

	if(animTime < cursorFadeOnTime){
		t = animTime / cursorOnTime;
	}
	else{
		const float onTime = animTime - cursorFadeOnTime;

		if(onTime < cursorOnTime){
			t = 1.f;
		}
		else{
			const float fadeOffTime = onTime - cursorOnTime;

			if(fadeOffTime < cursorFadeOffTime){
				t = 1.f - (fadeOffTime / cursorFadeOffTime);
			}
			else{
				const float offTime = fadeOffTime - cursorFadeOffTime;

				if(offTime < cursorOffTime){
					t = 0.f;
				}
				else{
					animTime = std::fmod(animTime, totalAnimTime);
					update(render, 0.f);
					return;
				}
			}
		}
	}

	auto alpha = std::uint8_t(std::round(t * 255.f));
	m_cursorColor.color()->a = alpha;
}

void TextEdit::render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept{
	SDL_Rect rect;
	rect.x = bounds->x;
	rect.y = bounds->y;
	rect.w = bounds->w;
	rect.h = bounds->h;

	auto clipRect = getClipRect(render, bounds);

	auto newClipRect = rectUnion(&rect, &clipRect);

	SDL_RenderSetClipRect(render, &newClipRect);

	auto &&bkgrnd = m_backgroundColor;

	SDL_SetRenderDrawColor(render, bkgrnd.r, bkgrnd.g, bkgrnd.b, bkgrnd.a);

	SDL_RenderFillRect(render, &rect);

	auto img = m_img.get();
	if(img) img->render(render, &rect);

	m_cursorColor.render(render, &rect);

	SDL_RenderSetClipRect(render, &clipRect);
}

Glyph::Glyph(FT_GlyphSlot glyph){
	FT_Render_Glyph(glyph, FT_RENDER_MODE_LCD);

	auto srf = SDL_CreateRGBSurfaceWithFormat(0, glyph->bitmap.width, glyph->bitmap.rows, 32, SDL_PIXELFORMAT_RGBA32);

	SDL_SetSurfaceBlendMode(srf, SDL_BLENDMODE_BLEND);

	SDL_SetSurfaceRLE(srf, 1);

	SDL_LockSurface(srf);

	for(unsigned int y = 0; y < glyph->bitmap.rows; y++){
		auto glyphRowIdx = y * glyph->bitmap.pitch;
		auto srfRowIdx = y * srf->pitch;

		auto srfRowData = reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(srf->pixels) + srfRowIdx);

		for(unsigned int x = 0; x < glyph->bitmap.width; x++){
			std::uint8_t val = glyph->bitmap.buffer[glyphRowIdx + x];
			srfRowData[x] = SDL_MapRGBA(srf->format, 255, 255, 255, val);
		}
	}

	SDL_UnlockSurface(srf);

	m_srf = srf;

	m_pos.x = glyph->metrics.horiBearingX;
	m_pos.y = glyph->metrics.horiBearingY;
	m_pos.xAdv = glyph->metrics.horiAdvance;
	m_pos.yAdv = glyph->metrics.vertAdvance;
	m_pos.top = glyph->bitmap_top;
	m_pos.left = glyph->bitmap_left;
}

Glyph::~Glyph(){
	if(m_srf) SDL_FreeSurface(m_srf);
}

const Glyph *FontFace::getGlyph(const std::uint32_t glyphIdx) const{
	auto res = m_glyphs.find(glyphIdx);
	if(res != end(m_glyphs)) return &res->second;

	auto ftFace = hb_ft_font_get_face(m_hbFont);

	FT_Load_Glyph(ftFace, glyphIdx, FT_LOAD_RENDER);

	auto glyph = ftFace->glyph;

	auto emplRes = m_glyphs.try_emplace(glyphIdx, Glyph(glyph));
	if(!emplRes.second) return nullptr;

	return &emplRes.first->second;
}

template<typename Cont, typename Val>
auto sortedInsert(Cont &c, Val &&val){
	auto it = std::upper_bound(cbegin(c), cend(c), val);
	c.insert(it, std::forward<Val>(val));
}

EditWindow::EditWindow(const std::uint16_t w, const std::uint16_t h, const FontFace *textFont) noexcept
	: Widget(nullptr)
	, m_w(w), m_h(h), m_textFont(textFont)
	, m_layout(this)
	, m_btnLayout(&m_layout)
	, m_background(SDL_Color{ 60, 60, 60, 255 }, SDL_Rect{ .x = 0, .y = 0, .w = w, .h = h })
	, m_textEdit(m_textFont, SDL_Color{ 40, 40, 40, 255 }, SDL_Color{ 200, 200, 200, 255 }, &m_layout)
	, m_newLabel(m_textFont, Text{ "New", SDL_Color{ 255, 255, 255, 255 } }, &m_newBtn)
	, m_runLabel(m_textFont, Text{ "Run", SDL_Color{ 255, 255, 255, 255 } }, &m_runBtn)
	, m_compileLabel(m_textFont, Text{ "Compile", SDL_Color{ 255, 255, 255, 255 } }, &m_compileBtn)
	, m_newBtn(&m_newLabel, []{ fmt::print(stderr, "PRESSED 'NEW'\n"); }, &m_btnLayout)
	, m_runBtn(&m_runLabel, []{ fmt::print(stderr, "PRESSED 'RUN'\n"); }, &m_btnLayout)
	, m_compileBtn(&m_compileLabel, []{ fmt::print(stderr, "PRESSED 'COMPILE'\n"); }, &m_btnLayout)
{
	m_window = SDL_CreateWindow(
		"Lemni Edit - Untitled",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		w, h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
	);
	if(!m_window){
		bail("Error in SDL_CreateWindow: %s\n", SDL_GetError());
	}

	m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
	if(!m_render){
		SDL_DestroyWindow(m_window);
		bail("Error in SDL_CreateRenderer: %s\n", SDL_GetError());
	}

	SDL_SetRenderDrawBlendMode(m_render, SDL_BLENDMODE_BLEND);

	SDL_RenderSetLogicalSize(m_render, m_w, m_h);

	m_btnLayout.addWidget(&m_newBtn);
	m_btnLayout.addWidget(&m_runBtn);
	m_btnLayout.addWidget(&m_compileBtn);

	m_btns.reserve(3);
	sortedInsert(m_btns, &m_newBtn);
	sortedInsert(m_btns, &m_runBtn);
	sortedInsert(m_btns, &m_compileBtn);

	m_layout.addWidget(&m_textEdit);
	m_layout.addWidget(&m_btnLayout);
}

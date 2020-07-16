#ifndef LEMNI_EDIT_WIDGETS_HPP
#define LEMNI_EDIT_WIDGETS_HPP 1

#include <cstdint>

#include <memory>
#include <vector>
#include <filesystem>
#include <functional>
#include <map>

#include "SDL.h"
#include "SDL_ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "harfbuzz/hb-ft.h"

#include "utf8.h"

#include "layout.h"

namespace edit{
	[[noreturn]]
	void bail(const char *fmt, ...);

	namespace detail{
		inline SDL_Rect rectUnion(const SDL_Rect &lhs, const SDL_Rect &rhs) noexcept{
			return {
				.x = std::max(lhs.x, rhs.x),
				.y = std::max(lhs.y, rhs.y),
				.w = std::min(lhs.w, rhs.w),
				.h = std::min(lhs.h, rhs.h)
			};
		}
	}

	struct Style{
		SDL_Color backgroundColor;
		SDL_Color buttonColor;
		SDL_Color buttonHoverColor;
	};

	class Widget{
		public:
			explicit Widget(const Widget *parent_ = nullptr) noexcept
				: m_parent(parent_){}

			virtual ~Widget() = default;

			const Widget *parent() const noexcept{ return m_parent; }

			virtual void onEnter(SDL_Renderer *render, const std::uint16_t xRel, const std::uint16_t yRel) noexcept{
				(void)render; (void)xRel; (void)yRel;
			}

			virtual void onLeave(SDL_Renderer *render, const std::uint16_t xRel, const std::uint16_t yRel) noexcept{
				(void)render; (void)xRel; (void)yRel;
			}

			virtual void onPress(SDL_Renderer *render, const std::uint8_t btn, const std::uint16_t xRel, const std::uint16_t yRel) noexcept{
				(void)render; (void)btn; (void)xRel; (void)yRel;
			}

			virtual void onRelease(SDL_Renderer *render, const std::uint8_t btn, const std::uint16_t xRel, const std::uint16_t yRel) noexcept{
				(void)render; (void)btn; (void)xRel; (void)yRel;
			}

			virtual void update(SDL_Renderer *render, const float dt) noexcept{ (void)render; (void)dt; }
			virtual void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept = 0;

			virtual const Widget *raycast(const std::uint16_t x, const std::uint16_t y, const SDL_Rect *bounds) const noexcept{
				auto b = this->bounds();
				if(!b) return nullptr;

				auto testRect = detail::rectUnion(*bounds, *b);

				return
						(x >= testRect.x && y >= testRect.y) &&
						(x <= testRect.x + testRect.w) &&
						(y <= testRect.y + testRect.h)
					? this
					: nullptr;
			}

			virtual const SDL_Rect *bounds() const noexcept{
				if(m_parent) return m_parent->bounds();
				else return nullptr;
			}

		private:
			const Widget *m_parent;
	};

	class Layout: public Widget{
		public:
			Layout(const Widget *parent_);

			virtual ~Layout() override;

			void addWidget(const Widget *w) noexcept{
				m_widgets.emplace_back(w);
			}

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;

			const SDL_Rect *bounds() const noexcept override{ return parent()->bounds(); }

			const Widget *raycast(const std::uint16_t x, const std::uint16_t y, const SDL_Rect *bounds) const noexcept override;

		protected:
			std::vector<const Widget*> m_widgets;

			mutable lay_context m_ctx;
			mutable std::vector<lay_id> m_widgetIds;

			virtual lay_box_flags layType() const noexcept = 0;
	};

	class XLayout: public Layout{
		public:
			using Layout::Layout;

		protected:
			lay_box_flags layType() const noexcept override{ return LAY_ROW; }
	};

	class YLayout: public Layout{
		public:
			using Layout::Layout;

		private:
			lay_box_flags layType() const noexcept override{ return LAY_COLUMN; }
	};

	class Image: public Widget{
		public:
			Image(SDL_Texture *tex, int w, int h, const Widget *parent_ = nullptr) noexcept
				: Widget(parent_), m_tex(tex), m_bounds{ .x = 0, .y = 0, .w = w, .h = h }{}

			Image(SDL_Renderer *render, const std::filesystem::path &path, const Widget *parent_ = nullptr);

			~Image(){
				if(m_tex) SDL_DestroyTexture(m_tex);
			}

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;
			const SDL_Rect *bounds() const noexcept override{ return &m_bounds; }

		private:
			SDL_Texture *m_tex;
			SDL_Rect m_bounds;
	};

	class Rect: public Widget{
		public:
			Rect(std::uint16_t w_, std::uint16_t h_, const Widget *inner_, const Widget *parent_ = nullptr) noexcept
				: Widget(parent_), m_bounds{ .x = 0, .y = 0, .w = w_, .h = h_ }, m_inner(inner_){}

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;
			const SDL_Rect *bounds() const noexcept override{ return &m_bounds; }

		private:
			SDL_Rect m_bounds;
			const Widget *m_inner;
	};

	class SolidColor: public Widget{
		public:
			SolidColor(SDL_Color color_, SDL_Rect rect_, const Widget *parent_ = nullptr)
				: Widget(parent_), m_color(color_), m_rect(rect_){}

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;

			const SDL_Rect *bounds() const noexcept override{ return &m_rect; }

			SDL_Rect *bounds() noexcept{ return &m_rect; }

			const SDL_Color *color() const noexcept{ return &m_color; }

			SDL_Color *color() noexcept{ return &m_color; }

		private:
			SDL_Color m_color;
			SDL_Rect m_rect;
	};

	class Button: public Widget{
		public:
			Button(const Widget *inner_, std::function<void()> onPress_, const Widget *parent_) noexcept
				: Widget(parent_), m_inner(inner_)
				, m_color(regularColor()), m_lastColor(m_color)
				, m_onPress(std::move(onPress_)){}

			const Widget *inner() noexcept{ return m_inner; }

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;

			const Widget *raycast(const std::uint16_t x, const std::uint16_t y, const SDL_Rect *bounds) const noexcept override;

			void onEnter(SDL_Renderer *render, const std::uint16_t x, const std::uint16_t y) noexcept override{
				(void)render; (void)x; (void)y;
				if(m_color == regularColor()){
					auto lastColor = m_color;
					m_color = hoverColor();
					m_lastColor = lastColor;
				}
			}

			void onLeave(SDL_Renderer *render, const std::uint16_t x, const std::uint16_t y) noexcept override{
				(void)render; (void)x; (void)y;
				if(m_color == hoverColor()){
					auto lastColor = m_color;
					m_color = regularColor();
					m_lastColor = lastColor;
				}
			}

			void onPress(SDL_Renderer *render, const std::uint8_t btn, const std::uint16_t x, const std::uint16_t y) noexcept override{
				(void)render; (void)btn; (void)x; (void)y;
				auto lastColor = m_color;
				m_color = pressColor();
				m_lastColor = lastColor;

				m_onPress();
			}

			void onRelease(SDL_Renderer *render, const std::uint8_t btn, const std::uint16_t x, const std::uint16_t y) noexcept override{
				(void)render; (void)btn; (void)x; (void)y;
				auto lastColor = m_color;
				m_color = m_lastColor;
				m_lastColor = lastColor;
			}

		private:
			const Widget *m_inner;
			const SDL_Color *m_color;
			const SDL_Color *m_lastColor;
			std::function<void()> m_onPress;

			static const SDL_Color *regularColor(){
				static const SDL_Color ret{
					40, 40, 40, 255
				};
				return &ret;
			}

			static const SDL_Color *hoverColor(){
				static const SDL_Color ret{
					50, 50, 50, 255
				};
				return &ret;
			}

			static const SDL_Color *pressColor(){
				static const SDL_Color ret{
					35, 35, 35, 255
				};
				return &ret;
			}
	};

	class Glyph{
		public:
			Glyph(Glyph &&other) noexcept
				: m_srf(other.m_srf)
				, m_pos(other.m_pos)
			{
				other.m_srf = nullptr;
			}

			~Glyph();

		private:
			Glyph(FT_GlyphSlot glyph);

			SDL_Surface *m_srf;

			struct GlyphPos{
				int x, y;
				int xAdv, yAdv;
				int top, left;
			} m_pos;

			friend class FontFace;
			friend class TextEdit;
			friend class Label;
	};

	class FontFace{
		public:
			FontFace(FontFace &&other) noexcept
				: m_hbFont(other.m_hbFont)
			{
				other.m_hbFont = nullptr;
			}

			~FontFace(){
				if(m_hbFont){
					FT_Done_Face(hb_ft_font_get_face(m_hbFont));
					hb_font_destroy(m_hbFont);
				}
			}

			FontFace &operator=(FontFace &&other) noexcept{
				if(m_hbFont){
					FT_Done_Face(hb_ft_font_get_face(m_hbFont));
					hb_font_destroy(m_hbFont);
				}

				m_hbFont = other.m_hbFont;
				other.m_hbFont = nullptr;

				return *this;
			}

			std::string_view styleName() const noexcept{
				return hb_ft_font_get_face(m_hbFont)->style_name;
			}

			std::string_view familyName() const noexcept{
				return hb_ft_font_get_face(m_hbFont)->family_name;
			}

			hb_font_t *hbFont() const noexcept{ return m_hbFont; }

			std::uint32_t getGlyphIdx(const std::uint32_t codepoint) const{
				return FT_Get_Char_Index(hb_ft_font_get_face(m_hbFont), codepoint);
			}

			const Glyph *getGlyph(const std::uint32_t glyphIdx) const;

		private:
			explicit FontFace(FT_Face face) noexcept
				: m_hbFont(hb_ft_font_create_referenced(face)){}

			mutable hb_font_t *m_hbFont;

			mutable std::map<std::uint32_t, Glyph> m_glyphs;

			friend class Font;
			friend class Label;
			friend class TextEdit;
	};

	class Font{
		public:
			Font(FT_Library ft, const std::filesystem::path &path, const int ptSize = 14) noexcept{
				if(FT_New_Face(ft, path.c_str(), -1, &m_face) != 0){
					bail("Error in FT_New_Face");
				}

				float vdpi, hdpi;
				SDL_GetDisplayDPI(0, nullptr, &hdpi, &vdpi);

				FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);

				auto numFaces = m_face->num_faces;

				for(long i = 0; i < numFaces; i++){
					FT_Face face;
					FT_New_Face(ft, path.c_str(), i, &face);

					FT_Select_Charmap(face, FT_ENCODING_UNICODE);
					FT_Set_Char_Size(face, ptSize * 64, 0, FT_UInt(hdpi), FT_UInt(vdpi));

					auto fontFace = FontFace(face);
					m_faces.emplace_back(std::move(fontFace));
				}
			}

			Font(Font &&other) noexcept
				: m_face(other.m_face)
				, m_faces(std::move(other.m_faces))
			{
				other.m_face = nullptr;
			}

			~Font(){
				if(m_face) FT_Done_Face(m_face);
			}

			std::size_t numFaces() const noexcept{ return m_face->num_faces; }

			const FontFace *faces() const noexcept{ return m_faces.data(); }

		private:
			FT_Face m_face;
			std::vector<FontFace> m_faces;
	};

	struct Text{
		std::string str;
		SDL_Color color;
	};

	using TextBuffer = std::vector<Text>;

	class Label: public Widget{
		public:
			Label(const FontFace *face_, const Text &text_, const Widget *parent_ = nullptr) noexcept;

			~Label() override;

			const Text &text() const noexcept{ return m_text; }

			void setText(const Text &text_) noexcept{
				m_text = text_;
				m_isDirty = true;
			}

			void setStr(std::string str) noexcept{
				m_text.str = std::move(str);
				m_isDirty = true;
			}

			const SDL_Rect *bounds() const noexcept override{
				if(m_img) return m_img->bounds();
				else return Widget::bounds();
			}

			void update(SDL_Renderer *render, const float dt) noexcept override;

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;

		private:
			const FontFace *m_face;
			Text m_text;
			std::unique_ptr<Image> m_img;
			hb_buffer_t *m_hbBuf;
			bool m_isDirty = true;
	};

	class TextEdit: public Widget{
		public:
			TextEdit(
				const FontFace *face_, SDL_Color backgroundColor, SDL_Color textColor_,
				const Widget *parent_ = nullptr
			) noexcept;

			~TextEdit(){}

			std::string_view contents() const noexcept{ return m_text.str; }
			const Image *image() const noexcept{ return m_img.get(); }

			void setText(std::string text){
				m_text.str = std::move(text);
				m_textDirty = true;
			}

			void appendText(std::string_view text){
				m_text.str.insert(end(m_text.str), begin(text), end(text));
				m_textDirty = true;
			}

			void backspace(){
				if(m_text.str.empty()) return;

				auto it = end(m_text.str);
				auto start = begin(m_text.str);

				utf8::prior(it, start);

				m_text.str.erase(it, end(m_text.str));

				m_textDirty = true;
			}

			const SDL_Rect *bounds() const noexcept override{
				if(m_img) return m_img->bounds();
				else return Widget::bounds();
			}

			void update(SDL_Renderer *render, const float dt) noexcept override;

			void render(SDL_Renderer *render, const SDL_Rect *bounds) const noexcept override;

		private:
			const FontFace *m_face;
			SDL_Color m_backgroundColor, m_textColor;
			SolidColor m_cursorColor;
			hb_buffer_t *m_hbBuf;
			Text m_text;
			std::unique_ptr<Image> m_img;
			std::map<std::uint32_t, FT_GlyphSlot> m_glyphs;
			float animTime = 0.f;
			float cursorOnTime = 0.66f;
			float cursorOffTime = 0.66f;
			float cursorFadeOnTime = 0.05f;
			float cursorFadeOffTime = 0.1f;
			float totalAnimTime = 0.f;
			bool m_timeDirty = true;
			bool m_textDirty = false;
	};

	class EditWindow: public Widget{
		public:
			EditWindow(const std::uint16_t w, const std::uint16_t h, const FontFace *textFont) noexcept;

			~EditWindow(){
				SDL_DestroyRenderer(m_render);
				SDL_DestroyWindow(m_window);
			}

			auto sdl2Handle() noexcept{ return m_window; }

			void update(SDL_Renderer*, const float dt) noexcept override{
				m_textEdit.update(m_render, dt);
				m_newLabel.update(m_render, dt);
				m_runLabel.update(m_render, dt);
				m_compileLabel.update(m_render, dt);
				m_newBtn.update(m_render, dt);
				m_runBtn.update(m_render, dt);
				m_compileBtn.update(m_render, dt);
			}

			void render(SDL_Renderer*, const SDL_Rect*) const noexcept override{
				auto clearColor = m_background.color();
				SDL_SetRenderDrawColor(m_render, clearColor->r, clearColor->g, clearColor->b, clearColor->a);
				SDL_RenderClear(m_render);

				m_layout.render(m_render, m_background.bounds());

				SDL_RenderPresent(m_render);
			}

			void present() const noexcept{
				return render(nullptr, nullptr);
			}

			void onTextEntry(std::string_view text){
				m_textEdit.appendText(text);
			}

			void onBackspace(){
				m_textEdit.backspace();
			}

			void doMotion(const std::uint16_t x, const std::uint16_t y) noexcept{
				auto w = m_layout.raycast(x, y, bounds());
				if(w != m_hoveredWidget){
					if(m_hoveredWidget) m_hoveredWidget->onLeave(m_render, x, y);

					auto it = std::find(cbegin(m_btns), cend(m_btns), w);
					if(it != end(m_btns)){
						m_hoveredWidget = *it;
						(*it)->onEnter(m_render, x, y);
					}
				}
			}

			void doPress(const std::uint8_t btn, const std::uint16_t x, const std::uint16_t y) noexcept{
				auto w = m_layout.raycast(x, y, bounds());

				auto it = std::find(cbegin(m_btns), cend(m_btns), w);
				if(it != end(m_btns)){
					m_pressedWidgets[btn].emplace_back(*it);
					(*it)->onPress(m_render, btn, x, y);
				}
			}

			void doRelease(const std::uint8_t btn, const std::uint16_t x, const std::uint16_t y) noexcept{
				auto &&widgets = m_pressedWidgets[btn];
				if(widgets.empty()) return;

				for(auto &&w : widgets){
					w->onRelease(m_render, btn, x, y);
				}

				widgets.clear();
			}

			std::uint16_t width() const noexcept{ return m_w; }
			std::uint16_t height() const noexcept{ return m_h; }

			const SDL_Rect *bounds() const noexcept override{ return m_background.bounds(); }

		private:
			std::uint16_t m_w, m_h;
			const FontFace *m_textFont;
			XLayout m_layout;
			YLayout m_btnLayout;
			SolidColor m_background;
			TextEdit m_textEdit;
			Label m_newLabel, m_runLabel, m_compileLabel;
			Button m_newBtn, m_runBtn, m_compileBtn;
			SDL_Window *m_window;
			SDL_Renderer *m_render;
			std::vector<Widget*> m_btns;
			std::map<std::uint8_t, std::vector<Widget*>> m_pressedWidgets;
			Widget *m_hoveredWidget = nullptr;
	};
}

#endif // !LEMNI_EDIT_WIDGETS_HPP

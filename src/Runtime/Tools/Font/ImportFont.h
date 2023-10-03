
#pragma once
#ifndef _IMPORTFONT_
#define _IMPORTFONT_

#include <cstddef>
#include "msdfgen/core/Shape.h"
#include "freetype/freetype.h"


namespace msdfgen {

typedef unsigned char byte;
typedef unsigned unicode_t;

class FreetypeHandle;
class FontHandle;

class GlyphIndex {

public:
    explicit GlyphIndex(unsigned index = 0);
    unsigned getIndex() const;

private:
    unsigned index;

};

/// Global metrics of a typeface (in font units).
struct FontMetrics {
    /// The size of one EM.
    double emSize;
    /// The vertical position of the ascender and descender relative to the baseline.
    double ascenderY, descenderY;
    /// The vertical difference between consecutive baselines.
    double lineHeight;
    /// The vertical position and thickness of the underline.
    double underlineY, underlineThickness;
};

/// A structure to model a given axis of a variable font.
struct FontVariationAxis {
    /// The name of the variation axis.
    const char *name;
    /// The axis's minimum coordinate value.
    double minValue;
    /// The axis's maximum coordinate value.
    double maxValue;
    /// The axis's default coordinate value. FreeType computes meaningful default values for Adobe MM fonts.
    double defaultValue;
};




/// Creates a FontHandle from FT_Face that was loaded by the user. destroyFont must still be called but will not affect the FT_Face.
FontHandle * adoptFreetypeFont(FT_Face ftFace);
/// Converts the geometry of FreeType's FT_Outline to a Shape object.
FT_Error readFreetypeOutline(Shape &output, FT_Outline *outline);






class FreetypeHandle {
public:
	static FreetypeHandle* initializeFreetype();
	static void deinitializeFreetype(FreetypeHandle* library);
	static bool setFontVariationAxis(FreetypeHandle* library, FontHandle* font, const char* name, double coordinate);
	static bool listFontVariationAxes(std::vector<FontVariationAxis>& axes, FreetypeHandle* library, FontHandle* font);

	FT_Library library;

};

class FontHandle {
public:

	static FontHandle* adoptFreetypeFont(FT_Face ftFace);
	static FontHandle* loadFont(FreetypeHandle* library, const char* filename);
	static FontHandle* loadFontData(FreetypeHandle* library, const byte* data, int length);
	static void destroyFont(FontHandle* font);
	static bool getFontMetrics(FontMetrics& metrics, FontHandle* font);
	static bool getFontWhitespaceWidth(double& spaceAdvance, double& tabAdvance, FontHandle* font);
	static bool getGlyphIndex(GlyphIndex& glyphIndex, FontHandle* font, unicode_t unicode);
	static bool loadGlyph(Shape& output, FontHandle* font, GlyphIndex glyphIndex, double* advance = nullptr);
	static bool loadGlyph(Shape& output, FontHandle* font, unicode_t unicode, double* advance = nullptr);
	static bool getKerning(double& output, FontHandle* font, GlyphIndex glyphIndex1, GlyphIndex glyphIndex2);
	static bool getKerning(double& output, FontHandle* font, unicode_t unicode1, unicode_t unicode2);


	FT_Face face;
	bool ownership;

};

struct FtContext {
	Point2 position;
	Shape* shape;
	Contour* contour;
};

}

#endif
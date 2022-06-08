
namespace MXRender
{
    enum class ENUM_TEXTURE_TYPE
    {
        ENUM_TYPE_NOT_VALID=0,
        ENUM_TYPE_2D,
        ENUM_TYPE_CUBE_MAP,
        ENUM_TYPE_2D_DYNAMIC,
    };

    enum class ENUM_FRAMEBUFFER_TYPE
    {
        ENUM_TYPE_INVALID=0,
        ENUM_TYPE_BASIC,
        ENUM_TYPE_RGBF1_DEPTH,
        ENUM_TYPE_RGBF2_DEPTH,
        ENUM_TYPE_RGBF3_DEPTH,
        ENUM_TYPE_MSAA,
        ENUM_TYPE_COLOR,
        ENUM_TYPE_RED,
        ENUM_TYPE_COLOR_FLOAT,
        ENUM_TYPE_DEPTH,
        ENUM_TYPE_CUBE_DEPTH,
        ENUM_TYPE_GBUFFER,
        ENUM_TYPE_RAYTRACING,
        ENUM_TYPE_RTX,
        ENUM_TYPE_DYNAMIC_COLOR,
    };
    enum class ENUM_PASS_TYPE {
        ENUM_TYPE_INVALID=0,
        ENUM_PASS_COLOR,
        ENUM_PASS_DEPTH,
    };
} // namespace name

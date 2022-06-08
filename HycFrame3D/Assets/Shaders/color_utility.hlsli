static const float3x3 sRGB_2_AP0_MAT = {
    0.4397010f, 0.0897923f, 0.0175440f,
    0.3829780f, 0.8134230f, 0.1115440f,
    0.1773350f, 0.0967616f, 0.8707040f
};

static const float3x3 AP0_2_AP1_MAT = {
    1.4514393161f, -0.0765537734f, 0.0083161484f,
    -0.2365107469f, 1.1762296998f, -0.0060324498f,
    -0.2149285693f, -0.0996759264f, 0.9977163014f
};

static const float3x3 AP1_2_AP0_MAT = {
    0.6954522414f, 0.0447945634f, -0.0055258826f,
    0.1406786965f, 0.8596711185f, 0.0040252103f,
    0.1638690622f, 0.0955343182f, 1.0015006723f
};

static const float3x3 AP1_2_XYZ_MAT = {
    0.6624541811f, 0.2722287168f, -0.0055746495f,
    0.1340042065f, 0.6740817658f, 0.0040607335f,
    0.1561876870f, 0.0536895174f, 1.0103391003f
};

static const float3x3 XYZ_2_AP1_MAT = {
    1.6410233797f, -0.6636628587f, 0.0117218943f,
    -0.3248032942f, 1.6153315917f, -0.0082844420f,
    -0.2364246952f, 0.0167563477f, 0.9883948585f
};

static const float3x3 XYZ_2_REC709_MAT = {
    3.2409699419f, -0.9692436363f, 0.0556300797f,
    -1.5373831776f, 1.8759675015f, -0.2039769589f,
    -0.4986107603f, 0.0415550574f, 1.0569715142f
};

static const float3x3 RRT_SAT_MAT = {
    0.9708890f, 0.0108892f, 0.0108892f,
    0.0269633f, 0.9869630f, 0.0269633f,
    0.00214758f, 0.00214758f, 0.96214800f
};

static const float3x3 ODT_SAT_MAT = {
    0.949056f, 0.019056f, 0.019056f,
    0.0471857f, 0.9771860f, 0.0471857f,
    0.00375827f, 0.00375827f, 0.93375800f
};

static const float3x3 D60_2_D65_CAT = {
    0.98722400f, -0.00759836f, 0.00307257f,
    -0.00611327f, 1.00186000f, -0.00509595f,
    0.0159533f, 0.0053302f, 1.0816800f
};

float log10(float x)
{
    return log2(x) / log2(10.f);
}

float3 sRGBToACES(float3 srgb)
{
    return mul(sRGB_2_AP0_MAT, srgb);
}

float min_f3(float3 a)
{
    return min(a.x, min(a.y, a.z));
}

float max_f3(float3 a)
{
    return max(a.x, max(a.y, a.z));
}

float rgb_2_saturation(float3 rgb)
{
    const float TINY = 1e-10;
    float mi = min_f3(rgb);
    float ma = max_f3(rgb);
    return (max(ma, TINY) - max(mi, TINY)) / max(ma, 1e-2);
}

float rgb_2_yc(float3 rgb)
{
    const float ycRadiusWeight = 1.75f;
    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;
    float chroma = sqrt(b * (b - g) + g * (g - r) + r * (r - b));
    return (b + g + r + ycRadiusWeight * chroma) / 3.f;
}

float sigmoid_shaper(float x)
{
    float t = max(1.f - abs(x / 2.f), 0.f);
    float y = 1.f + (float)sign(x) * (1.f - t * t);
    return y / 2.f;
}

float glow_fwd(float ycIn, float glowGainIn, float glowMid)
{
    float glowGainOut;

    if (ycIn <= 2.f / 3.f * glowMid)
    {
        glowGainOut = glowGainIn;
    }
    else if (ycIn >= 2.f * glowMid)
    {
        glowGainOut = 0.f;
    }
    else
    {
        glowGainOut = glowGainIn * (glowMid / ycIn - 1.f / 2.f);
    }

    return glowGainOut;
}

#define PI (3.14159265358979323846f)

float rgb_2_hue(float3 rgb)
{
    float hue;
    if (rgb.r == rgb.g && rgb.g == rgb.b)
    {
        hue = 0.0;
    }
    else
    {
        hue = (180.f / PI) * atan2(sqrt(3.f) * (rgb.g - rgb.b),
            2.f * rgb.r - rgb.g - rgb.b);
    }
    if (hue < 0.f) { hue += 360.f; }
    return hue;
}

float center_hue(float hue, float centerH)
{
    float hueCentered = hue - centerH;
    if (hueCentered < -180.f) { hueCentered = hueCentered + 360.f; }
    else if (hueCentered > 180.f) { hueCentered = hueCentered - 360.f; }
    return hueCentered;
}

float cubic_basis_shaper(float x, float w)
{
    float M[4][4] =
    {
        { -1.f / 6.f, 3.f / 6.f, -3.0 / 6.f, 1.f / 6.f },
        { 3.f / 6.f, -6.f / 6.f, 3.0 / 6.f, 0.f / 6.f },
        { -3.f / 6.f, 0.f / 6.f, 3.0 / 6.f, 0.f / 6.f },
        { 1.f / 6.f, 4.f / 6.f, 1.0 / 6.f, 0.f / 6.f }
    };

    float knots[5] =
    {
        -w / 2.f,
        -w / 4.f,
        0.f,
        w / 4.f,
        w / 2.f
    };

    float y = 0.f;
    if ((x > knots[0]) && (x < knots[4]))
    {
        float knot_coord = (x - knots[0]) * 4.f / w;
        int j = int(knot_coord);
        float t = knot_coord - j;

        float monomials[4] = { t * t * t, t * t, t, 1.f };

        if (j == 3)
        {
            y = monomials[0] * M[0][0] + monomials[1] * M[1][0] +
                monomials[2] * M[2][0] + monomials[3] * M[3][0];
        }
        else if (j == 2)
        {
            y = monomials[0] * M[0][1] + monomials[1] * M[1][1] +
                monomials[2] * M[2][1] + monomials[3] * M[3][1];
        }
        else if (j == 1)
        {
            y = monomials[0] * M[0][2] + monomials[1] * M[1][2] +
                monomials[2] * M[2][2] + monomials[3] * M[3][2];
        }
        else if (j == 0)
        {
            y = monomials[0] * M[0][3] + monomials[1] * M[1][3] +
                monomials[2] * M[2][3] + monomials[3] * M[3][3];
        }
        else
        {
            y = 0.f;
        }
    }

    return y * 3.f / 2.f;
}

static const float3x3 M = {
    0.5f, -1.f, 0.5f,
    -1.f, 1.f, 0.5f,
    0.5f, 0.f, 0.f
};

float segmented_spline_c5_fwd(float x)
{
    const float coefsLow[6] = { -4.0000000000f, -4.0000000000f, -3.1573765773f, -0.4852499958f, 1.8477324706f, 1.8477324706f };
    const float coefsHigh[6] = { -0.7185482425f, 2.0810307172f, 3.6681241237f, 4.0000000000f, 4.0000000000f, 4.0000000000f };
    const float2 minPoint = float2(0.18f * exp2(-15.f), 0.0001f);
    const float2 midPoint = float2(0.18f, 0.48f);
    const float2 maxPoint = float2(0.18f * exp2(18.f), 10000.f);
    const float slopeLow = 0.0;
    const float slopeHigh = 0.0;

    const int N_KNOTS_LOW = 4;
    const int N_KNOTS_HIGH = 4;

    float xCheck = x;
    if (xCheck <= 0.f) { xCheck = 0.00006103515f; } // = pow(2.0, -14.0)

    float logx = log10(xCheck);
    float logy;

    if (logx <= log10(minPoint.x))
    {
        logy = logx * slopeLow + (log10(minPoint.y) - slopeLow * log10(minPoint.x));
    }
    else if ((logx > log10(minPoint.x)) && (logx < log10(midPoint.x)))
    {
        float knot_coord = (float)(N_KNOTS_LOW - 1) * (logx - log10(minPoint.x)) / (log10(midPoint.x) - log10(minPoint.x));
        int j = int(knot_coord);
        float t = knot_coord - (float)j;

        float3 cf = float3(coefsLow[j], coefsLow[j + 1], coefsLow[j + 2]);
        float3 monomials = float3(t * t, t, 1.f);
        logy = dot(monomials, mul(M, cf));
    }
    else if((logx >= log10(midPoint.x)) && (logx < log10(maxPoint.x)))
    {
        float knot_coord = (float)(N_KNOTS_HIGH - 1) * (logx - log10(midPoint.x)) / (log10(maxPoint.x) - log10(midPoint.x));
        int j = int(knot_coord);
        float t = knot_coord - (float)j;

        float3 cf = float3(coefsHigh[j], coefsHigh[j + 1], coefsHigh[j + 2]);
        float3 monomials = float3(t * t, t , 1.f);
        logy = dot(monomials, mul(M, cf));
    }
    else
    {
        logy = logx * slopeHigh + (log10(maxPoint.y) - slopeHigh * log10(maxPoint.x));
    }

    return pow(10.f, logy);
}

float segmented_spline_c9_fwd(float x)
{
    const float coefsLow[10] = { -1.6989700043f, -1.6989700043f, -1.4779000000f, -1.2291000000f, -0.8648000000f, -0.4480000000f, 0.0051800000f, 0.4511080334f, 0.9113744414f, 0.9113744414f };
    const float coefsHigh[10] = { 0.5154386965f, 0.8470437783f, 1.1358000000f, 1.3802000000f, 1.5197000000f, 1.5985000000f, 1.6467000000f, 1.6746091357f, 1.6878733390f, 1.6878733390f };
    const float2 minPoint = float2(segmented_spline_c5_fwd(0.18f * exp2(-6.5f)), 0.02f);
    const float2 midPoint = float2(segmented_spline_c5_fwd(0.18f), 4.8f);
    const float2 maxPoint = float2(segmented_spline_c5_fwd(0.18f * exp2(6.5f)), 48.0f);
    const float slopeLow = 0.f;
    const float slopeHigh = 0.04f;

    const int N_KNOTS_LOW = 8;
    const int N_KNOTS_HIGH = 8;

    float xCheck = x;
    if (xCheck <= 0.f) { xCheck = 1e-4; }

    float logx = log10(xCheck);
    float logy;

    if (logx <= log10(minPoint.x))
    {
        logy = logx * slopeLow + (log10(minPoint.y) - slopeLow * log10(minPoint.x));
    }
    else if ((logx > log10(minPoint.x)) && (logx < log10(midPoint.x)))
    {
        float knot_coord = (float)(N_KNOTS_LOW - 1) * (logx - log10(minPoint.x)) / (log10(midPoint.x) - log10(minPoint.x));
        int j = int(knot_coord);
        float t = knot_coord - (float)j;

        float3 cf = float3(coefsLow[j], coefsLow[j + 1], coefsLow[j + 2]);
        float3 monomials = float3(t * t, t, 1.f);
        logy = dot(monomials, mul(M, cf));
    }
    else if ((logx >= log10(midPoint.x)) && (logx < log10(maxPoint.x)))
    {
        float knot_coord = (float)(N_KNOTS_HIGH - 1) * (logx - log10(midPoint.x)) / (log10(maxPoint.x) - log10(midPoint.x));
        int j = int(knot_coord);
        float t = knot_coord - (float)j;

        float3 cf = float3(coefsHigh[j], coefsHigh[j + 1], coefsHigh[j + 2]);
        float3 monomials = float3(t * t, t, 1.0);
        logy = dot(monomials, mul(M, cf));
    }
    else
    {
        logy = logx * slopeHigh + (log10(maxPoint.y) - slopeHigh * log10(maxPoint.x));
    }

    return pow(10.f, logy);
}

static const float RRT_GLOW_GAIN = 0.05f;
static const float RRT_GLOW_MID = 0.08f;

static const float RRT_RED_SCALE = 0.82f;
static const float RRT_RED_PIVOT = 0.03f;
static const float RRT_RED_HUE = 0.f;
static const float RRT_RED_WIDTH = 135.f;

static const float RRT_SAT_FACTOR = 0.96f;
static const float HALF_MAX = 65504.f;

float3 RRT(float3 aces)
{
    // --- Glow module --- //
    float saturation = rgb_2_saturation(aces);
    float ycIn = rgb_2_yc(aces);
    float s = sigmoid_shaper((saturation - 0.4) / 0.2);
    float addedGlow = 1.0 + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
    aces *= addedGlow;

    // --- Red modifier --- //
    float hue = rgb_2_hue(aces);
    float centeredHue = center_hue(hue, RRT_RED_HUE);
    float hueWeight = cubic_basis_shaper(centeredHue, RRT_RED_WIDTH);

    aces.r += hueWeight * saturation * (RRT_RED_PIVOT - aces.r) * (1.f - RRT_RED_SCALE);

    // --- ACES to RGB rendering space --- //
    aces = clamp(aces, (float3)0.f, HALF_MAX);  // avoids saturated negative colors from becoming positive in the matrix
    float3 rgbPre = mul(AP0_2_AP1_MAT, aces);
    rgbPre = clamp(rgbPre, 0.f, HALF_MAX);

    // --- Global desaturation --- //
    rgbPre = mul(RRT_SAT_MAT, rgbPre);

    // --- Apply the tonescale independently in rendering-space RGB --- //
    float3 rgbPost;
    rgbPost.x = segmented_spline_c5_fwd(rgbPre.x);
    rgbPost.y = segmented_spline_c5_fwd(rgbPre.y);
    rgbPost.z = segmented_spline_c5_fwd(rgbPre.z);

    // --- RGB rendering space to OCES --- //
    float3 rgbOces = mul(AP1_2_AP0_MAT,rgbPost);

    // Assign OCES RGB to output variables (OCES)
    return rgbOces;
}

float3 Y_2_linCV(float3 Y, float Ymax, float Ymin)
{
    return (Y - Ymin) / (Ymax - Ymin);
}

float3 XYZ_2_xyY(float3 XYZ)
{
    float divisor = max(dot(XYZ, (1.f).xxx), 1e-4);
    return float3(XYZ.xy / divisor, XYZ.y);
}

float3 xyY_2_XYZ(float3 xyY)
{
    float m = xyY.z / max(xyY.y, 1e-4);
    float3 XYZ = float3(xyY.xz, (1.f - xyY.x - xyY.y));
    XYZ.xz *= m;
    return XYZ;
}

#define DIM_SURROUND_GAMMA (0.9811f)

float3 darkSurround_to_dimSurround(float3 linearCV)
{
    float3 XYZ = mul(AP1_2_XYZ_MAT, linearCV);

    float3 xyY = XYZ_2_xyY(XYZ);
    xyY.z = clamp(xyY.z, 0.f, HALF_MAX);
    xyY.z = pow(xyY.z, DIM_SURROUND_GAMMA);
    XYZ = xyY_2_XYZ(xyY);

    return mul(XYZ_2_AP1_MAT, XYZ);
}

float moncurve_r(float y, float gamma, float offs)
{
    // Reverse monitor curve
    float x;
    const float yb = pow(offs * gamma / ((gamma - 1.f) * (1.f + offs)), gamma);
    const float rs = pow((gamma - 1.f) / offs, gamma - 1.f) * pow((1.f + offs) / gamma, gamma);
    if (y >= yb)
    {
        x = (1.f + offs) * pow(y, 1.f / gamma) - offs;
    }
    else
    {
        x = y * rs;
    }
    return x;
}

#define CINEMA_WHITE (48.f)
#define CINEMA_BLACK (CINEMA_WHITE / 2400.f)

// NOTE: The EOTF is *NOT* gamma 2.4, it follows IEC 61966-2-1:1999
#define DISPGAMMA (2.4f)
#define OFFSET (0.055f)

float3 ODT_RGBmonitor_100nits_dim(float3 oces)
{
    // OCES to RGB rendering space
    float3 rgbPre = mul(AP0_2_AP1_MAT, oces);

    // Apply the tonescale independently in rendering-space RGB
    float3 rgbPost;
    rgbPost.x = segmented_spline_c9_fwd(rgbPre.x);
    rgbPost.y = segmented_spline_c9_fwd(rgbPre.y);
    rgbPost.z = segmented_spline_c9_fwd(rgbPre.z);

    // Scale luminance to linear code value
    float3 linearCV = Y_2_linCV(rgbPost, CINEMA_WHITE, CINEMA_BLACK);

    // Apply gamma adjustment to compensate for dim surround
    linearCV = darkSurround_to_dimSurround(linearCV);

    // Apply desaturation to compensate for luminance difference
    linearCV = mul(ODT_SAT_MAT, linearCV);

    // Convert to display primary encoding
    // Rendering space RGB to XYZ
    float3 XYZ = mul(AP1_2_XYZ_MAT, linearCV);

    // Apply CAT from ACES white point to assumed observer adapted white point
    XYZ = mul(D60_2_D65_CAT, XYZ);

    // CIE XYZ to display primaries
    // linearCV = XYZ_2_DISPLAY_PRI_MAT * XYZ;
    linearCV = mul(XYZ_2_REC709_MAT, XYZ);

    // Handle out-of-gamut values
    // Clip values < 0 or > 1 (i.e. projecting outside the display primaries)
    linearCV = clamp(linearCV, 0.f , 1.f);

    float3 outputCV;
    outputCV.x = moncurve_r(linearCV.x, DISPGAMMA, OFFSET);
    outputCV.y = moncurve_r(linearCV.y, DISPGAMMA, OFFSET);
    outputCV.z = moncurve_r(linearCV.z, DISPGAMMA, OFFSET);
    return outputCV;
}

float3 ACESTonemapping(float3 aces)
{
    return ODT_RGBmonitor_100nits_dim(RRT(aces));
}

#define SCALE_0 (1.f / 12.92f)
#define SCALE_1 (1.f / 1.055f)
#define OFFSET_1 (0.055f * SCALE_1)

float LinearToSRGB_F(float color)
{
    color = clamp(color, 0.f, 1.f);
    if(color < 0.0031308f)
    {
        return color * 12.92f;
    }
    return 1.055f * pow(color, 0.41666f) - 0.055f;
}

float3 LinearToSRGB(float3 color)
{
    return float3(
        LinearToSRGB_F(color.x),
        LinearToSRGB_F(color.y),
        LinearToSRGB_F(color.z));
}
#define RADAR_SIZE_X (256.0f)
#define RADAR_SIZE_Y (256.0f)

#define RADAR_NUM_TILES (8)
#define RADAR_TILE_SIZE (RADAR_SIZE_X / RADAR_NUM_TILES)

#define RADAR_MIN_RANGE (14.0f)// og (7.8f)  - > 120.0f
#define RADAR_MAX_RANGE (48.0f)// og (22.5f) - > 350.0f
#define RADAR_MIN_SPEED (0.3f)
#define RADAR_MAX_SPEED (0.9f)

#define RADAR_LEFT (8.0f)
#define RADAR_BOTTOM (8.0f)
#define RADAR_WIDTH (82.0f) 
#define RADAR_HEIGHT (82.0f)

#define PAUSE_RADAR_LEFT (240.0f)
#define PAUSE_RADAR_BOTTOM (56.0f)
#define PAUSE_RADAR_WIDTH (160.0f)
#define PAUSE_RADAR_HEIGHT (160.0f)

#define RADAR_BLIPS_SIZE (7.0f)
#define RADAR_FRAME_PADDING (1.0f)

#include "plugin.h"
#include "common.h"
#include "CMenuManager.h"
#include "CSprite2d.h"
#include "Utility.h"
#include "CText.h"
#include "CPlayerPed.h"
#include "CPed.h"
#include "CGame.h"
#include "CObject.h"
#include "CGlobal.h"
#include "CKeybrd.h"
#include "CHud.h"
#include "CFont.h"
#include "CTheScripts.h"

#include "tVideo.h"
#include "cDMAudio.h"
#include "GBH.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>

using namespace plugin;

static config_file config(PLUGIN_PATH("GTA2Radar.ini"));
static int EnableBuiltinArrows = config["EnableBuiltinArrows"].asInt(0);
static float DynamicArrowsDistance = config["DynamicArrowsDistance"].asFloat(1.0);
static float RadarBlipsSize = config["RadarBlipsSize"].asFloat(RADAR_BLIPS_SIZE);
static int EnablePauseStatsRadar = config["EnablePauseStatsRadar"].asInt(1);
static float PauseRadarLeft = config["PauseRadarLeft"].asFloat(PAUSE_RADAR_LEFT);
static float PauseRadarBottom = config["PauseRadarBottom"].asFloat(PAUSE_RADAR_BOTTOM);
static float PauseRadarWidth = config["PauseRadarWidth"].asFloat(PAUSE_RADAR_WIDTH);
static float PauseRadarHeight = config["PauseRadarHeight"].asFloat(PAUSE_RADAR_HEIGHT);
static float PauseRadarBlipsSize = config["PauseRadarBlipsSize"].asFloat(4.0f);
static float PauseRadarRange = config["PauseRadarRange"].asFloat(100.0f);
static int EnablePauseStatsLayout = config["EnablePauseStatsLayout"].asInt(1);
static int PauseStatsTextY = config["PauseStatsTextY"].asInt(52);
static int PauseStatsSpriteY = config["PauseStatsSpriteY"].asInt(64);
static int EnablePickupBlips = config["EnablePickupBlips"].asInt(1);
static int EnablePickupIcons = config["EnablePickupIcons"].asInt(1);
static int EnableVehiclePickupBlips = config["EnableVehiclePickupBlips"].asInt(1);
static int EnableWeaponPickupBlips = config["EnableWeaponPickupBlips"].asInt(1);
static int EnableBonusPickupBlips = config["EnableBonusPickupBlips"].asInt(1);
static int EnableTokenPickupBlips = config["EnableTokenPickupBlips"].asInt(1);
static int EnableFrenzyPickupBlips = config["EnableFrenzyPickupBlips"].asInt(1);
static int EnableOtherPickupBlips = config["EnableOtherPickupBlips"].asInt(1);
static int EnablePickupBlipLog = config["EnablePickupBlipLog"].asInt(1);
static float PickupBlipMaxDistance = config["PickupBlipMaxDistance"].asFloat(1.1f);

enum eEnableBuiltinArrows {
    DISABLED,
    ENABLED,
    DYNAMIC
};

static int states[D3DRENDERSTATE_RANGEFOGENABLE];

struct tHardCodedBlips {
    CVector pos;
    short sprite;
};

std::vector<tHardCodedBlips> hardCodedBlips;

enum eHudSprites {
    SPRITE_RADAR_CENTRE,
    SPRITE_RADAR_NORTH,
    SPRITE_RADAR_RECT,
    SPRITE_RADAR_RECT_PAUSE,
    SPRITE_RADAR_LOONIE,
    SPRITE_RADAR_YAKUZA,
    SPRITE_RADAR_ZAIBATSU,
    SPRITE_RADAR_REDNECK,
    SPRITE_RADAR_SCIENTISTS,
    SPRITE_RADAR_KRISHNA,
    SPRITE_RADAR_MAFIA,
    SPRITE_RADAR_PHONE,
    SPRITE_RADAR_SAVE,
    SPRITE_RADAR_SPRAY,
    NUM_HUD_SPRITES,
};

CSprite2d radarSprites[64];
CSprite2d hudSprites[NUM_HUD_SPRITES];

const char* hudSpritesFileNames[NUM_HUD_SPRITES] = {
    "data\\hud\\radar_centre.dds",
    "data\\hud\\radar_north.dds",
    "data\\hud\\radar_rect.dds",
    "data\\hud\\radar_rect_256x256.dds",
    "data\\hud\\radar_loonie.dds",
    "data\\hud\\radar_yakuza.dds",
    "data\\hud\\radar_zaibatsu.dds",
    "data\\hud\\radar_redneck.dds",
    "data\\hud\\radar_scientists.dds",
    "data\\hud\\radar_krishna.dds",
    "data\\hud\\radar_mafia.dds",
    "data\\hud\\radar_phone.dds",
    "data\\hud\\radar_save.dds",
    "data\\hud\\radar_spray.dds",
};

static float cachedSin = 0.0f;
static float cachedCos = 1.0f;
static float radarRange = 1.0f;
static CVector2D radarOrigin;

struct tRadarViewport {
    float left;
    float bottom;
    float width;
    float height;
    float blipSize;
};

static tRadarViewport radarViewport = { RADAR_LEFT, RADAR_BOTTOM, RADAR_WIDTH, RADAR_HEIGHT, RADAR_BLIPS_SIZE };

enum ePickupBlipGroup {
    PICKUP_GROUP_OTHER,
    PICKUP_GROUP_VEHICLE,
    PICKUP_GROUP_WEAPON,
    PICKUP_GROUP_BONUS,
    PICKUP_GROUP_TOKEN,
    PICKUP_GROUP_FRENZY,
};

struct tPickupBlip {
    unsigned short model;
    ePickupBlipGroup group;
    CRGBA color;
};

static const tPickupBlip pickupBlips[] = {
    { 10,  PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 13,  PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 56,  PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 128, PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 138, PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 149, PICKUP_GROUP_VEHICLE, CRGBA(210, 210, 210, 255) },
    { 182, PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 183, PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 200, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 201, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 202, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 203, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 204, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 205, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 206, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 207, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 208, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 209, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 210, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 215, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 216, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 217, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 218, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 223, PICKUP_GROUP_WEAPON,  CRGBA(90, 180, 255, 255) },
    { 228, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 229, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 230, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 231, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 232, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 233, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 234, PICKUP_GROUP_BONUS,   CRGBA(0, 220, 255, 255) },
    { 235, PICKUP_GROUP_BONUS,   CRGBA(255, 80, 40, 255) },
    { 236, PICKUP_GROUP_BONUS,   CRGBA(255, 230, 40, 255) },
    { 237, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 238, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 239, PICKUP_GROUP_BONUS,   CRGBA(180, 80, 255, 255) },
    { 240, PICKUP_GROUP_BONUS,   CRGBA(120, 255, 120, 255) },
    { 254, PICKUP_GROUP_OTHER,   CRGBA(210, 210, 210, 255) },
    { 265, PICKUP_GROUP_OTHER,   CRGBA(255, 210, 40, 255) },
    { 266, PICKUP_GROUP_TOKEN,   CRGBA(255, 210, 40, 255) },
    { 286, PICKUP_GROUP_FRENZY,  CRGBA(255, 40, 180, 255) },
};

static const int NUM_PICKUP_SPRITES = sizeof(pickupBlips) / sizeof(pickupBlips[0]);
static CSprite2d pickupSprites[NUM_PICKUP_SPRITES];
static bool pickupSpriteLoaded[NUM_PICKUP_SPRITES];
static float pickupSpriteAspectX[NUM_PICKUP_SPRITES];
static float pickupSpriteAspectY[NUM_PICKUP_SPRITES];
static bool radarD3DReady = true;

static IDirect3DDevice3* TryGetRadarD3DDevice() {
    if (!GetModuleHandleA("d3ddll.dll"))
        return nullptr;

    tGraphics* graphics = GetGBH();
    return graphics ? graphics->device : nullptr;
}

static bool RestoreRadarD3DResources() {
    if (!TryGetRadarD3DDevice())
        return false;

    for (int i = 0; i < 64; i++) {
        radarSprites[i].Reset();
    }

    for (int i = 0; i < NUM_HUD_SPRITES; i++) {
        hudSprites[i].Reset();
    }

    for (int i = 0; i < NUM_PICKUP_SPRITES; i++) {
        pickupSprites[i].Reset();
        pickupSpriteLoaded[i] = pickupSprites[i].m_pTexture != NULL;
    }

    return true;
}

static void RadarLog(const char* format, ...) {
    if (!EnablePickupBlipLog)
        return;

    FILE* file = fopen(PLUGIN_PATH("GTA2Radar.log"), "a");
    if (!file)
        return;

    fprintf(file, "[%lu] ", GetTickCount());

    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);

    fprintf(file, "\n");
    fclose(file);
}

static void ResetRadarLog() {
    if (!EnablePickupBlipLog)
        return;

    FILE* file = fopen(PLUGIN_PATH("GTA2Radar.log"), "w");
    if (!file)
        return;

    fprintf(file, "GTA2Radar log started. EnablePickupBlips=%d EnablePickupIcons=%d RadarBlipsSize=%.2f PauseRadarBlipsSize=%.2f PickupBlipMaxDistance=%.2f\n",
        EnablePickupBlips, EnablePickupIcons, RadarBlipsSize, PauseRadarBlipsSize, PickupBlipMaxDistance);
    fclose(file);
}

class GTA2Radar {
public:
    static void SetRadarViewport(float left, float bottom, float width, float height, float blipSize) {
        radarViewport.left = left;
        radarViewport.bottom = bottom;
        radarViewport.width = width;
        radarViewport.height = height;
        radarViewport.blipSize = blipSize;
    }

    static void DrawRadarMask() {
        CVector2D corners[4] = {
            CVector2D(1.0f, -1.0f),
            CVector2D(1.0f, 1.0f),
            CVector2D(-1.0f, 1.0f),
            CVector2D(-1.0, -1.0f)
        };

        RenderStateSet(D3DRENDERSTATE_TEXTUREHANDLE, (void*)NULL);
        RenderStateSet(D3DRENDERSTATE_FOGENABLE, (void*)FALSE);
        RenderStateSet(D3DRENDERSTATE_TEXTUREMAG, (void*)D3DFILTER_LINEAR);
        RenderStateSet(D3DRENDERSTATE_TEXTUREMIN, (void*)D3DFILTER_LINEAR);
        RenderStateSet(D3DRENDERSTATE_SHADEMODE, (void*)D3DSHADE_FLAT);
        RenderStateSet(D3DRENDERSTATE_ZENABLE, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_ZWRITEENABLE, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_VERTEXBLEND, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_ALPHABLENDENABLE, (void*)TRUE);

        RenderStateSet(D3DRENDERSTATE_SRCBLEND, (void*)D3DBLEND_ZERO);
        RenderStateSet(D3DRENDERSTATE_DESTBLEND, (void*)D3DBLEND_ONE);

        CRect rect(0.0f, 0.0f, SCREEN_SCALE_X(radarViewport.width / 2), SCREEN_SCALE_Y(radarViewport.height));
        rect.Translate(SCREEN_SCALE_X(radarViewport.left), SCREEN_SCALE_FROM_BOTTOM(radarViewport.bottom + radarViewport.height));

        rect.Grow(SCREEN_SCALE_X(4.0f), SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(4.0f), SCREEN_SCALE_Y(4.0f));

        CRGBA col = { 255, 255, 255, 255 };
        CSprite2d::SetVertices(rect, col, col, col, col);
        RenderPrimitive(D3DPT_TRIANGLEFAN, CSprite2d::ms_aVertices, 4);
        CVector2D out[8];
        CVector2D in;

        for (int i = 0; i < 4; i++) {
            in.x = corners[i].x;
            in.y = corners[i].y;
            TransformRadarPointToScreenSpace(out[0], in);

            for (int j = 0; j < 7; j++) {
                in.x = corners[i].x * cos(j * (M_PI / 2.0f / 6.0f));
                in.y = corners[i].y * sin(j * (M_PI / 2.0f / 6.0f));
                TransformRadarPointToScreenSpace(out[j + 1], in);
            };

            CSprite2d::SetMaskVertices(8, (float*)out);
            RenderPrimitive(D3DPT_TRIANGLEFAN, CSprite2d::ms_aVertices, 8);
        }
    }

    static bool CanDrawPlayerRadar() {
        CPlayerPed* playa = GetGame()->FindPlayerPed(0);
        return playa && playa->GetPed();
    }

    static float ResolveRadarPlacementX(float value, float width) {
        if (value == -1.0f)
            return ((SCREEN_WIDTH - SCREEN_SCALE_X(width)) * 0.5f) / SCREEN_SCALE_X(1.0f);

        return value;
    }

    static float ResolveRadarPlacementY(float value, float height) {
        if (value == -1.0f)
            return ((SCREEN_HEIGHT - SCREEN_SCALE_Y(height)) * 0.5f) / SCREEN_SCALE_Y(1.0f);

        return value;
    }

    static void DrawMap(float rangeOverride = 0.0f) {
        CPlayerPed* playa = GetGame()->FindPlayerPed(0);

        if (rangeOverride > 0.0f)
            radarRange = rangeOverride;
        else
            radarRange = RADAR_MIN_RANGE + (playa->m_ViewCamera.m_vPosInterp.FromInt16().z * 2.0f);

        radarOrigin = playa->GetPed()->GetPosition2D().FromInt16();
        DrawRadarMap();
    }

    static void DrawRadarFrame(bool usePauseFrame) {
        float x = SCREEN_SCALE_X(radarViewport.left);
        float y = SCREEN_SCALE_FROM_BOTTOM(radarViewport.bottom + radarViewport.height);
        CRect rect(x, y, SCREEN_SCALE_X(radarViewport.width) + x, SCREEN_SCALE_Y(radarViewport.height) + y);
        rect.left -= SCREEN_SCALE_X(RADAR_FRAME_PADDING);
        rect.top -= SCREEN_SCALE_Y(RADAR_FRAME_PADDING);
        rect.right += SCREEN_SCALE_X(RADAR_FRAME_PADDING);
        rect.bottom += SCREEN_SCALE_Y(RADAR_FRAME_PADDING);

        int frameSprite = usePauseFrame && hudSprites[SPRITE_RADAR_RECT_PAUSE].m_pTexture
            ? SPRITE_RADAR_RECT_PAUSE
            : SPRITE_RADAR_RECT;
        hudSprites[frameSprite].Draw(rect, CRGBA(255, 255, 255, 255));
    }

    static void DrawRadarWidget(float left, float bottom, float width, float height, float blipSize,
        float rangeOverride = 0.0f, bool usePauseFrame = false) {
        IDirect3DDevice3* device = TryGetRadarD3DDevice();
        if (!device)
            return;

        if (!radarD3DReady) {
            if (!RestoreRadarD3DResources())
                return;

            radarD3DReady = true;
            device = TryGetRadarD3DDevice();
            if (!device)
                return;
        }

        if (!CanDrawPlayerRadar())
            return;

        left = ResolveRadarPlacementX(left, width);
        bottom = ResolveRadarPlacementY(bottom, height);

        LPDIRECT3DTEXTURE2 savedTexture = NULL;
        bool textureSaved = SUCCEEDED(device->GetTexture(0, &savedTexture));

        GetStates();
        SetRadarViewport(left, bottom, width, height, blipSize);
        DrawMap(rangeOverride);
        DrawRadarFrame(usePauseFrame);
        DrawBlips();
        RestoreStates();

        if (textureSaved)
            device->SetTexture(0, savedTexture);

        if (savedTexture)
            savedTexture->Release();
    }

    static void DrawRadarMap() {
        DrawRadarMask();

        int x = floor(radarOrigin.x / RADAR_TILE_SIZE);
        int y = ceil(radarOrigin.y / RADAR_TILE_SIZE);

        RenderStateSet(D3DRENDERSTATE_FOGENABLE, (void*)FALSE);
        RenderStateSet(D3DRENDERSTATE_SRCBLEND, (void*)D3DBLEND_SRCALPHA);
        RenderStateSet(D3DRENDERSTATE_DESTBLEND, (void*)D3DBLEND_INVSRCALPHA);
        RenderStateSet(D3DRENDERSTATE_TEXTUREMAG, (void*)D3DFILTER_LINEAR);
        RenderStateSet(D3DRENDERSTATE_TEXTUREMIN, (void*)D3DFILTER_LINEAR);
        RenderStateSet(D3DRENDERSTATE_SHADEMODE, (void*)D3DSHADE_FLAT);
        RenderStateSet(D3DRENDERSTATE_ZENABLE, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_ZWRITEENABLE, (void*)FALSE);
        RenderStateSet(D3DRENDERSTATE_VERTEXBLEND, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_ALPHABLENDENABLE, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_TEXTUREADDRESS, (void*)D3DTADDRESS_CLAMP);
        RenderStateSet(D3DRENDERSTATE_TEXTUREPERSPECTIVE, (void*)FALSE);

        for (int i = x - 4; i < x + 4; i++) {
            for (int j = y - 4; j < y + 4; j++) {
                DrawRadarSection(i, j);
            }
        }
    }

    static void DrawRadarSection(int x, int y) {
        CVector2D worldPoly[8];
        CVector2D radarCorners[4];
        CVector2D radarPoly[8];
        CVector2D texCoords[8];
        CVector2D screenPoly[8];

        GetTextureCorners(x, y, worldPoly);
        ClipRadarTileCoords(x, y);

        int index = x + RADAR_NUM_TILES * y;
        LPDIRECT3DTEXTURE2 texture = radarSprites[index].m_pTexture;

        for (int i = 0; i < 4; i++)
            TransformRealWorldPointToRadarSpace(radarCorners[i], worldPoly[i]);

        int numVertices = ClipRadarPoly(radarPoly, radarCorners);

        if (numVertices < 3)
            return;

        for (int i = 0; i < numVertices; i++) {
            TransformRadarPointToRealWorldSpace(worldPoly[i], radarPoly[i]);
            TransformRealWorldToTexCoordSpace(texCoords[i], worldPoly[i], x, y);
            TransformRadarPointToScreenSpace(screenPoly[i], radarPoly[i]);
        }

        RenderStateSet(D3DRENDERSTATE_TEXTUREHANDLE, texture);
        CSprite2d::SetVertices(numVertices, (float*)screenPoly, (float*)texCoords, CRGBA(255, 255, 255, 255));
        RenderPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLEFAN, CSprite2d::ms_aVertices, numVertices);
    }

    static void GetTextureCorners(int x, int y, CVector2D* out) {
        // bottom left
        out[0].x = RADAR_TILE_SIZE * (x);
        out[0].y = RADAR_TILE_SIZE * (y + 1);

        // bottom right
        out[1].x = RADAR_TILE_SIZE * (x + 1);
        out[1].y = RADAR_TILE_SIZE * (y + 1);

        // top right
        out[2].x = RADAR_TILE_SIZE * (x + 1);
        out[2].y = RADAR_TILE_SIZE * (y);

        // top left
        out[3].x = RADAR_TILE_SIZE * (x);
        out[3].y = RADAR_TILE_SIZE * (y);
    }

    static void ClipRadarTileCoords(int& x, int& y) {
        if (x < 0)
            x = 0;
        if (x > RADAR_NUM_TILES - 1)
            x = RADAR_NUM_TILES - 1;
        if (y < 0)
            y = 0;
        if (y > RADAR_NUM_TILES - 1)
            y = RADAR_NUM_TILES - 1;
    }

    static int ClipPolyPlane(const CVector2D* in, int nin, CVector2D* out, CVector* plane) {
        int j;
        int nout;
        int x1, x2;
        float d1, d2, t;

        nout = 0;
        for (j = 0; j < nin; j++) {
            x1 = j;
            x2 = (j + 1) % nin;

            d1 = plane->x * in[x1].x + plane->y * in[x1].y + plane->z;
            d2 = plane->x * in[x2].x + plane->y * in[x2].y + plane->z;
            if (d1 * d2 < 0.0f) {
                t = d1 / (d1 - d2);
                out[nout++] = in[x1] * (1.0f - t) + in[x2] * t;
            }
            if (d2 >= 0.0f)
                out[nout++] = in[x2];
        }
        return nout;
    }

    static int ClipRadarPoly(CVector2D* poly, const CVector2D* rect) {
        CVector planes[4] = {
            CVector(-1.0f, 0.0f, 1.0f),
            CVector(1.0f, 0.0f, 1.0f),
            CVector(0.0f, -1.0f, 1.0f),
            CVector(0.0f,  1.0f, 1.0f)
        };
        CVector2D tmp[8];
        int n;
        if (n = ClipPolyPlane(rect, 4, tmp, &planes[0]), n == 0) return 0;
        if (n = ClipPolyPlane(tmp, n, poly, &planes[1]), n == 0) return 0;
        if (n = ClipPolyPlane(poly, n, tmp, &planes[2]), n == 0) return 0;
        if (n = ClipPolyPlane(tmp, n, poly, &planes[3]), n == 0) return 0;
        return n;
    }

    static void TransformRealWorldPointToRadarSpace(CVector2D& out, const CVector2D& in) {
        float x = (in.x - radarOrigin.x) * (1.0f / radarRange);
        float y = (in.y - radarOrigin.y) * (1.0f / radarRange);

        out.x = cachedSin * y + cachedCos * x;
        out.y = cachedCos * y - cachedSin * x;
    }

    static void TransformRadarPointToRealWorldSpace(CVector2D& out, const CVector2D& in) {
        float s = -cachedSin;
        float c = cachedCos;

        out.x = s * in.y + c * in.x;
        out.y = c * in.y - s * in.x;

        out = out * radarRange + radarOrigin;
    }

    static void TransformRealWorldToTexCoordSpace(CVector2D& out, const CVector2D& in, int x, int y) {
        out.x = in.x - (x * RADAR_TILE_SIZE);
        out.y = in.y - (y * RADAR_TILE_SIZE);
        out.x /= RADAR_TILE_SIZE;
        out.y /= RADAR_TILE_SIZE;
    }

    static void TransformRadarPointToScreenSpace(CVector2D& out, const CVector2D& in) {
        out.x = (in.x + 1.0f) * 0.5f * SCREEN_SCALE_X(radarViewport.width) + SCREEN_SCALE_X(radarViewport.left);
        out.y = (in.y + 1.0f) * 0.5f * SCREEN_SCALE_Y(radarViewport.height) + SCREEN_SCALE_FROM_BOTTOM(radarViewport.bottom + radarViewport.height);
    }

    static float LimitRadarPoint(CVector2D& pos) {
        float dist = pos.Magnitude();
        pos.x = CLAMP(pos.x, -1.0f, 1.0f);
        pos.y = CLAMP(pos.y, -1.0f, 1.0f);
        return dist;
    }

    static void DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, int alpha) {
        CVector curPosn[4];
        const float correctedAngle = angle - M_PI / 4.f;
        float scale = SCREEN_SCALE_Y(radarViewport.blipSize);

        for (unsigned int i = 0; i < 4; i++) {
            const float cornerAngle = i * (M_PI / 2) + correctedAngle;
            curPosn[i].x = x + (0.0f * cos(cornerAngle) + 1.0f * sin(cornerAngle)) * scale;
            curPosn[i].y = y - (0.0f * sin(cornerAngle) - 1.0f * cos(cornerAngle)) * scale;
        }

        sprite->Draw(curPosn[3].x, curPosn[3].y, curPosn[2].x, curPosn[2].y, curPosn[0].x, curPosn[0].y, curPosn[1].x, curPosn[1].y, CRGBA(255, 255, 255, alpha));
    }

    static void DrawBlip(CSprite2d* sprite, CVector2D out, CRGBA const& col) {
        float scale = SCREEN_SCALE_Y(radarViewport.blipSize);
        sprite->Draw(CRect(out.x - scale, out.y - scale, out.x + scale, out.y + scale), col);
    }

    static unsigned char CalculateBlipAlpha(float dist) {
        if (dist <= 1.0f)
            return 255;

        if (dist <= 5.0f)
            return (128.0f * ((dist - 1.0f) / 4.0f)) + ((1.0f - (dist - 1.0f) / 4.0f) * 255.0f);

        return 128;
    }

    template <typename T>
    static bool TryRead(uintptr_t address, T& out) {
        if (!address || IsBadReadPtr(reinterpret_cast<void*>(address), sizeof(T)))
            return false;

        memcpy(&out, reinterpret_cast<void*>(address), sizeof(T));
        return true;
    }

    static const tPickupBlip* FindPickupBlip(unsigned short model) {
        for (auto& pickup : pickupBlips) {
            if (pickup.model == model)
                return &pickup;
        }

        return nullptr;
    }

    static int GetPickupBlipIndex(const tPickupBlip* pickup) {
        if (!pickup)
            return -1;

        return pickup - pickupBlips;
    }

    static bool IsPickupBlipGroupEnabled(ePickupBlipGroup group) {
        switch (group) {
        case PICKUP_GROUP_VEHICLE:
            return EnableVehiclePickupBlips != 0;
        case PICKUP_GROUP_WEAPON:
            return EnableWeaponPickupBlips != 0;
        case PICKUP_GROUP_BONUS:
            return EnableBonusPickupBlips != 0;
        case PICKUP_GROUP_TOKEN:
            return EnableTokenPickupBlips != 0;
        case PICKUP_GROUP_FRENZY:
            return EnableFrenzyPickupBlips != 0;
        default:
            return EnableOtherPickupBlips != 0;
        }
    }

    static bool ResolvePickupObjectScanStartForBase(uintptr_t gta2Base, uintptr_t& scanStart, char* source, size_t sourceSize) {
        static constexpr uintptr_t savedGameFirstObjectOffset = 0x275784;
        static constexpr uintptr_t firstObjectOffset = 0x275788;

        uintptr_t savedGameFirstObjectPtr = gta2Base + savedGameFirstObjectOffset;
        uintptr_t firstObjectPtr = gta2Base + firstObjectOffset;
        uintptr_t firstObjectPtr2 = 0;
        uintptr_t savedGameFirstObjectPtr2 = 0;
        uintptr_t firstObjectAddr = 0;

        if (source && sourceSize)
            source[0] = '\0';

        if (TryRead(savedGameFirstObjectPtr, savedGameFirstObjectPtr2) &&
            savedGameFirstObjectPtr2 &&
            TryRead(savedGameFirstObjectPtr2 + 52, firstObjectPtr2) &&
            firstObjectPtr2) {
            firstObjectPtr2 += 44;

            if (source && sourceSize)
                sprintf(source, "base=0x%08X saved savedPtr=0x%08X firstPtr=0x%08X", gta2Base, savedGameFirstObjectPtr2, firstObjectPtr2);
        }
        else {
            if (!TryRead(firstObjectPtr, firstObjectPtr2)) {
                if (source && sourceSize)
                    sprintf(source, "base=0x%08X failed reading firstObjectPtr=0x%08X", gta2Base, firstObjectPtr);
                return false;
            }

            firstObjectPtr2 += 0xC;

            if (source && sourceSize)
                sprintf(source, "base=0x%08X new rawFirst=0x%08X firstPtr=0x%08X", gta2Base, firstObjectPtr2 - 0xC, firstObjectPtr2);
        }

        if (!TryRead(firstObjectPtr2, firstObjectAddr) || !firstObjectAddr) {
            if (source && sourceSize) {
                char previous[128] = {};
                strncpy(previous, source, sizeof(previous) - 1);
                sprintf(source, "%s firstObjectAddr missing", previous);
            }
            return false;
        }

        scanStart = firstObjectAddr + 24;
        return !IsBadReadPtr(reinterpret_cast<void*>(scanStart), 64);
    }

    static bool GetPickupObjectScanStart(uintptr_t& scanStart, char* source, size_t sourceSize) {
        static const uintptr_t gta2Bases[] = {
            0x3F0000,
            0x400000,
            0x3E0000,
        };

        char attempts[384] = {};

        for (uintptr_t base : gta2Bases) {
            if (ResolvePickupObjectScanStartForBase(base, scanStart, source, sourceSize))
                return true;

            if (source && source[0]) {
                if (attempts[0])
                    strncat(attempts, " | ", sizeof(attempts) - strlen(attempts) - 1);
                strncat(attempts, source, sizeof(attempts) - strlen(attempts) - 1);
            }
        }

        if (source && sourceSize)
            strncpy(source, attempts[0] ? attempts : "all base attempts failed", sourceSize - 1);

        return false;
    }

    static void LoadPickupSprites() {
        int loaded = 0;

        for (int i = 0; i < NUM_PICKUP_SPRITES; i++) {
            pickupSpriteLoaded[i] = false;
            pickupSpriteAspectX[i] = 1.0f;
            pickupSpriteAspectY[i] = 1.0f;
        }

        if (!EnablePickupIcons) {
            RadarLog("pickup icons: disabled");
            return;
        }

        for (int i = 0; i < NUM_PICKUP_SPRITES; i++) {
            char path[MAX_PATH] = {};
            sprintf(path, "data\\hud\\%u.dds", pickupBlips[i].model);

            pickupSprites[i].SetTexture(path);
            pickupSpriteLoaded[i] = pickupSprites[i].m_pTexture != NULL;

            if (pickupSpriteLoaded[i]) {
                unsigned int width = 0;
                unsigned int height = 0;

                FILE* file = fopen(path, "rb");
                if (file) {
                    unsigned char header[20] = {};
                    if (fread(header, 1, sizeof(header), file) == sizeof(header) &&
                        header[0] == 'D' && header[1] == 'D' && header[2] == 'S' && header[3] == ' ') {
                        memcpy(&height, header + 12, sizeof(height));
                        memcpy(&width, header + 16, sizeof(width));
                    }
                    fclose(file);
                }

                if (width > 0 && height > 0) {
                    float longestSide = static_cast<float>(width > height ? width : height);
                    pickupSpriteAspectX[i] = static_cast<float>(width) / longestSide;
                    pickupSpriteAspectY[i] = static_cast<float>(height) / longestSide;
                }

                loaded++;
            }
        }

        RadarLog("pickup icons: enabled=%d loaded=%d total=%d path=data\\hud\\<model>.dds",
            EnablePickupIcons, loaded, NUM_PICKUP_SPRITES);
    }

    static void DrawPickupMarker(const tPickupBlip* pickup, CVector2D pos, int level, CRGBA col) {
        int index = GetPickupBlipIndex(pickup);
        float scale = SCREEN_SCALE_Y(radarViewport.blipSize);

        if (EnablePickupIcons && index >= 0 && index < NUM_PICKUP_SPRITES && pickupSpriteLoaded[index]) {
            float scaleX = scale * pickupSpriteAspectX[index];
            float scaleY = scale * pickupSpriteAspectY[index];
            pickupSprites[index].Draw(CRect(pos.x - scaleX, pos.y - scaleY, pos.x + scaleX, pos.y + scaleY), CRGBA(255, 255, 255, col.a));
            return;
        }

        DrawLevel(pos, level, scale, col);
    }

    static void DrawPickupBlips() {
        if (!EnablePickupBlips)
            return;

        static unsigned int logFrame = 0;
        static bool loggedDisabled = false;
        logFrame++;

        const bool shouldLog = EnablePickupBlipLog && (logFrame == 1 || (logFrame % 300) == 0);

        uintptr_t scanStart = 0;
        char scanSource[128] = {};
        if (!GetPickupObjectScanStart(scanStart, scanSource, sizeof(scanSource))) {
            if (shouldLog)
                RadarLog("pickup scan: no object list (%s)", scanSource[0] ? scanSource : "unknown reason");
            return;
        }

        CPlayerPed* playa = GetGame()->FindPlayerPed(0);
        if (!playa || !playa->GetPed()) {
            if (!loggedDisabled) {
                RadarLog("pickup scan: no player ped");
                loggedDisabled = true;
            }
            return;
        }

        static constexpr int objectScanBytes = 100000;
        static constexpr int objectSlotSize = 44;
        int readSlots = 0;
        int matched = 0;
        int disabledByGroup = 0;
        int readFailures = 0;
        int outOfBounds = 0;
        int hiddenByDistance = 0;
        int drawn = 0;
        int samplesLogged = 0;

        for (int i = 0; i < objectScanBytes; i += objectSlotSize) {
            unsigned short model = 0xFFFF;
            if (!TryRead(scanStart + i, model))
                continue;

            readSlots++;

            const tPickupBlip* pickup = FindPickupBlip(model);
            if (!pickup)
                continue;

            matched++;

            if (!IsPickupBlipGroupEnabled(pickup->group)) {
                disabledByGroup++;
                continue;
            }

            uintptr_t objectAddr = 0;
            int x = 0;
            int y = 0;
            int z = 0;

            if (!TryRead(scanStart + i - 20, objectAddr) ||
                !TryRead(objectAddr + 20, x) ||
                !TryRead(objectAddr + 24, y) ||
                !TryRead(objectAddr + 28, z)) {
                readFailures++;
                continue;
            }

            CVector pos = {
                static_cast<float>(x) / 16384.0f,
                static_cast<float>(y) / 16384.0f,
                static_cast<float>(z) / 16384.0f
            };

            if (pos.x < 0.0f || pos.x > RADAR_SIZE_X || pos.y < 0.0f || pos.y > RADAR_SIZE_Y) {
                outOfBounds++;
                continue;
            }

            CVector2D out = {};
            CVector2D in = {};
            TransformRealWorldPointToRadarSpace(in, { pos.x, pos.y });
            float dist = LimitRadarPoint(in);
            if (dist > PickupBlipMaxDistance) {
                hiddenByDistance++;
                if (shouldLog && samplesLogged < 6) {
                    RadarLog("pickup sample hidden: model=%u pos=%.2f,%.2f,%.2f dist=%.2f object=0x%08X",
                        model, pos.x, pos.y, pos.z, dist, objectAddr);
                    samplesLogged++;
                }
                continue;
            }

            TransformRadarPointToScreenSpace(out, in);

            int level = 0;
            float diff = playa->GetPed()->GetPosition().FromInt16().z - pos.z;
            if (diff > 0.1f)
                level = 1;
            else if (diff < -0.1f)
                level = 2;

            CRGBA col = pickup->color;
            col.a = CalculateBlipAlpha(dist);
            DrawPickupMarker(pickup, out, level, col);
            drawn++;

            if (shouldLog && samplesLogged < 6) {
                RadarLog("pickup sample drawn: model=%u pos=%.2f,%.2f,%.2f dist=%.2f screen=%.1f,%.1f object=0x%08X",
                    model, pos.x, pos.y, pos.z, dist, out.x, out.y, objectAddr);
                samplesLogged++;
            }
        }

        if (shouldLog) {
            RadarLog("pickup scan: source=(%s) scanStart=0x%08X slots=%d matched=%d disabledByGroup=%d readFailures=%d outOfBounds=%d hiddenByDistance=%d drawn=%d range=%.2f origin=%.2f,%.2f",
                scanSource, scanStart, readSlots, matched, disabledByGroup, readFailures, outOfBounds, hiddenByDistance, drawn, radarRange, radarOrigin.x, radarOrigin.y);
        }
    }

    static void DrawRadarNorth() {
        CVector2D out;
        CVector2D in;
        out.x = radarOrigin.x;
        out.y = -M_SQRT2 * radarRange + radarOrigin.y;
        TransformRealWorldPointToRadarSpace(in, out);
        LimitRadarPoint(in);
        TransformRadarPointToScreenSpace(out, in);

        float scale = SCREEN_SCALE_X(radarViewport.blipSize);
        hudSprites[SPRITE_RADAR_NORTH].Draw(CRect(out.x - scale, out.y - scale, out.x + scale, out.y + scale), CRGBA(255, 255, 255, 255));
    }

    static void DrawRadarCentre() {
        CVector2D out;
        CVector2D in = CVector2D(0.0f, 0.0f);
        TransformRadarPointToScreenSpace(out, in);

        CPlayerPed* playa = GetGame()->FindPlayerPed(0);
        CCar* car = playa->m_pPed->m_pCurrentCar;

        float angle = 0.0f;
        if (car)
            angle = DEGTORAD(car->m_pSprite->m_nRotation / 4.0f);
        else
            angle = DEGTORAD(playa->m_pPed->m_pObject->m_pSprite->m_nRotation / 4.0f);

        DrawRotatingRadarSprite(&hudSprites[SPRITE_RADAR_CENTRE], out.x, out.y, angle, 255);
    }

    static void DrawBlips() {
        RenderStateSet(D3DRENDERSTATE_ZWRITEENABLE, (void*)FALSE);
        RenderStateSet(D3DRENDERSTATE_ZENABLE, (void*)FALSE);
        RenderStateSet(D3DRENDERSTATE_VERTEXBLEND, (void*)TRUE);
        RenderStateSet(D3DRENDERSTATE_SRCBLEND, (void*)D3DBLEND_SRCALPHA);
        RenderStateSet(D3DRENDERSTATE_DESTBLEND, (void*)D3DBLEND_INVSRCALPHA);
        RenderStateSet(D3DRENDERSTATE_FOGENABLE, (void*)FALSE);

        DrawRadarNorth();

        // Hardcoded blips
        for (auto& it : hardCodedBlips) {
            CVector2D out = {};
            CVector2D in = {};
            TransformRealWorldPointToRadarSpace(in, { it.pos.x, it.pos.y });
            float dist = LimitRadarPoint(in);
            TransformRadarPointToScreenSpace(out, in);

            if (dist > 1.1f)
                continue;

            unsigned char a = CalculateBlipAlpha(dist);
            DrawBlip(&hudSprites[it.sprite], out, CRGBA(255, 255, 255, a));
        }

        CPlayerPed* playa = GetGame()->FindPlayerPed(0);
        for (int i = 0; i < MAX_HUD_ARROWS; i++) {
            CHudArrow* arrow = &GetHud()->m_HudArrows[i];

            if (!arrow->AreBothArrowTracesUsed() /* && arrow->IsArrowVisible()*/) {
                int sprite = arrow->m_nSpriteId;
                int type = arrow->m_nType;

                CRGBA col = { 255, 0, 255, 255 };
                CVector pos = arrow->m_ArrowTrace.m_vPos.FromInt16();

                CVector2D out = {};
                CVector2D in = {};
                TransformRealWorldPointToRadarSpace(in, { pos.x, pos.y });
                float dist = LimitRadarPoint(in);
                TransformRadarPointToScreenSpace(out, in);

                int level = 0;
                float diff = playa->GetPed()->GetPosition().FromInt16().z - pos.z;

                if (diff > 0.1f)
                    level = 1;
                else if (diff < -0.1f)
                    level = 2;
                else
                    level = 0;

                const bool onAMission = !GetTheScripts()->OnAMissionFlag || *GetTheScripts()->OnAMissionFlag;

                switch (sprite) {
                case SPRITE_BIGARROW:
                    DrawLevel(out, level, SCREEN_SCALE_X(radarViewport.blipSize), col);
                    break;
                case SPRITE_GREENARROW:
                case SPRITE_BLUEARROW:
                case SPRITE_GREYARROW:
                case SPRITE_BLUELIGHT:
                case SPRITE_YELLOW:
                case SPRITE_ORANGE:
                case SPRITE_RED:
                    if (!onAMission) {
                        unsigned char a = CalculateBlipAlpha(dist);
                        switch (type) {
                        case TYPE_GREEN:
                            if (dist <= 2.0f || arrow->IsArrowVisible())
                                DrawBlip(&hudSprites[SPRITE_RADAR_PHONE], out, CRGBA(0, 190, 0, a));
                            break;
                        case TYPE_RED:
                            if (dist <= 2.0f || arrow->IsArrowVisible())
                                DrawBlip(&hudSprites[SPRITE_RADAR_PHONE], out, CRGBA(190, 0, 0, a));
                            break;
                        case TYPE_YELLOW:
                            if (dist <= 2.0f || arrow->IsArrowVisible())
                                DrawBlip(&hudSprites[SPRITE_RADAR_PHONE], out, CRGBA(190, 180, 0, a));
                            break;
                        default:
                            DrawBlip(&hudSprites[SPRITE_RADAR_LOONIE + sprite - 1], out, CRGBA(255, 255, 255, a));
                            break;
                        }
                    }
                    break;
                case SPRITE_SMALLYELLOW:
                case SPRITE_SMALLGREEN:
                case SPRITE_SMALLRED:
                    break;
                }
            }
        }

        DrawPickupBlips();
        DrawRadarCentre();
    }

    static void GetStates() {
        for (int i = 0; i < D3DRENDERSTATE_RANGEFOGENABLE; i++) {
            if (i == D3DRENDERSTATE_TEXTUREHANDLE)
                continue;

            RenderStateGet((D3DRENDERSTATETYPE)i, (void*)&states[i]);
        }
    }

    static void RestoreStates() {
        for (int i = 0; i < D3DRENDERSTATE_RANGEFOGENABLE; i++) {
            if (i == D3DRENDERSTATE_TEXTUREHANDLE)
                continue;

            RenderStateSet((D3DRENDERSTATETYPE)i, (void*)states[i]);
        }
    }

    static void ApplyPauseStatsLayout() {
        if (!EnablePauseStatsRadar || !EnablePauseStatsLayout)
            return;

        plugin::patch::SetInt(0x4CA3F6 + 1, PauseStatsTextY);
        plugin::patch::SetInt(0x4CA1A3 + 1, PauseStatsSpriteY);
    }

    /* Calls from 0x4C82D5 to 0x4C7050 (IsArrowVisible) are redirected here if EnableBuiltinArrows = DYNAMIC
    *  We can get arrowAddress either from ECX register or top of stack
    *  Stack needs to be unchanged upon return, but cdecl might reuse stack space if optimization is on
    *  Using fastcall instead since it takes argument from ECX and doesn't mess with the stack
    */
    static int __fastcall HandleDynamicArrows(int arrowAddress) {
        CHudArrow* arrow = (CHudArrow*)arrowAddress;
        if (arrow->m_bVisible != 1) {
            return 0;
        }
        else {
            CVector pos = arrow->m_ArrowTrace.m_vPos.FromInt16();
            CVector2D in = {};
            TransformRealWorldPointToRadarSpace(in, { pos.x, pos.y });
            if (LimitRadarPoint(in) <= DynamicArrowsDistance) {
                return 1;
            }
            else {
                return 0;
            }
        }
    }

    GTA2Radar() {
        ApplyPauseStatsLayout();

        ThiscallEvent <AddressList<0x462028, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(CGame*)> onGameInit;
        onGameInit += []() {
            ResetRadarLog();
            RadarLog("game init");

            for (int i = 0; i < NUM_HUD_SPRITES; i++) {
                hudSprites[i].SetTexture(hudSpritesFileNames[i]);
            }

            LoadPickupSprites();

            char mapName[32] = {};
            const char* s = gGlobal.mapName;
            s += 5;
            
            int i = 0;
            while (*s) {
                if (*s == '.') {
                    mapName[i] = '\0';
                    break;
                }
            
                mapName[i] = *s;
                i++;
                s++;
            }

            // Hardcoded blips
            hardCodedBlips.clear();
            hardCodedBlips.shrink_to_fit();
            if (!strcmp(mapName, "wil")) {
                hardCodedBlips.push_back({ { 159.00, 137.00, 2.00 }, SPRITE_RADAR_SAVE }); // Save
                hardCodedBlips.push_back({ { 204.50, 221.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 219.50, 34.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 24.50, 60.50, 3.00 },  SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 46.50, 136.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 86.50, 160.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 209.50, 121.50, 2.00 }, SPRITE_RADAR_SPRAY });
            }
            else if (!strcmp(mapName, "ste")) {
                hardCodedBlips.push_back({ { 113.00, 123.00, 2.00 }, SPRITE_RADAR_SAVE }); // Save
                hardCodedBlips.push_back({ { 6.50, 183.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 178.50, 216.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 215.50, 76.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 94.50, 27.50, 2.00 }, SPRITE_RADAR_SPRAY });
            }
            else if (!strcmp(mapName, "bil")) {
                hardCodedBlips.push_back({ { 44.50, 102.00, 2.00 }, SPRITE_RADAR_SAVE }); // Save
                hardCodedBlips.push_back({ { 177.50, 14.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 234.50, 154.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ { 30.50, 216.50, 2.00 }, SPRITE_RADAR_SPRAY });
                hardCodedBlips.push_back({ {64.50, 90.50, 2.00}, SPRITE_RADAR_SPRAY });
            }

            for (int i = 0; i < 64; i++) {
                char buff[MAX_PATH];
                sprintf(buff, "data\\radar\\%s\\%s_%02d.dds", mapName, mapName, i + 1);
                radarSprites[i].SetTexture(buff);
            }
        };

        plugin::Events::shutdownEngineEvent += []() {
            for (int i = 0; i < 64; i++) {
                radarSprites[i].Delete();
            }
            
            for (int i = 0; i < NUM_HUD_SPRITES; i++) {
                hudSprites[i].Delete();
            }

            for (int i = 0; i < NUM_PICKUP_SPRITES; i++) {
                pickupSprites[i].Delete();
                pickupSpriteLoaded[i] = false;
            }
        };

        plugin::Events::d3dLostEvent += []() {
            radarD3DReady = false;
        };

        plugin::Events::d3dResetEvent += []() {
            // The GTA2 graphics pointer can still be null while dgVoodoo is
            // returning focus. Restore on the first HUD frame with a valid
            // graphics object and device instead of dereferencing it here.
            radarD3DReady = false;
        };

        plugin::Events::drawHudEvent += []() {
            if (GetGame()->GetIsUserPaused() && EnablePauseStatsRadar) {
                DrawRadarWidget(PauseRadarLeft, PauseRadarBottom, PauseRadarWidth, PauseRadarHeight,
                    PauseRadarBlipsSize, PauseRadarRange, true);
                return;
            }

            DrawRadarWidget(RADAR_LEFT, RADAR_BOTTOM, RADAR_WIDTH, RADAR_HEIGHT, RadarBlipsSize);
        };     

        if (EnableBuiltinArrows == DISABLED)
        {
            plugin::patch::Nop(0x4CA4D2, 11);
        }
        else if (EnableBuiltinArrows == DYNAMIC) {
            plugin::patch::ReplaceFunctionCall(0x4C82D5, HandleDynamicArrows);
        }

    };
} gta2Radar;

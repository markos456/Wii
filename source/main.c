#include <gccore.h>
#include <wiiuse/wpad.h>
#include <math.h>
#include <stdio.h>

static GXRModeObj *videoMode;
static void *framebuffer;

static void vertex(f32 x, f32 y, f32 z, GXColor color) {
    GX_Position3f32(x, y, z);
    GX_Color4u8(color.r, color.g, color.b, color.a);
}

static void quad(
    f32 x1, f32 y1, f32 z1,
    f32 x2, f32 y2, f32 z2,
    f32 x3, f32 y3, f32 z3,
    f32 x4, f32 y4, f32 z4,
    GXColor color
) {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    vertex(x1, y1, z1, color);
    vertex(x2, y2, z2, color);
    vertex(x3, y3, z3, color);
    vertex(x4, y4, z4, color);
    GX_End();
}

static void box(f32 cx, f32 cy, f32 cz,
                f32 sx, f32 sy, f32 sz, GXColor c) {
    f32 x0 = cx - sx * 0.5f, x1 = cx + sx * 0.5f;
    f32 y0 = cy - sy * 0.5f, y1 = cy + sy * 0.5f;
    f32 z0 = cz - sz * 0.5f, z1 = cz + sz * 0.5f;

    quad(x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1, c);
    quad(x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0, c);
    quad(x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0, c);
    quad(x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1, c);
    quad(x0,y1,z1, x1,y1,z1, x1,y1,z0, x0,y1,z0, c);
    quad(x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z1, c);
}

static void setupGX(void) {
    GX_Init(NULL, 0);

    GX_SetViewport(0, 0, videoMode->fbWidth, videoMode->efbHeight, 0.0f, 1.0f);
    GX_SetScissor(0, 0, videoMode->fbWidth, videoMode->efbHeight);
    GX_SetDispCopyYScale(GX_GetYScaleFactor(videoMode->efbHeight, videoMode->xfbHeight));
    GX_SetDispCopySrc(0, 0, videoMode->fbWidth, videoMode->efbHeight);
    GX_SetDispCopyDst(videoMode->fbWidth, videoMode->xfbHeight);
    GX_SetCopyFilter(videoMode->aa, videoMode->sample_pattern, GX_TRUE, videoMode->vfilter);
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetCopyClear((GXColor){ 90, 170, 230, 255 }, 0x00ffffff);

    GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetCullMode(GX_CULL_NONE);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                   GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
    GX_SetNumTexGens(0);
    GX_SetNumTevStages(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_InvVtxCache();
}

static void drawScene(f32 carX, f32 carZ, f32 speed) {
    Mtx view;
    Mtx44 projection;
    guVector camera = { carX * 0.55f, 5.0f, carZ + 9.5f };
    guVector up = { 0.0f, 1.0f, 0.0f };
    guVector target = { carX, 0.3f, carZ - 5.0f };

    guLookAt(view, &camera, &up, &target);
    guPerspective(projection, 55.0f,
        (f32)videoMode->fbWidth / (f32)videoMode->efbHeight, 0.1f, 250.0f);
    GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    const GXColor grass = { 55, 130, 55, 255 };
    const GXColor road = { 58, 62, 68, 255 };
    const GXColor line = { 245, 235, 175, 255 };
    const GXColor edge = { 230, 230, 230, 255 };
    const GXColor car = { 220, 35, 45, 255 };
    const GXColor glass = { 55, 175, 220, 255 };
    const GXColor tire = { 22, 22, 25, 255 };
    const GXColor lamp = { 250, 245, 190, 255 };

    /* Flat grass and road, extending around the moving car. */
    box(0.0f, -0.40f, carZ - 35.0f, 200.0f, 0.25f, 150.0f, grass);
    box(0.0f, -0.24f, carZ - 35.0f, 10.0f, 0.08f, 150.0f, road);
    box(-4.85f, -0.18f, carZ - 35.0f, 0.18f, 0.04f, 150.0f, edge);
    box( 4.85f, -0.18f, carZ - 35.0f, 0.18f, 0.04f, 150.0f, edge);

    for (int i = -100; i <= 30; i += 8) {
        box(0.0f, -0.17f, carZ + (f32)i, 0.12f, 0.025f, 3.0f, line);
    }

    /* Simple low-poly car built from boxes: no external models required. */
    box(carX, 0.35f, carZ, 1.75f, 0.42f, 3.25f, car);
    box(carX, 0.68f, carZ - 0.20f, 1.18f, 0.42f, 1.55f, glass);

    box(carX - 0.78f, 0.18f, carZ - 1.05f, 0.32f, 0.40f, 0.62f, tire);
    box(carX + 0.78f, 0.18f, carZ - 1.05f, 0.32f, 0.40f, 0.62f, tire);
    box(carX - 0.78f, 0.18f, carZ + 1.05f, 0.32f, 0.40f, 0.62f, tire);
    box(carX + 0.78f, 0.18f, carZ + 1.05f, 0.32f, 0.40f, 0.62f, tire);

    box(carX - 0.48f, 0.39f, carZ - 1.64f, 0.25f, 0.12f, 0.05f, lamp);
    box(carX + 0.48f, 0.39f, carZ - 1.64f, 0.25f, 0.12f, 0.05f, lamp);

    (void)speed;
}

int main(void) {
    VIDEO_Init();
    WPAD_Init();

    videoMode = VIDEO_GetPreferredMode(NULL);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
    VIDEO_Configure(videoMode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    setupGX();

    f32 carX = 0.0f;
    f32 carZ = 0.0f;
    f32 speed = 0.35f;

    while (1) {
        WPAD_ScanPads();
        u32 held = WPAD_ButtonsHeld(0);
        u32 down = WPAD_ButtonsDown(0);

        if (down & WPAD_BUTTON_HOME)
            break;

        if (held & WPAD_BUTTON_LEFT)  carX -= 0.075f;
        if (held & WPAD_BUTTON_RIGHT) carX += 0.075f;
        if (held & WPAD_BUTTON_A)     speed += 0.008f;
        if (held & WPAD_BUTTON_B)     speed -= 0.012f;

        if (speed < 0.15f) speed = 0.15f;
        if (speed > 1.25f) speed = 1.25f;
        if (carX < -3.4f) carX = -3.4f;
        if (carX >  3.4f) carX =  3.4f;

        carZ -= speed;

        GX_SetCopyClear((GXColor){ 90, 170, 230, 255 }, 0x00ffffff);
        drawScene(carX, carZ, speed);
        GX_DrawDone();
        GX_CopyDisp(framebuffer, GX_TRUE);
        VIDEO_SetNextFramebuffer(framebuffer);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    WPAD_Shutdown();
    return 0;
}

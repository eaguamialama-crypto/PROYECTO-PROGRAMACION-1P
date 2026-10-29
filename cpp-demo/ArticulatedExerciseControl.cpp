#include "ArticulatedExerciseControl.h"

#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <memory>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

namespace fitflex {
namespace {

constexpr wchar_t kControlClassName[] = L"FitFlexArticulatedExerciseControl";
constexpr float kPi = 3.14159265358979323846f;
constexpr float kUpperLegLength = 72.0f;
constexpr float kLowerLegLength = 72.0f;
constexpr float kTorsoLength = 82.0f;
constexpr float kUpperArmLength = 50.0f;
constexpr float kForearmLength = 48.0f;

Color MakeColor(BYTE red, BYTE green, BYTE blue) {
    return Color(255, red, green, blue);
}

PointF MoveFrom(PointF origin, float length, float angleDegrees) {
    const float radians = angleDegrees * kPi / 180.0f;
    return PointF(
        origin.X + std::sin(radians) * length,
        origin.Y + std::cos(radians) * length);
}

void DrawLimb(
    Graphics& graphics,
    PointF start,
    PointF end,
    float width,
    Color color) {
    Pen pen(color, width);
    pen.SetStartCap(LineCapRound);
    pen.SetEndCap(LineCapRound);
    pen.SetLineJoin(LineJoinRound);
    graphics.DrawLine(&pen, start, end);
}

void DrawJoint(Graphics& graphics, PointF center, float radius) {
    SolidBrush fill(MakeColor(236, 183, 151));
    Pen outline(MakeColor(206, 145, 117), 1.0f);
    graphics.FillEllipse(
        &fill, center.X - radius, center.Y - radius, radius * 2.0f, radius * 2.0f);
    graphics.DrawEllipse(
        &outline, center.X - radius, center.Y - radius, radius * 2.0f, radius * 2.0f);
}

void DrawShoe(Graphics& graphics, PointF ankle, float scale) {
    SolidBrush shoe(MakeColor(119, 119, 122));
    SolidBrush sole(Color::White);
    const float width = 25.0f * scale;
    const float height = 11.0f * scale;
    graphics.FillEllipse(
        &sole, ankle.X - width * 0.38f, ankle.Y - height * 0.25f,
        width * 1.15f, height * 0.7f);
    graphics.FillEllipse(
        &shoe, ankle.X - width * 0.30f, ankle.Y - height * 0.8f,
        width, height);
}

void DrawWoman(
    Graphics& graphics,
    PointF hip,
    float scale,
    const PoseAngles& pose,
    bool showWall) {
    SolidBrush hair(MakeColor(83, 49, 29));
    SolidBrush skin(MakeColor(243, 201, 168));
    SolidBrush top(MakeColor(223, 23, 136));
    Pen leggings(MakeColor(20, 20, 22), 15.0f * scale);
    Pen farLeg(MakeColor(43, 42, 44), 13.0f * scale);
    leggings.SetStartCap(LineCapRound);
    leggings.SetEndCap(LineCapRound);
    leggings.SetLineJoin(LineJoinRound);
    farLeg.SetStartCap(LineCapRound);
    farLeg.SetEndCap(LineCapRound);
    farLeg.SetLineJoin(LineJoinRound);

    const float radians = (180.0f - pose.torsoLean) * kPi / 180.0f;
    const PointF shoulder(
        hip.X + std::sin(radians) * kTorsoLength * scale,
        hip.Y + std::cos(radians) * kTorsoLength * scale);
    const PointF neck(
        shoulder.X + std::sin(radians) * 11.0f * scale,
        shoulder.Y + std::cos(radians) * 11.0f * scale);
    const PointF head(
        neck.X + 4.0f * scale,
        neck.Y - 12.0f * scale);

    if (showWall) {
        Pen wall(MakeColor(204, 164, 185), 3.0f * scale);
        graphics.DrawLine(
            &wall, PointF(hip.X + 120.0f * scale, shoulder.Y - 65.0f * scale),
            PointF(hip.X + 120.0f * scale, hip.Y + 45.0f * scale));
    }

    const float leftHipX = hip.X - 3.0f * scale;
    const float rightHipX = hip.X + 3.0f * scale;

    // Piernas lejanas primero para conservar el orden visual de profundidad.
    PointF leftKnee = MoveFrom(
        PointF(leftHipX, hip.Y), kUpperLegLength * scale, pose.leftThigh);
    PointF leftAnkle = MoveFrom(
        leftKnee, kLowerLegLength * scale, pose.leftShin);
    graphics.DrawLine(&farLeg, PointF(leftHipX, hip.Y), leftKnee);
    graphics.DrawLine(&farLeg, leftKnee, leftAnkle);
    DrawShoe(graphics, leftAnkle, scale);

    PointF rightKnee = MoveFrom(
        PointF(rightHipX, hip.Y), kUpperLegLength * scale, pose.rightThigh);
    PointF rightAnkle = MoveFrom(
        rightKnee, kLowerLegLength * scale, pose.rightShin);
    graphics.DrawLine(&leggings, PointF(rightHipX, hip.Y), rightKnee);
    graphics.DrawLine(&leggings, rightKnee, rightAnkle);
    DrawShoe(graphics, rightAnkle, scale);

    // Torso rosa: poligono con una ligera curva en hombros y cadera.
    const float halfWidth = 12.0f * scale;
    const float perpendicularX = -std::cos(radians) * halfWidth;
    const float perpendicularY = std::sin(radians) * halfWidth;
    PointF torsoPoints[] = {
        PointF(shoulder.X - perpendicularX, shoulder.Y - perpendicularY),
        PointF(shoulder.X + perpendicularX, shoulder.Y + perpendicularY),
        PointF(hip.X + perpendicularX * 0.72f, hip.Y + perpendicularY * 0.72f),
        PointF(hip.X - perpendicularX * 0.72f, hip.Y - perpendicularY * 0.72f)
    };
    graphics.FillPolygon(&top, torsoPoints, 4);

    // Brazos en posicion lateral. Cada codo y muneca se calcula a partir
    // de sus angulos para que las dos fases compartan la misma anatomia.
    const PointF leftShoulder(shoulder.X - 3.0f * scale, shoulder.Y + 3.0f * scale);
    const PointF rightShoulder(shoulder.X + 3.0f * scale, shoulder.Y + 5.0f * scale);
    PointF leftElbow = MoveFrom(
        leftShoulder, kUpperArmLength * scale, pose.leftUpperArm);
    PointF leftWrist = MoveFrom(
        leftElbow, kForearmLength * scale, pose.leftForearm);
    PointF rightElbow = MoveFrom(
        rightShoulder, kUpperArmLength * scale, pose.rightUpperArm);
    PointF rightWrist = MoveFrom(
        rightElbow, kForearmLength * scale, pose.rightForearm);

    DrawLimb(graphics, leftShoulder, leftElbow, 10.0f * scale, MakeColor(227, 168, 130));
    DrawLimb(graphics, leftElbow, leftWrist, 9.0f * scale, MakeColor(238, 185, 148));
    DrawLimb(graphics, rightShoulder, rightElbow, 11.0f * scale, MakeColor(231, 176, 138));
    DrawLimb(graphics, rightElbow, rightWrist, 10.0f * scale, MakeColor(244, 194, 157));

    // El cuello y la cabeza no incluyen rasgos faciales.
    DrawLimb(graphics, neck, shoulder, 8.0f * scale, MakeColor(232, 179, 143));
    graphics.FillEllipse(
        &skin, head.X - 14.0f * scale, head.Y - 17.0f * scale,
        29.0f * scale, 34.0f * scale);

    // Cabello recogido en una coleta, visto de perfil hacia la derecha.
    PointF ponytailPoints[] = {
        PointF(head.X - 7.0f * scale, head.Y - 12.0f * scale),
        PointF(head.X - 23.0f * scale, head.Y - 4.0f * scale),
        PointF(head.X - 26.0f * scale, head.Y + 26.0f * scale),
        PointF(head.X - 34.0f * scale, head.Y + 42.0f * scale),
        PointF(head.X - 14.0f * scale, head.Y + 32.0f * scale),
        PointF(head.X - 10.0f * scale, head.Y + 5.0f * scale)
    };
    graphics.FillPolygon(&hair, ponytailPoints, 6);
    graphics.FillEllipse(
        &hair, head.X - 15.0f * scale, head.Y - 19.0f * scale,
        27.0f * scale, 22.0f * scale);

    DrawJoint(graphics, leftKnee, 4.0f * scale);
    DrawJoint(graphics, rightKnee, 4.0f * scale);
    DrawJoint(graphics, leftElbow, 3.5f * scale);
    DrawJoint(graphics, rightElbow, 3.5f * scale);

}

ExerciseDefinition MakeSquat() {
    ExerciseDefinition exercise;
    exercise.title = L"Sentadillas";
    exercise.initialPose.torsoLean = 0.0f;
    exercise.initialPose.leftUpperArm = 92.0f;
    exercise.initialPose.leftForearm = 92.0f;
    exercise.initialPose.rightUpperArm = 92.0f;
    exercise.initialPose.rightForearm = 92.0f;
    exercise.initialPose.leftThigh = 5.0f;
    exercise.initialPose.leftShin = -5.0f;
    exercise.initialPose.rightThigh = 5.0f;
    exercise.initialPose.rightShin = -5.0f;

    exercise.movementPose = exercise.initialPose;
    exercise.movementPose.torsoLean = 17.0f;
    exercise.movementPose.leftThigh = 43.0f;
    exercise.movementPose.leftShin = -43.0f;
    exercise.movementPose.rightThigh = 43.0f;
    exercise.movementPose.rightShin = -43.0f;
    return exercise;
}

ExerciseDefinition MakeWallPushUp() {
    ExerciseDefinition exercise;
    exercise.title = L"Flexiones en pared";
    exercise.initialPose.torsoLean = 18.0f;
    exercise.initialPose.leftUpperArm = 92.0f;
    exercise.initialPose.leftForearm = 92.0f;
    exercise.initialPose.rightUpperArm = 92.0f;
    exercise.initialPose.rightForearm = 92.0f;
    exercise.initialPose.leftThigh = 2.0f;
    exercise.initialPose.leftShin = -2.0f;
    exercise.initialPose.rightThigh = 2.0f;
    exercise.initialPose.rightShin = -2.0f;

    exercise.movementPose = exercise.initialPose;
    exercise.movementPose.torsoLean = 22.0f;
    exercise.movementPose.rightUpperArm = 58.0f;
    exercise.movementPose.rightForearm = 135.0f;
    exercise.movementPose.leftUpperArm = 58.0f;
    exercise.movementPose.leftForearm = 135.0f;
    return exercise;
}

} // namespace

bool ArticulatedExerciseControl::RegisterWindowClass(HINSTANCE instance) {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kControlClassName;

    if (RegisterClassExW(&windowClass) != 0) {
        return true;
    }

    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

HWND ArticulatedExerciseControl::Create(
    HWND parent,
    HINSTANCE instance,
    int controlId,
    const RECT& bounds) {
    window_ = CreateWindowExW(
        0,
        kControlClassName,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        bounds.left,
        bounds.top,
        bounds.right - bounds.left,
        bounds.bottom - bounds.top,
        parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId)),
        instance,
        this);

    return window_;
}

void ArticulatedExerciseControl::SetExercise(ExerciseKind kind) {
    exercise_ = kind == ExerciseKind::Squat ? MakeSquat() : MakeWallPushUp();
    if (window_ != nullptr) {
        InvalidateRect(window_, nullptr, TRUE);
    }
}

void ArticulatedExerciseControl::SetExercise(const ExerciseDefinition& definition) {
    exercise_ = definition;
    if (window_ != nullptr) {
        InvalidateRect(window_, nullptr, TRUE);
    }
}

LRESULT CALLBACK ArticulatedExerciseControl::WindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    ArticulatedExerciseControl* control =
        reinterpret_cast<ArticulatedExerciseControl*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
        const auto* createData = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        control = static_cast<ArticulatedExerciseControl*>(createData->lpCreateParams);
        SetWindowLongPtrW(
            window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(control));
        control->window_ = window;
    }

    if (control != nullptr) {
        return control->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT ArticulatedExerciseControl::HandleMessage(
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC deviceContext = BeginPaint(window_, &paint);
        Paint(deviceContext, paint.rcPaint);
        EndPaint(window_, &paint);
        return 0;
    }
    default:
        return DefWindowProcW(window_, message, wParam, lParam);
    }
}

void ArticulatedExerciseControl::Paint(
    HDC deviceContext,
    const RECT& clientRect) const {
    (void)clientRect;
    RECT bounds{};
    GetClientRect(window_, &bounds);
    Graphics graphics(deviceContext);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    SolidBrush background(MakeColor(255, 224, 240));
    graphics.FillRectangle(
        &background,
        static_cast<REAL>(bounds.left),
        static_cast<REAL>(bounds.top),
        static_cast<REAL>(bounds.right - bounds.left),
        static_cast<REAL>(bounds.bottom - bounds.top));

    DrawExercise(
        graphics,
        static_cast<float>(bounds.right - bounds.left),
        static_cast<float>(bounds.bottom - bounds.top));
}

void ArticulatedExerciseControl::DrawExercise(
    Graphics& graphics,
    float width,
    float height) const {
    const float artWidth = std::min(width, height * 0.96f);
    const float artHeight = artWidth;
    const float left = (width - artWidth) * 0.5f;
    const float top = (height - artHeight) * 0.5f;

    GraphicsState state = graphics.Save();
    graphics.TranslateTransform(left, top);
    graphics.ScaleTransform(artWidth / 768.0f, artHeight / 768.0f);

    SolidBrush pinkBackground(MakeColor(255, 224, 240));
    graphics.FillRectangle(&pinkBackground, 0.0f, 0.0f, 768.0f, 768.0f);

    FontFamily family(L"Segoe UI");
    Font titleFont(&family, 42.0f, FontStyleBold, UnitPixel);
    SolidBrush titleBrush(Color::White);
    RectF titleRect(32.0f, 38.0f, 704.0f, 66.0f);
    StringFormat centered;
    centered.SetAlignment(StringAlignmentCenter);
    centered.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(
        exercise_.title.c_str(),
        static_cast<INT>(exercise_.title.size()),
        &titleFont,
        titleRect,
        &centered,
        &titleBrush);
    Pen floorPen(MakeColor(232, 166, 199), 2.0f);
    graphics.DrawLine(&floorPen, 28.0f, 585.0f, 356.0f, 585.0f);
    graphics.DrawLine(&floorPen, 412.0f, 585.0f, 740.0f, 585.0f);

    constexpr float scale = 1.35f;
    constexpr float leftHipX = 155.0f;
    constexpr float rightHipX = 535.0f;
    const float floorY = 585.0f;

    DrawPose(
        graphics, leftHipX, floorY, scale,
        exercise_.initialPose, exercise_.title == L"Flexiones en pared");
    DrawPose(
        graphics, rightHipX, floorY, scale,
        exercise_.movementPose, exercise_.title == L"Flexiones en pared");

    Pen arrowPen(MakeColor(215, 121, 167), 4.0f);
    arrowPen.SetStartCap(LineCapRound);
    arrowPen.SetEndCap(LineCapRound);
    arrowPen.SetLineJoin(LineJoinRound);
    graphics.DrawBezier(
        &arrowPen, PointF(346.0f, 320.0f), PointF(356.0f, 294.0f),
        PointF(388.0f, 294.0f), PointF(403.0f, 320.0f));
    graphics.DrawLine(&arrowPen, PointF(391.0f, 310.0f), PointF(403.0f, 320.0f));
    graphics.DrawLine(&arrowPen, PointF(403.0f, 320.0f), PointF(393.0f, 329.0f));

    Font labelFont(&family, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush labelBrush(MakeColor(133, 73, 106));
    RectF leftLabel(28.0f, 615.0f, 328.0f, 30.0f);
    RectF rightLabel(412.0f, 615.0f, 328.0f, 30.0f);
    graphics.DrawString(
        exercise_.initialLabel.c_str(),
        static_cast<INT>(exercise_.initialLabel.size()),
        &labelFont,
        leftLabel,
        &centered,
        &labelBrush);
    graphics.DrawString(
        exercise_.movementLabel.c_str(),
        static_cast<INT>(exercise_.movementLabel.size()),
        &labelFont,
        rightLabel,
        &centered,
        &labelBrush);

    graphics.Restore(state);
}

void ArticulatedExerciseControl::DrawPose(
    Graphics& graphics,
    float hipX,
    float floorY,
    float scale,
    const PoseAngles& pose,
    bool showWall) const {
    // Calcula la altura de la cadera desde los angulos de pierna: los pies
    // permanecen sobre el suelo al pasar de la postura inicial a la sentadilla.
    const float legReach =
        (kUpperLegLength * std::cos(pose.leftThigh * kPi / 180.0f) +
         kLowerLegLength * std::cos(pose.leftShin * kPi / 180.0f)) * scale;
    const PointF hip(hipX, floorY - legReach);
    DrawWoman(graphics, hip, scale, pose, showWall);
}

} // namespace fitflex

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>

namespace Gdiplus {
    class Graphics;
}

namespace fitflex {

// Angulos en grados. 0 apunta hacia abajo; 90 hacia la derecha; -90 hacia
// la izquierda. Los angulos de torso positivos inclinan el pecho hacia delante.
struct PoseAngles {
    float torsoLean = 0.0f;
    float leftUpperArm = 25.0f;
    float leftForearm = 25.0f;
    float rightUpperArm = 25.0f;
    float rightForearm = 25.0f;
    float leftThigh = 5.0f;
    float leftShin = -5.0f;
    float rightThigh = 5.0f;
    float rightShin = -5.0f;
};

struct ExerciseDefinition {
    std::wstring title;
    PoseAngles initialPose;
    PoseAngles movementPose;
    std::wstring initialLabel = L"1 · POSICION INICIAL";
    std::wstring movementLabel = L"2 · MOVIMIENTO";
};

enum class ExerciseKind {
    Squat,
    WallPushUp
};

class ArticulatedExerciseControl {
public:
    ArticulatedExerciseControl() = default;

    static bool RegisterWindowClass(HINSTANCE instance);

    HWND Create(
        HWND parent,
        HINSTANCE instance,
        int controlId,
        const RECT& bounds);

    void SetExercise(ExerciseKind kind);
    void SetExercise(const ExerciseDefinition& definition);

private:
    HWND window_ = nullptr;
    ExerciseDefinition exercise_;

    static LRESULT CALLBACK WindowProcedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    LRESULT HandleMessage(
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    void Paint(HDC deviceContext, const RECT& clientRect) const;
    void DrawExercise(Gdiplus::Graphics& graphics, float width, float height) const;
    void DrawPose(
        Gdiplus::Graphics& graphics,
        float hipX,
        float floorY,
        float scale,
        const PoseAngles& pose,
        bool showWall) const;
};

} // namespace fitflex

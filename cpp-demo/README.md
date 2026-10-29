# Control de ejercicios articulados en C++ y GDI+

Este ejemplo es una aplicacion Win32 nativa: no requiere C++/CLI ni WPF. El
control `ArticulatedExerciseControl` se puede agregar a una ventana existente
y dibuja dos poses laterales sobre un fondo rosa.

## Archivos

- `ArticulatedExerciseControl.h`: angulos, definicion de ejercicio y API del
  control reutilizable.
- `ArticulatedExerciseControl.cpp`: ventana hija y renderizado vectorial GDI+.
- `main.cpp`: aplicacion de demostracion. Presiona **Espacio** para alternar
  sentadilla y flexion contra la pared.
- `CMakeLists.txt`: configuracion para Visual Studio con CMake.

## Abrir y ejecutar en Visual Studio

1. Abre Visual Studio 2022 con la carga de trabajo **Desarrollo para el
   escritorio con C++**.
2. Selecciona **Archivo > Abrir > Carpeta** y elige esta carpeta `cpp-demo`.
   Visual Studio detectara `CMakeLists.txt`.
3. Selecciona el perfil x64 y compila/ejecuta `FitFlexFigure`.
4. En un proyecto Win32 existente, agrega `ArticulatedExerciseControl.cpp` y
   el encabezado. La aplicacion debe inicializar GDI+ antes de pintar y enlazar
   `gdiplus.lib` (el ejemplo ya lo hace).

## Incrustarlo en una ventana existente

Registra la clase del control una vez y crea una instancia despues de crear la
ventana padre:

```cpp
fitflex::ArticulatedExerciseControl figura;

fitflex::ArticulatedExerciseControl::RegisterWindowClass(hInstance);
RECT bounds{ 12, 12, 780, 780 };
figura.Create(hWndPadre, hInstance, 1001, bounds);
figura.SetExercise(fitflex::ExerciseKind::Squat);
```

Inicializa GDI+ en el arranque de la aplicacion y manten su token vivo hasta
destruir todas las ventanas que dibujan con GDI+:

```cpp
Gdiplus::GdiplusStartupInput input;
ULONG_PTR token = 0;
Gdiplus::GdiplusStartup(&token, &input, nullptr);
// Crear y ejecutar las ventanas.
Gdiplus::GdiplusShutdown(token);
```

## Ajustar una pose

Los angulos usan grados y se miden desde el eje vertical que apunta hacia
abajo: `0` es abajo, `90` a la derecha y `-90` a la izquierda. `torsoLean`
inclina el torso hacia delante. Los angulos de cada brazo, antebrazo, muslo y
pierna inferior se calculan independientemente; los segmentos se unen en el
codo y la rodilla. Por ejemplo:

```cpp
fitflex::ExerciseDefinition nueva;
nueva.title = L"Mi ejercicio";
nueva.initialPose.rightUpperArm = 25.0f;
nueva.initialPose.rightForearm = 40.0f;
nueva.movementPose = nueva.initialPose;
nueva.movementPose.torsoLean = 18.0f;
nueva.movementPose.rightUpperArm = 80.0f;
nueva.movementPose.rightForearm = 125.0f;
figura.SetExercise(nueva);
```

Cada fase calcula las articulaciones mediante trigonometria de segmentos
(`longitud * sin(angulo)`, `longitud * cos(angulo)`). La altura de la cadera
se obtiene de las longitudes y angulos de las piernas para mantener los pies
aproximadamente en el suelo durante la sentadilla.

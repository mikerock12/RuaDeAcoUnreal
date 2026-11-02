package com.smellsliketech.ruadeaco;

// Android supplies the surface, input and lifecycle to raylib's native backend.
// Simulation, menus, rendering and audio use the same C++ frontend as Windows.
public class MainActivity extends android.app.NativeActivity {
    static { System.loadLibrary("ruadeaco"); }
}

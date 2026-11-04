package com.smellsliketech.ruadeaco;

// Android supplies the surface, input and lifecycle to raylib's native backend.
// Simulation, menus, rendering and audio use the same C++ frontend as Windows.
public class MainActivity extends android.app.NativeActivity {
    static { System.loadLibrary("ruadeaco"); }

    private static native void nativeSetPaused(boolean paused);
    @Override protected void onPause(){nativeSetPaused(true);super.onPause();}
    @Override protected void onResume(){super.onResume();nativeSetPaused(false);}

    @Override public void onCreate(android.os.Bundle state) {
        // Configure the first surface before NativeActivity starts C++.
        setRequestedOrientation(android.content.pm.ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().getDecorView().setSystemUiVisibility(
            android.view.View.SYSTEM_UI_FLAG_FULLSCREEN | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        super.onCreate(state);
    }
    public void vibrateCombat() {
        android.os.Vibrator vibrator=(android.os.Vibrator)getSystemService(android.content.Context.VIBRATOR_SERVICE);
        if(vibrator!=null&&vibrator.hasVibrator())vibrator.vibrate(android.os.VibrationEffect.createOneShot(18,60));
    }
    public void openOnline(float master,float music,float effects,float opacity) { runOnUiThread(()->startActivity(new android.content.Intent(this,OnlineActivity.class).putExtra("master",master).putExtra("music",music).putExtra("effects",effects).putExtra("opacity",opacity))); }
}

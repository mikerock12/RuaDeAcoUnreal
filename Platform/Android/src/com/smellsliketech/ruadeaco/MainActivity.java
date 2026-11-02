package com.smellsliketech.ruadeaco;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import java.io.InputStream;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("ruadeaco");
    }

    public static native void nativeInitMatch(int p1Id, int p2Id, int modeId, int stageId);
    public static native void nativeStep(int p1Mask, int difficulty, float[] outState);

    private GameView gameView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        );
        gameView = new GameView(this);
        setContentView(gameView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (gameView != null) gameView.resume();
    }

    @Override
    protected void onPause() {
        if (gameView != null) gameView.pause();
        super.onPause();
    }

    private static final class GameView extends SurfaceView implements Runnable {
        private Thread loopThread;
        private volatile boolean running = false;
        private volatile int inputMask = 0;
        private int screenMode = 0; // 0 = menu/select, 1 = match
        private int selectedP1 = 0;
        private int selectedP2 = 1;
        private int selectedStage = 0;
        private int difficulty = 1;
        private final float[] state = new float[24];
        private final Paint paint = new Paint();

        private final String[] fighterNames = {
            "GUTO BARBA", "RAFA MARE", "NOIR REFLEXO", "ASTRO RISO", "DANTE SINAL", "LEO VIOLETA"
        };
        private final String[] fighterFolders = {
            "GutoBarba", "RafaMare", "NoirReflexo", "AstroRiso", "DanteSinal", "LeoVioleta"
        };
        private final String[] stageNames = {
            "CAIS DA CIDADE", "COZINHA MACABRA", "SITIO"
        };
        private final String[] stageFolders = {
            "CaisDaCidade", "CozinhaMacabra", "Sitio"
        };
        private final Bitmap[] stageBitmaps = new Bitmap[3];
        private final Bitmap[] idleBitmaps = new Bitmap[6];
        private final Bitmap[] attackBitmaps = new Bitmap[6];

        GameView(Context context) {
            super(context);
            paint.setAntiAlias(false);
            paint.setFilterBitmap(false);
            loadAssets(context);
        }

        private Bitmap loadAssetBitmap(Context context, String path) {
            try (InputStream is = context.getAssets().open(path)) {
                BitmapFactory.Options opts = new BitmapFactory.Options();
                opts.inScaled = false;
                return BitmapFactory.decodeStream(is, null, opts);
            } catch (Exception ignored) {
                return null;
            }
        }

        private void loadAssets(Context context) {
            for (int i = 0; i < 3; i++) {
                stageBitmaps[i] = loadAssetBitmap(context, "RuaDeAco/Art/Environments/" + stageFolders[i] + "/background.png");
            }
            for (int i = 0; i < 6; i++) {
                idleBitmaps[i] = loadAssetBitmap(context, "RuaDeAco/Art/Characters/" + fighterFolders[i] + "/idle.png");
                attackBitmaps[i] = loadAssetBitmap(context, "RuaDeAco/Art/Characters/" + fighterFolders[i] + "/standing-heavy.png");
            }
        }

        void resume() {
            running = true;
            loopThread = new Thread(this, "RuaDeAcoLoop");
            loopThread.start();
        }

        void pause() {
            running = false;
            if (loopThread != null) {
                try { loopThread.join(500); } catch (InterruptedException ignored) {}
                loopThread = null;
            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            int action = event.getActionMasked();
            int w = Math.max(1, getWidth());
            int h = Math.max(1, getHeight());

            if (screenMode == 0) {
                if (action == MotionEvent.ACTION_DOWN) {
                    float lx = event.getX() * (640f / w);
                    float ly = event.getY() * (360f / h);
                    if (ly >= 60 && ly <= 170) {
                        int idx = Math.min(5, Math.max(0, (int) ((lx - 20f) / 100f)));
                        selectedP1 = idx;
                        selectedP2 = (idx + 1) % 6;
                    } else if (ly >= 185 && ly <= 250) {
                        selectedStage = (selectedStage + 1) % 3;
                    } else if (ly >= 260) {
                        nativeInitMatch(selectedP1, selectedP2, 0, selectedStage);
                        screenMode = 1;
                    }
                }
                return true;
            }

            int mask = 0;
            if (action != MotionEvent.ACTION_UP && action != MotionEvent.ACTION_CANCEL) {
                int count = event.getPointerCount();
                for (int i = 0; i < count; i++) {
                    if (action == MotionEvent.ACTION_POINTER_UP && event.getActionIndex() == i) continue;
                    float lx = event.getX(i) * (640f / w);
                    float ly = event.getY(i) * (360f / h);

                    if (lx < 220 && ly > 180) {
                        float dx = lx - 84f;
                        float dy = ly - 284f;
                        if (dx < -14f) mask |= (1 << 0);
                        if (dx > 14f) mask |= (1 << 1);
                        if (dy < -18f) mask |= (1 << 2);
                        if (dy > 18f) mask |= (1 << 3);
                    } else {
                        if (dist2(lx, ly, 520f, 290f) <= 28f * 28f) mask |= (1 << 4); // Light
                        if (dist2(lx, ly, 566f, 254f) <= 28f * 28f) mask |= (1 << 5); // Heavy
                        if (dist2(lx, ly, 608f, 290f) <= 28f * 28f) mask |= (1 << 6); // Special
                        if (dist2(lx, ly, 566f, 324f) <= 28f * 28f) mask |= (1 << 7); // Block
                        if (dist2(lx, ly, 474f, 254f) <= 28f * 28f) mask |= (1 << 8); // Grab
                        if (dist2(lx, ly, 474f, 324f) <= 28f * 28f) mask |= (1 << 9); // Super
                        if (lx > 280f && lx < 360f && ly < 54f && action == MotionEvent.ACTION_DOWN) {
                            screenMode = 0;
                        }
                    }
                }
            }
            inputMask = mask;
            return true;
        }

        private float dist2(float x1, float y1, float x2, float y2) {
            float dx = x1 - x2;
            float dy = y1 - y2;
            return dx * dx + dy * dy;
        }

        @Override
        public void run() {
            SurfaceHolder holder = getHolder();
            while (running) {
                long startNs = System.nanoTime();
                if (screenMode == 1) {
                    nativeStep(inputMask, difficulty, state);
                }
                if (holder.getSurface().isValid()) {
                    Canvas canvas = holder.lockCanvas();
                    if (canvas != null) {
                        try {
                            drawLogical(canvas);
                        } finally {
                            holder.unlockCanvasAndPost(canvas);
                        }
                    }
                }
                long elapsedMs = (System.nanoTime() - startNs) / 1_000_000L;
                long sleepMs = Math.max(2L, 16L - elapsedMs);
                try { Thread.sleep(sleepMs); } catch (InterruptedException ignored) {}
            }
        }

        private void drawLogical(Canvas canvas) {
            int w = Math.max(1, canvas.getWidth());
            int h = Math.max(1, canvas.getHeight());
            canvas.save();
            canvas.scale(w / 640f, h / 360f);

            paint.setColor(Color.rgb(5, 7, 17));
            canvas.drawRect(0, 0, 640, 360, paint);

            Bitmap bg = stageBitmaps[selectedStage];
            if (bg != null) {
                canvas.drawBitmap(bg, null, new RectF(0, 0, 640, 360), paint);
            }

            if (screenMode == 0) {
                paint.setColor(Color.argb(200, 8, 12, 26));
                canvas.drawRect(0, 0, 640, 360, paint);
                paint.setColor(Color.rgb(41, 217, 255));
                paint.setTextSize(22f);
                canvas.drawText("RUA DE ACO — ANDROID NATIVO ARM64", 110, 38, paint);

                paint.setTextSize(12f);
                for (int i = 0; i < 6; i++) {
                    float bx = 20 + i * 100;
                    paint.setColor(selectedP1 == i ? Color.rgb(41, 217, 255) : Color.rgb(30, 42, 68));
                    canvas.drawRect(bx, 68, bx + 92, 162, paint);
                    paint.setColor(Color.WHITE);
                    canvas.drawText(fighterNames[i], bx + 4, 120, paint);
                }

                paint.setColor(Color.rgb(255, 213, 92));
                canvas.drawRect(140, 190, 500, 240, paint);
                paint.setColor(Color.rgb(12, 18, 36));
                canvas.drawText("ARENA: " + stageNames[selectedStage] + " (TOQUE PARA TROCAR)", 162, 220, paint);

                paint.setColor(Color.rgb(246, 64, 112));
                canvas.drawRect(180, 268, 460, 326, paint);
                paint.setColor(Color.WHITE);
                paint.setTextSize(18f);
                canvas.drawText("INICIAR LUTA (60 HZ)", 225, 304, paint);
            } else {
                drawFighter(canvas, selectedP1, state[7], state[8], (int) state[9], (int) state[10], (int) state[11]);
                drawFighter(canvas, selectedP2, state[16], state[17], (int) state[18], (int) state[19], (int) state[20]);

                // HUD
                paint.setColor(Color.argb(215, 12, 18, 36));
                canvas.drawRect(0, 0, 640, 50, paint);
                float r1 = state[13] > 0 ? state[12] / state[13] : 0f;
                float r2 = state[22] > 0 ? state[21] / state[22] : 0f;
                paint.setColor(Color.rgb(41, 217, 255));
                canvas.drawRect(20, 18, 20 + 220 * r1, 30, paint);
                paint.setColor(Color.rgb(246, 64, 112));
                canvas.drawRect(620 - 220 * r2, 18, 620, 30, paint);

                paint.setColor(Color.rgb(255, 213, 92));
                paint.setTextSize(16f);
                canvas.drawText(String.valueOf((int) state[4]), 308, 30, paint);

                // Touch controls
                paint.setColor(Color.argb(110, 41, 217, 255));
                canvas.drawCircle(84f, 284f, 46f, paint);
                drawBtn(canvas, 520f, 290f, "L", Color.rgb(41, 217, 255));
                drawBtn(canvas, 566f, 254f, "H", Color.rgb(246, 64, 112));
                drawBtn(canvas, 608f, 290f, "S", Color.rgb(255, 213, 92));
                drawBtn(canvas, 566f, 324f, "B", Color.rgb(92, 242, 155));
                drawBtn(canvas, 474f, 254f, "GRAB", Color.WHITE);
                drawBtn(canvas, 474f, 324f, "SUP", Color.rgb(228, 63, 90));
            }
            canvas.restore();
        }

        private void drawFighter(Canvas canvas, int fighterIdx, float x, float y, int facing, int anim, int stateFrame) {
            Bitmap sheet = (anim >= 9 && anim <= 16) ? attackBitmaps[fighterIdx] : idleBitmaps[fighterIdx];
            if (sheet == null) return;
            int frameH = Math.max(1, sheet.getHeight());
            int frames = Math.max(1, sheet.getWidth() / frameH);
            int frame = (stateFrame / 8) % frames;
            Rect src = new Rect(frame * frameH, 0, (frame + 1) * frameH, frameH);
            float size = 160f;
            canvas.save();
            canvas.translate(x, y - size + 18f);
            if (facing < 0) {
                canvas.scale(-1f, 1f);
            }
            canvas.drawBitmap(sheet, src, new RectF(-size * 0.5f, 0, size * 0.5f, size), paint);
            canvas.restore();
        }

        private void drawBtn(Canvas canvas, float cx, float cy, String label, int color) {
            paint.setColor(Color.argb(145, 12, 18, 36));
            canvas.drawCircle(cx, cy, 20f, paint);
            paint.setColor(color);
            paint.setTextSize(9f);
            canvas.drawText(label, cx - 8f, cy + 3f, paint);
        }
    }
}

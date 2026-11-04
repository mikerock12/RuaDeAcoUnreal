package com.smellsliketech.ruadeaco;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceError;
import android.webkit.WebSettings;
import android.webkit.WebChromeClient;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.graphics.Color;

/** The official client preserves room protocol, browser cross-play and current game rules. */
public final class OnlineActivity extends Activity {
    private static final String GAME="https://mikerock12.github.io/RuaDeAco/";
    private WebView web;
    private TextView state;
    private boolean preferencesApplied=false,pageError=false;
    private View fullScreenView;
    private WebChromeClient.CustomViewCallback fullScreenCallback;
    @Override public void onCreate(Bundle saved) {
        super.onCreate(saved);
        getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN|View.SYSTEM_UI_FLAG_HIDE_NAVIGATION|View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        LinearLayout content=new LinearLayout(this);content.setOrientation(LinearLayout.VERTICAL);content.setBackgroundColor(Color.BLACK);
        LinearLayout toolbar=new LinearLayout(this);Button back=new Button(this);back.setText("Voltar ao jogo offline");back.setOnClickListener(view->finish());toolbar.addView(back);
        state=new TextView(this);state.setTextColor(Color.WHITE);state.setText("Carregando online...");toolbar.addView(state,new LinearLayout.LayoutParams(0,-1,1));content.addView(toolbar);
        web=new WebView(this);content.addView(web,new LinearLayout.LayoutParams(-1,0,1));setContentView(content);
        WebSettings settings=web.getSettings();settings.setJavaScriptEnabled(true);settings.setDomStorageEnabled(true);
        settings.setAllowFileAccess(false);settings.setAllowContentAccess(false);settings.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);settings.setMediaPlaybackRequiresUserGesture(true);
        web.setWebChromeClient(new WebChromeClient(){
            @Override public void onShowCustomView(View view,CustomViewCallback callback){
                if(fullScreenView!=null){callback.onCustomViewHidden();return;}
                fullScreenView=view;fullScreenCallback=callback;
                addContentView(view,new android.view.ViewGroup.LayoutParams(-1,-1));
            }
            @Override public void onHideCustomView(){hideFullScreen();}
        });
        web.setWebViewClient(new WebViewClient(){
            @Override public boolean shouldOverrideUrlLoading(WebView view,WebResourceRequest request){return !request.getUrl().toString().startsWith(GAME);}
            @Override public void onPageStarted(WebView view,String url,android.graphics.Bitmap favicon){pageError=false;}
            @Override public void onPageFinished(WebView view,String url){
                if(pageError){state.setText("Falha de conexao. Volte e tente novamente.");web.setVisibility(View.VISIBLE);return;}
                if(!preferencesApplied&&url.startsWith(GAME)) {
                    preferencesApplied=true;
                    float master=getIntent().getFloatExtra("master",.85f),music=getIntent().getFloatExtra("music",.65f),effects=getIntent().getFloatExtra("effects",.85f),opacity=getIntent().getFloatExtra("opacity",.65f);
                    String script="(()=>{try{let s=JSON.parse(localStorage.getItem('rua-de-aco:settings:v1')||'{}');Object.assign(s,{masterVolume:"+master+",musicVolume:"+music+",effectsVolume:"+effects+",touchOpacity:"+opacity+",muted:"+(master==0)+"});localStorage.setItem('rua-de-aco:settings:v1',JSON.stringify(s));}catch(e){}})();";
                    view.evaluateJavascript(script,result->view.reload());return;
                }
                web.setVisibility(View.VISIBLE);state.setText("Escolha Online para criar ou entrar em uma sala");
            }
            @Override public void onReceivedError(WebView view,WebResourceRequest request,WebResourceError error){if(request.isForMainFrame()){pageError=true;state.setText("Falha de conexao. Volte e tente novamente.");}}
        });
        web.setVisibility(View.INVISIBLE);
        if(android.os.Build.VERSION.SDK_INT>=33)getOnBackInvokedDispatcher().registerOnBackInvokedCallback(android.window.OnBackInvokedDispatcher.PRIORITY_DEFAULT,this::back);
        web.loadUrl(GAME);
    }
    private void hideFullScreen(){if(fullScreenView==null)return;((android.view.ViewGroup)fullScreenView.getParent()).removeView(fullScreenView);fullScreenView=null;fullScreenCallback.onCustomViewHidden();fullScreenCallback=null;}
    private void back(){if(fullScreenView!=null)hideFullScreen();else finish();}
    @Override protected void onPause(){if(web!=null){web.onPause();web.pauseTimers();}super.onPause();}
    @Override protected void onResume(){super.onResume();if(web!=null){web.resumeTimers();web.onResume();}}
    @Override public void onBackPressed(){back();}
    @Override protected void onDestroy(){if(web!=null){web.stopLoading();web.destroy();web=null;}super.onDestroy();}
}

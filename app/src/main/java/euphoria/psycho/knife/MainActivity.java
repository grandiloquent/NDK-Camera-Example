package euphoria.psycho.knife;

import android.Manifest.permission;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build.VERSION;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("nativelib");
    }

    public boolean imagePreview(int[] bitmapArray, int w, int h) {
//        if (bitmapReady) {
//            bitmapReady = false;
//            Bitmap bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
//            bitmap.setPixels(bitmapArray, 0, w, 0, 0, w, h);
//            runOnUiThread(new Runnable() {
//                @Override
//                public void run() {
//                    mImageView.setImageBitmap(bitmap);
//                }
//            });
//            bitmapReady = true;
//        }
        return true;
    }

    private Handler mHandler = new Handler();


    private void initialize() {
        setContentView(R.layout.main);
        View rootView = findViewById(R.id.root_view);
        setSystemUiVisibility(rootView);
        Window win = getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        winParams.buttonBrightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_OFF;
        winParams.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        win.setAttributes(winParams);
        win.setBackgroundDrawable(null);
        getActionBar().hide();
        File dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "Images");
        if (!dir.isDirectory()) {
            dir.mkdirs();
        }
        openCamera(true, this);
        cameraPreview();
        rootView.setOnClickListener(v -> {
            //mHandler.removeCallbacks(null);
            if (mIsRunning) return;
            takePhoto();
            triggerScanImages();
        });
    }

    private void triggerScanImages() {
        Context context = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                File dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "Images");
                File[] files = dir.listFiles();
                MediaScannerConnection.scanFile(context,
                        Arrays.stream(files).map(i -> i.getAbsolutePath())
                                .toArray(String[]::new),
                        new String[]{"image/*"}, null);
            }
        }).start();
    }

    boolean mIsRunning = false;

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            if (mIsRunning) return true;
            takePhoto();
            mHandler.postDelayed(mTakePhoto, 5000);
            mIsRunning = true;
        } else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            finish();
        }
        return true;
    }

    private Runnable mTakePhoto = new Runnable() {
        @Override
        public void run() {
            takePhoto();
            mHandler.postDelayed(mTakePhoto, 5000);
        }
    };

    private void setSystemUiVisibility(View rootView) {
        rootView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        List<String> permissions = new ArrayList<>();
        if (checkSelfPermission(permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission.CAMERA);
            return;
        }
        if (VERSION.SDK_INT <= 28 && checkSelfPermission(permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission.WRITE_EXTERNAL_STORAGE);
        }
        if (permissions.size() > 0) {
            requestPermissions(permissions.toArray(new String[0]), 0);
            return;
        }
        initialize();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (Arrays.stream(grantResults).anyMatch(x -> x == PackageManager.PERMISSION_DENIED)) {
            Toast.makeText(this, "????????????????????????????????????", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        initialize();
    }

    native void openCamera(boolean isCameraBack, MainActivity mainActivity);

    native void cameraPreview();

    native void takePhoto();

    native void deleteCamera();

}
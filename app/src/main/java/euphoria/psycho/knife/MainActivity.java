package euphoria.psycho.knife;

import android.Manifest;
import android.Manifest.permission;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("nativelib");
    }

    native void openCamera(boolean isCameraBack, Surface surface);

    native void cameraPreview();

    native void takePhoto();

    native void deleteCamera();

    SurfaceView surfaceView;
    SurfaceHolder surfaceHolder;
    View mView;

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
            Toast.makeText(this, "缺少必要权限程序无法运行", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        initialize();
    }

    private void initialize() {
        setContentView(R.layout.main);
        surfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Display display = getWindowManager().getDefaultDisplay();
                int height = display.getMode().getPhysicalHeight();
                int width = display.getMode().getPhysicalWidth();
                //startCamera(holder.getSurface(), 1920, 1080);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }
        });
        mView = findViewById(R.id.view);
        File dir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        if (!dir.isDirectory()) {
            dir.mkdirs();
        }
        mView.setOnClickListener(v -> {
            takePhoto();
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
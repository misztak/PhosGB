package org.phos.phos;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class PhosActivity extends SDLActivity {
    private final String tag = getClass().getName();
    private final String[] joypadIds = {"RIGHT", "LEFT", "UP", "DOWN", "A", "B", "START", "SELECT"};
    private final String[] permissions = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.VIBRATE
    };

    private PhosActivity phosActivity;

    private Handler fileDialogHandler;
    private final Lock lock = new ReentrantLock();
    private final Condition selectedFileCondition = lock.newCondition();
    private String selectedFilePath;
    private boolean isFileSelected = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        phosActivity = this;

        boolean hasAllRequiredPermissions = false;
        while (!hasAllRequiredPermissions)
            hasAllRequiredPermissions = checkPermissions();

        // sorry
        int index = 0;
        for (final String idName : joypadIds) {
            String buttonId = "joypad_" + idName;
            int id = getResources().getIdentifier(buttonId, "id", getPackageName());
            Button button = findViewById(id);
            button.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View view, MotionEvent motionEvent) {
                    if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
                        Log.v(tag, "Pressed button " + idName);
                        // emulator.handleInputDown(index);
                    } else if (motionEvent.getAction() == MotionEvent.ACTION_UP) {
                        // emulator.handleInputUp(index);
                        Log.v(tag, "Released button " + idName);
                    }
                    return true;
                }
            });
            index++;
        }

        // file chooser
        final SimpleFileChooser.FileSelectedListener fsListener = new SimpleFileChooser.FileSelectedListener() {
            @Override
            public void onFileSelected(File file) {
                lock.lock();
                try {
                    selectedFilePath = file.getAbsolutePath();
                    isFileSelected = true;
                    selectedFileCondition.signal();
                } finally {
                    lock.unlock();
                }
            }
        };

        fileDialogHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                SimpleFileChooser fileChooser = new SimpleFileChooser(phosActivity, Environment.getExternalStorageDirectory(), fsListener);
                fileChooser.showDialog();
            }
        };

    }

    private boolean checkPermissions() {
        int result;
        List<String> requiredPermissions = new ArrayList<>();
        for (String p : permissions) {
            result = ContextCompat.checkSelfPermission(phosActivity, p);
            if (result != PackageManager.PERMISSION_GRANTED)
                requiredPermissions.add(p);
        }
        if (!requiredPermissions.isEmpty()) {
            ActivityCompat.requestPermissions(phosActivity,
                    requiredPermissions.toArray(new String[0]), 100);
            return false;
        }
        return true;
    }

    // this will be called by the main emulator function
    public String getFilePath() {
        isFileSelected = false;
        fileDialogHandler.sendMessage(new Message());

        lock.lock();
        try {
            while (!isFileSelected)
                selectedFileCondition.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            lock.unlock();
        }

        return selectedFilePath;
    }

}

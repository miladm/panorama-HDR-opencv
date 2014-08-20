package edu.stanford.cvgl.panohdr;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import edu.stanford.cvgl.panohdr.ImageProcessingTask.TaskType;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.Toast;

public class CameraActivity extends Activity implements CvCameraViewListener2, OnTouchListener
{

    private static final String TAG = "CameraActivity";

    private CameraView mCameraView;
    private TaskType mActiveTask;
    private List<String> mImagePaths;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this)
    {
        @Override
        public void onManagerConnected(int status)
        {
            if (status == LoaderCallbackInterface.SUCCESS)
            {
                System.loadLibrary("PanoHDR");
                mCameraView.enableView();
                mCameraView.setOnTouchListener(CameraActivity.this);
            } else
            {
                super.onManagerConnected(status);
            }
        }
    };
    
    private class UIImageProcTask extends ImageProcessingTask
    {
        UIImageProcTask(Context ctx, List<String> imagePaths, String outputImagePath, TaskType taskType)
        {
            super(ctx, imagePaths, outputImagePath, taskType);
        }
        
        @Override
        protected void onImageProcessingComplete(String outputImagePath)
        {
            File outputFile = new File(outputImagePath);
            if (outputFile.exists())
            {
                //Display the generated image.
                Intent intent = new Intent();
                intent.setAction(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.parse("file://"+outputImagePath), "image/*");
                startActivity(intent);
            }
            else
            {
                //Display error message
                Toast.makeText(CameraActivity.this,
                        "No output image was found!", Toast.LENGTH_LONG).show();
            }
            clearCapturedImages();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Setup our camera view
        setContentView(R.layout.camera_layout);
        mCameraView = (CameraView) findViewById(R.id.primary_camera_view);
        mCameraView.setVisibility(SurfaceView.VISIBLE);
        mCameraView.setCvCameraViewListener(this);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mImagePaths = new ArrayList<String>();
        mActiveTask = TaskType.PANORAMA_LIVE;

        // Setup Tabs
        final ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
        ActionBar.TabListener tabListener = new ActionBar.TabListener()
        {
            @Override
            public void onTabSelected(Tab tab, FragmentTransaction ft)
            {
                TaskType newTask = (TaskType) tab.getTag();
                if (newTask != mActiveTask)
                {
                    mActiveTask = (TaskType) tab.getTag();
                    clearCapturedImages();
                }
                handleTaskTrigger();
            }

            @Override
            public void onTabReselected(Tab tab, FragmentTransaction ft)
            {
                handleTaskTrigger();
            }

            @Override
            public void onTabUnselected(Tab tab, FragmentTransaction ft)
            {
            }
        };
        actionBar.addTab(actionBar.newTab().setText("Panorama").setTag(TaskType.PANORAMA_LIVE).setTabListener(tabListener));
        actionBar.addTab(actionBar.newTab().setText("Panorama Preset").setTag(TaskType.PANORAMA_PRESET).setTabListener(tabListener));
        actionBar.addTab(actionBar.newTab().setText("HDR Preset 1").setTag(TaskType.HDR_PRESET_1).setTabListener(tabListener));
        actionBar.addTab(actionBar.newTab().setText("HDR Preset 2").setTag(TaskType.HDR_PRESET_2).setTabListener(tabListener));
        
    }
    
    static private File getPictureStorageDir()
    {
        File picDir = new File(Environment.getExternalStorageDirectory(), "PanoHDR-Pictures");
        if (!picDir.exists())
        {
            try
            {
                picDir.mkdir();
            } catch (Exception e)
            {
                Log.e(TAG, "Encountered exception while creating picture dir.");
                e.printStackTrace();
            }
        }
        return picDir;
    }

    static private File makeUniqueImagePath()
    {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss.SSS", Locale.US);
        File picDir = getPictureStorageDir();
        String imageName = sdf.format(new Date()) + ".jpg";
        return new File(picDir, imageName);
    }
    
    private void executeWithTestImages(int [] resourceIds, String prefix)
    {        
        File picDir = getPictureStorageDir();
        List<String> inputImagePaths = new ArrayList<String>();
        for(int i=0; i<resourceIds.length; ++i)
        {
            try
            {
                File imgFile = new File(picDir, prefix+"-"+i+".jpg");
                InputStream is = getResources().openRawResource(resourceIds[i]);
                OutputStream os = new FileOutputStream(imgFile);
                byte[] data = new byte[is.available()];
                is.read(data);
                os.write(data);
                is.close();
                os.close();
                inputImagePaths.add(imgFile.getPath());
            }
            catch(IOException e)
            {
                Log.e(TAG, "Failed to write test image!");
            }
        }
        executeImageProcTask(inputImagePaths);
    }
    
    private void handleTaskTrigger()
    {
        if(mActiveTask==TaskType.HDR_PRESET_1)
        {
            //Image credits: Paul Debevec
            int [] testIds = { R.raw.mem_chu_1, R.raw.mem_chu_2 };
            executeWithTestImages(testIds, "HDR-MemChu");
        }
        else if(mActiveTask==TaskType.HDR_PRESET_2)
        {
            //Image credits: Jacques Joffre
            int [] testIds = { R.raw.gc_1, R.raw.gc_2 };
            executeWithTestImages(testIds, "HDR-MemChu");        	
        }
        else if(mActiveTask==TaskType.PANORAMA_PRESET)
        {
            int [] testIds = { R.raw.mountain1, R.raw.mountain2 };
            executeWithTestImages(testIds, "Pano-Mtn");
        }
    }

    private void takePicture()
    {
        File imageFile = makeUniqueImagePath();
        mCameraView.takePicture(imageFile.getPath());
        Toast.makeText(this, "Picture saved as " + imageFile.getName(), Toast.LENGTH_SHORT).show();
        mImagePaths.add(imageFile.getPath());
    }
    
    private void clearCapturedImages()
    {
        if (!mImagePaths.isEmpty())
        {
            mImagePaths.clear();
            Toast.makeText(CameraActivity.this, "Captured images cleared.", Toast.LENGTH_SHORT).show();
        }
    }

    private void checkCaptureStatus()
    {
        if (mImagePaths.size()!=mActiveTask.requiredImageCount)
        {
            // We're not yet ready to begin processing.
            return;
        }

        // We're done capturing images. Ask the user if we want to continue
        // with the captured pictures, or restart the process.
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Continue with these pictures?").setTitle("Stage 1 Complete");
        builder.setPositiveButton("Continue", new DialogInterface.OnClickListener()
        {
            public void onClick(DialogInterface dialog, int id)
            {
                executeImageProcTask(mImagePaths);
            }
        });
        builder.setNegativeButton("Restart", new DialogInterface.OnClickListener()
        {
            public void onClick(DialogInterface dialog, int id)
            {
                for (String imgPath : mImagePaths)
                {
                    File imgFile = new File(imgPath);
                    imgFile.delete();
                }
                clearCapturedImages();
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }
    
    private void executeImageProcTask(List<String> inputImagePaths)
    {
        String outputPath = makeUniqueImagePath().getPath();               
        UIImageProcTask task = new UIImageProcTask(CameraActivity.this,
                inputImagePaths, outputPath, mActiveTask);
        task.execute();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mCameraView != null)
        {
            mCameraView.disableView();
        }
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_5, this, mLoaderCallback);
    }

    public void onDestroy()
    {
        super.onDestroy();
        if (mCameraView != null)
        {
            mCameraView.disableView();
        }
    }

    @Override
    public boolean onTouch(View view, MotionEvent event)
    {
        // TODO: Add support for HDR (extra credit)
        if(mActiveTask==TaskType.PANORAMA_LIVE)
        {
            takePicture();
            checkCaptureStatus();
        }
        return false;
    }

    @Override
    public void onCameraViewStarted(int width, int height)
    {
        Toast.makeText(this, "Tap the screen to take a picture.", Toast.LENGTH_LONG).show();
    }

    @Override
    public void onCameraViewStopped()
    {
    }

    @Override
    public Mat onCameraFrame(CvCameraViewFrame inputFrame)
    {
        return inputFrame.rgba();
    }
}

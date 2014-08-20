package edu.stanford.cvgl.panohdr;

import java.util.List;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;

public class ImageProcessingTask extends AsyncTask<Void, Void, Void>
{
	
	private static final int NATIVE_OP_CODE_PANORAMA = 0;
	private static final int NATIVE_OP_CODE_HDR 	 = 1;
	
    public enum TaskType
    {
        PANORAMA_LIVE(NATIVE_OP_CODE_PANORAMA, "Panorama", 2),
        PANORAMA_PRESET(NATIVE_OP_CODE_PANORAMA, "Panorama from Preset", 2),
        HDR_PRESET_1(NATIVE_OP_CODE_HDR, "Memorial Church HDR", 2),
        HDR_PRESET_2(NATIVE_OP_CODE_HDR, "Grand Canal HDR", 2);
        
        public final int code;
        public final int requiredImageCount;
        public final String description;

        TaskType(int code, String desc, int minCount)
        {
        	this.code = code;
            this.requiredImageCount = minCount;
            this.description = desc;
        }
    }
    
    private List<String> mImagePaths;
    private String mOutputImagePath;
    private TaskType mTaskType;
    private ProgressDialog mProgressDialog;
    private Context mContext;
    
    ImageProcessingTask(Context ctx, List<String> imagePaths, String outputImagePath, TaskType taskType)
    {
        this.mContext = ctx;
        this.mImagePaths = imagePaths;
        this.mOutputImagePath = outputImagePath;
        this.mTaskType = taskType;
    }
    
    @Override
    protected void onPreExecute()
    {
        mProgressDialog = ProgressDialog.show(mContext,
                "Creating "+mTaskType.description,
                "Waiting for image processing to complete...");
        
    }
    
    @Override
    protected Void doInBackground(Void... params)
    {
        String[] inputPaths = mImagePaths.toArray(new String[mImagePaths.size()]);
        processImages(inputPaths, mOutputImagePath, mTaskType.code);     
        return null;
    }
    
    @Override
    protected void onPostExecute(Void result)
    {
        mProgressDialog.dismiss();
        onImageProcessingComplete(mOutputImagePath);
    }
    
    protected void onImageProcessingComplete(String outputImagePath)
    {        
    }
    
    native void processImages(String[] images, String outputPath, int opCode);
}

#include "PanoHDR.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

//In Panorama.cpp:
void CreatePanorama(vector<string> inputImagePaths, string outputImagePath);
bool CreateHDR(vector<string> inputImagePaths, string outputImagePath);

enum ImageOperation
{
    IMAGE_OP_PANORAMA = 0,
    IMAGE_OP_HDR = 1
};

static string convertToString(JNIEnv* env, jstring js)
{
    const char* stringChars = env->GetStringUTFChars(js, 0);
    string s = string(stringChars);
    env->ReleaseStringUTFChars(js, stringChars);
    return s;
}

JNIEXPORT void JNICALL Java_edu_stanford_cvgl_panohdr_ImageProcessingTask_processImages(JNIEnv* env,
        jobject, jobjectArray inputImages, jstring outputPath, jint opCode)
{
    string outputImagePath = convertToString(env, outputPath);
    vector<string> inputImagePaths;
    int imageCount = env->GetArrayLength(inputImages);
    for(int i = 0; i < imageCount; ++i)
    {
        jstring js = (jstring) (env->GetObjectArrayElement(inputImages, i));
        inputImagePaths.push_back(convertToString(env, js));
    }

    switch(opCode)
    {
        case IMAGE_OP_PANORAMA:
            CreatePanorama(inputImagePaths, outputImagePath);
            break;
        case IMAGE_OP_HDR:
            CreateHDR(inputImagePaths, outputImagePath);
            break;
        default:
            cerr << "Invalid operation code provided: " << opCode << endl;
            break;
    }
}

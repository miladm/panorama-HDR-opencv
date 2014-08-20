#include <jni.h>
/* Header for class edu_stanford_cvgl_panohdr_ImageProcessingTask */

#ifndef _Included_edu_stanford_cvgl_panohdr_ImageProcessingTask
#define _Included_edu_stanford_cvgl_panohdr_ImageProcessingTask
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     edu_stanford_cvgl_panohdr_ImageProcessingTask
 * Method:    processImages
 * Signature: ([Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_edu_stanford_cvgl_panohdr_ImageProcessingTask_processImages
  (JNIEnv *, jobject, jobjectArray, jstring, jint);

#ifdef __cplusplus
}
#endif
#endif

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_panayotis_jubler_preview_decoders_FFMPEG */

#ifndef _Included_com_panayotis_jubler_preview_decoders_FFMPEG
#define _Included_com_panayotis_jubler_preview_decoders_FFMPEG
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_panayotis_jubler_preview_decoders_FFMPEG
 * Method:    grabFrame
 * Signature: (Ljava/lang/String;JZ)[I
 */
JNIEXPORT jintArray JNICALL Java_com_panayotis_jubler_preview_decoders_FFMPEG_grabFrame
  (JNIEnv *, jobject, jstring, jlong, jboolean);

/*
 * Class:     com_panayotis_jubler_preview_decoders_FFMPEG
 * Method:    makeCache
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_panayotis_jubler_preview_decoders_FFMPEG_makeCache
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_panayotis_jubler_preview_decoders_FFMPEG
 * Method:    createClip
 * Signature: (Ljava/lang/String;Ljava/lang/String;JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_panayotis_jubler_preview_decoders_FFMPEG_createClip
  (JNIEnv *, jobject, jstring, jstring, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif

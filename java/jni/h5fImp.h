/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ncsa_hdf_hdf5lib_H5_H5F */

#ifndef _Included_hdf_hdf5lib_H5_H5F
#define _Included_hdf_hdf5lib_H5_H5F
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fopen
 * Signature: (Ljava/lang/String;IJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Fopen
  (JNIEnv*, jclass, jstring, jint, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fcreate
 * Signature: (Ljava/lang/String;IJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Fcreate
  (JNIEnv*, jclass, jstring, jint, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fflush
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fflush
  (JNIEnv*, jclass, jlong, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_name
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Fget_1name
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fis_hdf5
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Fis_1hdf5
  (JNIEnv*, jclass, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_create_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Fget_1create_1plist
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_access_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Fget_1access_1plist
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_intent
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fget_1intent
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Fclose
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fmount
 * Signature: (JLjava/lang/String;JJ)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fmount
  (JNIEnv*, jclass, jlong, jstring, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Funmount
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Funmount
  (JNIEnv*, jclass, jlong, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_freespace
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Fget_1freespace
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Freopen
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Freopen
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_obj_ids_long
 * Signature: (JIJ[J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Fget_1obj_1ids_1long
  (JNIEnv*, jclass, jlong, jint, jlong, jlongArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_obj_ids
 * Signature: (JII[J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fget_1obj_1ids
  (JNIEnv*, jclass, jlong, jint, jint, jlongArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_obj_count(hid_t file_id, unsigned int types )
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fget_1obj_1count
  (JNIEnv*, jclass, jlong, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_obj_count_long
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Fget_1obj_1count_1long
  (JNIEnv*, jclass, jlong, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_name
 * Signature: (JLjava/lang/String;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Fget_2name
  (JNIEnv*, jclass, jlong, jstring, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_filesize
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Fget_1filesize
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_mdc_hit_rate
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_hdf_hdf5lib_H5_H5Fget_1mdc_1hit_1rate
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Fget_mdc_size
 * Signature: (J[J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Fget_1mdc_1size
  (JNIEnv*, jclass, jlong, jlongArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Freset_mdc_hit_rate_stats
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Freset_1mdc_1hit_1rate_1stats
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5export_dataset
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5export_1dataset
  (JNIEnv*, jclass, jstring, jstring, jstring, jint);

#ifdef __cplusplus
}
#endif
#endif

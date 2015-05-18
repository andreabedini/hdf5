/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ncsa_hdf_hdf5lib_H5_H5D */

#ifndef _Included_ncsa_hdf_hdf5lib_H5_H5D
#define _Included_ncsa_hdf_hdf5lib_H5_H5D

#ifdef __cplusplus
extern "C" {
#endif

extern JavaVM *jvm;
extern jobject visit_callback;   

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dcreate
 * Signature: (JLjava/lang/String;JJJ)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dcreate
  (JNIEnv*, jclass, jlong, jstring, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dchdir_ext
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dchdir_1ext
  (JNIEnv*, jclass, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dgetdir_1ext
 * Signature: ([Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dgetdir_1ext
  (JNIEnv*, jclass, jobjectArray, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dopen
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dopen
  (JNIEnv*, jclass, jlong, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dget_space
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dget_1space
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dget_type
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dget_1type
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dget_create_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dget_1create_1plist
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread
 * Signature: (JJJJJ[BZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jbyteArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite
 * Signature: (JJJJJ[BZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jbyteArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dextend
 * Signature: (J[B)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dextend
  (JNIEnv*, jclass, jlong, jbyteArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dclose
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dget_storage_size
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dget_1storage_1size
  (JNIEnv*, jclass, jlong);

/*
 * Copies the content of one dataset to another dataset
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dcopy
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dcopy
  (JNIEnv*, jclass, jlong, jlong);

/*
 * Copies the content of one dataset to another dataset
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dvlen_get_buf_size
 * Signature: (JJJ[I)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dvlen_1get_1buf_1size
  (JNIEnv*, jclass, jlong, jlong, jlong, jintArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dvlen_reclaim
 * Signature: (JJJ[B)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dvlen_1reclaim
  (JNIEnv*, jclass, jlong, jlong, jlong, jbyteArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dget_space_status
 * Signature: (J[I)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dget_1space_1status
  (JNIEnv*, jclass, jlong, jintArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_short
 * Signature: (JJJJJ[SZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1short
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jshortArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_int
 * Signature: (JJJJJ[IZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1int
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jintArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_long
 * Signature: (JJJJJ[JZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1long
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jlongArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_float
 * Signature: (JJJJJ[FZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1float
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jfloatArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_double
 * Signature: (JJJJJ[DZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1double
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jdoubleArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_string
 * Signature: (JJJJJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1string
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite_short
 * Signature: (JJJJJ[SZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite_1short
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jshortArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite_int
 * Signature: (JJJJJ[IZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite_1int
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jintArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite_long
 * Signature: (JJJJJ[JZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite_1long
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jlongArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite_float
 * Signature: (JJJJJ[FZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite_1float
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jfloatArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dwrite_double
 * Signature: (JJJJJ[DZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dwrite_1double
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jdoubleArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5DwriteString
 * Signature: (JJJJJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5DwriteString
(JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5DwriteNotString
 * Signature: (JJJJJ[BZ)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5DwriteNotString
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jbyteArray, jboolean);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5DreadVL
 * Signature: (JJJJJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5DreadVL
(JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_reg_ref
 * Signature: (JJJJJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1reg_1ref
(JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dread_reg_ref_data
 * Signature: (JJJJJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dread_1reg_1ref_1data
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dcreate2
 * Signature: (JLjava/lang/String;JJJJJ)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dcreate2
  (JNIEnv*, jclass, jlong, jstring, jlong, jlong, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dopen2
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dopen2
  (JNIEnv*, jclass, jlong, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Dcreate_anon
 * Signature: (JJJJJ)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5__1H5Dcreate_1anon
  (JNIEnv*, jclass, jlong, jlong, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dget_space_status
 * Signature: (J)I;
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dget_1space_1status
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dget_access_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dget_1access_1plist
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dget_offset
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dget_1offset
  (JNIEnv*, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dvlen_get_buf_size_long
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dvlen_1get_1buf_1size_1long
  (JNIEnv*, jclass, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dfill
 * Signature: ([BJ[BJJ)V
 */
JNIEXPORT void JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dfill
  (JNIEnv*, jclass, jbyteArray, jlong, jbyteArray, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Dset_extent
 * Signature: (J[J)V
 */
JNIEXPORT void JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Dset_1extent
  (JNIEnv*, jclass, jlong, jlongArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Diterate
 * Signature: ([BJJLjava/lang/Object;Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_hdf5lib_H5_H5Diterate
  (JNIEnv*, jclass, jbyteArray, jlong, jlong, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif

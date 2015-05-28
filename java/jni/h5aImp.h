/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ncsa_hdf_hdf5lib_H5_H5A */

#ifndef _Included_hdf_hdf5lib_H5_H5A
#define _Included_hdf_hdf5lib_H5_H5A
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Acreate
 * Signature: (JLjava/lang/String;JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate
  (JNIEnv *, jclass, jlong, jstring, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aopen_name
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1name
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aopen_idx
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1idx
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Awrite
 * Signature: (JJ[B)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Awrite
  (JNIEnv *, jclass, jlong, jlong, jbyteArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5AwriteVL
 * Signature: (JJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5AwriteVL
  (JNIEnv *, jclass, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aread
 * Signature: (JJ[B)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aread
  (JNIEnv *, jclass, jlong, jlong, jbyteArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_space
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aget_1space
  (JNIEnv *, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_type
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aget_1type
  (JNIEnv *, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_name
 * Signature: (JJ[Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Aget_1name
  (JNIEnv *, jclass, jlong, jlong, jobjectArray);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_num_attrs
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aget_1num_1attrs
  (JNIEnv *, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Adelete
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Adelete
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Aclose
  (JNIEnv *, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5AreadVL
 * Signature: (JJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5AreadVL
  (JNIEnv *, jclass, jlong, jlong, jobjectArray);

/*
 * Copies the content of one dataset to another dataset
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Acopy
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Acopy
  (JNIEnv *, jclass, jlong, jlong);
/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Acreate2
 * Signature: (JLjava/lang/String;JJJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate2
(JNIEnv *, jclass, jlong, jstring, jlong, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Aopen
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen
  (JNIEnv *, jclass, jlong, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Aopen_by_idx
 * Signature: (JLjava/lang/String;IIJJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1by_1idx
  (JNIEnv *, jclass, jlong, jstring, jint, jint, jlong, jlong, jlong);

/*
* Class:     ncsa_hdf_hdf5lib_H5
* Method:    _H5Acreate_by_name
* Signature: (JLjava/lang/String;Ljava/lang/String;JJJJJ)J
*/
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate_1by_1name
(JNIEnv *, jclass, jlong, jstring, jstring, jlong, jlong, jlong, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aexists_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Aexists_1by_1name
  (JNIEnv *, jclass, jlong, jstring, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Arename
 * Signature: (JLjava/lang/String;Ljava/lang/String)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Arename
  (JNIEnv *, jclass, jlong, jstring, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Arename_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Arename_1by_1name
  (JNIEnv *, jclass, jlong, jstring, jstring, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_name_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Aget_1name_1by_1idx
  (JNIEnv *, jclass, jlong, jstring, jint, jint, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_storage_size
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Aget_1storage_1size
  (JNIEnv *, jclass, jlong);


/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_info
 * Signature: (J)Lncsa/hdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info
  (JNIEnv *, jclass, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_info_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Lncsa/hdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info_1by_1idx
  (JNIEnv *, jclass, jlong, jstring, jint, jint, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aget_info_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)Lncsa/hdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info_1by_1name
  (JNIEnv *, jclass, jlong, jstring, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Adelete_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Adelete_1by_1name
  (JNIEnv *, jclass, jlong, jstring, jstring, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Aexists
 * Signature: (JLjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Aexists
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    H5Adelete_by_idx
 * Signature: (JLjava/lang/String;IIJJ)V
 */
JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Adelete_1by_1idx
  (JNIEnv *, jclass, jlong, jstring, jint, jint, jlong, jlong);

/*
 * Class:     ncsa_hdf_hdf5lib_H5
 * Method:    _H5Aopen_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;JJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1by_1name
  (JNIEnv *, jclass, jlong, jstring, jstring, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif

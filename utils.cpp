/*****************************************************************************
 *  utils.cpp
 *****************************************************************************
 * Copyright © 2010-2013 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include <jni.h>

#define LOG_TAG "YXL/JNI/Util"
#include "log.h"

/** Unique Java VM instance, as defined in libvlcjni.c */
extern JavaVM *myVm;

jint getInt(JNIEnv *env, jobject thiz, const char* field)
{
    //jclass clazz = (*env)->GetObjectClass(env, thiz);
	jclass clazz = env->GetObjectClass(thiz);
    //jfieldID fieldMP = (*env)->GetFieldID(env, clazz,field, "I");
	jfieldID fieldMP = env->GetFieldID(clazz,field, "I");
    //return (*env)->GetIntField(env, thiz, fieldMP);
	return env->GetIntField(thiz, fieldMP);
}
void setInt(JNIEnv *env, jobject item, const char* field, jint value) {
    jclass cls;
    jfieldID fieldId;

    /* Get a reference to item's class */
    //cls = (*env)->GetObjectClass(env, item);
	cls = env->GetObjectClass(item);

    /* Look for the instance field s in cls */
    //fieldId = (*env)->GetFieldID(env, cls, field, "I");
	fieldId = env->GetFieldID(cls, field, "I");
    if (fieldId == NULL)
        return;

    //(*env)->SetIntField(env, item, fieldId, value);
	env->SetIntField(item, fieldId, value);
}

jlong getLong(JNIEnv *env, jobject thiz, const char* field) {
    //jclass clazz = (*env)->GetObjectClass(env, thiz);
	jclass clazz = env->GetObjectClass(thiz);
    //jfieldID fieldMP = (*env)->GetFieldID(env, clazz,field, "J");
	jfieldID fieldMP = env->GetFieldID(clazz,field, "J");
    //return (*env)->GetLongField(env, thiz, fieldMP);
	return env->GetLongField(thiz, fieldMP);
}
void setLong(JNIEnv *env, jobject item, const char* field, jlong value) {
    jclass cls;
    jfieldID fieldId;

    /* Get a reference to item's class */
    //cls = (*env)->GetObjectClass(env, item);
	cls = env->GetObjectClass(item);

    /* Look for the instance field s in cls */
    //fieldId = (*env)->GetFieldID(env, cls, field, "J");
	fieldId = env->GetFieldID(cls, field, "J");
    if (fieldId == NULL)
        return;

    //(*env)->SetLongField(env, item, fieldId, value);
	env->SetLongField(item, fieldId, value);
}

void setFloat(JNIEnv *env, jobject item, const char* field, jfloat value) {
    jclass cls;
    jfieldID fieldId;

    /* Get a reference to item's class */
    //cls = (*env)->GetObjectClass(env, item);
	cls = env->GetObjectClass(item);

    /* Look for the instance field s in cls */
    //fieldId = (*env)->GetFieldID(env, cls, field, "F");
	fieldId = env->GetFieldID(cls, field, "F");

    if (fieldId == NULL)
        return;

    //(*env)->SetFloatField(env, item, fieldId, value);
	env->SetFloatField(item, fieldId, value);
}

void setString(JNIEnv *env, jobject item, const char* field, const char* text)
{
    jclass cls;
    jfieldID fieldId;
    jstring jstr;

	cls = env->GetObjectClass(item);

	fieldId = env->GetFieldID(cls, field, "Ljava/lang/String;");

    if (fieldId == NULL)
        return;

	jstr = env->NewStringUTF(text);
    if (jstr == NULL)
        return;

	env->SetObjectField(item, fieldId, jstr);
}

jobject getEventHandlerReference(JNIEnv *env, jobject thiz, jobject eventHandler)
{
	jclass cls = env->GetObjectClass(eventHandler);
    if (!cls) {
        LOGE("setEventHandler: failed to get class reference");
        return NULL;
    }

	jmethodID methodID = env->GetMethodID(cls, "onJniNotifyEvent", "(I)V");
    if (!methodID) {
        LOGE("setEventHandler: failed to get the callback method");
        return NULL;
    }

	return env->NewGlobalRef(eventHandler);
}

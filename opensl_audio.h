//////////////////////////////////////////////////////////////////////////////////////////
//		Android apensl es 录音头文件 opensl_audio.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_OPENSL_ES_AUDIO_HEAD_FILE__
#define __BOWEN_HU_OPENSL_ES_AUDIO_HEAD_FILE__

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <queue>
#include <android/log.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "localdef.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////
const uint32_t N_MAX_INTERFACES = 3;
const uint32_t N_MAX_OUTPUT_DEVICES = 6;
const uint32_t N_MAX_INPUT_DEVICES = 3;

const uint32_t N_REC_SAMPLES_PER_SEC = 16000;  // Default fs
const uint32_t N_PLAY_SAMPLES_PER_SEC = 16000;  // Default fs

const uint32_t N_REC_CHANNELS = 1;
const uint32_t N_PLAY_CHANNELS = 1;

const uint32_t REC_BUF_SIZE_IN_SAMPLES = 320;
const uint32_t PLAY_BUF_SIZE_IN_SAMPLES = 320;

const uint32_t REC_MAX_TEMP_BUF_SIZE_PER_10ms =
    N_REC_CHANNELS * REC_BUF_SIZE_IN_SAMPLES * sizeof(int16_t);

const uint32_t PLAY_MAX_TEMP_BUF_SIZE_PER_10ms =
    N_PLAY_CHANNELS * PLAY_BUF_SIZE_IN_SAMPLES * sizeof(int16_t);

// Number of the buffers in playout queue
const uint16_t N_PLAY_QUEUE_BUFFERS = 8;
// Number of buffers in recording queue
// TODO(xian): Reduce the numbers of buffers to improve the latency.
const uint16_t N_REC_QUEUE_BUFFERS = 8;
// Some values returned from getMinBufferSize
// (Nexus S playout  72ms, recording 64ms)
// (Galaxy,         167ms,           44ms)
// (Nexus 7,         72ms,           48ms)
// (Xoom             92ms,           40ms)

#define REC_BUF_SIZE (N_REC_CHANNELS * sizeof(int16_t) * REC_BUF_SIZE_IN_SAMPLES)

typedef std::queue<int8_t*>		CAudioQueue;

#define AAC_ENCODE_SIZE				1024				//AAC编码大小
////////////////////////////////////////////////////////////////////////////////////////////

bool OnNotifyScreenCap(int nEventID);

#endif //__BOWEN_HU_OPENSL_ES_AUDIO_HEAD_FILE__

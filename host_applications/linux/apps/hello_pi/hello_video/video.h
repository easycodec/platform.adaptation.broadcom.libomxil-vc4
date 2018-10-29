#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "bcm_host.h"
#include "ilclient.h"
#include <tbm_type.h>
#include <tbm_surface.h>
#include <tbm_bufmgr.h>
#include "tsemaphore.h"

#define ALIGN(x, a)       (((x) + (a) - 1) & ~((a) - 1))

typedef struct appPrivateType{
  tsem_t* stateSem;
  tsem_t* decoderEventSem;
  tsem_t* eofSem;
  OMX_HANDLETYPE videodechandle;
  OMX_STATETYPE state;
} appPrivateType;

/* Callback prototypes for video decoder */
OMX_ERRORTYPE videodecEventHandler(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_EVENTTYPE eEvent,
  OMX_OUT OMX_U32 Data1,
  OMX_OUT OMX_U32 Data2,
  OMX_OUT OMX_PTR pEventData);

OMX_ERRORTYPE videodecEmptyBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE videodecFillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

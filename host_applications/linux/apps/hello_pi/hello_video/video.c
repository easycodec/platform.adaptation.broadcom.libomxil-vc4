/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video deocode demo using OpenMAX IL though the ilcient helper library

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "video.h"
/*
#define VERSIONMAJOR    1
#define VERSIONMINOR    1
#define VERSIONREVISION 0
#define VERSIONSTEP     0

#define OMX_VERSION ((VERSIONSTEP<<24) | (VERSIONREVISION<<16) | (VERSIONMINOR<<8) | VERSIONMAJOR)

#define OMX_INIT_STRUCTURE(a) \
memset(&(a), 0, sizeof(a)); \
(a).nSize = sizeof(a); \
(a).nVersion.nVersion = OMX_VERSION; \
(a).nVersion.s.nVersionMajor = VERSIONMAJOR; \
(a).nVersion.s.nVersionMinor = VERSIONMINOR; \
(a).nVersion.s.nRevision = VERSIONREVISION; \
(a).nVersion.s.nStep = VERSIONSTEP

static void setHeader(OMX_PTR header, OMX_U32 size) {
  OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
  *((OMX_U32*)header) = size;

  ver->s.nVersionMajor = VERSIONMAJOR;
  ver->s.nVersionMinor = VERSIONMINOR;
  ver->s.nRevision = VERSIONREVISION;
  ver->s.nStep = VERSIONSTEP;
}
*/

/*
void allocateBuffer(int index, int actualBuffer, int width, int height)
{
  int i, err;
  tbm_bo_handle handle_bo;
  int y_size, uv_size;

  y_size = ALIGN(width, 16) * ALIGN(height, 16);
  uv_size = y_size /2;

  for (i = 0; i < actualBuffer; i++) {

    unsigned char *tmp;

    //tmp = (MMVideoBuffer*) malloc(sizeof(MMVideoBuffer));
    //tmp->handle.bo[0] = tbm_bo_alloc(hBufmgr, y_size, TBM_BO_WC);
    handle_bo = tbm_bo_get_handle(tmp->handle.bo[0], TBM_DEVICE_CPU);
    tmp = handle_bo.ptr;
	//handle_bo = tbm_bo_get_handle(tmp->handle.bo[0], TBM_DEVICE_MM);
	//tmp->handle.dmabuf_fd[0] = handle_bo.u32;
	//tmp->size[0] = y_size;


	if (index == 1) {
	  tmp->handle.bo[1] = tbm_bo_alloc(hBufmgr, uv_size, TBM_BO_WC);
	  handle_bo = tbm_bo_get_handle(tmp->handle.bo[1], TBM_DEVICE_CPU);
	  tmp->data[1] = handle_bo.ptr;
	  handle_bo = tbm_bo_get_handle(tmp->handle.bo[1], TBM_DEVICE_MM);
	  tmp->handle.dmabuf_fd[1] = handle_bo.u32;
	  tmp->size[1] = uv_size;
	}

	if (index == 0) {
      err = OMX_UseBuffer(appPriv->videodechandle, &pInBuffer[i], 0, NULL, y_size, tmp);
      printf( "index : %d -> OMX_UseBuffer :%x\n", index, err);
      queue(pInBufQueue, pInBuffer[i]);
      //queue(pInBufQueue, tmp);
	} else if (index == 1) {
      err = OMX_UseBuffer(appPriv->videodechandle, &pOutBuffer[i], 1, NULL, y_size + uv_size, tmp);
      printf( "index : %d -> OMX_UseBuffer :%i\n", index, err);
      queue(pOutBufQueue, pOutBuffer[i]);
	}

  }

}
*/

#define COMPONENT_NAME_BASE "OMX.broadcom.video_decode"
#define BASE_ROLE "video_decoder.avc"

OMX_VIDEO_PARAM_PORTFORMATTYPE format;
OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
COMPONENT_T *video_decode = NULL;
COMPONENT_T *list[5];
TUNNEL_T tunnel[4];
ILCLIENT_T *client;
FILE *in;
int status = 0;
unsigned int data_len = 0;
int err;
int actualInBufferCount;
int actualOutBufferCount;
int nInBufferSize, nOutBufferSize;
int nInBufferAlignment, nOutBufferAlignment;
OMX_BUFFERHEADERTYPE *pInBuffer[20], *pOutBuffer[16];
OMX_STRING full_component_name;
appPrivateType* appPriv;
tbm_bufmgr hBufmgr;
int drm_fd;
queue_t* pInBufQueue;
queue_t* pOutBufQueue;

OMX_CALLBACKTYPE videodeccallbacks = {
    .EventHandler = videodecEventHandler,
    .EmptyBufferDone = videodecEmptyBufferDone,
    .FillBufferDone = videodecFillBufferDone
  };
/*
static void setHeader(OMX_PTR header, OMX_U32 size) {
  OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
  *((OMX_U32*)header) = size;

  ver->s.nVersionMajor = VERSIONMAJOR;
  ver->s.nVersionMinor = VERSIONMINOR;
  ver->s.nRevision = VERSIONREVISION;
  ver->s.nStep = VERSIONSTEP;
}
*/
OMX_ERRORTYPE test_OMX_ComponentNameEnum() {
  char * name;
  int index;

  OMX_ERRORTYPE err = OMX_ErrorNone;

  printf("GENERAL TEST %s\n", __func__);

  name = malloc(OMX_MAX_STRINGNAME_SIZE);
  index = 0;
  while(1) {
    err = OMX_ComponentNameEnum (name, OMX_MAX_STRINGNAME_SIZE, index);
    if ((name != NULL) && (err == OMX_ErrorNone)) {
      printf("component %i is %s\n", index, name);
    } else break;
    if (err != OMX_ErrorNone) break;
    index++;
  }
  free(name);
  name = NULL;
  printf("GENERAL TEST %s result %i\n", __func__, err);
  return err;
}

OMX_ERRORTYPE test_OMX_RoleEnum(OMX_STRING component_name) {
  OMX_U32 no_of_roles;
  OMX_U8 **string_of_roles;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  int index;

  printf("GENERAL TEST %s\n", __func__);
  printf("Getting roles of %s. Passing Null first...\n", component_name);
  err = OMX_GetRolesOfComponent(component_name, &no_of_roles, NULL);
  if (err != OMX_ErrorNone) {
    printf("Not able to retrieve the number of roles of the given component\n");
    printf("GENERAL TEST %s result %i\n", __func__, err);
    return err;
  }
  printf("The number of roles for the component %s is: %i\n", component_name, (int)no_of_roles);

  if(no_of_roles == 0) {
    printf("The Number or roles is 0.\nThe component selected is not correct for the purpose of this test.\nExiting...\n");
    err = OMX_ErrorInvalidComponentName;
  } else {
    string_of_roles = malloc(no_of_roles * sizeof(OMX_STRING));
    for (index = 0; index < no_of_roles; index++) {
      *(string_of_roles + index) = malloc(no_of_roles * OMX_MAX_STRINGNAME_SIZE);
    }
    printf("...then buffers\n");

    err = OMX_GetRolesOfComponent(component_name, &no_of_roles, string_of_roles);
    if (err != OMX_ErrorNone) {
      printf("Not able to retrieve the roles of the given component\n");
    } else if(string_of_roles != NULL) {
      for (index = 0; index < no_of_roles; index++) {
        printf("The role %i for the component:  %s \n", (index + 1), *(string_of_roles+index));
      }
    } else {
      printf("role string is NULL!!! Exiting...\n");
      err = OMX_ErrorInvalidComponentName;
    }
  }
  printf("GENERAL TEST %s result %i\n", __func__, err);
  return err;
}

OMX_ERRORTYPE test_OMX_ComponentEnumByRole(OMX_STRING role_name) {
  OMX_U32 no_of_comp_per_role;
  OMX_U8 **string_of_comp_per_role;
  OMX_ERRORTYPE err;
  int index;

  printf("GENERAL TEST %s\n", __func__);
  string_of_comp_per_role = malloc (10 * sizeof(OMX_STRING));
  for (index = 0; index < 10; index++) {
    string_of_comp_per_role[index] = malloc(OMX_MAX_STRINGNAME_SIZE);
  }

  printf("Getting number of components per role for %s\n", role_name);

  err = OMX_GetComponentsOfRole(role_name, &no_of_comp_per_role, NULL);
  if (err != OMX_ErrorNone) {
    printf("Not able to retrieve the number of components of a given role\n");
    printf( "GENERAL TEST %s result %i\n", __func__, err);
    return err;
  }
  printf("Number of components per role for %s is %i\n", role_name, (int)no_of_comp_per_role);

  err = OMX_GetComponentsOfRole(role_name, &no_of_comp_per_role, string_of_comp_per_role);
  if (err != OMX_ErrorNone) {
    printf("Not able to retrieve the components of a given role\n");
    printf( "GENERAL TEST %s result %i\n",__func__, err);
    return err;
  }

  printf(" The components are:\n");
  for (index = 0; index < no_of_comp_per_role; index++) {
    printf("%s\n", string_of_comp_per_role[index]);
  }
  for (index = 0; index<10; index++) {
    if(string_of_comp_per_role[index]) {
      free(string_of_comp_per_role[index]);
      string_of_comp_per_role[index] = NULL;
    }
  }

  if(string_of_comp_per_role)  {
    free(string_of_comp_per_role);
    string_of_comp_per_role = NULL;
  }
  printf("GENERAL TEST %s result OMX_ErrorNone\n", __func__);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE test_OpenClose(OMX_STRING component_name) {
  OMX_ERRORTYPE err = OMX_ErrorNone;

  printf("GENERAL TEST %s\n",__func__);
  err = OMX_GetHandle(&appPriv->videodechandle, component_name, NULL /*appPriv */, &videodeccallbacks);
  if(err != OMX_ErrorNone) {
    printf("No component found\n");
  } else {
    err = OMX_FreeHandle(appPriv->videodechandle);
  }
  printf("GENERAL TEST %s result %i\n", __func__, err);
  return err;
}

void allocateBuffer(int index, int actualBuffer, int width, int height, int aligned_size)
{
  int i, err;
  tbm_bo_handle handle_bo;
  int y_size, uv_size;
  int size;

  y_size = ALIGN(width + 128, 16) * ALIGN(height + 128, 16);
  uv_size = y_size /2;
  size = y_size + uv_size;

  for (i = 0; i < actualBuffer; i++) {

    void *tmp;
	unsigned char *buf;



	if (index == 130) {
		buf = (unsigned char*) malloc (sizeof(unsigned char) * aligned_size);
        err = OMX_UseBuffer(appPriv->videodechandle, &pInBuffer[i], 130, NULL, ALIGN(aligned_size, 16), buf);
	    printf("index : %d -> OMX_UseBuffer : %p, %x\n", index, pInBuffer[i], err);
		//queue(pInBufQueue, pInBuffer[i]);
	} else if (index == 131) {
        tmp = tbm_bo_alloc(hBufmgr, ALIGN(aligned_size, 16), TBM_BO_WC);
        handle_bo = tbm_bo_get_handle(tmp, TBM_DEVICE_CPU);
	    buf = handle_bo.ptr;

        err = OMX_UseBuffer(appPriv->videodechandle, &pOutBuffer[i], 131, NULL, ALIGN(aligned_size, 16), buf);
	    printf("index : %d -> OMX_UseBuffer : %p, %x\n", index, pOutBuffer[i], err);
		//queue(pOutBufQueue, pOutBuffer[i]);
	} else
		printf("invalid port\n");
/*

	if (index == 1) {
	  tmp->handle.bo[1] = tbm_bo_alloc(hBufmgr, uv_size, TBM_BO_WC);
	  handle_bo = tbm_bo_get_handle(tmp->handle.bo[1], TBM_DEVICE_CPU);
	  tmp->data[1] = handle_bo.ptr;
	  handle_bo = tbm_bo_get_handle(tmp->handle.bo[1], TBM_DEVICE_MM);
	  tmp->handle.dmabuf_fd[1] = handle_bo.u32;
	  tmp->size[1] = uv_size;
	}

	if (index == 0) {
      err = OMX_UseBuffer(appPriv->videodechandle, &pInBuffer[i], 0, NULL, y_size, tmp);
      DEBUG(DEFAULT_MESSAGES, "index : %d -> OMX_UseBuffer :%x\n", index, err);
      queue(pInBufQueue, pInBuffer[i]);
      //queue(pInBufQueue, tmp);
	} else if (index == 1) {
      err = OMX_UseBuffer(appPriv->videodechandle, &pOutBuffer[i], 1, NULL, y_size + uv_size, tmp);
      DEBUG(DEFAULT_MESSAGES, "index : %d -> OMX_UseBuffer :%i\n", index, err);
      queue(pOutBufQueue, pOutBuffer[i]);
	}
*/
  }

}
/*
void deallocateBuffer(int index, int actualBuffer)
{
  int i, err;
  MMVideoBuffer *pTmp;

  for (i = 0; i < actualBuffer; i++) {
    if (index == 0) {
		//pTmp = (MMVideoBuffer*)dequeue(pInBufQueue);
		pTmp = pInBuffer[i];
	} else if (index == 1) {
		//pTmp = (MMVideoBuffer*)dequeue(pOutBufQueue);
		pTmp = pOutBuffer[i];
	}

	tbm_bo_unref(pTmp->handle.bo[0]);
	if (index == 1)
		tbm_bo_unref(pTmp->handle.bo[1]);

	err = OMX_FreeBuffer (appPriv->videodechandle, index, pTmp);
    DEBUG(DEFAULT_MESSAGES, "%d port %d buffer free : %d\n", index, i, err);
  }
}
*/
const char * omx_state_to_string(OMX_STATETYPE state)
{
	switch (state)
	{
		case OMX_StateInvalid: return "Invalid"; break;
		case OMX_StateLoaded: return "Loaded"; break;
		case OMX_StateIdle: return "Idle"; break;
		case OMX_StateExecuting: return "Executing"; break;
		case OMX_StatePause: return "Pause"; break;
		case OMX_StateWaitForResources:  return "Wait for resources"; break;
		default: return "Bad state"; break;
	}
}

void wait_for(appPrivateType *app, OMX_STATETYPE state)
{
	while (app->state != state)
	{
		printf("wait for %s \n", omx_state_to_string(state));
		tsem_down(app->stateSem);
		if (app->state != state) {
			printf("we got an event, but still waiting for %s \n", omx_state_to_string(state));
		}

	}
}

unsigned int FromByteStream2NalUnit(FILE *ByteStream, unsigned char *Nal)
{
	unsigned int NalLeng=0;

	unsigned char Val, ZeroCount, i;
	ZeroCount = 0;
	if(feof(ByteStream))
		return 0;
	Val = fgetc(ByteStream);
	while(!Val)
	{
		if(ZeroCount == 3)
			break;
		ZeroCount++;
		Val = fgetc(ByteStream);
	}

	if( (ZeroCount != 3) || (Val != 1) )
	{
		for(i=0;i<ZeroCount;i++)
			Nal[NalLeng++] = 0;
		Nal[NalLeng++] = Val;
	}

	ZeroCount = 0;
	while(1) {
		Val = fgetc(ByteStream);
		if(feof(ByteStream))
			break;
		if(!Val)
			ZeroCount++;
		else {
			if( (ZeroCount == 3) && (Val == 1) )
				break;
			else {
				for(i=0;i<ZeroCount;i++)
					Nal[NalLeng++] = 0;
				Nal[NalLeng++] = Val;
				ZeroCount = 0;
			}
		}
	}

	return NalLeng;
}

void hex_dump(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	if (desc != NULL)
		printf("%s:\n", desc);

	for (i = 0; i < len; i++) {

		if ((i % 16) == 0) {
			if (i != 0)
				printf("  %s\n", buff);

			printf("  %04x ", i);
		}

		printf(" %02x", pc[i]);

		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}
	printf("  %s\n", buff);
}

static int video_decode_test(char *filename)
{
   int i;
   int offset;
   int data_read;
   int fd;

   memset(list, 0, sizeof(list));

   if((in = fopen(filename, "rb")) == NULL)
      return -2;
   if ((fd = open(filename, O_RDONLY)) == -1)
	   return -2;
/*
   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      return -3;
   }
*/
   /* Initialize application private data */
   appPriv = malloc(sizeof(appPrivateType));
   appPriv->decoderEventSem = malloc(sizeof(tsem_t));
   appPriv->stateSem =  malloc(sizeof(tsem_t));
   appPriv->state = OMX_StateInvalid;

   appPriv->eofSem = malloc(sizeof(tsem_t));
   tsem_init(appPriv->decoderEventSem, 0);
   tsem_init(appPriv->stateSem, 0);
   tsem_init(appPriv->eofSem, 0);

   printf("Init the OMX Core\n");

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      return -4;
   }
   printf("Omx core is initialized \n");

/*
   // create video_decode
   if (ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS) != 0)
      status = -14;
   list[0] = video_decode;
*/

   printf("------------------------------------\n");
   test_OMX_ComponentNameEnum();
   printf("------------------------------------\n");
   test_OMX_RoleEnum(COMPONENT_NAME_BASE);
   printf("------------------------------------\n");
   test_OMX_ComponentEnumByRole(BASE_ROLE);
   printf("------------------------------------\n");
   test_OpenClose(COMPONENT_NAME_BASE);
   printf("------------------------------------\n");

   full_component_name = malloc(sizeof(char*) * OMX_MAX_STRINGNAME_SIZE);
   strcpy(full_component_name, COMPONENT_NAME_BASE);
   printf("The component selected for decoding is %s\n", full_component_name);

   err = OMX_GetHandle(&appPriv->videodechandle, full_component_name, NULL, &videodeccallbacks);
   if(err != OMX_ErrorNone){
    printf("No video decoder component found. Exiting...\n");
    exit(1);
  } else {
    printf("Found The component for decoding is %s\n", full_component_name);
  }

   /** sending command to video decoder component to go to idle state */
   //pInBuffer1 = pInBuffer2 = NULL;
   err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandStateSet, OMX_StateIdle, NULL);

   //wait_for(appPriv, OMX_StateIdle);
   printf("state changed to Idle :%i\n", err);

   hBufmgr = tbm_bufmgr_init(drm_fd);
   if(hBufmgr == NULL){
  	 printf("TBM initialization failed\n");
   }


   OMX_PARAM_PORTDEFINITIONTYPE port_def;
   memset(&port_def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
   port_def.nVersion.nVersion = OMX_VERSION;
   port_def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
   port_def.nPortIndex = 130;

   err = OMX_GetParameter (appPriv->videodechandle, OMX_IndexParamPortDefinition, &port_def);
   printf( "OMX_GetParameter :%x, input buffer count : %d, nBufferSize : %d, nBufferAlignment : %d\n", err, port_def.nBufferCountActual, port_def.nBufferSize, port_def.nBufferAlignment);
   actualInBufferCount = port_def.nBufferCountActual;
   nInBufferSize = port_def.nBufferSize;
   nInBufferAlignment = port_def.nBufferAlignment;

   port_def.nPortIndex = 131;
   err = OMX_GetParameter (appPriv->videodechandle, OMX_IndexParamPortDefinition, &port_def);
   printf( "OMX_GetParameter :%x, output buffer count : %d, nBufferSize : %d, nBufferAlignment : %d\n", err, port_def.nBufferCountActual,  port_def.nBufferSize, port_def.nBufferAlignment);
   actualOutBufferCount = port_def.nBufferCountActual;
   nOutBufferSize = port_def.nBufferSize;
   nOutBufferAlignment = port_def.nBufferAlignment;


   err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortDisable, 130, NULL);
   printf( "Input port OMX_CommandPortDisable : %x\n", err);

   err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortDisable, 131, NULL);
   printf( "Output port OMX_CommandPortDisable : %x\n", err);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   err = OMX_SetParameter(appPriv->videodechandle, OMX_IndexParamVideoPortFormat, &format);
   printf("Input OMX_IndexParamVideoPortFormat: %x\n", err);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 131;
   err = OMX_SetParameter(appPriv->videodechandle, OMX_IndexParamVideoPortFormat, &format);
   printf("Output OMX_IndexParamVideoPortFormat: %x\n", err);


   pInBufQueue = calloc(1,sizeof(queue_t));
   pOutBufQueue = calloc(1,sizeof(queue_t));

   queue_init(pInBufQueue);
   queue_init(pOutBufQueue);

   //err = ilclient_change_component_state(video_decode, OMX_StateExecuting);
   //printf("ilclient_change_component_state -> OMX_StateExecuting : err : %d\n", err);

   err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 130, NULL);
   printf( "Input port OMX_CommandPortEnable : %x\n", err);
   err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 131, NULL);
   printf( "Output port OMX_CommandPortEnable : %x\n", err);

   allocateBuffer(130, actualInBufferCount, 1280, 720, nInBufferSize);
   allocateBuffer(131, actualOutBufferCount, 1280, 720, nOutBufferSize);

  wait_for(appPriv, OMX_StateIdle);

  /** sending command to video decoder component to go to executing state */
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
  printf("send Excuting :%d\n", err);

  wait_for(appPriv, OMX_StateExecuting);
  printf("Excuting state\n");

  for (i = 0; i < actualOutBufferCount; i++) {
    err = OMX_FillThisBuffer(appPriv->videodechandle, pOutBuffer[i]);
    printf( "FillThisBuffer : %p, %x\n",pOutBuffer[i], err);
  }

  for (i = 0; i < actualInBufferCount; i++) {
    //pthread_mutex_lock(&mutex);
	unsigned char *pTmp;
	offset = 0;

	pTmp = pInBuffer[i]->pBuffer;
	pTmp[offset++] = 0;
	pTmp[offset++] = 0;
	pTmp[offset++] = 0;
	pTmp[offset++] = 1;

	data_read = FromByteStream2NalUnit(in, pTmp + offset);

    pInBuffer[i]->nFilledLen = data_read + offset;
    pInBuffer[i]->nOffset = 0;
    hex_dump("src0", pInBuffer[i]->pBuffer, 16);

	//pthread_mutex_unlock(&mutex);

    err = OMX_EmptyThisBuffer(appPriv->videodechandle, pInBuffer[i]);
    printf("EmptyThisBuffer %x, %d\n", pInBuffer[i]->pBuffer, data_read + offset);
	usleep(100000);
  }

  tsem_down(appPriv->decoderEventSem);
  printf("port setting changed\n");

  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortDisable, 131, NULL);
  printf("Sending Port Disable Command : %d\n", err);
#if 0
   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0 &&
	  ilclient_enable_port_buffers(video_decode, 131, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf, *outbuf;
      int port_settings_changed = 0;
      int first_packet = 1;


     while ((outbuf = ilclient_get_output_buffer(video_decode, 131, 1)) != NULL)
	 {
		 unsigned char *tmp = outbuf->pBuffer;

		err = OMX_FillThisBuffer(ILC_GET_HANDLE(video_decode), outbuf);
		if (err != OMX_ErrorNone)
		{
		    printf("Failed to call OMX_FillThisBuffer\n");
			status = -6;
			break;
		}
		printf("call OMX_FillThisBuffer : %d\n", err);
	 }

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         data_len += fread(dest, 1, buf->nAllocLen-data_len, in);
         printf( "data_len :%i\n", data_len);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            //err = ilclient_change_component_state(video_decode, OMX_StateExecuting);
			//printf("ilclient_change_component_state -> OMX_StateExecuting : err : %d\n", err);
			printf("port setting changed \n");
            port_def.nPortIndex = 131;
            err = OMX_GetParameter (ILC_GET_HANDLE(video_decode), OMX_IndexParamPortDefinition, &port_def);
            printf( "OMX_GetParameter :%i\n", err);
            actualOutBufferCount = port_def.nBufferCountActual;
            printf("outbuf count :%d\n", actualOutBufferCount);
		 }

			if(!data_len)
				break;

			buf->nFilledLen = data_len;
			data_len = 0;

			buf->nOffset = 0;
			if(first_packet)
			{
				buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
				first_packet = 0;
			}
			else
				buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

			if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
			{
			    printf("Failed to call OMX_EmptyThisBuffer\n");
				status = -6;
				break;
			}
	  }
/*
      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         data_len += fread(dest, 1, buf->nAllocLen-data_len, in);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to video_render
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            ilclient_change_component_state(video_render, OMX_StateExecuting);
         }
         if(!data_len)
            break;

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // wait for EOS from render
      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
                              ILCLIENT_BUFFER_FLAG_EOS, -1);

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);
*/
   }
#endif
   fclose(in);

   //ilclient_disable_tunnel(tunnel);
   //ilclient_disable_tunnel(tunnel+1);
   //ilclient_disable_tunnel(tunnel+2);
   //ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   //ilclient_teardown_tunnels(tunnel);

   //ilclient_state_transition(list, OMX_StateIdle);
   //ilclient_state_transition(list, OMX_StateLoaded);

   //ilclient_cleanup_components(list);

   OMX_Deinit();

   //ilclient_destroy(client);
   return status;
}

int main (int argc, char **argv)
{
   if (argc < 2) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }
   bcm_host_init();
   return video_decode_test(argv[1]);
}


OMX_ERRORTYPE videodecEventHandler(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_EVENTTYPE eEvent,
  OMX_OUT OMX_U32 Data1,
  OMX_OUT OMX_U32 Data2,
  OMX_OUT OMX_PTR pEventData) {

  OMX_ERRORTYPE err = OMX_ErrorNone;

  printf("Hi there, I am in the %s callback\n", __func__);
  if(eEvent == OMX_EventCmdComplete) {
    if (Data1 == OMX_CommandStateSet) {
      appPriv->state = (int)Data2;
      printf( "State changed in ");
      switch ((int)Data2) {
        case OMX_StateInvalid:
          printf( "OMX_StateInvalid\n");
          break;
        case OMX_StateLoaded:
          printf( "OMX_StateLoaded\n");
          break;
        case OMX_StateIdle:
          printf( "OMX_StateIdle\n");
          break;
        case OMX_StateExecuting:
          printf( "OMX_StateExecuting\n");
          break;
        case OMX_StatePause:
          printf( "OMX_StatePause\n");
          break;
        case OMX_StateWaitForResources:
          printf( "OMX_StateWaitForResources\n");
          break;
		default:
          printf( "default\n");
		  break;

      }
      //tsem_up(appPriv->decoderEventSem);
	  tsem_up(appPriv->stateSem);
      printf( "tsem up decoderEventSem\n");
    } else if (OMX_CommandPortEnable || OMX_CommandPortDisable) {
      printf( "In %s Received Port Enable/Disable Event\n",__func__);
      tsem_up(appPriv->decoderEventSem);
      printf( "tsem up decoderEventSem\n");
    }
  } else if(eEvent == OMX_EventPortSettingsChanged) {
    printf( "\n port settings change event handler in %s %i\n", __func__, Data2);
    if(Data2 == 0) {

/*
      if(!flagSetupTunnel) {
        err = setPortParameters();
        printf("set port param %i\n",err);
        printf("tsem_up\n");
        tsem_up(appPriv->decoderEventSem);
        if(flagIsColorConvRequested == 1) {
          pOutBufferColorConv1 = pOutBufferColorConv2 = NULL;
          err = OMX_SendCommand(appPriv->colorconv_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
#if 1
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv1, 0, NULL, buffer_out_size, pOutBuffer1->pBuffer);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv2, 0, NULL, buffer_out_size, pOutBuffer2->pBuffer);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }
#else
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv1, 0, NULL, buffer_out_size, pOutBuffer1->data[0]);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv2, 0, NULL, buffer_out_size, pOutBuffer2->data[0]);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }
#endif
          omx_colorconvPortDefinition.nPortIndex = 1;
          setHeader(&omx_colorconvPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
          err = OMX_GetParameter(appPriv->colorconv_handle, OMX_IndexParamPortDefinition, &omx_colorconvPortDefinition);
          outbuf_colorconv_size = omx_colorconvPortDefinition.nBufferSize;
          printf( " outbuf_colorconv_size : %d \n", (int)outbuf_colorconv_size);

          err = OMX_AllocateBuffer(appPriv->colorconv_handle, &pOutBufferColorConv1, 1, NULL, outbuf_colorconv_size);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to allocate buffer in color conv\n");
            exit(1);
          }
          err = OMX_AllocateBuffer(appPriv->colorconv_handle, &pOutBufferColorConv2, 1, NULL, outbuf_colorconv_size);
          if(err != OMX_ErrorNone) {
            printf(DEB_LEV_ERR, "Unable to allocate buffer in colro conv\n");
            exit(1);
          }

          printf( "Before locking on idle wait semaphore\n");
          tsem_down(appPriv->colorconvEventSem);
          printf( "color conv Sem free\n");

        }
      }
	  */
    }
  } else if(eEvent == OMX_EventBufferFlag) {
    printf( "In %s OMX_BUFFERFLAG_EOS\n", __func__);
    if((int)Data2 == OMX_BUFFERFLAG_EOS) {
      tsem_up(appPriv->eofSem);
    }
  } else {
    printf( "Param1 is %x\n", (int)Data1);
    printf( "Param2 is %x\n", (int)Data2);
  }

  return err;
}

OMX_ERRORTYPE videodecEmptyBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {

  OMX_ERRORTYPE err;
/*
  int data_read;
  static int iBufferDropped=0;
  unsigned char *pTmp;
  int offset = 0;

  printf( "Hi there, I am in the %s callback.\n", __func__);

  pthread_mutex_lock(&mutex);

  pTmp = pBuffer->pBuffer;
  pTmp[offset++] = 0;
  pTmp[offset++] = 0;
  pTmp[offset++] = 0;
  pTmp[offset++] = 1;

  data_read = FromByteStream2NalUnit(fd, pTmp+offset);
  usleep(3000);
  pBuffer->nFilledLen = data_read+offset;
  pBuffer->nOffset = 0;
  pthread_mutex_unlock(&mutex);

  if (data_read <= 0) {
	  printf( "In the %s no more input data available\n", __func__);
	  iBufferDropped++;
	  if(iBufferDropped>=2) {
		  bEOS=OMX_TRUE;
		  return OMX_ErrorNone;
	  }
	  pBuffer->nFilledLen=0;
	  pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
	  memset(pBuffer->pBuffer, 0x0, 640*480);
	  err = OMX_EmptyThisBuffer(hComponent, pBuffer);
	  printf( "EmptyThisBuffer for EOS %d\n", err);
	  hex_dump("src2", pBuffer->pBuffer, 16);
	  return OMX_ErrorNone;
  }

  if(!bEOS) {
    printf( "EmptyThisBuffer %x, %d\n", (int)pBuffer, offset);
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
    hex_dump("src1", pBuffer->pBuffer, 16);
  } else {
    printf( "In %s Dropping Empty This buffer to Audio Dec\n", __func__);
  }
*/
  return OMX_ErrorNone;
}


OMX_ERRORTYPE videodecFillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {

  OMX_ERRORTYPE err;
/*
  OMX_STATETYPE eState;

  printf( "Hi there, I am in the %s callback.\n", __func__);
  if(pBuffer != NULL) {
    if(!bEOS) {
      if(flagIsColorConvRequested && (!flagSetupTunnel)) {
        OMX_GetState(appPriv->colorconv_handle,&eState);
        if(eState == OMX_StateExecuting || eState == OMX_StatePause) {
          if(pInBufferColorConv1->pBuffer == pBuffer->pBuffer) {
            pInBufferColorConv1->nFilledLen = pBuffer->nFilledLen;
            err = OMX_EmptyThisBuffer(appPriv->colorconv_handle, pInBufferColorConv1);
          } else {
            pInBufferColorConv2->nFilledLen = pBuffer->nFilledLen;
            err = OMX_EmptyThisBuffer(appPriv->colorconv_handle, pInBufferColorConv2);
          }
          if(err != OMX_ErrorNone) {
            printf( "In %s Error %08x Calling FillThisBuffer\n", __func__,err);
          }
        } else {
          err = OMX_FillThisBuffer(hComponent, pBuffer);
        }
      } else if((pBuffer->nFilledLen > 0) && (!flagSetupTunnel)) {
          printf( "nFilledLen :%d \n", pBuffer->nFilledLen);
          //fwrite(pBuffer->pBuffer, 1,  pBuffer->nFilledLen, outfile);
		  decoder_output_dump(pBuffer->pBuffer);
          pBuffer->nFilledLen = 0;
      } else {
          printf( "In %s Empty buffer in Else\n", __func__);
          printf( "nFilledLen :%d \n", pBuffer->nFilledLen);
      }
      if(pBuffer->nFlags == OMX_BUFFERFLAG_EOS) {
        printf( "In %s: eos=%x Calling Empty This Buffer\n", __func__, (int)pBuffer->nFlags);
        bEOS = OMX_TRUE;
      }
      if(!bEOS && !flagIsColorConvRequested && (!flagSetupTunnel)) {
        err = OMX_FillThisBuffer(hComponent, pBuffer);
        printf( "FillThisBuffer :%d \n", err);
      }
    } else {
      printf( "In %s: eos=%x Dropping Empty This Buffer\n", __func__,(int)pBuffer->nFlags);
    }
  } else {
    printf( "Ouch! In %s: had NULL buffer to output...\n", __func__);
  }
*/
  return OMX_ErrorNone;
}

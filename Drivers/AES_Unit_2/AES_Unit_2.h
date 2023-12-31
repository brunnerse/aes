
#ifndef AES_UNIT_2_H
#define AES_UNIT_2_H

#ifdef __cplusplus
extern "C" {
#endif

/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xscugic.h"

/**************************** Type Definitions *****************************/
#define BLOCK_SIZE 16

#define AES_BYTE_ORDER LITTLE_ENDIAN
#define AES_NUM_CHANNELS 15
#define AES_ADDR_REGISTER_BITS 7

#define AES_MAX_PRIORITY (AES_NUM_CHANNELS-1)

#define ERROR_NONE 0
#define ERROR_READ 1
#define ERROR_WRITE 2

typedef void (*AES_CallbackFn)(void* CallbackRef);

typedef enum {
    MODE_ENCRYPTION = 0,
    MODE_KEYEXPANSION = 1,
    MODE_DECRYPTION = 2,
    MODE_KEYEXPANSION_AND_DECRYPTION = 3
} Mode;

typedef enum {
    CHAINING_MODE_ECB = 0,
    CHAINING_MODE_CBC = 1,
    CHAINING_MODE_CTR = 2,
	CHAINING_MODE_GCM = 3
} ChainingMode;

typedef enum {
	GCM_PHASE_INIT = 0,
	GCM_PHASE_HEADER = 1,
	GCM_PHASE_PAYLOAD = 2,
	GCM_PHASE_FINAL = 3
} GCMPhase;


typedef struct {
	const u16 DeviceId;	 /**< Unique ID  of device */
	const u32 BaseAddress; /**< Base address of device (IPIF) */
    const u32 NumChannels;
} AES_Config;

typedef struct {
	UINTPTR BaseAddress;	/**< Device base address */
    AES_CallbackFn  CallbackFn[AES_NUM_CHANNELS];
    void *CallbackRef[AES_NUM_CHANNELS];
} AES;



/************************** Function Prototypes ****************************/
AES_Config *AES_LookupConfig(u16 DeviceId);
s32 AES_CfgInitialize(AES *InstancePtr, const AES_Config *ConfigPtr);

void AES_SetKey(AES *InstancePtr, u32 channel, u8 key[BLOCK_SIZE]);
void AES_SetDataParameters(AES* InstancePtr, u32 channel, volatile u8* source, volatile u8* dest, u32 size);
void AES_Setup(AES* InstancePtr, u32 channel, Mode mode, ChainingMode chMode, u32 enabled, GCMPhase gcmPhase);
void AES_SetMode(AES *InstancePtr, u32 channel, Mode mode);
void AES_SetChainingMode(AES* InstancePtr, u32 channel, ChainingMode chainMode);
void AES_SetGCMPhase(AES* InstancePtr, u32 channel, GCMPhase gcmPhase);
void AES_SetPriority(AES* InstancePtr, u32 channel, u32 priority);
void AES_SetInterruptEnabled(AES* InstancePtr, u32 channel, u32 en);

void AES_SetInterruptCallback(AES* InstancePtr, u32 channel, AES_CallbackFn callbackFn, void* callbackRef);

void AES_startComputation(AES* InstancePtr, u32 channel);

void AES_SetIV(AES* InstancePtr, u32 channel, u8 *IV, u32 IVLen);
void AES_SetSusp(AES* InstancePtr, u32 channel, u8 Susp[BLOCK_SIZE*2]);


void AES_GetKey(AES *InstancePtr, u32 channel, u8 outKey[BLOCK_SIZE]);
Mode AES_GetMode(AES *InstancePtr, u32 channel);
ChainingMode AES_GetChainingMode(AES* InstancePtr, u32 channel);
GCMPhase AES_GetGCMPhase(AES* InstancePtr, u32 channel);
u32 AES_GetPriority(AES* InstancePtr, u32 channel);
u32 AES_GetInterruptEnabled(AES* InstancePtr, u32 channel);

void AES_GetIV(AES* InstancePtr, u32 channel, u8 outIV[BLOCK_SIZE]);
void AES_GetSusp(AES* InstancePtr, u32 channel, u8 outSusp[BLOCK_SIZE*2]);

u32 AES_isActive(AES* InstancePtr, u32 channel);

void AES_PerformKeyExpansion(AES *InstancePtr, u32 channel);

// Process an entire data chunk at once instead of processing Blocks one by one
void AES_startComputationMode(AES* InstancePtr, u32 channel, ChainingMode chmode, int encrypt, u8* data, u8* outData, u32 size, u8 IV[BLOCK_SIZE], AES_CallbackFn callbackFn, void* callbackRef);
void AES_startComputationECB(AES* InstancePtr, u32 channel, int encrypt, u8* data, u8* outData, u32 size, AES_CallbackFn callbackFn, void* callbackRef);
void AES_startComputationCBC(AES* InstancePtr, u32 channel, int encrypt, u8* data, u8* outData, u32 size, u8 IV[BLOCK_SIZE], AES_CallbackFn callbackFn, void* callbackRef);
// encrypt is indifferent, as decryption and encryption is the same process
void AES_startComputationCTR(AES* InstancePtr, u32 channel, u8* data, u8* outData, u32 size, u8 IV[12], AES_CallbackFn callbackFn, void* callbackRef);
void AES_startComputationGCM(AES* InstancePtr, u32 channel, int encrypt, u8* header, u32 headerLen, u8* payload, u8* outProcessedPayload, u32 payloadLen, u8 IV[12], AES_CallbackFn callbackFn, void* callbackRef);
void AES_calculateTagGCM(AES* InstancePtr, u32 channel, u32 headerLen, u32 payloadLen, u8 outTag[BLOCK_SIZE]);

// Functions block until computation has completed
void AES_processDataECB(AES* InstancePtr, u32 channel, int encrypt, u8* data, u8* outData, u32 size);
void AES_processDataCBC(AES* InstancePtr, u32 channel, int encrypt, u8* data, u8* outData, u32 size, u8 IV[16]);
// encrypt is indifferent, as decryption and encryption is the same process
void AES_processDataCTR(AES* InstancePtr, u32 channel, u8* data, u8* outData, u32 size, u8 IV[12]);
void AES_processDataGCM(AES* InstancePtr, u32 channel, int encrypt, u8* header, u32 headerLen, u8* payload, u8* outProcessedPayload, u32 payloadLen, u8 IV[12], u8 outTag[BLOCK_SIZE]);

// Compares two tags. Returns 0 if identical, otherwise -1
int AES_compareTags(u8 tag1[BLOCK_SIZE], u8 tag2[BLOCK_SIZE]);


// returns 1 if most recent computation has completed, otherwise 0
int AES_isComputationCompleted(AES* InstancePtr, u32 channel);
// blocks until computation is completed
void AES_waitUntilCompleted(AES* InstancePtr, u32 channel);
u32 AES_GetError(AES* InstancePtr, u32 channel);

void AES_clearCompletedStatus(AES* InstancePtr, u32 channel);

void AES_IntrHandler(void *HandlerRef);



/**
 *
 * Write/Read 32 bit value to/from AES_INTERFACE_M user logic memory (BRAM).
 *
 * @param   Address is the memory address of the AES_INTERFACE_M device.
 * @param   Data is the value written to user logic memory.
 *
 * @return  The data from the user logic memory.
 *
 * @note
 * C-style signature:
 * 	void AES_INTERFACE_M_mWriteMemory(u32 Address, u32 Data)
 * 	u32 AES_INTERFACE_M_mReadMemory(u32 Address)
 *
 */
#define AES_mWriteMemory(Address, Data) \
    Xil_Out32(Address, (u32)(Data))
#define AES_mReadMemory(Address) \
    Xil_In32(Address)
    
// Simple wrappers for mWriteMemory and mReadMemory 
#define AES_Write(InstancePtr, channel, Offset, Data) AES_mWriteMemory((InstancePtr)->BaseAddress + ((channel)<<AES_ADDR_REGISTER_BITS) + (Offset), (u32)(Data))
#define AES_Read(InstancePtr, channel, Offset) AES_mReadMemory((InstancePtr)->BaseAddress + ((channel)<<AES_ADDR_REGISTER_BITS) + (Offset))   

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the AES_INTERFACE_Minstance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus AES_Mem_SelfTest(void * baseaddr_p);


#ifdef __cplusplus
}
#endif
#endif // AES_INTERFACE_M_H

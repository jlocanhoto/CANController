#ifndef FRAME_POSITIONS_H_INCLUDE
#define FRAME_POSITIONS_H_INCLUDE

#define SOF_POS             0
#define ID_A_POS            1
#define RTR_SSR_POS         12
#define IDE_POS             13
#define r0_POS              14
#define DLC_POS             15
#define DATA_FIELD_POS      19
#define ID_B_POS            14
#define RTR_EXT_POS         32
#define r1_POS              33
#define r0_EXT_POS          34
#define DLC_EXT_POS         35
#define DATA_FIELD_EXT_POS  39

#define CRC_DELIM_OFFSET    15
#define ACK_SLOT_OFFSET     16
#define ACK_DELIM_OFFSET    17
#define EOF_OFFSET          18

#define DLC_SIZE            4
#define CRC_SIZE            15
#define ID_A_SIZE           11
#define ID_B_SIZE           18
#define EOF_SIZE            7
#define IFS_SIZE            3

#define MAX_PAYLOAD_SIZE    64
#define MAX_PAYLOAD_BYTES   8
#define MAX_FRAME_SIZE      128

#endif
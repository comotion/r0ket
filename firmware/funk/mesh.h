#ifndef __MESH_H_
#define __MESH_H_

#define MESHBUFSIZE 10
// buffer:
#define MESH_PACKET_INDEX_TIME 0
#define MESH_PACKET_RANDOM 1 // XXX: getRandom()

#define MESHPKTSIZE 32
// MESH PACKET
// 01234567890123456789012345678901
// TGttttDDDDDDDDDDDDDDDDDDDDDDDDCC
// T: type
//  G: generation
//   tttt: time
//       DD...: data body
//            CC: crc

/* packet types:
 i: invaders high score
 T: current time broadcast
*/
#define M_SENDINT 500
#define M_RECVINT 1000
#define M_RECVTIM 100

#define MESH_CHANNEL 83
#define MESH_MAC     "MESHB"

#define MO_TYPE(x)       (x[0])
#define MO_TYPE_set(x,y) (x[0]=y)
#define MO_GEN(x)        (x[1])
#define MO_GEN_set(x,y)  (x[1]=y)
#define MO_TIME(x)       (uint8ptouint32(x+2))
#define MO_TIME_set(x,y) (uint32touint8p(y,x+2))
#define MO_BODY(x)       (x+6)

typedef struct {
    uint8_t pkt[32];
    char flags;
} MPKT;

#define MF_FREE (0)
#define MF_USED (1<<0)
#define MF_LOCK (1<<1)

extern char meshgen; // Generation
extern char meshincctr; // Time checker
extern char meshnice; // Time checker
extern char meshmsg; // Is there something interesting?
extern MPKT meshbuffer[MESHBUFSIZE];

void initMesh(void);
void mesh_cleanup(void);
void mesh_recvloop(void);
void mesh_sendloop(void);
void mesh_systick(void);
MPKT * meshGetMessage(uint8_t type);

#endif

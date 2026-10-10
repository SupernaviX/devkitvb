#ifndef __VB_H__
#define __VB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



// ////////////////////////////////// VIP ////////////////////////////////// //

// Column table entry
typedef struct VB_COL {
    volatile uint8_t length;
    volatile uint8_t repeat;
} VB_COL;

// Object
typedef struct VB_OBJ {
    volatile int16_t  jx;
    volatile uint16_t jp;  // Contains JLON, JRON
    volatile int16_t  jy;
    volatile uint16_t jca; // Contains JPLTS, JHFLP, JVFLP
} VB_OBJ;

// World
typedef struct VB_WORLD {
    volatile uint16_t attribs;
    volatile int16_t  gx;
    volatile int16_t  gp;
    volatile int16_t  gy;
    volatile int16_t  mx;
    volatile int16_t  mp;
    volatile int16_t  my;
    volatile int16_t  w;
    volatile int16_t  h;
    volatile uint16_t paramBase;
    volatile uint16_t overplane;
    volatile int16_t  padding[5];
} VB_WORLD;

// Memory tables
typedef volatile uint16_t VB_BGMAP [4096];
typedef volatile uint8_t  VB_CHR   [  16];
typedef          VB_COL   VB_COLTBL[ 256];

// Memory ranges
#define VB_FBL0   (*((volatile uint8_t (*)[0x6000])0x00000000))
#define VB_CHRS0  (*((         VB_CHR  (*)[   512])0x00006000))
#define VB_FBL1   (*((volatile uint8_t (*)[0x6000])0x00008000))
#define VB_CHRS1  (*((         VB_CHR  (*)[   512])0x0000E000))
#define VB_FBR0   (*((volatile uint8_t (*)[0x6000])0x00010000))
#define VB_CHRS2  (*((         VB_CHR  (*)[   512])0x00016000))
#define VB_FBR1   (*((volatile uint8_t (*)[0x6000])0x00018000))
#define VB_CHRS3  (*((         VB_CHR  (*)[   512])0x0001E000))
#define VB_BGS    (*((         VB_BGMAP(*)[    16])0x00020000))
#define VB_WORLDS (*((         VB_WORLD(*)[    32])0x0003D800))
#define VB_COLSL  (* (         VB_COLTBL*)         0x0003DC00 )
#define VB_COLSR  (* (         VB_COLTBL*)         0x0003DE00 )
#define VB_OBJS   (*((         VB_OBJ  (*)[  1024])0x0003E000))
#define VB_CHRS   (*((         VB_CHR  (*)[  2048])0x00078000))
#define VB_OAM    VB_OBJS

// I/O registers
#define VB_INTPND (*(volatile uint16_t *)0x0005F800)
#define VB_INTENB (*(volatile uint16_t *)0x0005F802)
#define VB_INTCLR (*(volatile uint16_t *)0x0005F804)
#define VB_DPSTTS (*(volatile uint16_t *)0x0005F820)
#define VB_DPCTRL (*(volatile uint16_t *)0x0005F822)
#define VB_BRTA   (*(volatile uint16_t *)0x0005F824)
#define VB_BRTB   (*(volatile uint16_t *)0x0005F826)
#define VB_BRTC   (*(volatile uint16_t *)0x0005F828)
#define VB_REST   (*(volatile uint16_t *)0x0005F82A)
#define VB_FRMCYC (*(volatile uint16_t *)0x0005F82E)
#define VB_CTA    (*(volatile uint16_t *)0x0005F830)
#define VB_XPSTTS (*(volatile uint16_t *)0x0005F840)
#define VB_XPCTRL (*(volatile uint16_t *)0x0005F842)
#define VB_VER    (*(volatile uint16_t *)0x0005F844)
#define VB_SPT0   (*(volatile uint16_t *)0x0005F848)
#define VB_SPT1   (*(volatile uint16_t *)0x0005F84A)
#define VB_SPT2   (*(volatile uint16_t *)0x0005F84C)
#define VB_SPT3   (*(volatile uint16_t *)0x0005F84E)
#define VB_GPLT0  (*(volatile uint16_t *)0x0005F860)
#define VB_GPLT1  (*(volatile uint16_t *)0x0005F862)
#define VB_GPLT2  (*(volatile uint16_t *)0x0005F864)
#define VB_GPLT3  (*(volatile uint16_t *)0x0005F866)
#define VB_JPLT0  (*(volatile uint16_t *)0x0005F868)
#define VB_JPLT1  (*(volatile uint16_t *)0x0005F86A)
#define VB_JPLT2  (*(volatile uint16_t *)0x0005F86C)
#define VB_JPLT3  (*(volatile uint16_t *)0x0005F86E)
#define VB_BKCOL  (*(volatile uint16_t *)0x0005F870)

// Macros
#define VB_ATTRIBS(lon,ron,bgm,scx,scy,over,bgMapBase) \
(   (((lon)&1)<<15)|(((ron)&1)<<14)|(((bgm)&3)<<12)|(((scx)&3)<<10)| \
    (((scy)&3)<<8)|(((over)&1)<<7)|((bgMapBase)&15)   )
#define VB_BGCELL(gplts,bhflp,bvflp,character) \
    ((((gplts)&3)<<14)|(((bhflp)&1)<<13)|(((bvflp)&1)<<12)|((character)&0x7FF))
#define VB_END 0x0040
#define VB_JCA(jplts,jhflp,jvflp,jca) \
    ((((jplts)&3)<<14)|(((jhflp)&1)<<13)|(((jvflp)&1)<<12)|((jca)&0x7FF))
#define VB_JP(jlon, jron, jp) \
    ((((jlon)&1)<<15)|(((jron)&1)<<14)|((jp)&0x3FF))
#define VB_PALETTE(a,b,c) \
    ((((a)&3)<<2)|(((b)&3)<<4)|(((c)&3)<<6))

// Display control/status
#define VB_LOCK    0x0400
#define VB_SYNCE   0x0200
#define VB_RE      0x0100
#define VB_FCLK    0x0080
#define VB_SCANRDY 0x0040
#define VB_R1BSY   0x0020
#define VB_L1BSY   0x0010
#define VB_R0BSY   0x0008
#define VB_L0BSY   0x0004
#define VB_DISP    0x0002
#define VB_DPRST   0x0001

// Drawing control/status
#define VB_SBOUT    0x8000
#define VB_OVERTIME 0x0010
#define VB_F1BSY    0x0008
#define VB_F0BSY    0x0004
#define VB_XPEN     0x0002
#define VB_XPRST    0x0001
#define VB_SBCMP(x) (((x)&31)<<8)
#define VB_SBCOUNT  ((VB_XPSTTS>>8)&31)

// Interrupt flags
#define VB_TIMEERR    0x8000
#define VB_XPEND      0x4000
#define VB_SBHIT      0x2000
#define VB_FRAMESTART 0x0010
#define VB_GAMESTART  0x0008
#define VB_RFBEND     0x0004
#define VB_LFBEND     0x0002
#define VB_SCANERR    0x0001

// World types
#define VB_NORMAL    0
#define VB_HBIAS     1
#define VB_AFFINE    2
#define VB_OBJ_WORLD 3



// ////////////////////////////////// VSU ////////////////////////////////// //

// Memory ranges
#define VB_WAVES (*(volatile uint32_t(*)[5][32])0x01000000)
#define VB_MOD   (*(volatile int32_t (*)   [32])0x01000280)
#define VB_WAVE1 (VB_WAVES[0])
#define VB_WAVE2 (VB_WAVES[1])
#define VB_WAVE3 (VB_WAVES[2])
#define VB_WAVE4 (VB_WAVES[3])
#define VB_WAVE5 (VB_WAVES[4])
#define VB_BYTE(memexpr) (*(uint8_t *)(memexpr))

// I/O registers
#define VB_S1INT (*(volatile uint8_t *)0x01000400)
#define VB_S1LRV (*(volatile uint8_t *)0x01000404)
#define VB_S1FQL (*(volatile uint8_t *)0x01000408)
#define VB_S1FQH (*(volatile uint8_t *)0x0100040C)
#define VB_S1EV0 (*(volatile uint8_t *)0x01000410)
#define VB_S1EV1 (*(volatile uint8_t *)0x01000414)
#define VB_S1RAM (*(volatile uint8_t *)0x01000418)
#define VB_S2INT (*(volatile uint8_t *)0x01000440)
#define VB_S2LRV (*(volatile uint8_t *)0x01000444)
#define VB_S2FQL (*(volatile uint8_t *)0x01000448)
#define VB_S2FQH (*(volatile uint8_t *)0x0100044C)
#define VB_S2EV0 (*(volatile uint8_t *)0x01000450)
#define VB_S2EV1 (*(volatile uint8_t *)0x01000454)
#define VB_S2RAM (*(volatile uint8_t *)0x01000458)
#define VB_S3INT (*(volatile uint8_t *)0x01000480)
#define VB_S3LRV (*(volatile uint8_t *)0x01000484)
#define VB_S3FQL (*(volatile uint8_t *)0x01000488)
#define VB_S3FQH (*(volatile uint8_t *)0x0100048C)
#define VB_S3EV0 (*(volatile uint8_t *)0x01000490)
#define VB_S3EV1 (*(volatile uint8_t *)0x01000494)
#define VB_S3RAM (*(volatile uint8_t *)0x01000498)
#define VB_S4INT (*(volatile uint8_t *)0x010004C0)
#define VB_S4LRV (*(volatile uint8_t *)0x010004C4)
#define VB_S4FQL (*(volatile uint8_t *)0x010004C8)
#define VB_S4FQH (*(volatile uint8_t *)0x010004CC)
#define VB_S4EV0 (*(volatile uint8_t *)0x010004D0)
#define VB_S4EV1 (*(volatile uint8_t *)0x010004D4)
#define VB_S4RAM (*(volatile uint8_t *)0x010004D8)
#define VB_S5INT (*(volatile uint8_t *)0x01000500)
#define VB_S5LRV (*(volatile uint8_t *)0x01000504)
#define VB_S5FQL (*(volatile uint8_t *)0x01000508)
#define VB_S5FQH (*(volatile uint8_t *)0x0100050C)
#define VB_S5EV0 (*(volatile uint8_t *)0x01000510)
#define VB_S5EV1 (*(volatile uint8_t *)0x01000514)
#define VB_S5RAM (*(volatile uint8_t *)0x01000518)
#define VB_S5SWP (*(volatile uint8_t *)0x0100051C)
#define VB_S6INT (*(volatile uint8_t *)0x01000540)
#define VB_S6LRV (*(volatile uint8_t *)0x01000544)
#define VB_S6FQL (*(volatile uint8_t *)0x01000548)
#define VB_S6FQH (*(volatile uint8_t *)0x0100054C)
#define VB_S6EV0 (*(volatile uint8_t *)0x01000550)
#define VB_S6EV1 (*(volatile uint8_t *)0x01000554)
#define VB_SSTOP (*(volatile uint8_t *)0x01000580)

// Macros
#define VB_SAMPLE(mem,index,value) (*(uint8_t*)&(mem)[index]=(value))
#define VB_SxINT(enb,auto_,interval) \
    ((((enb)&1)<<7)|(((auto_)&1)<<5)|((interval)&31))
#define VB_SxLRV(left,right) \
    ((((left)&15)<<4)|((right)&15))
#define VB_SxEV0(value,dir,interval) \
    ((((value)&15)<<4)|(((dir)&1)<<3)|((interval)&7))
#define VB_SxEV1(rep,enb) \
    ((((rep)&1)<<1)|((env)&1))
#define VB_S5EV1_VAL(enb1,rep1,func,rep2,enb2) \
   ((((enb1)&1)<<6)|(((rep1)&1)<<5)|(((func)&1)<<4)|(((rep2)&1)<<1)|((enb2)&1))
#define VB_S6EV1_VAL(tap,rep,enb) \
    ((((tap)&7)<<4)|(((rep)&1)<<1)|((enb)&1))
#define VB_SxSWP(clk,interval,dir,shift) \
    ((((clk)&1)<<7)|(((interval)&7)<<4)|(((dir)&1)<<3)|((shift)&7))



// /////////////////////////// Miscellaneous I/O /////////////////////////// //

// I/O registers
#define VB_CCR  (*(volatile uint8_t *)0x02000000)
#define VB_CCSR (*(volatile uint8_t *)0x02000004)
#define VB_CDTR (*(volatile uint8_t *)0x02000008)
#define VB_CDRR (*(volatile uint8_t *)0x0200000C)
#define VB_SDLR (*(volatile uint8_t *)0x02000010)
#define VB_SDHR (*(volatile uint8_t *)0x02000014)
#define VB_TLR  (*(volatile uint8_t *)0x02000018)
#define VB_THR  (*(volatile uint8_t *)0x0200001C)
#define VB_TCR  (*(volatile uint8_t *)0x02000020)
#define VB_WCR  (*(volatile uint8_t *)0x02000024)
#define VB_SCR  (*(volatile uint8_t *)0x02000028)

// CCR control/status
#define VB_CINTINH 0x80
#define VB_CCLKSEL 0x10
#define VB_CSTART  0x04
#define VB_CSTAT   0x02

// CCSR control/status
#define VB_CCINTINH 0x80
#define VB_CCINTLEV 0x10
#define VB_CCSIG    0x08
#define VB_CCSMP    0x04
#define VB_CCWR     0x02
#define VB_CCRD     0x01

// SCR control/status
#define VB_KINTINH 0x80
#define VB_PARASI  0x20
#define VB_SOFTCK  0x10
#define VB_HWSI    0x04
#define VB_SISTAT  0x02
#define VB_SABTDIS 0x01

// TCR control/status
#define VB_TCLKSEL  0x10
#define VB_TIMZINT  0x08
#define VB_ZSTATCLR 0x04
#define VB_ZSTAT    0x02
#define VB_TENB     0x01

// WCR control/status
#define VB_EXP1W 0x02
#define VB_ROM1W 0x01

// Game pad keys
#define VB_RD  0x8000
#define VB_RL  0x4000
#define VB_SEL 0x2000
#define VB_STA 0x1000
#define VB_LU  0x0800
#define VB_LD  0x0400
#define VB_LL  0x0200
#define VB_LR  0x0100
#define VB_RR  0x0080
#define VB_RU  0x0040
#define VB_LT  0x0020
#define VB_RT  0x0010
#define VB_B   0x0008
#define VB_A   0x0004
#define VB_SGN 0x0002
#define VB_PWR 0x0001



// ////////////////////////////// CPU Helpers ////////////////////////////// //

// Instruction proxies
extern void     _cli();
extern void     _halt();
extern void     _ldsr(int id, uint32_t value);
extern void     _sei();
extern uint32_t _stsr(int id);

// System register IDs
#define VB_ADTRE 25
#define VB_CHCW  24
#define VB_ECR    4
#define VB_EIPC   0
#define VB_EIPSW  1
#define VB_FEPC   2
#define VB_FEPSW  3
#define VB_PIR    6
#define VB_PSW    5
#define VB_TKCW   7

// CHCW bits
#define VB_ICE 0x00000002

// Interrupt service routine marker
#define VB_IRQ __attribute__((interrupt)) void



// ///////////////////////////// API Commands ////////////////////////////// //

extern void     vbInitColumnTable();
extern uint16_t vbReadGamePad();



#ifdef __cplusplus
}
#endif

#endif // __VB_H__

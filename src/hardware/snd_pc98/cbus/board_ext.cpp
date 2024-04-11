#include    "np2glue.h"
//#include	"compiler.h"
//#include	"pccore.h"
//#include	"iocore.h"
//#include	"cbuscore.h"
#include	"pcm86io.h"
#include	"sound.h"
#include	"fmboard.h"
#include	"s98.h"
#include    "hardware/c86ctl/c86ctlwrap.h"
#include    "board_ext.h"

c86ctl::IRealChip3* chipif = NULL;
REG8 chipid = 0;

static void IOOUTCALL opnac86ctl_o188(UINT port, REG8 dat) {

    opn.addr = dat;
    opn.data = dat;
    (void)port;
}

static void IOOUTCALL opnac86ctl_o18a(UINT port, REG8 dat) {

    opn.data = dat;
    UINT addr = opn.addr;
    if (addr >= 0x100) {
        return;
    }
    S98_put(NORMAL2608, addr, dat);
    if (chipif) chipif->out(addr, dat);

    if (0x20 < addr && addr < 0x30 && addr != 0x28) {
        fmtimer_setreg(addr, dat);
    }
    if (addr != 0x0e) {
        opn.reg[addr] = dat;
    }
}

static void IOOUTCALL opnac86ctl_o18c(UINT port, REG8 dat) {

    if (opn.extend) {
        opn.addr = dat + 0x100;
        opn.data = dat;
    }
    (void)port;
}

static void IOOUTCALL opnac86ctl_o18e(UINT port, REG8 dat) {

    if (!opn.extend) {
        return;
    }
    opn.data = dat;
    UINT addr = opn.addr - 0x100;
    if (addr >= 0x100) {
        return;
    }
    S98_put(EXTEND2608, addr, dat);
    if (chipif) chipif->out(opn.addr, dat);
    (void)port;
}

static REG8 IOINPCALL opnac86ctl_i188(UINT port) {

    (void)port;
    return(fmtimer.status);
}

static REG8 IOINPCALL opnac86ctl_i18a(UINT port) {

    UINT addr = opn.addr;
    //c86ctlの古いverではバグっていてinの返りがいつも0なので使えず。
    //uint8_t din = chipif->in(addr);

    REG8 value = opn.data;
    if (addr == 0x0e) {
        value =(fmboard_getjoy(&psg1));
    } else if (addr < 0x10) {
        value = opn.reg[addr];
    } else if (addr == 0xff) {
        value = (0); // ID
    }
    (void)port;
    return value;
}

static REG8 IOINPCALL opnac86ctl_i18c(UINT port) {

    if (opn.extend) {
        return((fmtimer.status & 3) | (opn.adpcmmask & 8));
    }
    (void)port;
    return(0xff);
}

static REG8 IOINPCALL opnac86ctl_i18e(UINT port) {

    if (opn.extend) {
        //uint8_t din = chipif->in(opn.addr);
        UINT addr = opn.addr - 0x100;
        if ((addr == 0x08) || (addr == 0x0f)) {
            return(opn.reg[addr + 0x100]);
        }
        return(opn.data);
    }
    (void)port;
    return(0xff);
}



static const IOOUT opnac86ctl_o[4] = {
            opnac86ctl_o188,	opnac86ctl_o18a,	opnac86ctl_o18c,	opnac86ctl_o18e };

static const IOINP opnac86ctl_i[4] = {
            opnac86ctl_i188,	opnac86ctl_i18a,	opnac86ctl_i18c,	opnac86ctl_i18e };


void board86_c86ctl_bind(void) {
    C86CTLWrap::GetInstance()->Initialize();
    C86CTLWrap::GetInstance()->QueryChipIF([](c86ctl::ChipType t) {
        c86ctl::ChipType tm = static_cast<c86ctl::ChipType>(t & 0xffff);
	    return tm == c86ctl::CHIP_YM2608 || tm == c86ctl::CHIP_OPN3L || tm == c86ctl::CHIP_YMF297;
    }, &chipif);
    chipid = 1;
    pcm86io_bind();
    cbuscore_attachsndex(0x188 + opn.base, opnac86ctl_o, opnac86ctl_i);
}

void board26_c86ctl_bind(void) {
    C86CTLWrap::GetInstance()->Initialize();
    C86CTLWrap::GetInstance()->QueryChipIF([](c86ctl::ChipType t) {
        c86ctl::ChipType tm = static_cast<c86ctl::ChipType>(t & 0xffff);
        return tm == c86ctl::CHIP_YM2203;
    }, &chipif);
    chipid = 0;
    cbuscore_attachsndex(0x88 + opn.base, opnac86ctl_o, opnac86ctl_i);
}

void board_c86ctl_reset(void) {
    if (chipif) chipif->reset();
}


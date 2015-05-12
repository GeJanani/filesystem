/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/sys/rump/librump/rumpdev/../../librump/rumpdev/MAINBUS.ioconf"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>

static const struct cfiattrdata mainbuscf_iattrdata = {
	"mainbus", 0, {
		{ NULL, NULL, 0 },
	}
};
static const struct cfiattrdata pcibuscf_iattrdata = {
	"pcibus", 1, {
		{ "bus", "-1", -1 },
	}
};

static const struct cfiattrdata * const mainbus_attrs[] = { &mainbuscf_iattrdata, &pcibuscf_iattrdata, NULL };
CFDRIVER_DECL(mainbus, DV_DULL, mainbus_attrs);


static struct cfdriver * const cfdriver_ioconf_mainbus[] = {
	&mainbus_cd,
	NULL
};

extern struct cfattach mainbus_ca;


#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_mainbus[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
/*  0: mainbus0 at root */
    { "mainbus",	"mainbus",	 0, NORM,    NULL,      0, NULL },
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};

static struct cfattach * const mainbus_cfattachinit[] = {
	&mainbus_ca, NULL
};

static const struct cfattachinit cfattach_ioconf_mainbus[] = {
	{ "mainbus", mainbus_cfattachinit },
	{ NULL, NULL }
};

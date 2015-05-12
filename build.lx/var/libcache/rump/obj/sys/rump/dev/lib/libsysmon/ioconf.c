/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/sys/rump/dev/lib/libsysmon/SYSMON.ioconf"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>


CFDRIVER_DECL(swwdog, DV_DULL, NULL);


static struct cfdriver * const cfdriver_ioconf_swwdog[] = {
	&swwdog_cd,
	NULL
};



#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_swwdog[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};


static const struct cfattachinit cfattach_ioconf_swwdog[] = {
	{ NULL, NULL }
};

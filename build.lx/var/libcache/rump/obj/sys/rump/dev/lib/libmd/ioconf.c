/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/sys/rump/dev/lib/libmd/MD.ioconf"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>


CFDRIVER_DECL(md, DV_DISK, NULL);


static struct cfdriver * const cfdriver_ioconf_md[] = {
	&md_cd,
	NULL
};



#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_md[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};


static const struct cfattachinit cfattach_ioconf_md[] = {
	{ NULL, NULL }
};

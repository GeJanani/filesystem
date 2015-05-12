/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/sys/rump/dev/lib/libaudio/AUDIO.ioconf"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>

static const struct cfiattrdata audiobuscf_iattrdata = {
	"audiobus", 0, {
		{ NULL, NULL, 0 },
	}
};

CFDRIVER_DECL(audio, DV_AUDIODEV, NULL);


static struct cfdriver * const cfdriver_ioconf_audio[] = {
	&audio_cd,
	NULL
};

extern struct cfattach audio_ca;

static const struct cfparent pspec0 = {
	"audiobus", NULL, 0
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_audio[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
/*  0: audio* at audiobus? */
    { "audio",		"audio",	 0, STAR,    NULL,      0, &pspec0 },
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};

static struct cfattach * const audio_cfattachinit[] = {
	&audio_ca, NULL
};

static const struct cfattachinit cfattach_ioconf_audio[] = {
	{ "audio", audio_cfattachinit },
	{ NULL, NULL }
};

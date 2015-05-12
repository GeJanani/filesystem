/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/sys/rump/dev/lib/libusb/USB.ioconf"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>

static const struct cfiattrdata usbififcf_iattrdata = {
	"usbifif", 6, {
		{ "port", "-1", -1 },
		{ "configuration", "-1", -1 },
		{ "interface", "-1", -1 },
		{ "vendor", "-1", -1 },
		{ "product", "-1", -1 },
		{ "release", "-1", -1 },
	}
};
static const struct cfiattrdata usbdevifcf_iattrdata = {
	"usbdevif", 6, {
		{ "port", "-1", -1 },
		{ "configuration", "-1", -1 },
		{ "interface", "-1", -1 },
		{ "vendor", "-1", -1 },
		{ "product", "-1", -1 },
		{ "release", "-1", -1 },
	}
};
static const struct cfiattrdata usbuscf_iattrdata = {
	"usbus", 0, {
		{ NULL, NULL, 0 },
	}
};
static const struct cfiattrdata usbroothubifcf_iattrdata = {
	"usbroothubif", 0, {
		{ NULL, NULL, 0 },
	}
};

static const struct cfiattrdata * const usb_attrs[] = { &usbroothubifcf_iattrdata, NULL };
CFDRIVER_DECL(usb, DV_DULL, usb_attrs);

static const struct cfiattrdata * const uhub_attrs[] = { &usbififcf_iattrdata, &usbdevifcf_iattrdata, NULL };
CFDRIVER_DECL(uhub, DV_DULL, uhub_attrs);


static struct cfdriver * const cfdriver_ioconf_usb[] = {
	&usb_cd,
	&uhub_cd,
	NULL
};

extern struct cfattach usb_ca;
extern struct cfattach uroothub_ca;

static const struct cfparent pspec0 = {
	"usbus", "ugenhc", DVUNIT_ANY
};
static const struct cfparent pspec1 = {
	"usbroothubif", "usb", DVUNIT_ANY
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_usb[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
/*  0: usb* at ugenhc? */
    { "usb",		"usb",		 0, STAR,    NULL,      0, &pspec0 },
/*  1: uhub* at usb? */
    { "uhub",		"uroothub",	 0, STAR,    NULL,      0, &pspec1 },
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};

static struct cfattach * const usb_cfattachinit[] = {
	&usb_ca, NULL
};
static struct cfattach * const uhub_cfattachinit[] = {
	&uroothub_ca, NULL
};

static const struct cfattachinit cfattach_ioconf_usb[] = {
	{ "usb", usb_cfattachinit },
	{ "uhub", uhub_cfattachinit },
	{ NULL, NULL }
};

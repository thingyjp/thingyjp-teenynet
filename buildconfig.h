#define DEVELOPMENT

#define DEBUG
#ifdef DEBUG
#define NLDEBUG // debugging for netlink
#define WSDEBUG // debugging for wpa_supplicant

#define PSDEBUG // debugging for packetsocket
//#define PSNOBIND // don't bind the packetsocket to the interface

#define D4MDEBUG
#endif

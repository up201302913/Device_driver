#ifndef _SERP_H
#define _SERP_H


#define SERP_MAJOR 0   	/* dynamic major by default */
#define SERP_MINOR 4    /* serp0 through serp3 */
#define ADR_COM1 0x3f8	/* COM1 address */


/*
 * The different configurable parameters
 */
struct serp_devs {

	struct cdev serp_cdevs; // struct cdev for this serp device

	int cnt; // number of characters written to device

};


#endif

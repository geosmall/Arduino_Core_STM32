#include "example-selftest.h"

/* board driver */
#include "common.h"

/* --------------------------------------------------------------------------------------
 *  Static and extern variables
 * -------------------------------------------------------------------------------------- */

/* Just a handy variable to handle the icm426xx object */
static struct inv_icm426xx icm_driver;

/* --------------------------------------------------------------------------------------
 *  Functions definition
 * -------------------------------------------------------------------------------------- */

int SetupInvDevice(struct inv_icm426xx_serif *icm_serif)
{
	int     rc = 0;
	uint8_t who_am_i;

	/* Initialize device */
	INV_MSG(INV_MSG_LEVEL_INFO, "Initialize Icm426xx");
}
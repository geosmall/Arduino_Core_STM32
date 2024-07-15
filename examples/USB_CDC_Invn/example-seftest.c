#include "example-selftest.h"

#include "src/Icm426xx/Icm426xxDefs.h"
#include "src/Icm426xx/Icm426xxDriver_HL.h"
#include "src/Icm426xx/Icm426xxSelfTest.h"

/* InvenSense drivers and utils */
// #include "Invn/EmbUtils/Message.h"

/* board driver */
// #include "common.h"

/* --------------------------------------------------------------------------------------
 *  Static and extern variables
 * -------------------------------------------------------------------------------------- */

/* Just a handy variable to handle the icm426xx object */
static struct inv_icm426xx icm_driver;
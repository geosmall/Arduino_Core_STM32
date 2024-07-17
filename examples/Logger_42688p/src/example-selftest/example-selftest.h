#ifndef _EXAMPLE_SELFTEST_H_
#define _EXAMPLE_SELFTEST_H_

#include <stdint.h>
#include "src/Icm426xx/Icm426xxTransport.h"
#include "src/Icm426xx/Icm426xxDefs.h"
#include "src/Icm426xx/Icm426xxDriver_HL.h"
#include "src/Icm426xx/Icm426xxSelfTest.h"

/**
 * \brief This function is in charge of reseting and initializing Icm426xx device. It should
 * be succesfully executed before any access to Icm426xx device.
 *
 */
int SetupInvDevice(struct inv_icm426xx_serif *icm_serif);

/*!
 * \brief Run Self Test on Invensense device
 */
void RunSelfTest(void);

/*!
 * \brief Get Bias values calculated from selftest
 */
void GetBias(void);

#endif /* !_EXAMPLE_SELFTEST_H_ */
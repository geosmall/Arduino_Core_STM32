/**
 * @file    atomic.h
 * @brief   Priority-based atomic sections using ARM Cortex-M BASEPRI register
 *
 * OVERVIEW
 * --------
 * This header provides ATOMIC_BLOCK() for creating critical sections that block
 * interrupts at or below a specified priority level. Unlike __disable_irq() which
 * globally disables all interrupts, BASEPRI allows higher-priority interrupts to
 * still execute, reducing worst-case interrupt latency.
 *
 * HOW IT WORKS
 * ------------
 * ARM Cortex-M processors have a BASEPRI register that masks interrupts with
 * priority values >= BASEPRI. Priority 0 is highest, so:
 *   - BASEPRI = 0: No masking (all interrupts enabled)
 *   - BASEPRI = 16: Blocks priorities 16-255, allows 0-15
 *   - NVIC_PRIO_MAX (0 shifted): Blocks all maskable interrupts
 *
 * The ATOMIC_BLOCK macro uses GCC's __cleanup__ attribute to automatically
 * restore BASEPRI when the block exits (including early returns or exceptions).
 *
 * USAGE
 * -----
 * Basic usage - block all interrupts:
 *
 *     #include "atomic.h"
 *
 *     void readSharedData(void) {
 *         uint32_t value;
 *         ATOMIC_BLOCK(NVIC_PRIO_MAX) {
 *             value = sharedCounter;  // Safe read
 *         }
 *         // Interrupts restored here
 *         return value;
 *     }
 *
 * Block only lower-priority interrupts (allow urgent ones through):
 *
 *     // Only block priorities >= 2 (allow priority 0-1 interrupts)
 *     ATOMIC_BLOCK(2) {
 *         // Critical section
 *     }
 *
 * WHEN TO USE
 * -----------
 * Use ATOMIC_BLOCK when:
 *   - Reading/writing multi-byte variables shared with ISRs
 *   - Implementing lock-free data structures
 *   - Protecting hardware register sequences that must not be interrupted
 *
 * Prefer ATOMIC_BLOCK over __disable_irq() because:
 *   - Allows time-critical high-priority interrupts to still run
 *   - Automatic cleanup on all exit paths (return, break, goto)
 *   - Nestable (inner blocks restore to outer block's priority)
 *
 * PRIORITY VALUES
 * ---------------
 * STM32 typically uses 4 priority bits (16 levels, 0-15):
 *   - NVIC_PRIO_MAX (0): Block ALL maskable interrupts
 *   - 1-15: Block that priority and lower (higher numbers = lower priority)
 *
 * Note: The macro handles the shift for __NVIC_PRIO_BITS automatically.
 *
 * LIMITATIONS
 * -----------
 *   - Does not block NMI or HardFault (non-maskable)
 *   - Keep critical sections SHORT to minimize interrupt latency
 *   - Not available on Cortex-M0 (no BASEPRI register)
 */

#pragma once

#include "stm32_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Restore BASEPRI on scope exit (used by __cleanup__ attribute) */
static inline void __basepriRestoreMem(uint8_t *val)
{
    __set_BASEPRI(*val);
}

/** Set BASEPRI_MAX and return true to enter for-loop body */
static inline uint8_t __basepriSetMemRetVal(uint8_t prio)
{
    __set_BASEPRI_MAX(prio);
    return 1;
}

/**
 * @brief  Execute code block with elevated interrupt priority masking
 * @param  prio  Priority level to mask (0 = block all, higher = block fewer)
 *
 * Example:
 *     ATOMIC_BLOCK(NVIC_PRIO_MAX) {
 *         // All maskable interrupts blocked here
 *         critical_operation();
 *     }
 *     // BASEPRI automatically restored
 */
#define ATOMIC_BLOCK(prio) \
    for (uint8_t __basepri_save __attribute__((__cleanup__(__basepriRestoreMem))) = __get_BASEPRI(), \
         __ToDo = __basepriSetMemRetVal((prio) << (8U - __NVIC_PRIO_BITS)); \
         __ToDo; __ToDo = 0)

/**
 * Maximum priority level for ATOMIC_BLOCK - blocks priorities 1-15.
 *
 * IMPORTANT: Priority 0 interrupts CANNOT be blocked by BASEPRI.
 * Reserve priority 0 for safety-critical ISRs (e-stop, motor commutation,
 * fault handlers) that must never be delayed.
 *
 * After the shift in ATOMIC_BLOCK macro: (1) << (8-4) = 0x10
 * BASEPRI = 0x10 blocks all interrupts with priority >= 1.
 */
#define NVIC_PRIO_MAX 1

#ifdef __cplusplus
}
#endif

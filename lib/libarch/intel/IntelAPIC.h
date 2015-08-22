/*
 * Copyright (C) 2015 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBARCH_INTEL_APIC_H
#define __LIBARCH_INTEL_APIC_H

#include <Types.h>
#include <BitOperations.h>
#include <IntController.h>
#include "IntelIO.h"

/** Forward declarations */
class MemoryContext;
class IntelPIT;

// TODO (needed for SMP startup)
#define APIC_DEST(x) ((x) << 24)
#define APIC_DEST_FIELD         0x00000
#define APIC_DEST_LEVELTRIG     0x08000
#define APIC_DEST_ASSERT        0x04000
#define APIC_DEST_DM_INIT       0x00500
#define APIC_DEST_DM_STARTUP    0x00600

/**
 * Intel Advanced Programmable Interrupt Controller (APIC)
 */
class IntelAPIC : public IntController
{
  private:

    /** APIC memory mapped I/O register base offset (physical address). */
    static const uint IOBase = 0xfee00000;

    /** APIC timer interrupt vector is fixed at 48 */
    static const uint TimerVector = 48;

    /**
     * Hardware registers.
     */
    enum Registers
    {
        Identifier          = 0x20,
        Version             = 0x30,
        TaskPriority        = 0x80,
        ArbitrationPriority = 0x90,
        ProcessorPriority   = 0xa0,
        EndOfInterrupt      = 0xb0,
        SpuriousIntVec      = 0xf0,
        InService           = 0x100,
        TriggerMode         = 0x180,
        IntRequest          = 0x200,
        ErrorStatus         = 0x280,
        IntCommand1         = 0x300,
        IntCommand2         = 0x310,
        Timer               = 0x320,
        ThermalSensor       = 0x330,
        PerfCounters        = 0x340,
        LocalInt0           = 0x350,
        LocalInt1           = 0x360,
        Error               = 0x370,
        InitialCount        = 0x380,
        CurrentCount        = 0x390,
        DivideConfig        = 0x3e0
    };

    /**
     * Spurious Interrupt Vector Register flags.
     */
    enum SpuriousIntVecFlags
    {
        APICEnable = 0x100
    };

    /**
     * Timer Divide Configuration Register flags.
     */
    enum DivideConfigFlags
    {
        Divide16 = 3
    };

    /**
     * Timer Register flags.
     */
    enum TimerFlags
    {
        PeriodicMode = (1 << 17)
    };

  public:

    /**
     * Constructor
     */
    IntelAPIC();

    /**
     * Get timer interrupt number.
     *
     * The interrupt number for channel 0 is fixed to IRQ0.
     *
     * @return Interrupt number.
     */
    uint getTimerInterrupt();

    /**
     * Get timer interrupt frequency.
     *
     * @return Current interrupt frequency.
     */
    uint getTimerFrequency();

    /**
     * Start the timer.
     *
     * @param pit PIT instance used to measure the APIC bus speed for clock calibration.
     * @return Result code.
     */
    Result startTimer(IntelPIT *pit);

    /**
     * Initialize the APIC.
     *
     * @return Result code.
     */
    Result initialize();

    /**
     * Enable hardware interrupt (IRQ).
     *
     * @param irq Interrupt Request number.
     * @return Result code.
     */
    virtual Result enable(uint irq);

    /**
     * Disable hardware interrupt (IRQ).
     *
     * @param irq Interrupt Request number.
     * @return Result code.
     */
    virtual Result disable(uint irq);

    /**
     * Clear hardware interrupt (IRQ).
     *
     * Clearing marks the end of an interrupt service routine
     * and causes the controller to trigger the interrupt again
     * on the next trigger moment.
     *
     * @param irq Interrupt Request number to clear.
     * @return Result code.
     */
    virtual Result clear(uint irq);

  private:

    /** I/O object */
    IntelIO m_io;

    /** Interrupt frequency of the APIC timer */
    uint m_hertz;
};

#endif /* __LIBARCH_INTEL_APIC_H */
/* linux/arch/arm/plat-s5pc1xx/include/plat/gpio-bank-eint.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 * 	Ben Dooks <ben@simtec.co.uk>
 * 	http://armlinux.simtec.co.uk/
 *
 * GPIO External interrupt register and configuration definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#define S5PV210_GPIOREG(x)		(S5P_VA_GPIO + (x))

#define S5PV210_EINT0CON		S5PV210_GPIOREG(0xE00)		/* EINT0  ~ EINT7  */
#define S5PV210_EINT1CON		S5PV210_GPIOREG(0xE04)		/* EINT8  ~ EINT15 */
#define S5PV210_EINT2CON		S5PV210_GPIOREG(0xE08)		/* EINT16 ~ EINT23 */
#define S5PV210_EINT3CON		S5PV210_GPIOREG(0xE0C)		/* EINT24 ~ EINT31 */
#define S5PV210_EINTCON(x)		(S5PV210_EINT0CON+x*0x4)	/* EINT0  ~ EINT31  */

#define S5PV210_EINT0FLTCON0		S5PV210_GPIOREG(0xE80)		/* EINT0  ~ EINT3  */
#define S5PV210_EINT0FLTCON1		S5PV210_GPIOREG(0xE84)
#define S5PV210_EINT1FLTCON0		S5PV210_GPIOREG(0xE88)		/* EINT8 ~  EINT11 */
#define S5PV210_EINT1FLTCON1		S5PV210_GPIOREG(0xE8C)
#define S5PV210_EINT2FLTCON0		S5PV210_GPIOREG(0xE90)
#define S5PV210_EINT2FLTCON1		S5PV210_GPIOREG(0xE94)
#define S5PV210_EINT3FLTCON0		S5PV210_GPIOREG(0xE98)
#define S5PV210_EINT3FLTCON1		S5PV210_GPIOREG(0xE9C)
#define S5PV210_EINTFLTCON(x)		(S5PV210_EINT0FLTCON0+x*0x4)	/* EINT0  ~ EINT31 */

#define S5PV210_EINT0MASK		S5PV210_GPIOREG(0xF00)		/* EINT30[0] ~  EINT30[7]  */
#define S5PV210_EINT1MASK		S5PV210_GPIOREG(0xF04)		/* EINT31[0] ~  EINT31[7] */
#define S5PV210_EINT2MASK		S5PV210_GPIOREG(0xF08)		/* EINT32[0] ~  EINT32[7] */
#define S5PV210_EINT3MASK		S5PV210_GPIOREG(0xF0C)		/* EINT33[0] ~  EINT33[7] */
#define S5PV210_EINTMASK(x)		(S5PV210_EINT0MASK+x*0x4)	/* EINT0 ~  EINT31  */

#define S5PV210_EINT0PEND		S5PV210_GPIOREG(0xF40)		/* EINT30[0] ~  EINT30[7]  */
#define S5PV210_EINT1PEND		S5PV210_GPIOREG(0xF44)		/* EINT31[0] ~  EINT31[7] */
#define S5PV210_EINT2PEND		S5PV210_GPIOREG(0xF48)		/* EINT32[0] ~  EINT32[7] */
#define S5PV210_EINT3PEND		S5PV210_GPIOREG(0xF4C)		/* EINT33[0] ~  EINT33[7] */
#define S5PV210_EINTPEND(x)		(S5PV210_EINT0PEND+x*04)	/* EINT0 ~  EINT31  */


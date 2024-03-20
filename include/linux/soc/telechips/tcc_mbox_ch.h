// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

/*
 * ======
 * NOTICE
 * ======
 *
 * Currently, mailbox id 0 is used for Trust RVC. Differs to mailbox driver in
 * Linux kernel, the driver in u-boot has implemented only within fundamental
 * apis without a list of names and ids. However, it could be reused or changed
 * in further implementation. At that time, we need to define the mailbox
 * channels explicitly to take account of the usages.
 */

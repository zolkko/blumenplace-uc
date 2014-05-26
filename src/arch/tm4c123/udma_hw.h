/*
 * udma_hw.h -
 *
 * Copyright (c) 2014 Alexey Anisimov <zolkko@gmail.com>
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
 * along with program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __udma_hw_h__
#define __udma_hw_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*udam_isr_handler)(uint32_t status);

void udma_init(void);

bool udma_register_software_isr_handler(udma_isr_handler handler);

bool udma_register_error_isr_handler(udma_isr_handler);

#ifdef __cplusplus
}
#endif

#endif /* __udma_hw_h__ */

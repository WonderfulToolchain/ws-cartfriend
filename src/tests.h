#pragma once
/**
 * Copyright (c) 2023 Adrian Siekierka
 *
 * CartFriend is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * CartFriend is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with CartFriend. If not, see <https://www.gnu.org/licenses/>. 
 */

// CartFriend - tests

#include <stdbool.h>
#include <stdint.h>
#include <wonderful.h>
#include <ws.h>

/**
 * @brief Run a save read/write test.
 * @param slot Flash slot to test
 */
bool test_save_read_write(uint8_t x, uint8_t y, uint8_t slot);

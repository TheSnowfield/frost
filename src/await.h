// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _FROST_AWAIT_H
#define _FROST_AWAIT_H

/**
 * @brief create an awaiter
  
 * @return frost_awaiter_t*
 */
frost_awaiter_t* awaiter_create();

/**
 * @brief create an awaiter from value
 *
 * @param value the value
 * @param status
 * @return frost_awaiter_t*
 */
frost_awaiter_t* awaiter_from_value(frost_handle_t value, frost_errcode_t status);

/**
 * @brief awaiter destroy
 *
 * @param awaiter awaiter pointer
 * @return frost_errcode_t
 */
frost_errcode_t awaiter_destroy(frost_awaiter_t* awaiter);

/**
 * @brief wait an awaiter return
 *
 * @param awaiter awaiter pointer
 * @return frost_awaiter_t*
 */
frost_awaiter_t* awaiter_await(frost_awaiter_t* awaiter);

/**
 * @brief set awaiter status to finish
 *
 * @param awaiter awaiter pointer
 * @param result the result
 * @return frost_errcode_t
 */
frost_errcode_t awaiter_finish(frost_awaiter_t* awaiter, frost_handle_t result);

/**
 * @brief cancel awaiter
 *
 * @param awaiter awaiter pointer
 * @return frost_errcode_t
 */
frost_errcode_t awaiter_cancel(frost_awaiter_t* awaiter);

#endif /* _FROST_AWAIT_H */

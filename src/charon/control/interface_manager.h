/**
 * @file interface_manager.h
 *
 * @brief Interface of interface_manager_t.
 *
 */

/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef INTERFACE_MANAGER_H_
#define INTERFACE_MANAGER_H_

#include <bus/bus.h>

/**
 * callback to log things triggered by interface_manager.
 *
 * @param param			echoed parameter supplied when function invoked
 * @param signal		type of signal
 * @param level			verbosity level if log
 * @param ike_sa		associated IKE_SA, if any
 * @param format		printf like format string
 * @param args			list of arguments to use for format
 * @return				FALSE to return from invoked function
 * @ingroup control
 */
typedef bool(*interface_manager_cb_t)(void* param, signal_t signal, level_t level,
							   ike_sa_t* ike_sa, char* format, va_list args);

typedef struct interface_manager_t interface_manager_t;

/**
 * @brief The interface_manager loads control interfaces and has helper methods.
 *
 * One job of the interface manager is to load pluggable control interface
 * modules, implemented as interface_t.
 * @verbatim

   +---------+      +------------+         +--------------+     |
   |         |      |            |<----- +--------------+ |     |
   | daemon  |<-----| interface- |     +--------------+ |-+  <==|==> IPC
   |  core   |      | manager    |<----|  interfaces  |-+       |
   |         |<-----|            |     +--------------+         |
   |         |      |            |                              |
   +---------+      +------------+                              |

   @endverbatim
 * The manager does not really use the interfaces, instead, the interface
 * use the manager to fullfill their tasks (initiating, terminating, ...). 
 * The interface_manager starts actions by creating jobs. It then tries to
 * evaluate the result of the operation by listening on the bus.
 * 
 * @b Constructors:
 * - interface_manager_create()
 * 
 * @ingroup control
 */
struct interface_manager_t {

	/**
	 * @brief Create an iterator for all IKE_SAs.
	 *
	 * The iterator blocks the IKE_SA manager until it gets destroyed. Do
	 * not call another interface/manager method while the iterator is alive.
	 *
	 * @param this			calling object
	 * @return				iterator, locks IKE_SA manager until destroyed
	 */
	iterator_t* (*create_ike_sa_iterator)(interface_manager_t *this);

	/**
	 * @brief Initiate a CHILD_SA, and if required, an IKE_SA.
	 *
	 * @param this			calling object
	 * @param peer_cfg		peer_cfg to use for IKE_SA setup
	 * @param child_cfg		child_cfg to set up CHILD_SA from
	 * @param cb			logging callback
	 * @param param			parameter to include in each call of cb
	 * @return
	 *						- SUCCESS, if CHILD_SA established
	 *						- FAILED, if setup failed
	 *						- NEED_MORE, if callback returned FALSE
	 */
	status_t (*initiate)(interface_manager_t *this,
						 peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
						 interface_manager_cb_t callback, void *param);

	/**
	 * @brief Terminate an IKE_SA and all of its CHILD_SAs.
	 *
	 * @param this			calling object
	 * @param unique_id		unique id of the IKE_SA to terminate.
	 * @param cb			logging callback
	 * @param param			parameter to include in each call of cb
	 * @return
	 *						- SUCCESS, if CHILD_SA terminated
	 *						- NOT_FOUND, if no such CHILD_SA found
	 *						- NEED_MORE, if callback returned FALSE
	 */
	status_t (*terminate_ike)(interface_manager_t *this, u_int32_t unique_id, 
							  interface_manager_cb_t callback, void *param);
	
	/**
	 * @brief Terminate a CHILD_SA.
	 *
	 * @param this			calling object
	 * @param reqid			reqid of the CHILD_SA to terminate
	 * @param cb			logging callback
	 * @param param			parameter to include in each call of cb
	 * @return
	 *						- SUCCESS, if CHILD_SA terminated
	 *						- NOT_FOUND, if no such CHILD_SA found
	 *						- NEED_MORE, if callback returned FALSE
	 */
	status_t (*terminate_child)(interface_manager_t *this, u_int32_t reqid, 
								interface_manager_cb_t callback, void *param);
	
	/**
	 * @brief Route a CHILD_SA (install triggering policies).
	 *
	 * @param this			calling object
	 * @param peer_cfg		peer_cfg to use for IKE_SA setup, if triggered
	 * @param child_cfg		child_cfg to route
	 * @param cb			logging callback
	 * @param param			parameter to include in each call of cb
	 * @return
	 *						- SUCCESS, if CHILD_SA routed
	 *						- FAILED, if routing failed
	 *						- NEED_MORE, if callback returned FALSE
	 */
	status_t (*route)(interface_manager_t *this,
					  peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
					  interface_manager_cb_t callback, void *param);
	
	/**
	 * @brief Unroute a routed CHILD_SA (uninstall triggering policies).
	 *
	 * Only the route is removed, not the CHILD_SAs the route triggered.
	 *
	 * @param this			calling object
	 * @param reqid			reqid of the CHILD_SA to unroute
	 * @param cb			logging callback
	 * @param param			parameter to include in each call of cb
	 * @return
	 *						- SUCCESS, if CHILD_SA terminated
	 *						- NOT_FOUND, if no such CHILD_SA routed
	 *						- NEED_MORE, if callback returned FALSE
	 */
	status_t (*unroute)(interface_manager_t *this, u_int32_t reqid, 
						interface_manager_cb_t callback, void *param);
	
	/**
	 * @brief Destroy a interface_manager_t instance.
	 * 
	 * @param this		interface_manager_t objec to destroy
	 */
	void (*destroy) (interface_manager_t *this);
};


/**
 * @brief Creates a interface_manager instance and loads all interface modules.
 * 
 * @return 			interface_manager_t object
 * 
 * @ingroup control
 */
interface_manager_t *interface_manager_create(void);

#endif /* INTERFACE_MANAGER_H_ */


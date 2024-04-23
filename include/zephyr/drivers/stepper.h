/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Carl Zeiss Meditec AG
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_STEPPER_H
#define ZEPHYR_INCLUDE_STEPPER_H

/** \addtogroup stepper
 *  @{
 */

/**
 * @file
 * @brief Public API for Stepper Motor Controller
 *
 */

#include <zephyr/kernel.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Stepper Motor micro step resolution options
 */
enum micro_step_resolution {
	STEPPER_FULL_STEP,
	STEPPER_MICRO_STEP_2,
	STEPPER_MICRO_STEP_4,
	STEPPER_MICRO_STEP_8,
	STEPPER_MICRO_STEP_16,
	STEPPER_MICRO_STEP_32,
	STEPPER_MICRO_STEP_64,
	STEPPER_MICRO_STEP_128,
	STEPPER_MICRO_STEP_256,
};

/**
 * @brief Attibutes to set/get as per the functionality offered by the stepper controller
 */
enum stepper_attribute {
	/** read/write actual a.k.a reference position in the stepper driver */
	STEPPER_ACTUAL_POSITION,
	/** read/write target a.k.a absolute position in the stepper driver */
	STEPPER_TARGET_POSITION,
	/** setting this attribute should activate constant velocity mode with the given velocity */
	STEPPER_CONSTANT_VELOCITY,
	/** start ramp velocity, for controllers which provide ramping functionality */
	STEPPER_RAMP_VELOCITY_START,
	/** maximum ramp velocity, for controllers which provide ramping functionality */
	STEPPER_RAMP_VELOCITY_MAX,
	/** stop ramp velocity, for controllers which provide ramping functionality */
	STEPPER_RAMP_VELOCITY_STOP,
	/** threshold value for sensor-less stall detection */
	STEPPER_STALL_DETECTION_THRESHOLD,
	/** enable/disable stall detection */
	STEPPER_STALL_DETECTION_STATUS,
	/** retrieve the actual load detected by motor controller */
	STEPPER_LOAD_DETECTION,
};

/**
 * @typedef stepper_enable_t
 * @brief enable or disable the stepper motor controller.
 */
typedef int32_t (*stepper_enable_t)(const struct device *dev, bool enable);

/**
 * @typedef stepper_move_t
 * @brief Stepper Motor Controller shall move the motor with given steps
 */
typedef int32_t (*stepper_move_t)(const struct device *dev, int32_t steps);

/**
 * @typedef stepper_set_velocity_t
 * @brief Set the velocity in steps per seconds. For controllers such as DRV8825 where you
 * toggle the STEP Pin, the pulse_length would have to be calculated based on this parameter in the
 * driver. For controllers where velocity can be set, this parameter corresponds to max_velocity
 */
typedef int32_t (*stepper_set_velocity_t)(const struct device *dev, uint32_t steps_per_second);

/**
 * @typedef stepper_set_micro_step_res_t
 * @brief Set the microstep resolution
 */
typedef int32_t (*stepper_set_micro_step_res_t)(const struct device *dev,
						enum micro_step_resolution resolution);

/**
 * @typedef stepper_set_attr_t
 * @brief Set the value of a certain attribute in stepper controller
 */
typedef int32_t (*stepper_set_attr_t)(const struct device *dev, enum stepper_attribute,
				      const int32_t value);

/**
 * @typedef stepper_get_attr_t
 * @brief Get the value of a certain attribute in stepper controller
 */
typedef int32_t (*stepper_get_attr_t)(const struct device *dev, enum stepper_attribute,
				      int32_t *value);

__subsystem struct stepper_api {
	stepper_enable_t enable;
	stepper_move_t move;
	stepper_set_velocity_t set_velocity;
	stepper_set_micro_step_res_t set_micro_step_res;
	stepper_set_attr_t set_attribute;
	stepper_get_attr_t get_attribute;
};

/**
 * @brief Enable or Disable Motor Controller
 * @param dev pointer to the stepper motor controller instance
 * @param enable Input enable or disable motor controller
 * @return <0 Error during Enabling
 *          0 Success
 */
__syscall int32_t stepper_enable(const struct device *dev, bool enable);

static inline int32_t z_impl_stepper_enable(const struct device *dev, bool enable)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	return api->enable(dev, enable);
}

/**
 * @brief Set the steps to be moved from the current position i.e. relative movement
 * @param dev pointer to the stepper motor controller instance
 * @param steps target steps to be moved from the current position
 * @return <0 Error during Enabling
 *          0 Success
 */
__syscall int32_t stepper_move(const struct device *dev, int32_t steps);

static inline int32_t z_impl_stepper_move(const struct device *dev, int32_t steps)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	return api->move(dev, steps);
}

/**
 * @brief Set the target velocity to be reached by the motor
 * @param dev pointer to the stepper motor controller instance
 * @param steps_per_second Enter the speed in steps per second
 * @return <0 Error during Enabling
 *          0 Success
 */
__syscall int32_t stepper_set_velocity(const struct device *dev, uint32_t steps_per_second);

static inline int32_t z_impl_stepper_set_velocity(const struct device *dev,
						  uint32_t steps_per_second)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	return api->set_velocity(dev, steps_per_second);
}

/**
 * @brief Set the microstep resolution in stepper motor controller
 * @param dev pointer to the stepper motor controller instance
 * @param resolution microstep resolution
 * @return -ENOSYS Function setting of micro step res not implemented
 *               0 Success
 */
__syscall int32_t stepper_set_micro_step_res(const struct device *dev,
					     enum micro_step_resolution resolution);

static inline int32_t z_impl_stepper_set_micro_step_res(const struct device *dev,
							enum micro_step_resolution resolution)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	if (api->set_micro_step_res == NULL) {
		return -ENOSYS;
	}
	return api->set_micro_step_res(dev, resolution);
}

/**
 * @brief Set the value of the attribute as per stepper_attribute
 * @param dev pointer to the stepper motor controller instance
 * @param attribute enum value containing the attribute that has to be set
 * @param value value of the attribute
 * @return -ENOTSUP If the write of this attribute is not supported by the stepper controller
 *          -ENOSYS Function get attribute not implemented
 *                0 Success
 */
__syscall int32_t stepper_set_attribute(const struct device *dev, enum stepper_attribute attribute,
					const int32_t value);

static inline int32_t z_impl_stepper_motor_set_attribute(const struct device *dev,
							 enum stepper_attribute attribute,
							 const int32_t value)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	if (api->set_attribute == NULL) {
		return -ENOSYS;
	}
	return api->set_attribute(dev, attribute, value);
}

/**
 * @brief Get the value of the attribute as per stepper_attribute
 * @param dev pointer to the stepper motor controller instance
 * @param attribute enum value containing the attribute, whose value has to be retrieved
 * @param value value of the attribute
 * @return -ENOTSUP If the read of this attribute is not supported by the stepper controller
 *          -ENOSYS Function get attribute not implemented
 *		  0 Success
 */
__syscall int32_t stepper_get_attribute(const struct device *dev, enum stepper_attribute attribute,
					int32_t *value);

static inline int32_t z_impl_stepper_motor_get_attribute(const struct device *dev,
							 enum stepper_attribute attribute,
							 int32_t *value)
{
	const struct stepper_api *api = (const struct stepper_api *)dev->api;

	if (api->get_attribute == NULL) {
		return -ENOSYS;
	}
	return api->get_attribute(dev, attribute, value);
}

#ifdef __cplusplus
}
#endif

/** @}*/

#include <syscalls/stepper.h>

#endif /* ZEPHYR_INCLUDE_STEPPER_H */

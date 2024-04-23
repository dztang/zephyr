/*
 * Copyright (c) 2022-2023 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_SENSING_SENSOR_H_
#define ZEPHYR_INCLUDE_SENSING_SENSOR_H_

#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sensing/sensing.h>

/**
 * @defgroup sensing_sensor Sensing Sensor API
 * @ingroup sensing
 * @defgroup sensing_sensor_callbacks Sensor Callbacks
 * @ingroup sensing_sensor
 */

/**
 * @brief Sensing Sensor API
 * @addtogroup sensing_sensor
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sensor registration information
 *
 */
struct sensing_sensor_register_info {
	/**
	 * Sensor flags
	 */
	uint16_t flags;

	/**
	 * Sample size in bytes for a single sample of the registered sensor.
	 * sensing runtime need this information for internal buffer allocation.
	 */
	uint16_t sample_size;

	/**
	 * The number of sensor sensitivities
	 */
	uint8_t sensitivity_count;

	/**
	 * Sensor version.
	 * Version can be used to identify different versions of sensor implementation.
	 */
	struct sensing_sensor_version version;
};

/**
 * @brief Enumeration for sensing subsystem event.
 *
 * This enumeration defines the event for sensing subsystem.
 */
enum {
	EVENT_CONFIG_READY, /**< Configuration is ready. */
};

/**
 * @brief Enumeration for sensor flag bit.
 *
 * This enumeration defines the bit for sensor flag.
 */
enum {
	SENSOR_LATER_CFG_BIT, /**< Sensor later configuration bit. */
};

/**
 * @brief Connection between a source and sink of sensor data
 */
struct sensing_connection {
	sys_snode_t snode;             /**< Node in the singly-linked list of connections. */
	struct sensing_sensor *source; /**< Source sensor of the connection. */
	struct sensing_sensor *sink;   /**< Sink sensor of the connection. */
	uint32_t interval;             /**< Interval of the connection. */
	/**< Sensitivity of the connection. */
	int sensitivity[CONFIG_SENSING_MAX_SENSITIVITY_COUNT];
	void *data;                    /**< Data buffer of the connection. */
	uint64_t next_consume_time;    /**< Next consume time of the connection. */
	struct sensing_callback_list *callback_list; /**< Callback list of the connection. */
};

/**
 * @brief Internal sensor instance data structure.
 *
 * Each sensor instance will have its unique data structure for storing all
 * it's related information.
 *
 * Sensor management will enumerate all these instance data structures,
 * build report relationship model base on them, etc.
 */
struct sensing_sensor {
	const struct device *dev;               /**< Device of the sensor instance. */
	const struct sensing_sensor_info *info; /**< Info of the sensor instance. */
	/**< Register info of the sensor instance. */
	const struct sensing_sensor_register_info *register_info;
	const uint16_t reporter_num; /**< Reporter number of the sensor instance. */
	sys_slist_t client_list;     /**< Client list of the sensor instance. */
	uint32_t interval;           /**< Interval of the sensor instance. */
	uint8_t sensitivity_count;   /**< Sensitivity count of the sensor instance. */
	/**< Sensitivity of the sensor instance. */
	int sensitivity[CONFIG_SENSING_MAX_SENSITIVITY_COUNT];
	enum sensing_sensor_state state;  /**< State of the sensor instance. */
	struct rtio_iodev *iodev;         /**< An RTIO device info of the sensor instance. */
	struct k_timer timer;             /**< Timer of the sensor instance. */
	struct rtio_sqe *stream_sqe;      /**< Stream sqe of the sensor instance. */
	atomic_t flag;                    /**< Flag of the sensor instance. */
	struct sensing_connection *conns; /**< Connections of the sensor instance. */
};

/**
 * @brief Macro to generate a name for a sensor info structure.
 *
 * This macro generates a name for a sensor info structure based on a node and an index.
 */
#define SENSING_SENSOR_INFO_NAME(node, idx)					\
	_CONCAT(_CONCAT(__sensing_sensor_info_, idx), DEVICE_DT_NAME_GET(node))

/**
 * @brief Macro to define a sensor info structure.
 *
 * This macro defines a sensor info structure based on a node and an index.
 * The structure includes the type, name, friendly name, vendor, model, and minimal interval of the
 * sensor.
 */
#define SENSING_SENSOR_INFO_DEFINE(node, idx)				\
	const static STRUCT_SECTION_ITERABLE(sensing_sensor_info,	\
			SENSING_SENSOR_INFO_NAME(node, idx)) = {	\
		.type = DT_PROP_BY_IDX(node, sensor_types, idx),	\
		.name = DT_NODE_FULL_NAME(node),			\
		.friendly_name = DT_PROP(node, friendly_name),		\
		.vendor = DT_NODE_VENDOR_OR(node, NULL),		\
		.model = DT_NODE_MODEL_OR(node, NULL),			\
		.minimal_interval = DT_PROP(node, minimal_interval),	\
	};

/**
 * @brief Macro to generate a name for a connections array.
 *
 * This macro generates a name for a connections array based on a node.
 */
#define SENSING_CONNECTIONS_NAME(node)					\
	_CONCAT(__sensing_connections_, DEVICE_DT_NAME_GET(node))

/**
 * @brief Macro to generate a name for a sensor source.
 *
 * This macro generates a name for a sensor source based on an index and a node.
 */
#define SENSING_SENSOR_SOURCE_NAME(idx, node)				\
	SENSING_SENSOR_NAME(DT_PHANDLE_BY_IDX(node, reporters, idx),	\
		DT_PROP_BY_IDX(node, reporters_index, idx))

/**
 * @brief Macro to declare an external sensor source.
 *
 * This macro declares an external sensor source based on an index and a node.
 */
#define SENSING_SENSOR_SOURCE_EXTERN(idx, node)				\
extern struct sensing_sensor SENSING_SENSOR_SOURCE_NAME(idx, node);

/**
 * @brief Macro to initialize a connection.
 *
 * This macro initializes a connection with a source name and a callback list pointer.
 */
#define SENSING_CONNECTION_INITIALIZER(source_name, cb_list_ptr)	\
{									\
	.callback_list = cb_list_ptr,					\
	.source = &source_name,						\
}

/**
 * @brief Macro to define a connection.
 *
 * This macro defines a connection based on an index, a node, and a callback list pointer.
 */
#define SENSING_CONNECTION_DEFINE(idx, node, cb_list_ptr)		\
	SENSING_CONNECTION_INITIALIZER(SENSING_SENSOR_SOURCE_NAME(idx, node), \
				       cb_list_ptr)

/**
 * @brief Macro to define an array of connections.
 *
 * This macro defines an array of connections based on a node, a number, and a callback list
 * pointer.
 */
#define SENSING_CONNECTIONS_DEFINE(node, num, cb_list_ptr)		\
	LISTIFY(num, SENSING_SENSOR_SOURCE_EXTERN,			\
				(), node)				\
	static struct sensing_connection				\
			SENSING_CONNECTIONS_NAME(node)[(num)] = {	\
		LISTIFY(num, SENSING_CONNECTION_DEFINE,			\
				(,), node, cb_list_ptr)			\
	};

/**
 * @brief Structure for sensor submit configuration.
 *
 * This structure represents a sensor submit configuration. It includes the channel, info index, and
 * streaming flag.
 */
struct sensing_submit_config {
	enum sensor_channel chan; /**< Channel of the sensor submit configuration. */
	const int info_index;     /**< Info index of the sensor submit configuration. */
	const bool is_streaming;  /**< Streaming flag of the sensor submit configuration. */
};

/**
 * @brief External declaration for the sensing I/O device API.
 *
 * This external declaration represents the sensing I/O device API.
 */
extern const struct rtio_iodev_api __sensing_iodev_api;

/**
 * @brief Macro to generate a name for a submit configuration.
 *
 * This macro generates a name for a submit configuration based on a node and an index.
 */
#define SENSING_SUBMIT_CFG_NAME(node, idx)						\
	_CONCAT(_CONCAT(__sensing_submit_cfg_, idx), DEVICE_DT_NAME_GET(node))

/**
 * @brief Macro to generate a name for a sensor I/O device.
 *
 * This macro generates a name for a sensor I/O device based on a node and an index.
 */
#define SENSING_SENSOR_IODEV_NAME(node, idx)						\
	_CONCAT(_CONCAT(__sensing_iodev_, idx), DEVICE_DT_NAME_GET(node))

/**
 * @brief Macro to define a sensor I/O device.
 *
 * This macro defines a sensor I/O device based on a node and an index.
 * The device includes a submit configuration with a streaming flag and an info index.
 */
#define SENSING_SENSOR_IODEV_DEFINE(node, idx)						\
	static struct sensing_submit_config SENSING_SUBMIT_CFG_NAME(node, idx) = {	\
		.is_streaming = DT_PROP(node, stream_mode),				\
		.info_index = idx,							\
	};										\
	RTIO_IODEV_DEFINE(SENSING_SENSOR_IODEV_NAME(node, idx),				\
			  &__sensing_iodev_api,						\
			  &SENSING_SUBMIT_CFG_NAME(node, idx));

/**
 * @brief Macro to generate a name for a sensor.
 *
 * This macro generates a name for a sensor based on a node and an index.
 */
#define SENSING_SENSOR_NAME(node, idx)					\
	_CONCAT(_CONCAT(__sensing_sensor_, idx), DEVICE_DT_NAME_GET(node))

/**
 * @brief Macro to define a sensor.
 *
 * This macro defines a sensor based on a node, a property, an index, a register info pointer, and a
 * callback list pointer. The sensor includes a device, info, register info, reporter number,
 * connections, and an I/O device.
 */
#define SENSING_SENSOR_DEFINE(node, prop, idx, reg_ptr, cb_list_ptr)	\
	SENSING_SENSOR_INFO_DEFINE(node, idx)				\
	SENSING_SENSOR_IODEV_DEFINE(node, idx)				\
	STRUCT_SECTION_ITERABLE(sensing_sensor,				\
				SENSING_SENSOR_NAME(node, idx)) = {	\
		.dev = DEVICE_DT_GET(node),				\
		.info = &SENSING_SENSOR_INFO_NAME(node, idx),		\
		.register_info = reg_ptr,				\
		.reporter_num = DT_PROP_LEN_OR(node, reporters, 0),	\
		.conns = SENSING_CONNECTIONS_NAME(node),		\
		.iodev = &SENSING_SENSOR_IODEV_NAME(node, idx),		\
	};

/**
 * @brief Macro to define sensors.
 *
 * This macro defines sensors based on a node, a register info pointer, and a callback list pointer.
 * It uses the DT_FOREACH_PROP_ELEM_VARGS macro to define each sensor.
 */
#define SENSING_SENSORS_DEFINE(node, reg_ptr, cb_list_ptr)		\
	DT_FOREACH_PROP_ELEM_VARGS(node, sensor_types,			\
			SENSING_SENSOR_DEFINE, reg_ptr, cb_list_ptr)
/**
 * @brief Like SENSOR_DEVICE_DT_DEFINE() with sensing specifics.
 *
 * @details Defines a sensor which implements the sensor API. May define an
 * element in the sensing sensor iterable section used to enumerate all sensing
 * sensors.
 *
 * @param node The devicetree node identifier.
 *
 * @param reg_ptr Pointer to the device's sensing_sensor_register_info.
 *
 * @param cb_list_ptr Pointer to devices callback list.
 *
 * @param init_fn Name of the init function of the driver.
 *
 * @param pm_device PM device resources reference (NULL if device does not use
 * PM).
 *
 * @param data_ptr Pointer to the device's private data.
 *
 * @param cfg_ptr The address to the structure containing the configuration
 * information for this instance of the driver.
 *
 * @param level The initialization level. See SYS_INIT() for details.
 *
 * @param prio Priority within the selected initialization level. See
 * SYS_INIT() for details.
 *
 * @param api_ptr Provides an initial pointer to the API function struct used
 * by the driver. Can be NULL.
 */
#define SENSING_SENSORS_DT_DEFINE(node, reg_ptr, cb_list_ptr,	\
				init_fn, pm_device,			\
				data_ptr, cfg_ptr, level, prio,		\
				api_ptr, ...)				\
	SENSOR_DEVICE_DT_DEFINE(node, init_fn, pm_device,		\
				data_ptr, cfg_ptr, level, prio,		\
				api_ptr, __VA_ARGS__);			\
	SENSING_CONNECTIONS_DEFINE(node,				\
				   DT_PROP_LEN_OR(node, reporters, 0),	\
				   cb_list_ptr);			\
	SENSING_SENSORS_DEFINE(node, reg_ptr, cb_list_ptr);

/**
 * @brief Like SENSING_SENSORS_DT_DEFINE() for an instance of a DT_DRV_COMPAT
 * compatible
 *
 * @param inst instance number. This is replaced by
 * <tt>DT_DRV_COMPAT(inst)</tt> in the call to SENSING_SENSORS_DT_DEFINE().
 *
 * @param ... other parameters as expected by SENSING_SENSORS_DT_DEFINE().
 */
#define SENSING_SENSORS_DT_INST_DEFINE(inst, ...)	\
	SENSING_SENSORS_DT_DEFINE(DT_DRV_INST(inst), __VA_ARGS__)

/**
 * @brief Get reporter handles	of a given sensor instance by sensor type.
 *
 * @param dev The sensor instance device structure.
 *
 * @param type The given type, \ref SENSING_SENSOR_TYPE_ALL to get reporters
 * with all types.
 *
 * @param max_handles The max count of the \p reporter_handles array input. Can
 * get real count number via \ref sensing_sensor_get_reporters_count
 *
 * @param reporter_handles Input handles array for receiving found reporter
 * sensor instances
 *
 * @return number of reporters found, 0 returned if not found.
 */
int sensing_sensor_get_reporters(
		const struct device *dev, int type,
		sensing_sensor_handle_t *reporter_handles, int max_handles);

/**
 * @brief Get reporters count of a given sensor instance by sensor type.
 *
 * @param dev The sensor instance device structure.
 *
 * @param type The sensor type for checking, \ref SENSING_SENSOR_TYPE_ALL
 *
 * @return Count of reporters by \p type, 0 returned if no reporters by \p type.
 */
int sensing_sensor_get_reporters_count(
		const struct device *dev, int type);

/**
 * @brief Get this sensor's state
 *
 * @param dev The sensor instance device structure.
 *
 * @param state Returned sensor state value
 *
 * @return 0 on success or negative error value on failure.
 */
int sensing_sensor_get_state(
		const struct device *dev,
		enum sensing_sensor_state *state);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*ZEPHYR_INCLUDE_SENSING_SENSOR_H_*/

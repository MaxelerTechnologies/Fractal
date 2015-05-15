/**\file */
#ifndef SLIC_DECLARATIONS_Fractal_H
#define SLIC_DECLARATIONS_Fractal_H
#include "MaxSLiCInterface.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define Fractal_PCIE_ALIGNMENT (16)
#define Fractal_UNROLL_FACTOR (96)


/*----------------------------------------------------------------------------*/
/*---------------------------- Interface default -----------------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'default'.
 * 
 * \param [in] param_c_imag Interface Parameter "c_imag".
 * \param [in] param_c_real Interface Parameter "c_real".
 * \param [in] param_log_width_in_pixels Interface Parameter "log_width_in_pixels".
 * \param [in] param_max_iterations Interface Parameter "max_iterations".
 * \param [in] param_size_in_pixels Interface Parameter "size_in_pixels".
 * \param [in] param_use_z0 Interface Parameter "use_z0".
 * \param [in] param_width_in_pixels Interface Parameter "width_in_pixels".
 * \param [in] instream_input The stream should be of size 32 bytes.
 * \param [out] outstream_output The stream should be of size (param_size_in_pixels * 2) bytes.
 */
void Fractal(
	double param_c_imag,
	double param_c_real,
	uint8_t param_log_width_in_pixels,
	uint32_t param_max_iterations,
	uint32_t param_size_in_pixels,
	uint8_t param_use_z0,
	uint32_t param_width_in_pixels,
	const double *instream_input,
	uint16_t *outstream_output);

/**
 * \brief Basic static non-blocking function for the interface 'default'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [in] param_c_imag Interface Parameter "c_imag".
 * \param [in] param_c_real Interface Parameter "c_real".
 * \param [in] param_log_width_in_pixels Interface Parameter "log_width_in_pixels".
 * \param [in] param_max_iterations Interface Parameter "max_iterations".
 * \param [in] param_size_in_pixels Interface Parameter "size_in_pixels".
 * \param [in] param_use_z0 Interface Parameter "use_z0".
 * \param [in] param_width_in_pixels Interface Parameter "width_in_pixels".
 * \param [in] instream_input The stream should be of size 32 bytes.
 * \param [out] outstream_output The stream should be of size (param_size_in_pixels * 2) bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *Fractal_nonblock(
	double param_c_imag,
	double param_c_real,
	uint8_t param_log_width_in_pixels,
	uint32_t param_max_iterations,
	uint32_t param_size_in_pixels,
	uint8_t param_use_z0,
	uint32_t param_width_in_pixels,
	const double *instream_input,
	uint16_t *outstream_output);

/**
 * \brief Advanced static interface, structure for the engine interface 'default'
 * 
 */
typedef struct { 
	double param_c_imag; /**<  [in] Interface Parameter "c_imag". */
	double param_c_real; /**<  [in] Interface Parameter "c_real". */
	uint8_t param_log_width_in_pixels; /**<  [in] Interface Parameter "log_width_in_pixels". */
	uint32_t param_max_iterations; /**<  [in] Interface Parameter "max_iterations". */
	uint32_t param_size_in_pixels; /**<  [in] Interface Parameter "size_in_pixels". */
	uint8_t param_use_z0; /**<  [in] Interface Parameter "use_z0". */
	uint32_t param_width_in_pixels; /**<  [in] Interface Parameter "width_in_pixels". */
	const double *instream_input; /**<  [in] The stream should be of size 32 bytes. */
	uint16_t *outstream_output; /**<  [out] The stream should be of size (param_size_in_pixels * 2) bytes. */
} Fractal_actions_t;

/**
 * \brief Advanced static function for the interface 'default'.
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in,out] interface_actions Actions to be executed.
 */
void Fractal_run(
	max_engine_t *engine,
	Fractal_actions_t *interface_actions);

/**
 * \brief Advanced static non-blocking function for the interface 'default'.
 *
 * Schedule the actions to run on the engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in] interface_actions Actions to be executed.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *Fractal_run_nonblock(
	max_engine_t *engine,
	Fractal_actions_t *interface_actions);

/**
 * \brief Group run advanced static function for the interface 'default'.
 * 
 * \param [in] group Group to use.
 * \param [in,out] interface_actions Actions to run.
 *
 * Run the actions on the first device available in the group.
 */
void Fractal_run_group(max_group_t *group, Fractal_actions_t *interface_actions);

/**
 * \brief Group run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule the actions to run on the first device available in the group and return immediately.
 * The status of the run must be checked with ::max_wait. 
 * Note that use of ::max_nowait is prohibited with non-blocking running on groups:
 * see the ::max_run_group_nonblock documentation for more explanation.
 *
 * \param [in] group Group to use.
 * \param [in] interface_actions Actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *Fractal_run_group_nonblock(max_group_t *group, Fractal_actions_t *interface_actions);

/**
 * \brief Array run advanced static function for the interface 'default'.
 * 
 * \param [in] engarray The array of devices to use.
 * \param [in,out] interface_actions The array of actions to run.
 *
 * Run the array of actions on the array of engines.  The length of interface_actions
 * must match the size of engarray.
 */
void Fractal_run_array(max_engarray_t *engarray, Fractal_actions_t *interface_actions[]);

/**
 * \brief Array run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule to run the array of actions on the array of engines, and return immediately.
 * The length of interface_actions must match the size of engarray.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * \param [in] engarray The array of devices to use.
 * \param [in] interface_actions The array of actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *Fractal_run_array_nonblock(max_engarray_t *engarray, Fractal_actions_t *interface_actions[]);

/**
 * \brief Converts a static-interface action struct into a dynamic-interface max_actions_t struct.
 *
 * Note that this is an internal utility function used by other functions in the static interface.
 *
 * \param [in] maxfile The maxfile to use.
 * \param [in] interface_actions The interface-specific actions to run.
 * \return The dynamic-interface actions to run, or NULL in case of error.
 */
max_actions_t* Fractal_convert(max_file_t *maxfile, Fractal_actions_t *interface_actions);

/**
 * \brief Initialise a maxfile.
 */
max_file_t* Fractal_init(void);

/* Error handling functions */
int Fractal_has_errors(void);
const char* Fractal_get_errors(void);
void Fractal_clear_errors(void);
/* Free statically allocated maxfile data */
void Fractal_free(void);
/* returns: -1 = error running command; 0 = no error reported */
int Fractal_simulator_start(void);
/* returns: -1 = error running command; 0 = no error reported */
int Fractal_simulator_stop(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SLIC_DECLARATIONS_Fractal_H */


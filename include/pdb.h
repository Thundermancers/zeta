/*!
 * @file
 * @brief This is the abstraction information relative to NB hardware
 *
 */

#ifndef PDB_H_
#define PDB_H_

#include <stddef.h>
#include <zephyr.h>
#include <zephyr/types.h>

/* Checking if PDB_PROPERTY_CREATE is defined and undef it */
#ifdef PDB_PROPERTY_CREATE
#undef PDB_PROPERTY_CREATE
#endif

/* Defining PDB_PROPERTY_CREATE for enum generating */
#define PDB_PROPERTY_CREATE(_name, _bytes, _validate, _get, _set, _in_flash, _observers, _id) PDB_##_name##_PROPERTY,

typedef enum {
#include "properties.def"
              PDB_PROPERTY_COUNT
} pdb_property_e;

#undef PDB_PROPERTY_CREATE

typedef struct {
    k_tid_t source_thread;
    pdb_property_e id;
} pdb_event_t;

struct pdb_property {
    char *name;
    u8_t *data;
    int (*validate)(u8_t *data, size_t size);
    int (*get)(pdb_property_e id, u8_t *property_value, size_t size);
    int (*set)(pdb_property_e id, u8_t *property_value, size_t size);
    u8_t size;
    u8_t in_flash;
    u8_t changed;
    u8_t observers;
    pdb_property_e id;
};
typedef struct pdb_property pdb_property_t;

/** 
 * Returns the property size.
 * 
 * @param id property ID.
 * @param error variable to handle possible errors.
 * 
 * @return property size.
 */
size_t pdb_property_get_size(pdb_property_e id, int *error);

/** 
 * Returns the property pointer reference.
 * 
 * @param id property ID.
 * 
 * @return property pointer reference.
 */
pdb_property_t *pdb_property_get_ref(pdb_property_e id);

/** 
 * Gets the property value.
 * 
 * @param id property ID.
 * @param property_value handle the property value.
 * @param size property value size.
 * 
 * @return error code.
 */
int pdb_property_get(pdb_property_e id, u8_t *property_value, size_t size);

/** 
 * Sets the property value.
 * 
 * @param id property ID.
 * @param property_value property value that must to be setted.
 * @param size property value size.
 * 
 * @return error code.
 */
int pdb_property_set(pdb_property_e id, u8_t *property_value, size_t size);

#endif

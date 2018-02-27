#ifndef __OEPCIE_H__
#define __OEPCIE_H__

// Version macros for compile-time API version detection
#define OE_VERSION_MAJOR 1
#define OE_VERSION_MINOR 0
#define OE_VERSION_PATCH 0

#define OE_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define OE_VERSION \
    OE_MAKE_VERSION(OE_VERSION_MAJOR, OE_VERSION_MINOR, OE_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define OE_RFRAMEHEADERSZ     32 // [uint64_t sample number, uint16_t n_devs, (22 reserved bytes), ...]
#define OE_RFRAMESAMPLEOFF    0  // Read frame sample number offset
#define OE_RFRAMENDEVOFF      8  // Read frame number of devices offset
#define OE_RFRAMENERROFF      10 // Read frame error offset

#define OE_WFRAMEHEADERSZ     32 // [(32 reserved bytes), ...]

#ifdef _WIN32
#define OE_DEFAULTCONFIGPATH  "\\\\.\\xillybus_cmd_32"
#define OE_DEFAULTREADPATH    "\\\\.\\xillybus_data_read_32"
#define OE_DEFAULTSIGNALPATH  "\\\\.\\xillybus_async_read_8"
#else
#define OE_DEFAULTCONFIGPATH  "/dev/xillybus_cmd_32"
#define OE_DEFAULTREADPATH    "/dev/xillybus_data_read_32"
#define OE_DEFAULTSIGNALPATH  "/dev/xillybus_async_read_8"
#endif

// Supported devices/IDs
// NB: If you add a device here, make sure to update oe_device_str() and
// potentially add registers to oedevices.h
enum oe_device_id {
    OE_IMMEDIATEIO = 0,
    OE_RHD2132,
    OE_RHD2164,
    OE_MPU9250,
    OE_ESTIM,

    // NB: Always on bottom
    OE_MAXDEVICEID
};

// Fixed width device types
typedef uint32_t oe_size_t;
typedef uint32_t oe_dev_id_t;
typedef uint32_t oe_reg_addr_t;
typedef uint32_t oe_reg_val_t;
typedef uint32_t oe_raw_t;

// Device type
typedef struct {
    oe_dev_id_t id;       // ID number; NB: Cannot use oe_device_id_t because this must be fixed width
    oe_size_t read_size;  // read size in bytes
    oe_raw_t read_type;   // read type
    oe_size_t write_size; // write size in bytes
    oe_raw_t write_type;  // write type

} oe_device_t;

// Frame type
typedef struct oe_frame {
    uint64_t clock;       // Base clock counter
    uint16_t num_dev;     // Number of devices in frame
    uint8_t corrupt;      // Is this frame corrupt?
    oe_size_t *dev_idxs;  // Array of device indices in frame
    size_t dev_idxs_sz;   // Size in bytes of dev_idxs buffer
    size_t *dev_offs;     // Device data offsets within data block
    size_t dev_offs_sz;   // Size in bytes of dev_idxs buffer
    uint8_t *data;        // Multi-device raw data block
    size_t data_sz;       // Size in bytes of data buffer

} oe_frame_t;

// Context options
enum {
    OE_CONFIGSTREAMPATH,
    OE_READSTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVICEMAP,
    OE_NUMDEVICES,
    OE_READFRAMESIZE,
    OE_WRITEFRAMESIZE,
    OE_RUNNING,
    OE_RESET,
    OE_SYSCLKHZ
};

// Allowed raw data types
enum {
    OE_UINT16 = 0,
    OE_UINT32,
};

// NB: If you add an error here, make sure to update oe_error_str()
enum {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EREINITCTX       = -2,  // Double initialization attempt
    OE_EDEVID           = -3,  // Invalid device ID on init or reg op
    OE_EREADFAILURE     = -4,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -5,  // Failure to write to a stream/register
    OE_ENULLCTX         = -6,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -7,  // Failure to seek on stream
    OE_EINVALSTATE      = -8,  // Invalid operation for the current context run state
    OE_EDEVIDX          = -9,  // Invalid device index
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
    OE_ECOBSPACK        = -13, // Invalid COBS packet
    OE_ERETRIG          = -14, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -15, // Supplied buffer is too small
    OE_EBADDEVMAP       = -16, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -17, // Bad dynamic memory allocation
    OE_ECLOSEFAIL       = -18, // File descriptor close failure, check errno
    OE_EDATATYPE        = -19, // Invalid underlying data types
    OE_EREADONLY        = -20, // Attempted write to read only object (register, context option, etc)
    OE_ERUNSTATESYNC    = -21, // Software and hardware run state out of sync
    OE_EINVALRAWTYPE    = -22, // Invalid raw data type

    // NB: Always at bottom
    OE_MINERRORNUM      = -23
};

// Context
typedef struct oe_ctx_impl *oe_ctx;

// Context manipulation
oe_ctx oe_create_ctx();
int oe_init_ctx(oe_ctx ctx);
int oe_destroy_ctx(oe_ctx ctx);

// Option getting/setting
int oe_get_opt(const oe_ctx ctx, int option, void* value, size_t *size);
int oe_set_opt(oe_ctx ctx, int option, const void* value, size_t size);

// Hardware inspection, manipulation, and IO
int oe_read_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t *value);
int oe_write_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value);
int oe_read_frame(const oe_ctx ctx, oe_frame_t **frame);
void oe_destroy_frame(oe_frame_t *frame);
//int oe_write_frame(const oe_ctx ctx, oe_frame_t *frame);

// Internal type conversion
void oe_version(int *major, int *minor, int *patch);
const char *oe_error_str(int err);
const char *oe_device_str(int dev_id);

#ifdef __cplusplus
}
#endif

#endif

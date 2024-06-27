/********************************** (C) COPYRIGHT *******************************
* File Name          : PAW3395.h
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/10
* Description        : 
*******************************************************************************/

#ifndef __PAW3395_H
#define __PAW3395_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SPI_CS              (GPIO_Pin_12)//A
#define SPI_SCK             (GPIO_Pin_13)
#define SPI_MOSI             (GPIO_Pin_14)
#define SPI_MISO            (GPIO_Pin_15)
#define SPI_MOTION            (GPIO_Pin_5)
#define SPI_NRESET            (GPIO_Pin_22)//B

typedef enum{
    PAW_PID1,
    PAW_PID2,
    PAW_MOTION_STATUS,
    PAW_DELTA_X_L,
    PAW_DELTA_X_H,
    PAW_DELTA_Y_L,
    PAW_DELTA_Y_H,

    PAW_MOTION_BURST = 0x16,
    PAW_POWER_UP_RESET = 0x3A,

    PAW_SET_RESOLUTION = 0x47,
    PAW_RESOLUTION_X_LOW,
    PAW_RESOLUTION_X_HIGH,
    PAW_RESOLUTION_Y_LOW,
    PAW_RESOLUTION_Y_HIGH,

    PAW_AXIS_CONTRAL = 0x5B,
    PAW_Angle_Tune,
    PAW_Frame_Capture,

    PAW_RUN_DOWNSHIFT = 0x77,
    PAW_REST1_PERIOD,
    PAW_REST1_DOWNSHIFT,
    PAW_REST2_PERIOD,
    PAW_REST2_DOWNSHIFT,
    PAW_REST3_PERIOD,
    PAW_RUN_DOWNSHIFT_MULT,
    PAW_REST_DOWNSHIFT_MULT,
}paw3395_reg_t;


struct paw3395_protocol {
    uint8_t address : 7;
    /* Transmission direction, 0: Read; 1: Write */
    uint8_t direction : 1;
};

struct paw3395_config {
    uint8_t reserved_0 : 3;
    /* Power down mode for lowest power consumption
     * 0 = Normal operation (Default)
     * 1 = Power down mode (but retain the sensor register settings) */
    uint8_t power_down : 1;
    uint8_t reserved_1 : 3;
    /* Full chip reset. This bit will be de-asserted automatically.
     * 0 = Normal operation mode (Default)
     * 1 = Full chip reset (to reset all the sensor's internal registers and states) */
    uint8_t reset      : 1;
};

/**
 * @brief Typically in the motion detection routine, the host controller will poll the sensor
 * for valid motion data by checking the Motion bit. If the Motion bit is set, the motion data in
 * Delta_X and Delta_Y registers are valid and ready to be read.
 *
 * Be sure to read Motion bit first before reading out Delta_X and Delta_Y registers.
 *
 * DXOVF bit and DYOVF bit show whether if the motion report buffers have overflowed since last read out.
 */
struct motion_status {
    /* Motion detected since last report
    * 0 = No motion (Default)
    * 1 = Motion detected, data in Delta_X and Delta_Y registers
    * are valid and ready to be read out */
    uint8_t reserved_0  : 2;
    /* Delta_Y overflowed since last read out
     * 0 = No overflow (Default)
     * 1 = Overflow occurred */
    uint8_t dyovf       : 1;
    /* Delta_X overflowed since last read out
     * 0 = No overflow (Default)
     * 1 = Overflow occurred */
    uint8_t dxovf       : 1;
    uint8_t reserved_1  : 3;
    uint8_t motion      : 1;
};

struct operation_mode {
    /* Wakeup sensor from Sleep mode.
     * Set '1' to wake up and then it will be reset to '0' automatically */
    uint8_t wakeup : 1;
    /* Force to enter Sleep1 mode.
     * Set '1' to enter Sleep1, and then it will be reset to '0' automatically */
    uint8_t sleep1_enter : 1;
    /* Force to enter Sleep2 mode.
     * Set '1' to enter Sleep2, and then it will be reset to '0' automatically */
    uint8_t sleep2_enter : 1;
    /* Enable/Disable Sleep2 mode
     * 0 = Disable (Default)
     * 1 = Enable */
    uint8_t sleep2_enbale : 1;
    /* Enable/Disable Sleep mode (including Sleep1 and Sleep2)
     * 0 = Disable (Default)
     * 1 = Enable */
    uint8_t sleep_enbale : 1;
    uint8_t reserved : 1;
};

/**
 * @brief To select the mouse X/Y direction and Delta_X, Delta_Y motion data length (8-bit or 12-bit).
 *
 */
struct mouse_option {
    uint8_t reserved_0  : 2;
    uint8_t bit8       : 1;
    /* To invert the X direction. Default is 0. */
    uint8_t x_inv       : 1;
    /* To invert the Y direction. Default is 0. */
    uint8_t y_inv       : 1;
    /* To swap the XY direction. Default is 0. */
    uint8_t xy_sw       : 1;
    uint8_t reserved_1  : 2;
};

/**
 * @brief Sleep1 register allows users to set the sampling frequency time during Sleep1 mode
 * and the entering time from Run mode to Sleep1 mode.
 */
struct sleep1_config {
    /* Each step is equivalent to 32ms. Relative to its value 0 ~ 15,
     * the entering time is 32ms ~ 512ms. Default 7 (256ms) */
    uint8_t etm1     : 4;
    /* Each step is equivalent to 4ms. Relative to its value 0 ~ 15,
     * the sampling frequency time is 4ms ~ 64ms. Default 7 (32ms) */
    uint8_t freq1    : 4;
};

void paw3395_write_byte(paw3395_reg_t reg, uint8_t data);
uint8_t paw3395_read_byte(paw3395_reg_t reg);

/**
 * @brief Write Protect register is used to avoid host controller mis-writing the registers after address 0x09.
 */
static inline void paw3395_write_protect_enable(void)
{
//    paw3395_write_byte(PAW_WRITE_PROTECT, 0);
}

static inline void paw3395_write_protect_disable(void)
{
//    paw3395_write_byte(PAW_WRITE_PROTECT, 0x5a);
}

void paw3395_write_protect_byte(paw3395_reg_t reg, uint8_t data);
static void paw3395_4K_Mode( void );
static void paw3395_gaming_Mode( void );
static void paw3395_lowpower_Mode( void );
static void paw3395_office_Mode( void );
void spi_write_byte(uint8_t data);
uint8_t spi_read_byte(void);
void mouse_init(void);
void paw3395_init(void);
#ifdef __cplusplus
}
#endif

#endif

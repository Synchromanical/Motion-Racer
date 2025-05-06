/**
 ******************************************************************************
 * @file    lsm6dsl.c
 * @author  ...
 * @brief   This file provides a set of functions needed to manage the LSM6DSL
 *          accelero and gyro devices
 ******************************************************************************
 */

#include "lsm6dsl.h"

/** @addtogroup BSP
  * @{
  */

/** @defgroup LSM6DSL
  * @{
  */

/** @defgroup LSM6DSL_Private_Variables
  * @{
  */
ACCELERO_DrvTypeDef Lsm6dslAccDrv =
{
  LSM6DSL_AccInit,
  LSM6DSL_AccDeInit,
  LSM6DSL_AccReadID,
  0,
  LSM6DSL_AccLowPower,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  LSM6DSL_AccReadXYZ
};

GYRO_DrvTypeDef Lsm6dslGyroDrv =
{
  LSM6DSL_GyroInit,
  LSM6DSL_GyroDeInit,
  LSM6DSL_GyroReadID,
  0,
  LSM6DSL_GyroLowPower,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  LSM6DSL_GyroReadXYZAngRate
};

/**
  * @brief  Set LSM6DSL Accelerometer Initialization.
  * @param  InitStruct: Init parameters
  */
void LSM6DSL_AccInit(uint16_t InitStruct)
{
  uint8_t ctrl = 0x00;
  uint8_t tmp;

  /* Read CTRL1_XL */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);

  /* Write value to ACC MEMS CTRL1_XL register: FS and Data Rate */
  ctrl = (uint8_t) InitStruct;
  tmp &= ~(0xFC);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, tmp);

  /* Read CTRL3_C */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C);

  /* Write value to ACC MEMS CTRL3_C register: BDU and Auto-increment */
  ctrl = ((uint8_t) (InitStruct >> 8));
  tmp &= ~(0x44);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}

/**
  * @brief  LSM6DSL Accelerometer De-initialization.
  */
void LSM6DSL_AccDeInit(void)
{
  uint8_t ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);

  /* Clear ODR bits => set Power down */
  ctrl &= ~(0xF0);
  ctrl |= 0x00;  /* LSM6DSL_ODR_POWER_DOWN = 0x00 */
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, ctrl);
}

/**
  * @brief  Read LSM6DSL ID.
  * @retval ID
  */
uint8_t LSM6DSL_AccReadID(void)
{
  /* IO interface initialization if needed */
  SENSOR_IO_Init();
  return SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);
}

/**
  * @brief  Set/Unset Accelerometer in low power mode.
  */
void LSM6DSL_AccLowPower(uint16_t status)
{
  uint8_t ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C);

  /* Clear Low Power bit in CTRL6_C */
  ctrl &= ~(0x10);

  if(status)
  {
    ctrl |= 0x10; /* LSM6DSL_ACC_GYRO_LP_XL_ENABLED */
  }
  else
  {
    ctrl |= 0x00; /* LSM6DSL_ACC_GYRO_LP_XL_DISABLED */
  }
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C, ctrl);
}

/**
  * @brief  Read X, Y & Z Acceleration values
  */
void LSM6DSL_AccReadXYZ(int16_t* pData)
{
  int16_t pnRawData[3];
  uint8_t buffer[6];
  uint8_t i;
  float sensitivity = 0;
  
  uint8_t ctrl1_xl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);

  /* Read 6 bytes: X (2), Y (2), Z (2) */
  SENSOR_IO_ReadMultiple(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW,
                         LSM6DSL_ACC_GYRO_OUTX_L_XL,
                         buffer,
                         6);

  /* Reconstruct the 16-bit raw data */
  for(i = 0; i < 3; i++)
  {
    pnRawData[i] = (int16_t)((((uint16_t)buffer[2*i+1]) << 8) + buffer[2*i]);
  }

  /* Determine sensitivity from FS bits */
  switch(ctrl1_xl & 0x0C)
  {
    case LSM6DSL_ACC_FULLSCALE_2G:
      sensitivity = LSM6DSL_ACC_SENSITIVITY_2G;
      break;
    case LSM6DSL_ACC_FULLSCALE_4G:
      sensitivity = LSM6DSL_ACC_SENSITIVITY_4G;
      break;
    case LSM6DSL_ACC_FULLSCALE_8G:
      sensitivity = LSM6DSL_ACC_SENSITIVITY_8G;
      break;
    case LSM6DSL_ACC_FULLSCALE_16G:
      sensitivity = LSM6DSL_ACC_SENSITIVITY_16G;
      break;
  }

  /* Convert the raw data to mg (int16) */
  for(i = 0; i < 3; i++)
  {
    pData[i] = (int16_t)(pnRawData[i] * sensitivity);
  }
}

/* =================== GYRO PART =================== */

/**
  * @brief  Set LSM6DSL Gyroscope Initialization.
  */
void LSM6DSL_GyroInit(uint16_t InitStruct)
{
  uint8_t ctrl = 0x00;
  uint8_t tmp;

  /* Read CTRL2_G */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);

  /* FS and Data Rate bits => from InitStruct */
  ctrl = (uint8_t)InitStruct;
  tmp &= ~(0xFC);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G, tmp);

  /* Read CTRL3_C */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C);

  /* BDU and Auto-increment => from (InitStruct >> 8) */
  ctrl = ((uint8_t)(InitStruct >> 8));
  tmp &= ~(0x44);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}

/**
  * @brief  LSM6DSL Gyroscope De-initialization
  */
void LSM6DSL_GyroDeInit(void)
{
  uint8_t ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);

  /* Clear ODR => power down */
  ctrl &= ~(0xF0);
  ctrl |= 0x00;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G, ctrl);
}

/**
  * @brief  Read ID address of LSM6DSL
  */
uint8_t LSM6DSL_GyroReadID(void)
{
  SENSOR_IO_Init();
  return SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);
}

/**
  * @brief  Set/Unset LSM6DSL Gyroscope in low power mode
  */
void LSM6DSL_GyroLowPower(uint16_t status)
{
  uint8_t ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL7_G);

  /* Clear LP bit */
  ctrl &= ~(0x80);

  if(status)
  {
    ctrl |= 0x80;  /* LSM6DSL_ACC_GYRO_LP_G_ENABLED */
  }
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL7_G, ctrl);
}

/**
  * @brief  Calculate the LSM6DSL angular data in dps
  * @param  pfData: 3 floats [X, Y, Z]
  */
void LSM6DSL_GyroReadXYZAngRate(float *pfData)
{
  int16_t pnRawData[3];
  uint8_t buffer[6];
  uint8_t ctrl2_g;
  uint8_t i;
  float sensitivity = 0;
  
  /* Read FS bits => ctrl2_g */
  ctrl2_g = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);

  /* Read output register X, Y & Z gyro => 6 bytes */
  SENSOR_IO_ReadMultiple(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW,
                         LSM6DSL_ACC_GYRO_OUTX_L_G,
                         buffer,
                         6);

  /* Reconstruct raw data */
  for(i=0; i<3; i++)
  {
    pnRawData[i] = (int16_t)(((uint16_t)buffer[2*i+1] << 8) + buffer[2*i]);
  }

  /* Determine sensitivity from FS bits in CTRL2_G[3:2] */
  switch(ctrl2_g & 0x0C)
  {
    case LSM6DSL_GYRO_FS_245:
      sensitivity = LSM6DSL_GYRO_SENSITIVITY_245DPS;
      break;
    case LSM6DSL_GYRO_FS_500:
      sensitivity = LSM6DSL_GYRO_SENSITIVITY_500DPS;
      break;
    case LSM6DSL_GYRO_FS_1000:
      sensitivity = LSM6DSL_GYRO_SENSITIVITY_1000DPS;
      break;
    case LSM6DSL_GYRO_FS_2000:
      sensitivity = LSM6DSL_GYRO_SENSITIVITY_2000DPS;
      break;
  }

  /* Convert the raw data to actual dps (float) */
  for(i=0; i<3; i++)
  {
    pfData[i] = (float)(pnRawData[i] * sensitivity);
  }
}

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : b_l475e_iot01a2_bus.c
  * @brief          : source file for the BSP BUS IO driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "b_l475e_iot01a2_bus.h"
#include "stm32l4xx_hal.h"

__weak HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef* hi2c);

/** @addtogroup BSP
  * @{
  */

/** @addtogroup B_L475E_IOT01A2
  * @{
  */

/** @defgroup B_L475E_IOT01A2_BUS B_L475E_IOT01A2 BUS
  * @{
  */

#define DIV_ROUND_CLOSEST(x, d)  (((x) + ((d) / 2U)) / (d))

#define I2C_ANALOG_FILTER_DELAY_MIN            50U     /* ns */
#define I2C_ANALOG_FILTER_DELAY_MAX            260U    /* ns */
#define I2C_ANALOG_FILTER_DELAY_DEFAULT        2U      /* ns */

#ifndef I2C_VALID_TIMING_NBR
  #define I2C_VALID_TIMING_NBR                 128U
#endif

#define I2C_SPEED_STANDARD                     0U    /* 100 kHz */
#define I2C_SPEED_FAST                         1U    /* 400 kHz */
#define I2C_SPEED_FAST_PLUS                    2U    /* 1 MHz */

#define I2C_PRESC_MAX                          16U
#define I2C_SCLDEL_MAX                         16U
#define I2C_SDADEL_MAX                         16U
#define I2C_SCLH_MAX                           256U
#define I2C_SCLL_MAX                           256U
#define SEC2NSEC                               1000000000UL

#if (USE_CUBEMX_BSP_V2 == 1)
/**
  * @brief I2C_charac
  *  freq: I2C bus speed (Hz)
  *  freq_min: 80% of I2C bus speed (Hz)
  *  freq_max: 100% of I2C bus speed (Hz)
  *  fall_max: Max fall time of both SDA and SCL signals (ns)
  *  rise_max: Max rise time of both SDA and SCL signals (ns)
  *  hddat_min: Min data hold time (ns)
  *  vddat_max: Max data valid time (ns)
  *  sudat_min: Min data setup time (ns)
  *  lscl_min: Min low period of the SCL clock (ns)
  *  hscl_min: Min high period of the SCL clock (ns)
  *  trise: Rise time (ns)
  *  tfall: Fall time (ns)
  *  dnf: Digital filter coefficient (0-16)
  */
typedef struct
{
  uint32_t freq;
  uint32_t freq_min;
  uint32_t freq_max;
  uint32_t hddat_min;
  uint32_t vddat_max;
  uint32_t sudat_min;
  uint32_t lscl_min;
  uint32_t hscl_min;
  uint32_t trise;
  uint32_t tfall;
  uint32_t dnf;
}I2C_Charac_t;

/**
  * @brief I2C timings parameters
  *  presc: Prescaler value
  *  tscldel: Data setup time
  *  tsdadel: Data hold time
  *  sclh: SCL high period (master mode)
  *  scll: SCL low period (master mode)
  */
typedef struct
{
  uint32_t presc;
  uint32_t tscldel;
  uint32_t tsdadel;
  uint32_t sclh;
  uint32_t scll;
}I2C_Timings_t;

static const I2C_Charac_t I2C_Charac[] =
{
  [I2C_SPEED_STANDARD] =
  {
    .freq = 100000,
    .freq_min = 80000,
    .freq_max = 120000,
    .hddat_min = 0,
    .vddat_max = 3450,
    .sudat_min = 250,
    .lscl_min = 4700,
    .hscl_min = 4000,
    .trise = 400,
    .tfall = 100,
    .dnf = I2C_ANALOG_FILTER_DELAY_DEFAULT,
  },
  [I2C_SPEED_FAST] =
  {
    .freq = 400000,
    .freq_min = 320000,
    .freq_max = 480000,
    .hddat_min = 0,
    .vddat_max = 900,
    .sudat_min = 100,
    .lscl_min = 1300,
    .hscl_min = 600,
    .trise = 250,
    .tfall = 100,
    .dnf = I2C_ANALOG_FILTER_DELAY_DEFAULT,
  },
  [I2C_SPEED_FAST_PLUS] =
  {
    .freq = 1000000,
    .freq_min = 800000,
    .freq_max = 1200000,
    .hddat_min = 0,
    .vddat_max = 450,
    .sudat_min = 50,
    .lscl_min = 500,
    .hscl_min = 260,
    .trise = 60,
    .tfall = 100,
    .dnf = I2C_ANALOG_FILTER_DELAY_DEFAULT,
  },
};
static I2C_Timings_t valid_timing[I2C_VALID_TIMING_NBR];
static uint32_t valid_timing_nbr = 0;
#endif

/** @defgroup B_L475E_IOT01A2_BUS_Exported_Variables BUS Exported Variables
  * @{
  */

//I2C_HandleTypeDef hi2c2;
/**
  * @}
  */

/** @defgroup B_L475E_IOT01A2_BUS_Private_Variables BUS Private Variables
  * @{
  */

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1U)
static uint32_t IsI2C2MspCbValid = 0;
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
static uint32_t I2C2InitCounter = 0;

/**
  * @}
  */

/** @defgroup B_L475E_IOT01A2_BUS_Private_FunctionPrototypes  BUS Private Function
  * @{
  */

static void I2C2_MspInit(I2C_HandleTypeDef* hI2c);
static void I2C2_MspDeInit(I2C_HandleTypeDef* hI2c);
#if (USE_CUBEMX_BSP_V2 == 1)
static uint32_t I2C_GetTiming(uint32_t clock_src_hz, uint32_t i2cfreq_hz);
static void Compute_PRESC_SCLDEL_SDADEL(uint32_t clock_src_freq, uint32_t I2C_Speed);
static uint32_t Compute_SCLL_SCLH (uint32_t clock_src_freq, uint32_t I2C_speed);
#endif

/**
  * @}
  */

/** @defgroup B_L475E_IOT01A2_LOW_LEVEL_Private_Functions B_L475E_IOT01A2 LOW LEVEL Private Functions
  * @{
  */

/** @defgroup B_L475E_IOT01A2_BUS_Exported_Functions B_L475E_IOT01A2_BUS Exported Functions
  * @{
  */

/* BUS IO driver over I2C Peripheral */
/*******************************************************************************
                            BUS OPERATIONS OVER I2C
*******************************************************************************/
/**
  * @brief  Initialize I2C HAL
  * @retval BSP status
  */
int32_t BSP_I2C2_Init(void)
{
  int32_t ret = BSP_ERROR_NONE;

  hi2c2.Instance = I2C2;

  if(I2C2InitCounter++ == 0)
  {
    /* If the I2C is already initialized, skip re-init */
    if (HAL_I2C_GetState(&hi2c2) == HAL_I2C_STATE_RESET)
    {
      I2C2_MspInit(&hi2c2);

      /* Basic config for 1MHz or 400K as you set in b_l475e_iot01a2_conf.h */
      hi2c2.Init.Timing           = 0x10D19CE4; /* Example timing for 1MHz on L4 */
      hi2c2.Init.OwnAddress1      = 0;
      hi2c2.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
      hi2c2.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
      hi2c2.Init.OwnAddress2      = 0;
      hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
      hi2c2.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
      hi2c2.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
      if (HAL_I2C_Init(&hi2c2) != HAL_OK)
      {
        ret = BSP_ERROR_BUS_FAILURE;
      }
      else
      {
        /* Activate Analog filter */
        if(HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        {
          ret = BSP_ERROR_BUS_FAILURE;
        }
      }
    }
  }
  return ret;
}

/**
  * @brief  DeInitialize I2C2
  */
int32_t BSP_I2C2_DeInit(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (I2C2InitCounter > 0)
  {
    if (--I2C2InitCounter == 0)
    {
      /* DeInit the I2C */
      if (HAL_I2C_DeInit(&hi2c2) != HAL_OK)
      {
        ret = BSP_ERROR_BUS_FAILURE;
      }
      I2C2_MspDeInit(&hi2c2);
    }
  }
  return ret;
}

/**
  * @brief  Check whether the I2C2 bus is ready.
  */
int32_t BSP_I2C2_IsReady(uint16_t DevAddr, uint32_t Trials)
{
  if (HAL_I2C_IsDeviceReady(&hi2c2, DevAddr, Trials, BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
    return BSP_ERROR_NONE;
  else
    return BSP_ERROR_BUSY;
}


/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write
  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP status
  */

/**
  * @brief  Write registers (8-bit) to device.
  */
int32_t BSP_I2C2_WriteReg(uint16_t Addr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  if(HAL_I2C_Mem_Write(&hi2c2, Addr, (uint16_t)Reg,
                       I2C_MEMADD_SIZE_8BIT, pData, Length,
                       BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
    return BSP_ERROR_NONE;
  else
    return BSP_ERROR_PERIPH_FAILURE;
}

/**
  * @brief  Read a register of the device through BUS
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to read
  * @param  pData  Pointer to data buffer to read
  * @param  Length Data Length
  * @retval BSP status
  */
/**
  * @brief  Read registers (8-bit) from device.
  */
int32_t BSP_I2C2_ReadReg(uint16_t Addr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  if(HAL_I2C_Mem_Read(&hi2c2, Addr, (uint16_t)Reg,
                      I2C_MEMADD_SIZE_8BIT, pData, Length,
                      BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
    return BSP_ERROR_NONE;
  else
    return BSP_ERROR_PERIPH_FAILURE;
}


/**

  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write

  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP statu
  */
int32_t BSP_I2C2_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = BSP_ERROR_NONE;

  if (HAL_I2C_Mem_Write(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, BUS_I2C2_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(&hi2c2) == HAL_I2C_ERROR_AF)
    {
      ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Read registers through a bus (16 bits)
  * @param  DevAddr: Device address on BUS
  * @param  Reg: The target register address to read
  * @param  Length Data Length
  * @retval BSP status
  */
int32_t  BSP_I2C2_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = BSP_ERROR_NONE;

  if (HAL_I2C_Mem_Read(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, BUS_I2C2_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF)
    {
      ret =  BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Send an amount width data through bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int32_t BSP_I2C2_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
  int32_t ret = BSP_ERROR_NONE;

  if (HAL_I2C_Master_Transmit(&hi2c2, DevAddr, pData, Length, BUS_I2C2_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF)
    {
      ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Receive an amount of data through a bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int32_t BSP_I2C2_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length) {
  int32_t ret = BSP_ERROR_NONE;

  if (HAL_I2C_Master_Receive(&hi2c2, DevAddr, pData, Length, BUS_I2C2_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF)
    {
      ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1U)
/**
  * @brief Register Default BSP I2C2 Bus Msp Callbacks
  * @retval BSP status
  */
int32_t BSP_I2C2_RegisterDefaultMspCallbacks (void)
{

  __HAL_I2C_RESET_HANDLE_STATE(&hi2c2);

  /* Register MspInit Callback */
  if (HAL_I2C_RegisterCallback(&hi2c2, HAL_I2C_MSPINIT_CB_ID, I2C2_MspInit)  != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Register MspDeInit Callback */
  if (HAL_I2C_RegisterCallback(&hi2c2, HAL_I2C_MSPDEINIT_CB_ID, I2C2_MspDeInit) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  IsI2C2MspCbValid = 1;

  return BSP_ERROR_NONE;
}

/**
  * @brief BSP I2C2 Bus Msp Callback registering
  * @param Callbacks     pointer to I2C2 MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_I2C2_RegisterMspCallbacks (BSP_I2C_Cb_t *Callbacks)
{
  /* Prevent unused argument(s) compilation warning */
  __HAL_I2C_RESET_HANDLE_STATE(&hi2c2);

   /* Register MspInit Callback */
  if (HAL_I2C_RegisterCallback(&hi2c2, HAL_I2C_MSPINIT_CB_ID, Callbacks->pMspInitCb)  != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Register MspDeInit Callback */
  if (HAL_I2C_RegisterCallback(&hi2c2, HAL_I2C_MSPDEINIT_CB_ID, Callbacks->pMspDeInitCb) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  IsI2C2MspCbValid = 1;

  return BSP_ERROR_NONE;
}
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */

/**
  * @brief  Return system tick in ms
  * @retval Current HAL time base time stamp
  */
int32_t BSP_GetTick(void) {
  return HAL_GetTick();
}

/* I2C2 init function */

__weak HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef* hi2c)
{
  HAL_StatusTypeDef ret = HAL_OK;

  hi2c->Instance = I2C2;
  hi2c->Init.Timing = 0x10D19CE4;
  hi2c->Init.OwnAddress1 = 0;
  hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c->Init.OwnAddress2 = 0;
  hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(hi2c) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  if (HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  if (HAL_I2CEx_ConfigDigitalFilter(hi2c, 0) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

/* Low-level clock, pin init for I2C2 */
static void I2C2_MspInit(I2C_HandleTypeDef* i2cHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef  PeriphClkInit = {0};

  /* Select PCLK1 as source for I2C2 clocks */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
  PeriphClkInit.I2c2ClockSelection   = RCC_I2C2CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  /* Enable GPIO clock for SCL/SDA pins */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PB10 -> I2C2_SCL, PB11 -> I2C2_SDA */
  GPIO_InitStruct.Pin       = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Enable I2C2 clock */
  __HAL_RCC_I2C2_CLK_ENABLE();
}

/* DeInit pins & clock */
static void I2C2_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{
  /* Disable I2C2 clock */
  __HAL_RCC_I2C2_CLK_DISABLE();

  /* DeInit I2C2 SCL and SDA pins (PB10, PB11) */
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
}


#if (USE_CUBEMX_BSP_V2 == 1)
/**
  * @brief  Convert the I2C Frequency into I2C timing.
  * @note   The algorithm to compute the different fields of the Timings register
  *         is described in the AN4235 and the charac timings are inline with
  *         the product RM.
  * @param  clock_src_freq : I2C source clock in HZ.
  * @param  i2c_freq : I2C frequency in Hz.
  * @retval I2C timing value
  */
static uint32_t I2C_GetTiming(uint32_t clock_src_freq, uint32_t i2c_freq)
{
  uint32_t ret = 0;
  uint32_t speed;
  uint32_t idx;

  if((clock_src_freq != 0U) && (i2c_freq != 0U))
  {
    for ( speed = 0 ; speed <=  (uint32_t)I2C_SPEED_FAST_PLUS ; speed++)
    {
      if ((i2c_freq >= I2C_Charac[speed].freq_min) &&
          (i2c_freq <= I2C_Charac[speed].freq_max))
      {
        Compute_PRESC_SCLDEL_SDADEL(clock_src_freq, speed);
        idx = Compute_SCLL_SCLH(clock_src_freq, speed);

        if (idx < I2C_VALID_TIMING_NBR)
        {
          ret = ((valid_timing[idx].presc  & 0x0FU) << 28) |\
                ((valid_timing[idx].tscldel & 0x0FU) << 20) |\
                ((valid_timing[idx].tsdadel & 0x0FU) << 16) |\
                ((valid_timing[idx].sclh & 0xFFU) << 8) |\
                ((valid_timing[idx].scll & 0xFFU) << 0);
        }
        break;
      }
    }
  }

  return ret;
}

/**
  * @brief  Compute PRESC, SCLDEL and SDADEL.
  * @param  clock_src_freq : I2C source clock in HZ.
  * @param  I2C_Speed : I2C frequency (index).
  * @retval None
  */
static void Compute_PRESC_SCLDEL_SDADEL(uint32_t clock_src_freq, uint32_t I2C_Speed)
{
  uint32_t prev_presc = I2C_PRESC_MAX;
  uint32_t ti2cclk;
  int32_t  tsdadel_min, tsdadel_max;
  int32_t  tscldel_min;
  uint32_t presc, scldel, sdadel;

  ti2cclk   = (SEC2NSEC + (clock_src_freq / 2U))/ clock_src_freq;

  /* tDNF = DNF x tI2CCLK
     tPRESC = (PRESC+1) x tI2CCLK
     SDADEL >= {tf +tHD;DAT(min) - tAF(min) - tDNF - [3 x tI2CCLK]} / {tPRESC}
     SDADEL <= {tVD;DAT(max) - tr - tAF(max) - tDNF- [4 x tI2CCLK]} / {tPRESC} */

  tsdadel_min = (int32_t)I2C_Charac[I2C_Speed].tfall + (int32_t)I2C_Charac[I2C_Speed].hddat_min -
    (int32_t)I2C_ANALOG_FILTER_DELAY_MIN - (int32_t)(((int32_t)I2C_Charac[I2C_Speed].dnf + 3) * (int32_t)ti2cclk);

  tsdadel_max = (int32_t)I2C_Charac[I2C_Speed].vddat_max - (int32_t)I2C_Charac[I2C_Speed].trise -
    (int32_t)I2C_ANALOG_FILTER_DELAY_MAX - (int32_t)(((int32_t)I2C_Charac[I2C_Speed].dnf + 4) * (int32_t)ti2cclk);

  /* {[tr+ tSU;DAT(min)] / [tPRESC]} - 1 <= SCLDEL */
  tscldel_min = (int32_t)I2C_Charac[I2C_Speed].trise + (int32_t)I2C_Charac[I2C_Speed].sudat_min;

  if (tsdadel_min <= 0)
  {
    tsdadel_min = 0;
  }

  if (tsdadel_max <= 0)
  {
    tsdadel_max = 0;
  }

  for (presc = 0; presc < I2C_PRESC_MAX; presc++)
  {
    for (scldel = 0; scldel < I2C_SCLDEL_MAX; scldel++)
    {
      /* TSCLDEL = (SCLDEL+1) * (PRESC+1) * TI2CCLK */
      uint32_t tscldel = (scldel + 1U) * (presc + 1U) * ti2cclk;

      if (tscldel >= (uint32_t)tscldel_min)
      {
        for (sdadel = 0; sdadel < I2C_SDADEL_MAX; sdadel++)
        {
          /* TSDADEL = SDADEL * (PRESC+1) * TI2CCLK */
          uint32_t tsdadel = (sdadel * (presc + 1U)) * ti2cclk;

          if ((tsdadel >= (uint32_t)tsdadel_min) && (tsdadel <= (uint32_t)tsdadel_max))
          {
            if(presc != prev_presc)
            {
              valid_timing[valid_timing_nbr].presc = presc;
              valid_timing[valid_timing_nbr].tscldel = scldel;
              valid_timing[valid_timing_nbr].tsdadel = sdadel;
              prev_presc = presc;
              valid_timing_nbr ++;

              if(valid_timing_nbr >= I2C_VALID_TIMING_NBR)
              {
                return;
              }
            }
          }
        }
      }
    }
  }
}

/**
  * @brief  Calculate SCLL and SCLH and find best configuration.
  * @param  clock_src_freq : I2C source clock in HZ.
  * @param  I2C_Speed : I2C frequency (index).
  * @retval config index (0 to I2C_VALID_TIMING_NBR], 0xFFFFFFFF : no valid config
  */
static uint32_t Compute_SCLL_SCLH (uint32_t clock_src_freq, uint32_t I2C_speed)
{
  uint32_t ret = 0xFFFFFFFFU;
  uint32_t ti2cclk;
  uint32_t ti2cspeed;
  uint32_t prev_error;
  uint32_t dnf_delay;
  uint32_t clk_min, clk_max;
  uint32_t scll, sclh;

  ti2cclk   = (SEC2NSEC + (clock_src_freq / 2U))/ clock_src_freq;
  ti2cspeed   = (SEC2NSEC + (I2C_Charac[I2C_speed].freq / 2U))/ I2C_Charac[I2C_speed].freq;

  /* tDNF = DNF x tI2CCLK */
  dnf_delay = I2C_Charac[I2C_speed].dnf * ti2cclk;

  clk_max = SEC2NSEC / I2C_Charac[I2C_speed].freq_min;
  clk_min = SEC2NSEC / I2C_Charac[I2C_speed].freq_max;

  prev_error = ti2cspeed;

  for (uint32_t count = 0; count < valid_timing_nbr; count++)
  {
    /* tPRESC = (PRESC+1) x tI2CCLK*/
    uint32_t tpresc = (valid_timing[count].presc + 1U) * ti2cclk;

    for (scll = 0; scll < I2C_SCLL_MAX; scll++)
    {
      /* tLOW(min) <= tAF(min) + tDNF + 2 x tI2CCLK + [(SCLL+1) x tPRESC ] */
      uint32_t tscl_l = I2C_ANALOG_FILTER_DELAY_MIN + dnf_delay + (2U * ti2cclk) + (scll + 1U) * tpresc;

      /* The I2CCLK period tI2CCLK must respect the following conditions:
      tI2CCLK < (tLOW - tfilters) / 4 and tI2CCLK < tHIGH */
      if ((tscl_l > I2C_Charac[I2C_speed].lscl_min) && (ti2cclk < ((tscl_l - I2C_ANALOG_FILTER_DELAY_MIN - dnf_delay) / 4U)))
      {
        for (sclh = 0; sclh < I2C_SCLH_MAX; sclh++)
        {
          /* tHIGH(min) <= tAF(min) + tDNF + 2 x tI2CCLK + [(SCLH+1) x tPRESC] */
          uint32_t tscl_h = I2C_ANALOG_FILTER_DELAY_MIN + dnf_delay + (2U * ti2cclk) + (sclh + 1U) * tpresc;

          /* tSCL = tf + tLOW + tr + tHIGH */
          uint32_t tscl = tscl_l + tscl_h + I2C_Charac[I2C_speed].trise + I2C_Charac[I2C_speed].tfall;

          if ((tscl >= clk_min) && (tscl <= clk_max) && (tscl_h >= I2C_Charac[I2C_speed].hscl_min) && (ti2cclk < tscl_h))
          {
            int32_t error = (int32_t)tscl - (int32_t)ti2cspeed;

            if (error < 0)
            {
              error = -error;
            }

            /* look for the timings with the lowest clock error */
            if ((uint32_t)error < prev_error)
            {
              prev_error = (uint32_t)error;
              valid_timing[count].scll = scll;
              valid_timing[count].sclh = sclh;
              ret = count;
            }
          }
        }
      }
    }
  }

  return ret;
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


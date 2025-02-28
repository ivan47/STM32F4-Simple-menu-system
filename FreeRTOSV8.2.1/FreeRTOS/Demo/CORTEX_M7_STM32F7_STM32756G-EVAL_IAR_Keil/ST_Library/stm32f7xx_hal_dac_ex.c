/**
  ******************************************************************************
  * @file    stm32f7xx_hal_dac_ex.c
  * @author  MCD Application Team
  * @version V1.0.0RC1
  * @date    24-March-2015
  * @brief   Extended DAC HAL module driver.
  *         This file provides firmware functions to manage the following 
  *         functionalities of DAC extension peripheral:
  *           + Extended features functions
  *     
  *
  @verbatim      
  ==============================================================================
                      ##### How to use this driver #####
  ==============================================================================
    [..]          
      (+) When Dual mode is enabled (i.e DAC Channel1 and Channel2 are used simultaneously) :
          Use HAL_DACEx_DualGetValue() to get digital data to be converted and use
          HAL_DACEx_DualSetValue() to set digital value to converted simultaneously in Channel 1 and Channel 2.  
      (+) Use HAL_DACEx_TriangleWaveGenerate() to generate Triangle signal.
      (+) Use HAL_DACEx_NoiseWaveGenerate() to generate Noise signal.
   
 @endverbatim    
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/** @addtogroup STM32F7xx_HAL_Driver
  * @{
  */

/** @defgroup DACEx DACEx
  * @brief DAC driver modules
  * @{
  */ 

#ifdef HAL_DAC_MODULE_ENABLED

#if defined(STM32F756xx) || defined(STM32F746xx)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup DACEx_Exported_Functions DAC Exported Functions
  * @{
  */

/** @defgroup DACEx_Exported_Functions_Group1 Extended features functions
 *  @brief    Extended features functions 
 *
@verbatim   
  ==============================================================================
                 ##### Extended features functions #####
  ==============================================================================  
    [..]  This section provides functions allowing to:
      (+) Start conversion.
      (+) Stop conversion.
      (+) Start conversion and enable DMA transfer.
      (+) Stop conversion and disable DMA transfer.
      (+) Get result of conversion.
      (+) Get result of dual mode conversion.
                     
@endverbatim
  * @{
  */

/**
  * @brief  Returns the last data output value of the selected DAC channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval The selected DAC channel data output value.
  */
uint32_t HAL_DACEx_DualGetValue(DAC_HandleTypeDef* hdac)
{
  uint32_t tmp = 0;
  
  tmp |= hdac->Instance->DOR1;
  
  tmp |= hdac->Instance->DOR2 << 16;
  
  /* Returns the DAC channel data output register value */
  return tmp;
}

/**
  * @brief  Enables or disables the selected DAC channel wave generation.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected 
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  Amplitude: Select max triangle amplitude. 
  *          This parameter can be one of the following values:
  *            @arg DAC_TRIANGLEAMPLITUDE_1: Select max triangle amplitude of 1
  *            @arg DAC_TRIANGLEAMPLITUDE_3: Select max triangle amplitude of 3
  *            @arg DAC_TRIANGLEAMPLITUDE_7: Select max triangle amplitude of 7
  *            @arg DAC_TRIANGLEAMPLITUDE_15: Select max triangle amplitude of 15
  *            @arg DAC_TRIANGLEAMPLITUDE_31: Select max triangle amplitude of 31
  *            @arg DAC_TRIANGLEAMPLITUDE_63: Select max triangle amplitude of 63
  *            @arg DAC_TRIANGLEAMPLITUDE_127: Select max triangle amplitude of 127
  *            @arg DAC_TRIANGLEAMPLITUDE_255: Select max triangle amplitude of 255
  *            @arg DAC_TRIANGLEAMPLITUDE_511: Select max triangle amplitude of 511
  *            @arg DAC_TRIANGLEAMPLITUDE_1023: Select max triangle amplitude of 1023
  *            @arg DAC_TRIANGLEAMPLITUDE_2047: Select max triangle amplitude of 2047
  *            @arg DAC_TRIANGLEAMPLITUDE_4095: Select max triangle amplitude of 4095                               
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_TriangleWaveGenerate(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t Amplitude)
{  
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_LFSR_UNMASK_TRIANGLE_AMPLITUDE(Amplitude));
  
  /* Process locked */
  __HAL_LOCK(hdac);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;
  
  /* Enable the selected wave generation for the selected DAC channel */
  MODIFY_REG(hdac->Instance->CR, (DAC_CR_WAVE1 | DAC_CR_MAMP1) << Channel, (DAC_CR_WAVE1_1 | Amplitude) << Channel);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Process unlocked */
  __HAL_UNLOCK(hdac);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Enables or disables the selected DAC channel wave generation.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC. 
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected 
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  Amplitude: Unmask DAC channel LFSR for noise wave generation. 
  *          This parameter can be one of the following values: 
  *            @arg DAC_LFSRUNMASK_BIT0: Unmask DAC channel LFSR bit0 for noise wave generation
  *            @arg DAC_LFSRUNMASK_BITS1_0: Unmask DAC channel LFSR bit[1:0] for noise wave generation  
  *            @arg DAC_LFSRUNMASK_BITS2_0: Unmask DAC channel LFSR bit[2:0] for noise wave generation
  *            @arg DAC_LFSRUNMASK_BITS3_0: Unmask DAC channel LFSR bit[3:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS4_0: Unmask DAC channel LFSR bit[4:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS5_0: Unmask DAC channel LFSR bit[5:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS6_0: Unmask DAC channel LFSR bit[6:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS7_0: Unmask DAC channel LFSR bit[7:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS8_0: Unmask DAC channel LFSR bit[8:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS9_0: Unmask DAC channel LFSR bit[9:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS10_0: Unmask DAC channel LFSR bit[10:0] for noise wave generation 
  *            @arg DAC_LFSRUNMASK_BITS11_0: Unmask DAC channel LFSR bit[11:0] for noise wave generation 
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_NoiseWaveGenerate(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t Amplitude)
{  
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_LFSR_UNMASK_TRIANGLE_AMPLITUDE(Amplitude));
  
  /* Process locked */
  __HAL_LOCK(hdac);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;
  
  /* Enable the selected wave generation for the selected DAC channel */
  MODIFY_REG(hdac->Instance->CR, (DAC_CR_WAVE1 | DAC_CR_MAMP1) << Channel, (DAC_CR_WAVE1_0 | Amplitude) << Channel);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Process unlocked */
  __HAL_UNLOCK(hdac);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Set the specified data holding register value for dual DAC channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *               the configuration information for the specified DAC.
  * @param  Alignment: Specifies the data alignment for dual channel DAC.
  *          This parameter can be one of the following values:
  *            DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @param  Data1: Data for DAC Channel2 to be loaded in the selected data holding register.
  * @param  Data2: Data for DAC Channel1 to be loaded in the selected data  holding register.
  * @note   In dual mode, a unique register access is required to write in both
  *          DAC channels at the same time.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualSetValue(DAC_HandleTypeDef* hdac, uint32_t Alignment, uint32_t Data1, uint32_t Data2)
{  
  uint32_t data = 0, tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_DAC_ALIGN(Alignment));
  assert_param(IS_DAC_DATA(Data1));
  assert_param(IS_DAC_DATA(Data2));
  
  /* Calculate and set dual DAC data holding register value */
  if (Alignment == DAC_ALIGN_8B_R)
  {
    data = ((uint32_t)Data2 << 8) | Data1; 
  }
  else
  {
    data = ((uint32_t)Data2 << 16) | Data1;
  }
  
  tmp = (uint32_t)hdac->Instance;
  tmp += DAC_DHR12RD_ALIGNMENT(Alignment);

  /* Set the dual DAC selected data holding register */
  *(__IO uint32_t *)tmp = data;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/**
  * @brief  Conversion complete callback in non blocking mode for Channel2 
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DAC_ConvCpltCallbackCh2 could be implemented in the user file
   */
}

/**
  * @brief  Conversion half DMA transfer callback in non blocking mode for Channel2 
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DACEx_ConvHalfCpltCallbackCh2 could be implemented in the user file
   */
}

/**
  * @brief  Error DAC callback for Channel2.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef *hdac)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DACEx_ErrorCallbackCh2 could be implemented in the user file
   */
}

/**
  * @brief  DMA underrun DAC callback for channel2.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_DMAUnderrunCallbackCh2(DAC_HandleTypeDef *hdac)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DACEx_DMAUnderrunCallbackCh2 could be implemented in the user file
   */
}

/**
  * @brief  DMA conversion complete callback. 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAConvCpltCh2(DMA_HandleTypeDef *hdma)   
{
  DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  
  HAL_DACEx_ConvCpltCallbackCh2(hdac); 
  
  hdac->State= HAL_DAC_STATE_READY;
}

/**
  * @brief  DMA half transfer complete callback. 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAHalfConvCpltCh2(DMA_HandleTypeDef *hdma)   
{
    DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
    /* Conversion complete callback */
    HAL_DACEx_ConvHalfCpltCallbackCh2(hdac); 
}

/**
  * @brief  DMA error callback 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAErrorCh2(DMA_HandleTypeDef *hdma)   
{
  DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
    
  /* Set DAC error code to DMA error */
  hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
    
  HAL_DACEx_ErrorCallbackCh2(hdac); 
    
  hdac->State= HAL_DAC_STATE_READY;
}

/**
  * @}
  */

#endif /* STM32F756xx || STM32F746xx */

#endif /* HAL_DAC_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

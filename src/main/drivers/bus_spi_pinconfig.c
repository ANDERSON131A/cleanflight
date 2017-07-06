/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#ifdef USE_SPI

#include "build/debug.h"

#include "drivers/bus_spi.h"
#include "drivers/bus_spi_impl.h"
#include "drivers/dma.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/rcc.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

// Pin defaults for backward compatibility
#ifndef SPI1_SCK_PIN
#define SPI1_SCK_PIN    PA5
#define SPI1_MISO_PIN   PA6
#define SPI1_MOSI_PIN   PA7
#endif

#ifndef SPI2_SCK_PIN
#define SPI2_SCK_PIN    PB13
#define SPI2_MISO_PIN   PB14
#define SPI2_MOSI_PIN   PB15
#endif

#ifndef SPI3_SCK_PIN
#define SPI3_SCK_PIN    PB3
#define SPI3_MISO_PIN   PB4
#define SPI3_MOSI_PIN   PB5
#endif

PG_DECLARE(spiPinConfig_t, spiPinConfig);

PG_REGISTER_WITH_RESET_FN(spiPinConfig_t, spiPinConfig, PG_SPI_PIN_CONFIG, 0);

typedef struct spiDefaultConfig_s {
    SPIDevice device;
    ioTag_t sck;
    ioTag_t miso;
    ioTag_t mosi;
} spiDefaultConfig_t;

const spiDefaultConfig_t spiDefaultConfig[] = {
#ifdef USE_SPI_DEVICE_1
    { SPIDEV_1, IO_TAG(SPI1_SCK_PIN), IO_TAG(SPI1_MISO_PIN), IO_TAG(SPI1_MOSI_PIN) },
#endif
#ifdef USE_SPI_DEVICE_2
    { SPIDEV_2, IO_TAG(SPI2_SCK_PIN), IO_TAG(SPI2_MISO_PIN), IO_TAG(SPI2_MOSI_PIN) },
#endif
#ifdef USE_SPI_DEVICE_3
    { SPIDEV_3, IO_TAG(SPI3_SCK_PIN), IO_TAG(SPI3_MISO_PIN), IO_TAG(SPI3_MOSI_PIN) },
#endif
#ifdef USE_SPI_DEVICE_4
    { SPIDEV_4, IO_TAG(SPI4_SCK_PIN), IO_TAG(SPI4_MISO_PIN), IO_TAG(SPI4_MOSI_PIN) },
#endif
};

void pgResetFn_spiPinConfig(spiPinConfig_t *spiPinConfig)
{
    for (size_t i = 0 ; i < ARRAYLEN(spiDefaultConfig) ; i++) {
        const spiDefaultConfig_t *defconf = &spiDefaultConfig[i];
        spiPinConfig->ioTagSck[defconf->device] = defconf->sck;
        spiPinConfig->ioTagMiso[defconf->device] = defconf->miso;
        spiPinConfig->ioTagMosi[defconf->device] = defconf->mosi;
    }
}


const spiHardware_t spiHardware[] = {
#ifdef STM32F1
    // Remapping is not supported and corresponding lines are commented out.
    // There also is some errata that may prevent these assignments from working:
    // http://www.st.com/content/ccc/resource/technical/document/errata_sheet/7d/02/75/64/17/fc/4d/fd/CD00190234.pdf/files/CD00190234.pdf/jcr:content/translations/en.CD00190234.pdf
    {
        .device = SPIDEV_1,
        .reg = SPI1,
        .sckPins = {
            { DEFIO_TAG_E(PA5) },
            // { DEFIO_TAG_E(PB3) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PA6) },
            // { DEFIO_TAG_E(PB4) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PA7) },
            // { DEFIO_TAG_E(PB5) },
        },
        .rcc = RCC_APB2(SPI1),
    },
    {
        .device = SPIDEV_2,
        .reg = SPI2,
        .sckPins = {
            { DEFIO_TAG_E(PB13) },
            // { DEFIO_TAG_E(PB3) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB14) },
            // { DEFIO_TAG_E(PB4) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB15) },
            // { DEFIO_TAG_E(PB5) },
        },
        .rcc = RCC_APB1(SPI2),
    },
#endif
#ifdef STM32F3

#ifndef GPIO_AF_SPI1
#define GPIO_AF_SPI1    GPIO_AF_5
#endif
#ifndef GPIO_AF_SPI2
#define GPIO_AF_SPI2    GPIO_AF_5
#endif
#ifndef GPIO_AF_SPI3
#define GPIO_AF_SPI3    GPIO_AF_6
#endif

    {
        .device = SPIDEV_1,
        .reg = SPI1,
        .sckPins = {
            { DEFIO_TAG_E(PA5) },
            { DEFIO_TAG_E(PB3) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PA6) },
            { DEFIO_TAG_E(PB4) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PA7) },
            { DEFIO_TAG_E(PB5) },
        },
        .af = GPIO_AF_SPI1,
        .rcc = RCC_APB2(SPI1),
    },
    {
        .device = SPIDEV_2,
        .reg = SPI2,
        .sckPins = {
            { DEFIO_TAG_E(PB13) },
            { DEFIO_TAG_E(PB3) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB14) },
            { DEFIO_TAG_E(PB4) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB15) },
            { DEFIO_TAG_E(PB5) },
        },
        .af = GPIO_AF_SPI2,
        .rcc = RCC_APB1(SPI2),
    },
    {
        .device = SPIDEV_3,
        .reg = SPI3,
        .sckPins = {
            { DEFIO_TAG_E(PB3) },
            { DEFIO_TAG_E(PC10) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB4) },
            { DEFIO_TAG_E(PC11) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB5) },
            { DEFIO_TAG_E(PC12) },
        },
        .af = GPIO_AF_SPI3,
        .rcc = RCC_APB1(SPI3),
    },
#endif
#ifdef STM32F4
    {
        .device = SPIDEV_1,
        .reg = SPI1,
        .sckPins = {
            { DEFIO_TAG_E(PA5) },
            { DEFIO_TAG_E(PB3) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PA6) },
            { DEFIO_TAG_E(PB4) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PA7) },
            { DEFIO_TAG_E(PB5) },
        },
        .af = GPIO_AF_SPI1,
        .rcc = RCC_APB2(SPI1),
    },
    {
        .device = SPIDEV_2,
        .reg = SPI2,
        .sckPins = {
            { DEFIO_TAG_E(PB10) },
            { DEFIO_TAG_E(PB13) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB14) },
            { DEFIO_TAG_E(PC2) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB15) },
            { DEFIO_TAG_E(PC3) },
        },
        .af = GPIO_AF_SPI2,
        .rcc = RCC_APB1(SPI2),
    },
    {
        .device = SPIDEV_3,
        .reg = SPI3,
        .sckPins = {
            { DEFIO_TAG_E(PB3) },
            { DEFIO_TAG_E(PC10) },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB4) },
            { DEFIO_TAG_E(PC11) },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB5) },
            { DEFIO_TAG_E(PC12) },
        },
        .af = GPIO_AF_SPI3,
        .rcc = RCC_APB1(SPI3),
    },
#endif
#ifdef STM32F7
    {
        .device = SPIDEV_1,
        .reg = SPI1,
        .sckPins = {
            { DEFIO_TAG_E(PA5), GPIO_AF5_SPI1 },
            { DEFIO_TAG_E(PB3), GPIO_AF5_SPI1 },
        },
        .misoPins = {
            { DEFIO_TAG_E(PA6), GPIO_AF5_SPI1 },
            { DEFIO_TAG_E(PB4), GPIO_AF5_SPI1 },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PA7), GPIO_AF5_SPI1 },
            { DEFIO_TAG_E(PB5), GPIO_AF5_SPI1 },
        },
        .rcc = RCC_APB2(SPI1),
        .dmaIrqHandler = DMA2_ST3_HANDLER,
    },
    {
        .device = SPIDEV_2,
        .reg = SPI2,
        .sckPins = {
            { DEFIO_TAG_E(PA9), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PB10), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PB13), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PD3), GPIO_AF5_SPI2 },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB14), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PC2), GPIO_AF5_SPI2 },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB15), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PC1), GPIO_AF5_SPI2 },
            { DEFIO_TAG_E(PC3), GPIO_AF5_SPI2 },
        },
        .rcc = RCC_APB1(SPI2),
        .dmaIrqHandler = DMA1_ST4_HANDLER,
    },
    {
        .device = SPIDEV_3,
        .reg = SPI3,
        .sckPins = {
            { DEFIO_TAG_E(PB3), GPIO_AF6_SPI3 },
            { DEFIO_TAG_E(PC10), GPIO_AF6_SPI3 },
        },
        .misoPins = {
            { DEFIO_TAG_E(PB4), GPIO_AF6_SPI3 },
            { DEFIO_TAG_E(PC11), GPIO_AF6_SPI3 },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PB2), GPIO_AF7_SPI3 },
            { DEFIO_TAG_E(PB5), GPIO_AF6_SPI3 },
            { DEFIO_TAG_E(PC12), GPIO_AF6_SPI3 },
            { DEFIO_TAG_E(PD6), GPIO_AF5_SPI3 },
        },
        .rcc = RCC_APB1(SPI3),
        .dmaIrqHandler = DMA1_ST7_HANDLER,
    },
    {
        .device = SPIDEV_4,
        .reg = SPI4,
        .sckPins = {
            { DEFIO_TAG_E(PE2), GPIO_AF5_SPI4 },
            { DEFIO_TAG_E(PE12), GPIO_AF5_SPI4 },
        },
        .misoPins = {
            { DEFIO_TAG_E(PE5), GPIO_AF5_SPI4 },
            { DEFIO_TAG_E(PE13), GPIO_AF5_SPI4 },
        },
        .mosiPins = {
            { DEFIO_TAG_E(PE6), GPIO_AF5_SPI4 },
            { DEFIO_TAG_E(PE14), GPIO_AF5_SPI4 },
        },
        .rcc = RCC_APB2(SPI4),
        .dmaIrqHandler = DMA2_ST1_HANDLER,
    },
#endif
};

void spiPinConfigure(void)
{
    const spiPinConfig_t *pConfig = spiPinConfig();

    for (size_t hwindex = 0 ; hwindex < ARRAYLEN(spiHardware) ; hwindex++) {
        const spiHardware_t *hw = &spiHardware[hwindex];

        if (!hw->reg) {
            continue;
        }

        SPIDevice device = hw->device;
        spiDevice_t *pDev = &spiDevice[device];

        for (int pindex = 0 ; pindex < MAX_SPI_PIN_SEL ; pindex++) {
            if (pConfig->ioTagSck[device] == hw->sckPins[pindex].pin) {
                pDev->sck = hw->sckPins[pindex].pin;
#ifdef STM32F7
                pDev->sckAF = hw->sckPins[pindex].af;
#endif
            }
            if (pConfig->ioTagMiso[device] == hw->misoPins[pindex].pin) {
                pDev->miso = hw->misoPins[pindex].pin;
#ifdef STM32F7
                pDev->misoAF = hw->misoPins[pindex].af;
#endif
            }
            if (pConfig->ioTagMosi[device] == hw->mosiPins[pindex].pin) {
                pDev->mosi = hw->mosiPins[pindex].pin;
#ifdef STM32F7
                pDev->mosiAF = hw->mosiPins[pindex].af;
#endif
            }
        }

        if (pDev->sck && pDev->miso && pDev->mosi) {
            pDev->dev = hw->reg;
#ifndef STM32F7
            pDev->af = hw->af;
#endif
            pDev->rcc = hw->rcc;
            pDev->leadingEdge = false; // XXX Should be part of transfer context
#ifdef USE_HAL_DRIVER
            pDev->dmaIrqHandler = hw->dmaIrqHandler;
#endif
        }
    }
}
#endif

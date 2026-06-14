#include "st7789.h"
#include "spi.h"

#define ST7789_X_OFFSET 0
#define ST7789_Y_OFFSET 0
#define ST7789_DMA_CHUNK_PIXELS ST7789_WIDTH

static uint8_t lcd_dma_line[ST7789_DMA_CHUNK_PIXELS * 2];
static volatile uint8_t lcd_spi_dma_done = 1;

static void ST7789_Select(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static void ST7789_Unselect(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void ST7789_CommandMode(void)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
}

static void ST7789_DataMode(void)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
}

static void ST7789_Reset(void)
{
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);
}

static void ST7789_WriteCommand(uint8_t command)
{
  ST7789_CommandMode();
  ST7789_Select();
  HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
  ST7789_Unselect();
}

static void ST7789_WriteData(const uint8_t *data, uint16_t size)
{
  ST7789_DataMode();
  ST7789_Select();
  HAL_SPI_Transmit(&hspi1, (uint8_t *)data, size, HAL_MAX_DELAY);
  ST7789_Unselect();
}

static void ST7789_WaitDmaDone(void)
{
  while (!lcd_spi_dma_done)
  {
  }
}

static void ST7789_WriteDataDma(const uint8_t *data, uint16_t size)
{
  ST7789_WaitDmaDone();
  lcd_spi_dma_done = 0;
  ST7789_DataMode();
  ST7789_Select();
  if (HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)data, size) != HAL_OK)
  {
    lcd_spi_dma_done = 1;
    ST7789_Unselect();
    Error_Handler();
  }
  ST7789_WaitDmaDone();
  ST7789_Unselect();
}

static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t data[4];

  x0 += ST7789_X_OFFSET;
  x1 += ST7789_X_OFFSET;
  y0 += ST7789_Y_OFFSET;
  y1 += ST7789_Y_OFFSET;

  ST7789_WriteCommand(0x2A);
  data[0] = (uint8_t)(x0 >> 8);
  data[1] = (uint8_t)(x0 & 0xFF);
  data[2] = (uint8_t)(x1 >> 8);
  data[3] = (uint8_t)(x1 & 0xFF);
  ST7789_WriteData(data, 4);

  ST7789_WriteCommand(0x2B);
  data[0] = (uint8_t)(y0 >> 8);
  data[1] = (uint8_t)(y0 & 0xFF);
  data[2] = (uint8_t)(y1 >> 8);
  data[3] = (uint8_t)(y1 & 0xFF);
  ST7789_WriteData(data, 4);

  ST7789_WriteCommand(0x2C);
}

void ST7789_Init(void)
{
  uint8_t data;

  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

  ST7789_Reset();

  ST7789_WriteCommand(0x01);
  HAL_Delay(150);

  ST7789_WriteCommand(0x11);
  HAL_Delay(120);

  ST7789_WriteCommand(0x36);
  data = 0x00; /* Portrait: 240 x 320 */
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0x3A);
  data = 0x55;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xB2);
  {
    const uint8_t porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    ST7789_WriteData(porch, sizeof(porch));
  }

  ST7789_WriteCommand(0xB7);
  data = 0x35;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xBB);
  data = 0x19;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xC0);
  data = 0x2C;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xC2);
  data = 0x01;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xC3);
  data = 0x12;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xC4);
  data = 0x20;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xC6);
  data = 0x0F;
  ST7789_WriteData(&data, 1);

  ST7789_WriteCommand(0xD0);
  {
    const uint8_t power[] = {0xA4, 0xA1};
    ST7789_WriteData(power, sizeof(power));
  }

  ST7789_WriteCommand(0xE0);
  {
    const uint8_t gamma_pos[] = {
      0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54,
      0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23
    };
    ST7789_WriteData(gamma_pos, sizeof(gamma_pos));
  }

  ST7789_WriteCommand(0xE1);
  {
    const uint8_t gamma_neg[] = {
      0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44,
      0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23
    };
    ST7789_WriteData(gamma_neg, sizeof(gamma_neg));
  }

  ST7789_WriteCommand(0x20); /* Display inversion OFF */
  ST7789_WriteCommand(0x29);
  HAL_Delay(20);
}

void ST7789_FillColor(uint16_t color)
{
  uint32_t pixels = (uint32_t)ST7789_WIDTH * ST7789_HEIGHT;
  uint16_t chunk_pixels = ST7789_DMA_CHUNK_PIXELS;
  uint16_t i;

  for (i = 0; i < chunk_pixels; i++)
  {
    lcd_dma_line[i * 2] = (uint8_t)(color >> 8);
    lcd_dma_line[i * 2 + 1] = (uint8_t)(color & 0xFF);
  }

  ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

  while (pixels > 0)
  {
    uint16_t send_pixels = (pixels > chunk_pixels) ? chunk_pixels : (uint16_t)pixels;
    ST7789_WriteDataDma(lcd_dma_line, send_pixels * 2);
    pixels -= send_pixels;
  }
}

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  uint8_t data[2];

  if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
  {
    return;
  }

  data[0] = (uint8_t)(color >> 8);
  data[1] = (uint8_t)(color & 0xFF);
  ST7789_SetAddressWindow(x, y, x, y);
  ST7789_WriteData(data, 2);
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image)
{
  uint32_t bytes = (uint32_t)width * height * 2;

  if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
  {
    return;
  }
  if ((x + width) > ST7789_WIDTH)
  {
    width = ST7789_WIDTH - x;
  }
  if ((y + height) > ST7789_HEIGHT)
  {
    height = ST7789_HEIGHT - y;
  }

  bytes = (uint32_t)width * height * 2;
  ST7789_SetAddressWindow(x, y, x + width - 1, y + height - 1);

  while (bytes > 0)
  {
    uint16_t chunk = (bytes > 60000U) ? 60000U : (uint16_t)bytes;
    ST7789_WriteDataDma(image, chunk);
    image += chunk;
    bytes -= chunk;
  }
}


void ST7789_DrawImage4BPP(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          const uint8_t *packed_pixels, const uint16_t *palette)
{
  uint16_t row;
  uint16_t col;

  if ((packed_pixels == NULL) || (palette == NULL) ||
      (width == 0U) || (height == 0U) ||
      (x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
  {
    return;
  }

  if (((uint32_t)x + width > ST7789_WIDTH) ||
      ((uint32_t)y + height > ST7789_HEIGHT))
  {
    return;
  }

  ST7789_SetAddressWindow(x, y, x + width - 1U, y + height - 1U);

  for (row = 0U; row < height; row++)
  {
    const uint32_t row_pixel_offset = (uint32_t)row * width;

    for (col = 0U; col < width; col++)
    {
      const uint32_t pixel_index = row_pixel_offset + col;
      const uint8_t packed_byte = packed_pixels[pixel_index >> 1U];
      const uint8_t palette_index =
          ((pixel_index & 1U) == 0U) ? (packed_byte >> 4U) : (packed_byte & 0x0FU);
      const uint16_t color = palette[palette_index];

      lcd_dma_line[col * 2U] = (uint8_t)(color >> 8U);
      lcd_dma_line[col * 2U + 1U] = (uint8_t)(color & 0xFFU);
    }

    ST7789_WriteDataDma(lcd_dma_line, width * 2U);
  }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1)
  {
    lcd_spi_dma_done = 1;
  }
}

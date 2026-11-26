# MSB Bootloader (Qwen) — загрузчик STM32 платы MSB

Загрузчик/монитор для платы **МСБ (плата коммутации режимов)**, MCU
**STM32F411RE**. Принимает и проверяет приложение, прошивает внешнюю FLASH
битстрима FPGA, предоставляет отладочный терминал (eAssist/DSPA) по UART.

## Целевое оборудование

| Параметр | Значение |
| :--- | :--- |
| MCU | STM32F411RE (512 К Flash, 128 К RAM), CubeIDE/GCC |
| Приложение | секторы 5–6: `0x08020000`–`0x0805FFFF` (256 К) |
| FPGA платы | Xilinx Spartan‑6, конфигурационная FLASH **W25Q128** на SPI1 |
| Терминал | USART1 (DMA RX/TX), протокол eAssist/DSPA |
| RTOS | FreeRTOS (CMSIS-RTOS v2) |

## Возможности

* **Загрузка приложения**: приём по терминалу, запись во внутреннюю Flash
  (сектора 5–6), проверка **CRC + длины** (`BSP_CHACK_APPL_CRC`), защищённый
  переход к приложению (`jump_main_application`: проверка SP, остановка периферии/DMA/NVIC,
  сброс VTOR, `bsp_deinit`) — `BSP_PERMISSION_JUMP_APPL_`;
  режим быстрого старта `BSP_PERMISSION_FAST_JUMP`;
* **Прошивка ПЛИС**: запись битстрима в **W25Q128** (SPI1) через терминальный
  объект `DD4-W25Q128JV/M` (`_ID_FPGA_FLASH`), мьютекс на шину FLASH
  (`BSP_W25Q128_USE_MUTEX`); выход `MR_PROG` (`pin_mr_prog`) активируется при
  инициализации BSP для удержания ПЛИС; дескриптор `pin_fpga_done`
  (вывод DONE ПЛИС) описан в pin_management;
* **Терминал eAssist/DSPA**: дерево сигналов (версия, статусы, управление
  загрузкой/прошивкой), сигналы `terminal_signals`, обёртка `wrapper`/`std_com`
  поверх UART+DMA;
* **pin_management**: централизованные дескрипторы выводов (FPGA_DONE, MR_PROG,
  CS внешней FLASH и др.) с резервными определениями (`gpio_fallbacks.h`);
* сведения о версии ПО/оборудования платы — `version_control`;
* вариант компиляции под плату **PSPM** — `BSP_MSB0_PSPM1_BOARD`.

## Структура репозитория

```
B_QWEN/
|-- Core/                          генерация CubeIDE: main, FreeRTOS, периферия
|-- board_support_package/
|   |-- board_support_package.c/.h конфигурация BSP, init_hardware, bsp_deinit,
|   |                              jump_main_application
|   |-- memory/application/        карта и драйвер внутренней Flash
|   |                              (сектора 5-6, check_application, CRC)
|   |-- memory/w25q128/            драйвер внешней FLASH (битстрим ПЛИС, SPI1)
|   |-- terminal/                  терминал eAssist: terminal.c, dspa,
|   |   |                          terminal_signals, wrapper
|   |   `-- version_control/       сведения о версии ПО/оборудования
|   |-- std_com/                   транспорт UART + DMA
|   `-- pin_management/            дескрипторы выводов, gpio_fallbacks.h
|-- Middlewares/                   FreeRTOS
|-- Drivers/                       HAL/CMSIS
`-- msb_bootloader_qwen.ioc        проект CubeIDE (STM32F411RE)
```

## Сборка

1. Импорт в **STM32CubeIDE** (существующий проект), целевая конфигурация `Debug`.
2. Сборка: `make -C Debug -j8 all` (или Build в IDE). Прошивка через ST‑LINK.
3. После регенерации `.ioc` проверять `STM32F411RETX_FLASH.ld`   (секция не должна перекрываться с областью приложения, начинающейся с адреса 0x08020000).

## Порядок загрузки изделия (типовой)

1. Включить питание: загрузчик инициализирует BSP и терминал, ожидает команд.
2. Через eAssist записать файл приложения: проверка CRC и размера, затем
   переход к приложению (непрерывный запуск — режим `BSP_PERMISSION_FAST_JUMP`).
3. Для ПЛИС: записать битстрим в W25Q128 терминальным объектом
   `DD4-W25Q128JV/M`; конфигурацию ПЛИС выполняет с этой FLASH.

## Связанные проекты

* Прошивка FPGA платы МСБ: [fpga_msb_combat](https://github.com/auramenka-it90/fpga_msb_combat)
* Родственный загрузчик БК (STM32F415): [stm32_bc_bootloader](https://github.com/auramenka-it90/stm32_bc_bootloader)

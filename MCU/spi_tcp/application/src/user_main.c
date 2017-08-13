///*8     888  .d8888b.  8888888888 8888888b.           888b     d888        d8888 8888888 888b    888
//888     888 d88P  Y88b 888        888   Y88b          8888b   d8888       d88888   888   8888b   888
//888     888 Y88b.      888        888    888          88888b.d88888      d88P888   888   88888b  888
//888     888  "Y888b.   8888888    888   d88P          888Y88888P888     d88P 888   888   888Y88b 888
//888     888     "Y88b. 888        8888888P"           888 Y888P 888    d88P  888   888   888 Y88b888
//888     888       "888 888        888 T88b            888  Y8P  888   d88P   888   888   888  Y88888
//Y88b. .d88P Y88b  d88P 888        888  T88b           888   "   888  d8888888888   888   888   Y8888
// "Y88888P"   "Y8888P"  8888888888 888   T88b 88888888 888       888 d88P     888 8888888 888    Y8*/
//
//
//
//
///*88888b.    8888888888    .d8888b.    888
//888  "Y88b   888          d88P  Y88b   888
//888    888   888          888    888   888
//888    888   8888888      888          888
//888    888   888          888          888
//888    888   888          888    888   888
//888  .d88P   888          Y88b  d88P   888
//8888888P"    8888888888    "Y8888P"    888888*/
//
//
////Function Declaration
//static void init_timer (void);
//static void init_uart (void);
//static void timer_isr (void * context);
//static void uart_isr(void* context);
//
//// Variable Declaration
//volatile int edge_capture;// hold the value of the button pio edge capture register.
//
//volatile  uint8_t led_out;
//volatile  uint8_t toggle_timer;
//volatile  uint8_t toggle_uart;
//volatile  int     edge_capture;
//volatile RingBuffer *uart_in_rb;
//
//
///*88888   888b    888   8888888   88888888888
//  888     8888b   888     888         888
//  888     88888b  888     888         888
//  888     888Y88b 888     888         888
//  888     888 Y88b888     888         888
//  888     888  Y88888     888         888
//  888     888   Y8888     888         888
//8888888   888    Y888   8888888       8*/
//
//
//static void init_timer(void)
//{
//    void* led_out_ptr = (void*) &led_out;
//
//    led_out = 0x01;
//    toggle_timer = 0x00;
//
//    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, led_out);
//
//    //Timer Initialization
//    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x0003);
//    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0);
//
//    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE, 0x7840);
//    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE, 0x017D);
//
//    //Register ISR for timer event
//    alt_ic_isr_register(TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID,
//                        TIMER_0_IRQ, timer_isr, led_out_ptr, 0);
//
//    //Start timer and begin the work
//    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x0007);
//}
//
//
//// Initialize the pio.
//static void init_uart (void)
//{
//    //Recast the edge_capture pointer to match the alt_irq_register() function
//    //prototype.
//    void* edge_capture_contex_ptr = (void*) &edge_capture;
//    // Enable bit 8 interrupts.
//    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(UART_PIO_BASE, 0x0100);
//    // Reset the edge capture register.
//    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(UART_PIO_BASE, 0x0);
//    // Register the interrupt handler.
//
//    alt_ic_isr_register(UART_PIO_IRQ_INTERRUPT_CONTROLLER_ID,
//                        UART_PIO_IRQ, uart_isr, edge_capture_contex_ptr, 0);
//
//    toggle_uart = 0;
//}
//
///*8    888          d8888   888b    888   8888888b.    888        8888888888   8888888b.     .d8888b.
//888    888         d88888   8888b   888   888  "Y88b   888        888          888   Y88b   d88P  Y88b
//888    888        d88P888   88888b  888   888    888   888        888          888    888   Y88b.
//8888888888       d88P 888   888Y88b 888   888    888   888        8888888      888   d88P    "Y888b.
//888    888      d88P  888   888 Y88b888   888    888   888        888          8888888P"        "Y88b.
//888    888     d88P   888   888  Y88888   888    888   888        888          888 T88b           "888
//888    888    d8888888888   888   Y8888   888  .d88P   888        888          888  T88b    Y88b  d88P
//888    888   d88P     888   888    Y888   8888888P"    88888888   8888888888   888   T88b    "Y8888*/
//
//
//static void timer_isr (void * context)
//{
//    volatile uint8_t* tmp_led_ptr;
//
//    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0);
//
//    tmp_led_ptr = (volatile uint8_t*) context;
//
//    //IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, *tmp_led_ptr & toggle_timer | 0x02 & toggle_uart);
//
//    toggle_timer = ~(toggle_timer);
//}
//
//uint32_t cnt_interrupt;
//
//static void uart_isr(void* context)
//{
//    int      rc_uart;
//    uint32_t rx_data;
//    char     rx_data_char;
//
//    //Cast context to edge_capture's type. It is important that this be
//    //declared volatile to avoid unwanted compiler optimization.
//    volatile int* edge_capture_ptr = (volatile int*) context;
//    // Store the value in the Button's edge capture register in *context.
//    *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(UART_PIO_BASE);
//
//    // send data back to the pio system
//    // cnt_interrupt = cnt_interrupt < 0xFF ? cnt_interrupt+1 : 0;
//    // IOWR_ALTERA_AVALON_PIO_DATA(UART_PIO_BASE, cnt_interrupt);
//
//    // Reset the Button's edge capture register.
//    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(UART_PIO_BASE, 0);
//
//    //  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, 0x02 & toggle_uart);
//
//    rx_data = IORD_ALTERA_AVALON_PIO_DATA(UART_PIO_BASE);
//
//    rx_data_char = rx_data & 0xFF;
//
//    rc_uart = RingBuffer_write(uart_in_rb, &rx_data_char, 1);
//    DBG_check_debug(rc_uart == 1, "incorrect ammount of data written to the buffer");
//
//    toggle_uart = ~(toggle_uart);
//
//    return;
//error:
//   DBG_debug("error uart_ist");
//}
//
///*8b     d888           d8888    8888888    888b    888
//8888b   d8888          d88888      888      8888b   888
//88888b.d88888         d88P888      888      88888b  888
//888Y88888P888        d88P 888      888      888Y88b 888
//888 Y888P 888       d88P  888      888      888 Y88b888
//888  Y8P  888      d88P   888      888      888  Y88888
//888   "   888     d8888888888      888      888   Y8888
//888       888    d88P     888    8888888    888    Y8*/
//
//
//int user_main (void)
//{
//    int rc_main;
//    uint32_t avail_data_s;
//    uart_in_rb = RingBuffer_create(20 * 3 * sizeof(char));
//
//    int return_code,ret;
//    char spi_command_string_rx[10] = "$HELLOABC*";
//
//    init_timer();
//    init_uart ();
//
//    printf(" size of int = %d\n ", sizeof(int));
//
//    printf("\n\n START ! \n\n");
//
//    while (1)
//    {
//        //IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, (0x01 & toggle_timer) | (0x02 & toggle_uart) );
//        avail_data_s = RingBuffer_available_data(uart_in_rb);
//
//        if (avail_data_s >= 20 )
//        {
//            char print_buff[avail_data_s];
//
//            rc_main = RingBuffer_read(uart_in_rb, print_buff, avail_data_s);
//            DBG_check_debug(rc_main == avail_data_s, "incorrect ammount of data read from the buffer");
//
//            printf("\n\nsize in buffer : %d", avail_data_s);
//            printf("\ndata in buffer : ");
//
//            printf("%2x", print_buff[0] & 0xff);
//
//            for (int i = 1; i < avail_data_s; i++)
//            {
//                printf(", %2x", print_buff[i] & 0xff);
//            }
//
//            return_code = alt_avalon_spi_command (SPI_0_BASE, 0,
//                                                  avail_data_s, print_buff,
//                                                  0, spi_command_string_rx,
//                                                  0);
//
//            DBG_check(return_code >= 0, "ERROR SPI TX RET = %x \n" , return_code);
//        }
//    }
//    return 0;
//error:
//     return -1;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
////

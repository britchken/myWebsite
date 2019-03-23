void commandEntered(void){
	if (strcmp(Command,"#help") == 0) {
		uart_clear_screen();
		display_help();
	}
	if (strcmp(Command,"#clear") == 0) {
		uart_clear_screen();
		uart_putc(0x1B);
		uart_puts("[s");
	} else if (strcmp(Command,"#search") == 0) {
		if (search_enable == 0){
			search_enable = 1;
			uart_puts("SEARCHING IS ENABLED!!\n");
		}else{
			search_enable = 0;
			uart_puts("SEARCHING IS DISABLED!!\n");
		}
	}
	else if (strcmp(Command,"#connnections") == 0) {


	}
	else if (strcmp(Command,"#matches") == 0) {


	} else {
		if (strcmp(Command,"#view") == 0) {


		}
		else if (strcmp(Command,"#message") == 0) {


		}
		else if (strcmp(Command,"#matchwith") == 0) {


		}
		else if (strcmp(Command,"#reject") == 0) {


		}else if (strcmp(Command,"#set") == 0) {


		} else {
			uart_puts("Invalid Command\n");
			new_line();
		}
	}

	P1OUT ^= RED_LED;
	Command[0] = '\0';
	Entered = 0;
}


















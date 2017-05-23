#include "includes.h"

#define str(x) #x

bool proc(void);
bool reboot(void);
void send_command(char input[1000]);
bool check_command(char *cmd, char *input);

extern struct task* first_task;

bool check_command(char *cmd, char *input) {
	if(strlen(input)!=strlen(cmd)) return false;
	while(*cmd) {
		if(*cmd!=*input) return false;
		*cmd++;
		*input++;
	}
	return true;
	//else return false;
}

void doNothing(void) {}

bool vesa_parameter(char *parameter) {
	char *ptr;
	uint16_t res;
	res=strtol(parameter,&ptr,16);
		
	if(check_command("text",ptr)) change_to_text();
	else set_vesa_mode(res);

	return true;
}

bool pci_parameter(char *parameter) {
	char *ptr;
	uint16_t res;
	pci_device pci;
	res=strtol(parameter,&ptr,16);
	if(check_command("text",ptr)) change_to_text();
	else set_vesa_mode(res);
	return true;
}

bool kill(char *parameter) {
	char *ptr;
	int res;
	res=strtol(parameter,&ptr,10);
	if(remove_task_by_pid(res)) kprintf("\nTask %d killed\n",res);
	return true;
}

void send_command(char input[1000]) {
	show_prefix=false;
	//kprintf("\n");
	//vesa_parameter(input);
	if(check_command("set ",substr(input,0,4))) vesa_parameter(substr(input,4,strlen(input)-2));
	else if(check_command("pci_info ",substr(input,0,9))) vesa_parameter(substr(input,9,strlen(input)-7));
	else if(check_command("pci",input)) get_pci_devices();
	else if(check_command("proc",input)) proc();
	else if(check_command("vesa",input)) get_vesa_modes();
	else if(check_command("reset",input)) reboot();
	else if(check_command("reboot",input)) reboot();
	else if(check_command("task_a",input)) init_task(task_a,NORMAL);
	else if(check_command("kill ",substr(input,0,5))) kill(substr(input,5,strlen(input)-2));
	else if(check_command("time",input)) { time(); kprintf("\n"); }
	/*else {
		//while(*input!=NULL && *input!='\0') {
			draw_char(input,-1,-1,0xffffff,0x000000);
			//*input++;
		//}
	}*/
	show_prefix=true;
	//kprintf("\n");
}

bool proc(void) {
	struct task* temp=first_task;
	kprintf("Prozesse:\n");
	while(temp!=NULL) {
		kprintf("PID: %d     EIP: %x     Type: ",temp->pid,temp->cpu_state->eip);
		if(temp->type == NORMAL) {
			kprintf("Normal\n");
		} else if(temp->type == IDLE) {
			kprintf("Idle\n");
		} else if(temp->type == V86) {
			kprintf("V86\n");
		}

		temp=temp->next;
	}
	return true;
}

bool reboot(void)
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    asm("cli; hlt");
	return true;
}

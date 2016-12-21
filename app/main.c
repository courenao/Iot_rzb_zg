/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "board.h"
#include "delay.h"

const uint8_t str_LOGO_RZB[] = { "\r\n"
		"	$$$$$$$\\ $$$$$$$$\$$$$$$$\\     \r\n"
		"	$$  __$$\\\\____$$  $$  __$$\\    \r\n"
		"	$$ |  $$ |   $$  /$$ |  $$ |   \r\n"
		"	$$$$$$$  |  $$  / $$$$$$$\\ |   \r\n"
		"	$$  __$$<  $$  /  $$  __$$\\    \r\n"
		"	$$ |  $$ |$$  /   $$ |  $$ |   \r\n"
		"	$$ |  $$ $$$$$$$$\\$$$$$$$  |   \r\n"
		"	\\__|  \\__\\________\\_______/    \r\n"
		"\r\n"

};

const uint8_t str_LOGO_ZG[] = { "\r\n"
		"	$$$$$$$$\\ $$$$$$\\    \r\n"
		"	\\____$$  $$  __$$\\  \r\n"
		"	    $$  /$$ /  \\__| \r\n"
		"	   $$  / $$ |$$$$\\  \r\n"
		"	  $$  /  $$ |\\_$$ | \r\n"
		"	 $$  /   $$ |  $$ | \r\n"
		"	$$$$$$$$\\ $$$$$$  | \r\n"
		" 	\\________|\\______/  \r\n"
		"\r\n" };

//��˾LOGO
const uint8_t str_LOGO_USE[] = { "\r\n"
		"	$$\\   $$\\ $$$$$$\\ $$$$$$$$\\    \r\n"
		"	$$ |  $$ $$  __$$\\$$  _____|   \r\n"
		"	$$ |  $$ $$ /  \\__$$ |         \r\n"
		"	$$ |  $$ \\$$$$$$\ $$$$$\\       \r\n"
		"	$$ |  $$ |\\____$$\$$  __|      \r\n"
		"	$$ |  $$ $$\\   $$ $$ |         \r\n"
		"	\\$$$$$$  \\$$$$$$  $$$$$$$$\\    \r\n"
		"	 \\______/ \\______/\\________|   \r\n"

};
//δ֪
const uint8_t str_LOGO_UNKNOWN[] = { "UNKNOWN DEVICE" };

void System_Init() {
#if BOOT_LOADER
	//BOOT������ַ����Ϊ0��Ĭ��ֵΪ0
#else
	//��ʼ���ж�����0x20000��ַ����ͳ�����д�ĵ�ַһ��
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
#endif
	SystemCoreClockUpdate();
	//�������ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SysTick_Init();

}

//��ʼ������Ӳ����Դ
void Board_Init() {
	printf_Init();
	Board_LEDInit(ALARM);
	Board_LEDInit(RUN);
	Board_PWM_Init();
	Board_Version_Init();
	//����Ӳ�������ȡ����
	//Board_Flash_Protect();

#if BOOT_LOADER

	printf("\n\r ############ ����ʱ�� ("__DATE__ " - " __TIME__ ")  ############ ");
	uint8_t *str_LOGO;
	if(Board_Is_RZB()) {
		str_LOGO=str_LOGO_RZB;
	}
	else {
		str_LOGO=str_LOGO_ZG;
	}
	printf("%s", str_LOGO);
#else//ֻ��Ӧ�ó����ִ��
	//����MAC��ַ
	Board_Mac_AutoGen();
	//��ʼ���豸��Ϣ
	Board_DeviceInfo_Init();
	//��ӡ�豸��Ϣ
	Board_DeviceInfo_Print();

#if RZG	//���ز�����̫��
	ETH_BSP_Config();
	//��ʼ��UDP
	Dprintf("��ʼ����̫��ͨѶ����.\n");
	LwIP_Init();//����IP��ַ����ʼ��Э��ջ
	UDP_Server_Init();//��ʼ��UDP������
#endif



#endif //ENDOF BOOT_LOADER
}
#if BOOT_LOADER
void Board_Boot_Service()
{
	if (IAP_Program_Check() == 0) {
		//����������
		for (int i = 0; i < 10; i++)
		Board_PWM_On();
		printf("Ӧ�ó��򲻴��ڣ���Ҫ������д����\n");
		//��������SHELL����Ҫ����Ӧ�ó���
		Board_Shell_Entry(0);
	} else {
		//����Shell
		Board_Shell_Entry(1);
	}

	//���¼��Ӧ�ó����Ƿ���ڣ��������������ѭ������������
	if (IAP_Program_Check() == 0) {
		printf("����Ӧ������ ʧ�ܣ�\n");
		FOREVER()
		; //Ӧ�ò����ڣ���ѭ��
	} else {
		printf("Ӧ�ó���ʼ��\n");

		IAP_Program_Run();
	}
}
#endif	//ENDOF BOOT_LOADER
int main() {
	System_Init(); //ϵͳ��ʼ��
	Board_Init(); //���ӳ�ʼ��
#if BOOT_LOADER
	Board_Boot_Service();
#else
	stack_main();
#endif
}

/****************************END OF FILE****/

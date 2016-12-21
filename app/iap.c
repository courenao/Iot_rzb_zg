/*
 * IAP���߱�̣�������������
 * */

#include "board.h"
#if BOOT_LOADER
#include "iap.h"
#include "xprintf.h"


#define FILE_LEN_VER		4		//�汾���ֽ���

#if BOOT_MD5_CHECK
#include "md5.h"
#endif

#define IAP_PROGRAM_LEN		1024*32		//Ӧ�ó�����ܴ�С
#define IAP_BUF_MAX_LEN		1024*100	//IAP�������ռ�,100K
uint8_t iap_buf[IAP_BUF_MAX_LEN]; //�������ݿռ䣬���ܳ���RAM��С��STM32F207VE RAM 128K
uint8_t iap_file_end;//��ȡ�������� ���ļ����ļ���ȡ����

#define IAP_FLASH_SECTOR 			FLASH_Sector_5		//IAP FLASH����
#define IAP_FLASH_BASE				FLASH_BASE_SECTOR_5	//IAP FLASH�׵�ַ
typedef void (*pFunction)(void);

//����ļ��Ƿ��ǵ�ǰ���ӵģ��Ƿ�ƥ�䵱ǰ����
uint8_t IAP_Board_Match(uint32_t id) {
#define FILE_ID_RZB 	(0X11<<24)
#define FILE_ID_ZG		(0x22<<24)
#define FILE_ID_MASK	(0XFF<<24)

	uint8_t ret = 0;
	if (Board_Is_RZB()) { //��ǰ������RZB
		if (id & FILE_ID_RZB) {
			ret = 1;
		}
	} else { //��ǰ������ZG
		if (id & FILE_ID_ZG) {
			ret = 1;
		}
	}
	return ret;
}

#if BOOT_MD5_CHECK

#define	FILE_LEN_MD5		16		//MD5ռ���ֽ���
//�Խ��յ������ݽ���MD5У�飬����ֵ1--У��ɹ���0--У��ʧ��
//len--IAP��������Ч���ݵĳ��ȣ�iap_buf[len]��ʼΪMD5��ֵ
uint8_t IAP_MD5_Match(uint32_t len)
{
	uint8_t hash[FILE_LEN_MD5];
	md5(hash, (const uint8_t*)iap_buf, len);
	for(uint8_t i=0;i<sizeof(hash);i++)
	{
		if(hash[i]!=iap_buf[len+i]) {
			return 0; //У��ʧ��
		}
	}
	Dprintf("MD5 У��ɹ���\n");
	return 1;//У��ɹ�
}
#endif

//IAP���������������ճ����ļ�����д��FLASH���Զ�����,len--�������ļ���С����Ҫ�ֶ����룬����У���ļ��Ƿ�����
//����ֵ��1--��ʾIAP��ȡ����д�ɹ���0--��ʾIAP��ȡ�ļ�����дʧ�ܣ���Ҫ���³���
uint8_t IAP_Service(uint32_t len) {
	uint32_t file_len = 0;
	uint32_t *ptr; //д��Flashָ��
	pFunction pf;//
	//uint8_t ret = 0;//����ֵ

	iap_file_end = 0;//��ʼ����־λΪ0

	uint8_t buf1k[1024];//1K������
	file_len = DbgReceive1KXModem(buf1k, iap_buf, sizeof(iap_buf));
	delayMS(200);//�ӳ�200ms
	if (file_len > 0 && file_len < (IAP_BUF_MAX_LEN)) {


		//�����յ����ļ��Ƿ���һ����Ч��Ӧ�ó���
		if (IAP_Program_Check_Buf()) {
			//Ѱ��0x1A1A,xmodem�������������˵�
			uint16_t *xPtr = iap_buf + file_len - 2;
#define XMODEM_END_U16	0x1a1a
			while (*xPtr == XMODEM_END_U16) {
				xPtr--;
				file_len -= 2; //�ļ����ȼ�ȥ���������ȣ�2�ֽڣ�
			}

#if BOOT_MD5_CHECK

			if(IAP_MD5_Match(file_len-FILE_LEN_MD5-FILE_LEN_VER)==0)
			{//У��ʧ��
				printf("\n��Ч�ļ���\n");
				return 0;
			}
#endif

			uint32_t *verPtr;
			verPtr = iap_buf + file_len - FILE_LEN_VER; //���4�ֽ�Ϊ�汾��
			if (IAP_Board_Match(*verPtr) && Board_Version_Check(*verPtr)) { //�ļ��Ƿ�ƥ�䵱ǰӲ�����Ұ汾�źϷ�

				printf("\n��ȡ���ļ����ȣ�%d,�汾��: ", file_len);
				Board_Version_Print((*verPtr) & BOARD_VERSION_MASK);
				Device_Info_t *info = Board_DeviceInfo_Get();
				//����������汾���Ƿ�Ϸ������Ϸ���ʾû���ù������԰汾�Ƚ�
				if (Board_Version_Check(info->ver_sw)) {

					//��ȡ���ļ��汾�ű���>=���Ϲ��е�����汾�ţ����򲻻�����
					if ((*verPtr) >= (info->ver_sw)) {
						//�ļ���Ч
						iap_file_end = 1;
					}

				} else { //��ǰ���ϰ汾��ȱʧ�����԰汾�űȽ�
					//�ļ���Ч
					iap_file_end = 1;
				}

			} else { //IAP_Program_Check_Buf ����ʧ��
				printf("\n��Ч�ļ���\n");
				return 0;
			}

		} else {
			printf("\n��Ч�ļ���\n");
			return 0;
		}

	} //ENDOF if (file_len > 0 && file_len < (IAP_BUF_MAX_LEN))

	if (iap_file_end) {
		//��ʼд��Flash
		FLASH_Unlock();
		FLASH_ClearFlag(
				FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
				| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
				| FLASH_FLAG_PGSERR);
		//�����������ݿ�
		while (FLASH_EraseSector(IAP_FLASH_SECTOR, VoltageRange_3)
				!= FLASH_COMPLETE)
		;

		//��ȡ��д��������׵�ַ
		ptr = (uint32_t) iap_buf;

		for (uint32_t i = 0; i < (file_len / sizeof(uint32_t)); i++) {
			FLASH_ProgramWord(IAP_FLASH_BASE + i * 4, ptr[i]);
		}
		//д�����
		FLASH_Lock();
		return 1;
	}
	return 0;
}
//��IAP_BUF�м���Ƿ����Ӧ�ó���
uint8_t IAP_Program_Check_Buf() {
	uint32_t *ptr = (uint32_t*) iap_buf;
	if (((*(__IO uint32_t*) ptr) & 0x2FFE0000) == 0x20020000) {
		return 1;
	}
	return 0;
}

//���IAP FLASH����ĳ����Ƿ����
uint8_t IAP_Program_Check() {
	//��Ϊ�û�����ʼλ��(0x08008000��)��ǰ4���ֽڴ�ŵ��Ƕ�ջ�ĵ�ַ��
	//��ջ��ַ�ض���ָ��RAM�ռ�ģ���STM32��RAM�ռ���ʼ��ַΪ0x20020000��
	if (((*(__IO uint32_t*) IAP_FLASH_BASE) & 0x2FFE0000) == 0x20020000) {
		return 1;
	}
	return 0;
}

void IAP_Program_Erase() {
	FLASH_Unlock();
	FLASH_ClearFlag(
			FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
			| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
			| FLASH_FLAG_PGSERR);
	//�����������ݿ�
	while (FLASH_EraseSector(IAP_FLASH_SECTOR, VoltageRange_3) != FLASH_COMPLETE)
	;

	FLASH_Lock();
}
//�����û�����
void IAP_Program_Run() {
	pFunction proPtr; //��λ����ָ��

	//����λ�����ж�����ָ��Ӧ�ó����ַ
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, IAP_FLASH_BASE - FLASH_BASE);
	//IAP_FLASH_BASE+4 ָ����Ǹ�λ�����ַ
	proPtr = *(__IO uint32_t*) (IAP_FLASH_BASE + 4);

	//��ջ�ĳ�ʼ���������趨ջ������ַ����ջ����ַ����Ϊ�û�����ָ���ջ����ַ��
	__set_MSP(*(__IO uint32_t*) IAP_FLASH_BASE);
	//ִ�и�λ����ϵͳ����λ��ִ���û�����
	proPtr();

}

#endif //EDNOF BOOT_LOADER

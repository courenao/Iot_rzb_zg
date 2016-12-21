#ifndef UDP_SERVER_H
#define UDP_SERVER_H
#include "board.h"

#if USE_INTERNET //ֻ��ʹ���������Ҫ
#include "netconf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "xprintf.h"

//udpͨѶ��PCB
static struct udp_pcb *udpPcbPrivate;
static struct udp_pcb *udprecv;
extern uint8_t zg_get_settings;

static struct ip_addr ipAddrTarget; //UDP Ŀ���ַ
static uint16_t sendPort; //UDP ���Ͷ˿ں�
static uint16_t recvPort; //UDP ���ն˿ں�

struct ip_addr * UDP_Target_Get() {
	return &ipAddrTarget;
}
//����UDPĿ��IP��ַ
void UDP_Target_Init() {
	Device_Info_t *devInfo = Board_DeviceInfo_Get();
	IP4_ADDR(&ipAddrTarget, devInfo->udp.ip_target[0],
			devInfo->udp.ip_target[1], devInfo->udp.ip_target[2],
			devInfo->udp.ip_target[3]);

	sendPort = devInfo->udp.send_port;

	recvPort = devInfo->udp.recv_port;

}

//��Ŀ�귢�����ݣ��Լ���װ��udp_send������Ӧ�÷���UDP��������������
uint8_t UDP_Target_Send(struct udp_pcb *pcb, struct pbuf *p) {
	return udp_sendto(pcb, p, &ipAddrTarget, sendPort);
}

//size ��С���ֽ�����
void swap_buf(uint16_t *out, uint16_t *in, uint16_t size) {
	for (uint16_t i = 0; i < (size / sizeof(uint16_t)); i++) {
		out[i] =ntohs( in[i]);
	}
}

///***********************************************************************
//�������ƣ�udp_data_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
//��    �ܣ�udp���ݽ��պͷ���
//���������
//���������
//��дʱ�䣺2013.4.25
//�� д �ˣ�
//ע    �⣺����һ���ص���������һ��udp�ε����������ʱ�ᱻ����
//***********************************************************************/
udp_data_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		struct ip_addr *addr, uint16_t port) {
	struct ip_addr destAddr = *addr; /* ��ȡԶ������ IP��ַ */
	if (p != NULL) {

		//�洢����
		if (p->len == sizeof(Network_setting_t)) {
			Network_setting_t settings;
			//����������õĶ���uint16_t����ģ����ֱ��swap_buf
			swap_buf(&settings, p->payload, p->len);

			//�����������
			if (Board_Network_Set(&settings)) { //�������óɹ�������־λ��Ϊ1���Ա����͸��ڵ�
				zg_get_settings = 1;
			}
		}

		if (p->len == sizeof(UDP_Net_Config_t)) { //���յ�ZG UDP������������
			Device_Info_t info_org;

			//printf("���յ�UDP������Ϣ��ZG��������\n");
			Board_DeviceInfo_Read(&info_org); //��ȡ�������в���

			//�����ֽ����׼Ϊ�����ǰ��IP[0~3]���õľ��Ǵ����ǰ�Ķ�ȡ��ʽ
			//udp������ֻ�����4���ֽ�Ϊuint16��ǰ���ȫ����uint8_t����Ҫ��С��ת��
			memcpy(&(info_org.udp),p->payload,p->len-4);
			uint16_t *pdata=(uint16_t *)(p->payload+(p->len-4));
			info_org.udp.send_port=ntohs(*pdata);
			pdata++;	//����
			info_org.udp.recv_port=ntohs(*pdata);

			Board_DeviceInfo_Write(&info_org);

			Board_Soft_Reset();
		}

		/******������ԭ������*******************/
		udp_sendto(pcb, p, &destAddr, port); /* ���յ��������ٷ��ͳ�ȥ */
		pbuf_free(p); /* �ͷŸ�UDP�� */
	}

}

/***********************************************************************
 �������ƣ�UDP_server_init(void)
 ��    �ܣ����UDP�������ĳ�ʼ������Ҫ��ʹ��UDPͨѶ��������״̬
 ���������
 ���������
 ��дʱ�䣺2013.4.25
 �� д �ˣ�
 ע    �⣺
 ***********************************************************************/
void UDP_Server_Init() {
	//��ʼ��Ŀ�����
	UDP_Target_Init();
	udpPcbPrivate = udp_new(); //����udp���ƿ�
	udp_bind(udpPcbPrivate, IP_ADDR_ANY, recvPort); /* �󶨽������ݶ˿ںţ���Ϊudp�������� */
	udp_recv(udpPcbPrivate, udp_data_recv, NULL); /* ����UDP�ε�ʱ�Ļص����� */
	//����������Ҫ�����ƶ��˿�
}
void UDP_Client_Init() {
	udpPcbPrivate = udp_new(); //����udp���ƿ�
	udp_bind(udpPcbPrivate, IP_ADDR_ANY, 1010); /* �󶨱���IP��ַ�Ͷ˿ںţ���Ϊup�������� */
	struct ip_addr ipaddr;
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	IP4_ADDR(&ipaddr, ptr->udp.ip_target[0], ptr->udp.ip_target[1],
			ptr->udp.ip_target[2], ptr->udp.ip_target[3]);
	udp_connect(udpPcbPrivate, &ipaddr, ptr->udp.send_port); //���ӷ�����
	udp_recv(udpPcbPrivate, udp_data_recv, NULL);

}

struct udp_pcb * UDP_Client_Pcb_Get() {
	return udpPcbPrivate;
}

#endif //ENDOF RZG
#endif  //UDP_SERVER_H

/*
 * "Copyright (c) 2006 Robert B. Reese ("AUTHOR")"
 * All rights reserved.
 * (R. Reese, reese@ece.msstate.edu, Mississippi State University)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the author appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE "AUTHOR" BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE "AUTHOR"
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE "AUTHOR" SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE "AUTHOR" HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 * Files in this software distribution may have different usage
 * permissions, see the header in each file. Some files have NO permission
 * headers since parts of the original sources in these files
 * came from vendor  sources with no usage restrictions.
 *
 */

/*
V0.2 added PC-based binding         21/July/2006  RBR
V0.1 Initial Release                10/July/2006  RBR

*/



#include "compiler.h"               //compiler specific
#include "lrwpan_common_types.h"
#include "lrwpan_config.h"
#include "ieee_lrwpan_defs.h"
#include "hal.h"
#include "console.h"
#include "debug.h"
#include "nwk.h"
#include "aps.h"
#include "neighbor.h"


//print functions for debugging

#define LRWPAN_FRAME_TYPE_BEACON 0
#define LRWPAN_FRAME_TYPE_DATA 1
#define LRWPAN_FRAME_TYPE_ACK 2
#define LRWPAN_FRAME_TYPE_MAC_COMMENT 3

BYTE debug_level;

#define PRINTBIT(x)\
{\
    if(x){ conPrintROMString("1");}\
 else {conPrintROMString("0");}}

#ifdef LRWPAN_DEBUG

BYTE *dbgPrintNwkPacket (BYTE *ptr);
BYTE *dbgPrintApsPacket (BYTE *ptr);
void dbgPrintAfPacket (BYTE *ptr);
static void dbgPrintBeacon(BYTE *ptr);
static void dbgPrintMacCmd(BYTE *ptr, BYTE plen);

void dbgPrintMacPacketFCS(BYTE *ptr)
{
    //FCS
    conPrintROMString("\nFCS bytes (2): ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString("  ")
    conPrintUINT8(*ptr);
    conPrintROMString("\n");
}

void dbgPrintPacket(BYTE *ptr, BYTE plen)
{

    ptr = dbgPrintMacPacket(ptr, plen);
    if (!ptr) return;
    ptr = dbgPrintNwkPacket(ptr);
    if (!ptr) return;
    ptr = dbgPrintApsPacket(ptr);
    if (!ptr) return;
    dbgPrintAfPacket(ptr);
}


//returns a ptr to the payload if this is a data packet
BYTE *dbgPrintMacPacket (BYTE *ptr, BYTE plen)
{

    BYTE c, fcflsb;
    BYTE srcmode, dstmode;
    BYTE addr[8];
    SADDR saddr;
    BYTE len;
    BYTE *orgptr;

    orgptr = ptr;
    len = 0;
    //FCS LSB
    c = *ptr;
    ptr++;
    len++;
    fcflsb = c;
    conPrintROMString("#########DUMPED PACKET################\n");
    conPrintROMString("Length: ");
    conPrintUINT8(plen);
    conPrintROMString("\n");
    conPrintROMString("Frame Control LSB: ");
    conPrintUINT8(c);
    conPrintROMString("\n");
    conPrintROMString("  Frame Type: ");
    switch(LRWPAN_GET_FRAME_TYPE(c)) {
        case LRWPAN_FRAME_TYPE_BEACON:
            conPrintROMString("BCN");
            break;
        case LRWPAN_FRAME_TYPE_DATA:
            conPrintROMString("DAT");
            break;
        case LRWPAN_FRAME_TYPE_ACK:
            conPrintROMString("ACK");
            break;
        case LRWPAN_FRAME_TYPE_MAC:
            conPrintROMString("MAC");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }
    conPrintROMString(", Security:");
    PRINTBIT(LRWPAN_GET_SECURITY_ENABLED(c));
    conPrintROMString(", FrmPend:");
    PRINTBIT(LRWPAN_GET_FRAME_PENDING(c));
    conPrintROMString(", AckReq:");
    PRINTBIT(LRWPAN_GET_ACK_REQUEST(c));
    conPrintROMString(", IntraPan:");
    PRINTBIT(LRWPAN_GET_INTRAPAN(c));
    if (len >= plen) return(NULL);

    //FCS MSB
    c = *ptr;
    ptr++;
    len++;
    dstmode = LRWPAN_GET_DST_ADDR(c);
    conPrintROMString("\nFrame Control MSB: ");
    conPrintUINT8(c);
    conPrintROMString("\n");
    conPrintROMString("  DstMode: ");
    switch(dstmode) {
        case LRWPAN_ADDRMODE_NOADDR:
            conPrintROMString("NONE");
            break;
        case LRWPAN_ADDRMODE_SADDR:
            conPrintROMString("SHORT");
            break;
        case LRWPAN_ADDRMODE_LADDR:
            conPrintROMString("LONG");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }
    conPrintROMString("\n  SrcMode: ");
    srcmode = LRWPAN_GET_SRC_ADDR(c);
    switch(srcmode) {
        case LRWPAN_ADDRMODE_NOADDR:
            conPrintROMString("NONE");
            break;
        case LRWPAN_ADDRMODE_SADDR:
            conPrintROMString("SHORT");
            break;
        case LRWPAN_ADDRMODE_LADDR:
            conPrintROMString("LONG");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }
    if (len >= plen) return(NULL);


    //DSN
    c = *ptr;
    ptr++;
    len++;
    conPrintROMString("\nData Sequence Num: ");
    conPrintUINT8(c);
    conPrintROMString("\n");
    if (len >= plen) return(NULL);

    if (LRWPAN_IS_ACK(fcflsb)) {
        dbgPrintMacPacketFCS(ptr);
        return(NULL);
    }


    //DEST PANID
    conPrintROMString("\n");
    if (dstmode != LRWPAN_ADDRMODE_NOADDR) {
        conPrintROMString(" DstPanID (MSB 1st): ");
        addr[0] = *ptr;
        ptr++;
        addr[1] = *ptr;
        ptr++;
        len = len + 2;
        saddr = ((UINT16)addr[1]) << 8;
        saddr = saddr + addr[0];
        conPrintUINT16(saddr);
    }
    //addressing information
    if (dstmode == LRWPAN_ADDRMODE_SADDR || dstmode == LRWPAN_ADDRMODE_LADDR) {
        conPrintROMString("  DstAddress (MSB 1st): ");
        if (dstmode == LRWPAN_ADDRMODE_SADDR) {
            addr[0] = *ptr;
            ptr++;
            addr[1] = *ptr;
            ptr++;
            len = len + 2;
            saddr = ((UINT16)addr[1]) << 8;
            saddr = saddr + addr[0];
            conPrintUINT16(saddr);
        } else {
            conPrintLADDR_bytes(ptr);
            len = len + 8;
            ptr = ptr + 8;
        }
    }

    conPrintROMString("\n");
    if ( !LRWPAN_GET_INTRAPAN(fcflsb) &&
         srcmode != LRWPAN_ADDRMODE_NOADDR
       ) {
        //PANID present if INTRAPAN is zero, and src is nonzero
        conPrintROMString(" SrcPanID(MSB 1st): ");
        addr[0] = *ptr;
        ptr++;
        addr[1] = *ptr;
        ptr++;
        len = len + 2;
        saddr = ((UINT16)addr[1]) << 8;
        saddr = saddr + addr[0];
        conPrintUINT16(saddr);

    }
    if (srcmode == LRWPAN_ADDRMODE_SADDR || srcmode == LRWPAN_ADDRMODE_LADDR) {
        conPrintROMString("  SrcAddress(MSB 1st): ");
        if (srcmode == LRWPAN_ADDRMODE_SADDR) {
            addr[0] = *ptr;
            ptr++;
            addr[1] = *ptr;
            ptr++;
            len = len + 2;
            saddr = ((UINT16)addr[1]) << 8;
            saddr = saddr + addr[0];
            conPrintUINT16(saddr);
        } else {
            conPrintLADDR_bytes(ptr);
            len = len + 8;
            ptr = ptr + 8;
        }
    }
    conPrintROMString("\n")


    //payload
    conPrintROMString("MAC payload size: ");
    conPrintUINT8(plen - (len + PACKET_FOOTER_SIZE) );
    conPCRLF();
    dbgPrintMacPacketFCS(orgptr + plen - PACKET_FOOTER_SIZE);
    if (LRWPAN_IS_DATA(fcflsb)) {
        return(ptr);
    } else if (LRWPAN_IS_BCN(fcflsb)) {
        if ((plen - (len + PACKET_FOOTER_SIZE)) == LRWPAN_NWK_BEACON_SIZE) dbgPrintBeacon(ptr);
        return(NULL);
    } else if (LRWPAN_IS_MAC(fcflsb)) {
        dbgPrintMacCmd(ptr, (plen - (len + PACKET_FOOTER_SIZE)));
        return(NULL);
    }
    return(NULL);
}

static void dbgPrintMacCmd(BYTE *ptr, BYTE plen)
{
    conPrintROMString("MAC CMD: ");
    switch(*ptr) {

        case LRWPAN_MACCMD_ASSOC_REQ :
            conPrintROMString("Assoc Request");
            break;
        case LRWPAN_MACCMD_ASSOC_RSP:
            conPrintROMString("Assoc Response");
            break;
        case LRWPAN_MACCMD_DISASSOC:
            conPrintROMString("DisAssoc Request");
            break;
        case LRWPAN_MACCMD_DATA_REQ :
            conPrintROMString("Data Request");
            break;
        case LRWPAN_MACCMD_PAN_CONFLICT :
            conPrintROMString("Pan Conflict");
            break;
        case LRWPAN_MACCMD_ORPHAN:
            conPrintROMString("Orphan Notification");
            break;
        case LRWPAN_MACCMD_BCN_REQ:
            conPrintROMString("Beacon Request");
            break;
        case LRWPAN_MACCMD_COORD_REALIGN:
            conPrintROMString("Coord Realign");
            break;
        case LRWPAN_MACCMD_GTS_REQ :
            conPrintROMString("GTS Request");
            break;
        default:
            conPrintROMString("Unknown");

    }
    conPCRLF();
    ptr++;
    plen--;
    conPrintROMString("MAC Payload bytes: ");
    while (plen) {
        conPrintUINT8(*ptr);
        ptr++, plen--;
        conPrintROMString(" ");
    }
    conPCRLF();
}

//works only for our beacons
static void dbgPrintBeacon(BYTE *ptr)
{
    BYTE i;

    conPrintROMString("Beacon Superframe, LSB: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", MSB: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPCRLF();
    conPrintROMString("GTS spec: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", Pending addr: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPCRLF();
    conPrintROMString("Beacon payload\n");
    conPrintROMString("Protocol: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", StkProfile: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", ProtoVer: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", Router Room?: ");
    if (*ptr) {
        conPrintROMString("Y");
    } else {
        conPrintROMString("N");
    }
    ptr++;
    conPrintROMString(", Depth: ");
    conPrintUINT8(*ptr);
    ptr++;
    conPrintROMString(", EndDev Room?: ");
    if (*ptr) {
        conPrintROMString("Y");
    } else {
        conPrintROMString("N");
    }
    ptr++;
    conPCRLF();
    conPrintROMString("Beacon Offset: ");
    for (i = 0; i < 3; i++) {
        conPrintUINT8(*ptr);
        conPrintROMString(" ");
        ptr++;
    }
#ifndef LRWPAN_ZIGBEE_BEACON_COMPLY
    conPrintROMString(", Magic Num: ");
    for (i = 0; i < 4; i++) {
        conPrintUINT8(*ptr);
        conPrintROMString(" ");
        ptr++;
    }
#endif

    conPCRLF();
}


//ptr is assumed to pointing to the network header
BYTE *dbgPrintNwkPacket (BYTE *ptr)
{
    BYTE fcflsb;
    BYTE i;
    SADDR saddr;

    fcflsb = *ptr;
    ptr++;
    conPrintROMString("NWK Header:\n");
    conPrintROMString(" Type: ");
    if (NWK_IS_DATA(fcflsb)) {
        conPrintROMString("DATA");
    } else if (NWK_IS_CMD(fcflsb)) {
        conPrintROMString("CMD");
    } else {
        conPrintROMString("RSV");
    }
    conPrintROMString(",  Protocol: ");
    i = NWK_GET_PROTOCOL(fcflsb);
    conPrintUINT8(i);
    conPrintROMString(",  RouteDiscovery: ");
    switch(NWK_GET_ROUTE(fcflsb)) {
        case  NWK_SUPPRESS_ROUTE_DISCOVER:
            conPrintROMString("SUPPRESS");
            break;
        case NWK_ENABLE_ROUTE_DISCOVER:
            conPrintROMString("ENABLE");
            break;
        case NWK_FORCE_ROUTE_DISCOVER:
            conPrintROMString("FORCE");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }//end switch()

    //get MSF FCF
    i = *ptr;
    ptr++;
    conPrintROMString(", Security: ");
    PRINTBIT(NWK_GET_ROUTE(i));
    conPCRLF();

    //DST Addr
    conPrintROMString("DstAddr: ");
    saddr = *ptr;
    ptr++;
    saddr += (((UINT16) * ptr) << 8);
    ptr++;
    conPrintUINT16(saddr);

    //SRC Addr
    conPrintROMString(", SrcAddr: ");
    saddr = *ptr;
    ptr++;
    saddr += (((UINT16) * ptr) << 8);
    ptr++;
    conPrintUINT16(saddr);

    //radius
    conPrintROMString(", Radius: ");
    conPrintUINT8(*ptr);
    ptr++;


    //sequence
    conPrintROMString(", Sequence#: ");
    conPrintUINT8(*ptr);
    ptr++;

    conPCRLF();
    if (NWK_IS_DATA(fcflsb)) {
        return(ptr);
    } else {
        return(NULL);
    }
}

//ptr is assumed to pointing to the APS header
BYTE *dbgPrintApsPacket (BYTE *ptr)
{
    BYTE i;
    BYTE fcf;
    UINT16 profile;

    i = *ptr;
    ptr++;
    fcf = i;
    conPrintROMString("APS Header:\n");
    conPrintROMString(" Type: ");
    switch(APS_GET_FRM_TYPE(i)) {
        case APS_FRM_TYPE_DATA:
            conPrintROMString("DAT");
            break;
        case APS_FRM_TYPE_CMD:
            conPrintROMString("CMD");
            break;
        case APS_FRM_TYPE_ACK:
            conPrintROMString("ACK");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }
    conPrintROMString(", DeliveryMode: ");
    switch(APS_GET_FRM_DLVRMODE(i)) {
        case APS_FRM_DLVRMODE_NORMAL:
            conPrintROMString("NORMAL");
            break;
        case APS_FRM_DLVRMODE_INDIRECT:
            conPrintROMString("INDIRECT");
            break;
        case APS_FRM_DLVRMODE_BCAST:
            conPrintROMString("BCAST");
            break;
        default:
            conPrintROMString("RSV");
            break;
    }
    conPrintROMString(", IndirectSubMode: ");
    PRINTBIT(APS_GET_FRM_INDIRECT_SUBMODE(i));
    conPrintROMString(", Security: ");
    PRINTBIT(APS_GET_FRM_SECURITY(i));
    conPrintROMString(", AckReq: ");
    PRINTBIT(APS_GET_FRM_ACKREQ(i));

    conPCRLF();

    //DST EP
    conPrintROMString(" DstEP: ");
    if ((APS_GET_FRM_DLVRMODE(fcf) == APS_FRM_DLVRMODE_INDIRECT) &&
        (APS_GET_FRM_INDIRECT_SUBMODE(fcf))) {
        //no dest EP
        conPrintROMString("NONE");
    } else {
        conPrintUINT8(*ptr);
        ptr++;
    }

    conPrintROMString(", Cluster: ");
    if (APS_GET_FRM_TYPE(fcf) == APS_FRM_TYPE_DATA) {
        conPrintUINT8(*ptr);
        ptr++;
    } else {
        conPrintROMString("NONE");
    }


    //Profile
    if ((APS_GET_FRM_TYPE(fcf) == APS_FRM_TYPE_DATA) ||
        (APS_GET_FRM_TYPE(fcf) == APS_FRM_TYPE_ACK)
       ) {
        profile = *ptr;
        ptr++;
        profile += (((UINT16) * ptr) << 8);
        ptr++;
        conPrintROMString(", Profile: ");
        conPrintUINT16(profile);
    }


    //SRC EP
    conPrintROMString(", SrcEP: ");

    if ((APS_GET_FRM_DLVRMODE(fcf) == APS_FRM_DLVRMODE_INDIRECT) &&
        (!APS_GET_FRM_INDIRECT_SUBMODE(fcf))) {
        //no src EP
        conPrintROMString("NONE");
    } else {
        conPrintUINT8(*ptr);
        ptr++;
    }
    conPCRLF();

    if (APS_GET_FRM_TYPE(fcf) == APS_FRM_TYPE_DATA) return(ptr);
    else return(NULL);

}

//ptr is assumed to pointing to the AF header
//we only understand how to print MSG types
void dbgPrintAfPacket (BYTE *ptr)
{
    BYTE i;
    BYTE fcf;
    BYTE len;

    conPrintROMString("AF Header: ");
    i = *ptr;
    ptr++;
    fcf = i;

    conPrintROMString(" TransCnt: ");
    conPrintUINT8(AF_GET_TRANS_COUNT(fcf));
    conPrintROMString(" FrmType: ");
    if (AF_GET_FRM_TYPE(fcf) == AF_FRM_TYPE_MSG) {
        conPrintROMString("MSG, Trans#: ");
        //only one transaction in MSG frame, get transaction umber
        conPrintUINT8(*ptr);
        ptr++;
        //only one transaction in MSG frame, first byte is length
        conPrintROMString(", Length: ");
        len = *ptr;
        conPrintUINT8(len);
        ptr++;
        conPCRLF();
        conPrintROMString("Data: ");
        i = 0;
        while(len) {
            conPrintUINT8(*ptr);
            ptr++;
            conPrintROMString(" ");
            i++;
            if (i == 16) {
                i = 0;
                conPCRLF();
            }
            len--;
        }
    } else if (AF_GET_FRM_TYPE(fcf) == AF_FRM_TYPE_KVP) {
        conPrintROMString("KVP");
    } else {
        conPrintROMString("RSV");
    }
    conPCRLF();
}

#if 0
void dbgPrintNeighborTable(void)
{
    NAYBORENTRY *nt_ptr;
    BYTE j, cnt;

    nt_ptr = &mac_nbr_tbl[0];
    cnt = 0;
    for (j = 0; j < NTENTRIES; j++, nt_ptr++) {
        if (nt_ptr->flags.bits.used) cnt++;
    }
    conPrintROMString("Number of Neighbors: ");
    conPrintUINT8(cnt);
    conPCRLF();
    cnt = 1;
    nt_ptr = &mac_nbr_tbl[0];
    for (j = 0; j < NTENTRIES; j++, nt_ptr++) {
        if (nt_ptr->flags.bits.used) {
            conPrintUINT8(cnt);
            cnt++;
            conPrintROMString(" LADDR: ");
            conPrintLADDR_bytes(&mac_addr_tbl[nt_ptr->map_index].laddr[0]);
            conPrintROMString(" SADDR: ");
            conPrintUINT16(mac_addr_tbl[nt_ptr->map_index].saddr);
            conPrintROMString(" CapInfo: ");
            conPrintUINT8(nt_ptr->capinfo);
            conPrintROMString(" LQI: ");
            conPrintUINT8(nt_ptr->flags.bits.lqi);
            conPCRLF();
        }
    }
    //print the address table
    cnt = 0;
    for (j = 0; j < LRWPAN_MAX_ADDRESS_MAP_ENTRIES; j++) {
        if (mac_addr_tbl[j].saddr != LRWPAN_BCAST_SADDR) cnt++;
    }
    conPrintROMString("Number of Address Map Entries: ");
    conPrintUINT8(cnt);
    conPCRLF();
    cnt = 1;
    for (j = 0; j < LRWPAN_MAX_ADDRESS_MAP_ENTRIES; j++) {
        if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR)continue;
        conPrintROMString(" LADDR: ");
        conPrintLADDR_bytes(&mac_addr_tbl[j].laddr[0]);
        conPrintROMString(" SADDR: ");
        conPrintUINT16(mac_addr_tbl[j].saddr);
        conPCRLF();
    }


}
#else

void dbgPrintNeighborTable(void)
{

}
#endif

#endif
